
#include <rthw.h>
#include <rtdevice.h>
#include <board.h>

#include "drv_i2c.h"

//#define DRV_BUG 

#define DBG_TAG               "drv_i2c"
#ifdef DRV_BUG
#define DBG_LVL               DBG_LOG
#else
#define DBG_LVL               DBG_INFO
#endif
#include <rtdbg.h>

static I2C_Type *base = (I2C_Type *)(0x021A4000);

static unsigned int i2c_check_ack_and_error(struct rt_i2c_bus_device *bus)
{
	unsigned int status;

	status = base->I2SR;

	base->I2SR = ~IIF;

	if(status & (1<<4))
	{
        base->I2SR &= ~(1<<4);
        base->I2CR &= ~(1 << 7);
        base->I2CR |= (1 << 7);
		LOG_E("Arbitration lost");
        return -RT_EIO;
	}
	else if(status & (1 << 0))
	{
		/* NACK */
	    return 1;
	}

	/* ACK */
	return RT_EOK;
}

static void imx6ull_i2c_start(struct rt_i2c_bus_device *bus)
{
	if(base->I2SR & IBB)
	{
		LOG_E("i2c busy");
		return;
	}
	base->I2CR |= MSTA | MTX; 
}

static void imx6ull_i2c_restart(struct rt_i2c_bus_device *bus)
{
	if(base->I2SR & IBB && (((base->I2CR) & MSTA) == 0))
	{
		LOG_E("i2c bus busy detect");
	}

	base->I2CR |= MTX | RSTA;
}

static void imx6ull_i2c_stop(struct rt_i2c_bus_device *bus)
{
	rt_uint32_t timeout = 0xFFFFF;

	base->I2CR &= ~(MSTA | MTX | TXAK);

	while(base->I2SR & IBB)
	{
		timeout--;
		if(timeout == 0)
		{
			LOG_D("send stop timeout");
			break;
		}	
	}
}

static rt_err_t imx6ull_i2c_send_address(struct rt_i2c_bus_device *bus,
											struct rt_i2c_msg        *msg)
{
	base->I2DR = (msg->addr << 1) | (msg->flags & RT_I2C_RD);
	while(!(base->I2SR & IIF));
	return i2c_check_ack_and_error(bus);
}

static void imx6ll_i2c_set_trans(rt_bool_t en)
{
	base->I2SR &= ~IIF;
	if (en)
	{
		base->I2CR |= MTX; 
	}

	else
	{
		base->I2CR &= ~MTX; 
	}
}

static void imx6ull_i2c_send_byte(rt_uint8_t val)
{
	base->I2DR = val;
	while(!(base->I2SR & IIF));
}

static rt_size_t imx6ull_i2c_send_bytes(struct rt_i2c_bus_device *bus,
                                               struct rt_i2c_msg        *msg)
{
	rt_int32_t ret = 0;
	rt_size_t bytes = 0;
	const rt_uint8_t *ptr = msg->buf;
	rt_int32_t count = msg->len;
	rt_uint16_t ignore_nack = msg->flags & RT_I2C_IGNORE_NACK;

	imx6ll_i2c_set_trans(RT_TRUE);
	
	while (count > 0)
	{
		/* send one data and check ack */
		imx6ull_i2c_send_byte(*ptr);
		ret = i2c_check_ack_and_error(bus);
		if ((ret == 0) || (ignore_nack && (ret > 0)))
		{
			count --;
			ptr ++;
			bytes ++;
		}
		else if (ret > 0)
		{
			LOG_D("send byte[%d]: NACK.", bytes);
	
			return 0;
		}
		else
		{
			LOG_E("send bytes: error %d", ret);
	
			return ret;
		}
	}
	
	return bytes;
}

static void imx6ull_i2c_send_ack_or_nack(struct rt_i2c_bus_device *bus, int count)
{
   if (count > 1)
   {
	   /* send ack */
	   base->I2CR &= ~TXAK;
   }
   else if (count == 1)
   {
	   /* send nack */
	   base->I2CR |= TXAK;
   }
   else
   {
	   LOG_E("invalid parameter");
   }
}

static rt_uint8_t imx6ull_i2c_recv_byte(void)
{
	rt_uint8_t val;
	val = base->I2DR;
	while(!(base->I2SR & IIF));
	base->I2SR &= ~IIF;

	return val;
}

static rt_size_t imx6ull_i2c_recv_bytes(struct rt_i2c_bus_device *bus,
                                               struct rt_i2c_msg        *msg)
{
	rt_int32_t bytes = 0;   /* actual bytes */
	rt_uint8_t *ptr = msg->buf;
	rt_int32_t count = msg->len;
	const rt_uint32_t flags = msg->flags;

	volatile rt_uint8_t dummy = 0;
	dummy++;

	imx6ll_i2c_set_trans(RT_FALSE);

	/* dummy read */
	dummy = imx6ull_i2c_recv_byte();
	
	while (count > 0)
	{
		/* config ACK */
		if (!(flags & RT_I2C_NO_READ_ACK))
		{
			imx6ull_i2c_send_ack_or_nack(bus, count);
		}

		/* read one data */
		*ptr = imx6ull_i2c_recv_byte();
		bytes ++;
		ptr++;
		count--;
		LOG_D("recieve bytes: 0x%02x, %s",
			   *ptr, (flags & RT_I2C_NO_READ_ACK) ?
			   "(No ACK/NACK)" : (count ? "ACK" : "NACK"));
	}

	return bytes;
}

static rt_size_t imx6ull_master_xfer(struct rt_i2c_bus_device *bus,
                         struct rt_i2c_msg msgs[],
                         rt_uint32_t num)
{
	struct rt_i2c_msg *msg;
	rt_int32_t i, ret;
	rt_uint16_t ignore_nack;

	if (num == 0) return 0;

    for (i = 0; i < num; i++)
    {
        msg = &msgs[i];
        ignore_nack = msg->flags & RT_I2C_IGNORE_NACK;

		/* if has start signal */
        if (!(msg->flags & RT_I2C_NO_START))
        {
            if (i)
            {
                imx6ull_i2c_restart(bus);
            }
			else
           	{
           	    LOG_D("send start condition");
                imx6ull_i2c_start(bus);
			}
            ret = imx6ull_i2c_send_address(bus, msg);
            if ((ret != RT_EOK) && !ignore_nack)
            {
                LOG_D("receive NACK from device addr 0x%02x msg %d",
                        msgs[i].addr, i);
                goto out;
            }
        }
        if (msg->flags & RT_I2C_RD)
        {
            ret = imx6ull_i2c_recv_bytes(bus, msg);
            if (ret >= 1)
                LOG_D("read %d byte%s", ret, ret == 1 ? "" : "s");
            if (ret < msg->len)
            {
                if (ret >= 0)
                    ret = -RT_EIO;
                goto out;
            }
        }
        else
        {
            ret = imx6ull_i2c_send_bytes(bus, msg);
            if (ret >= 1)
                LOG_D("write %d byte%s", ret, ret == 1 ? "" : "s");
            if (ret < msg->len)
            {
                if (ret >= 0)
                    ret = -RT_ERROR;
                goto out;
            }
        }
    }
    ret = i;

out:
    if (!(msg->flags & RT_I2C_NO_STOP))
    {
        imx6ull_i2c_stop(bus);
        LOG_D("send stop condition");
    }

	return ret;
}

static struct rt_i2c_bus_device_ops imx6ull_i2c_ops = 
{
	imx6ull_master_xfer,
	RT_NULL,
	RT_NULL,
};

void imx6ull_i2c_init(void)
{
	/* gpio init */
    IOMUXC_SetPinMux(IOMUXC_UART5_RX_DATA_I2C2_SDA, 1U);
    IOMUXC_SetPinConfig(IOMUXC_UART5_RX_DATA_I2C2_SDA, 
                        IOMUXC_SW_PAD_CTL_PAD_DSE(2U) |
                        IOMUXC_SW_PAD_CTL_PAD_SPEED(2U) |
                        IOMUXC_SW_PAD_CTL_PAD_PKE_MASK |
                        IOMUXC_SW_PAD_CTL_PAD_PUE_MASK |
                        IOMUXC_SW_PAD_CTL_PAD_PUS(1U));
    IOMUXC_SetPinMux(IOMUXC_UART5_TX_DATA_I2C2_SCL, 1U);
    IOMUXC_SetPinConfig(IOMUXC_UART5_TX_DATA_I2C2_SCL, 
                        IOMUXC_SW_PAD_CTL_PAD_DSE(2U) |
                        IOMUXC_SW_PAD_CTL_PAD_SPEED(2U) |
                        IOMUXC_SW_PAD_CTL_PAD_PKE_MASK |
                        IOMUXC_SW_PAD_CTL_PAD_PUE_MASK |
                        IOMUXC_SW_PAD_CTL_PAD_PUS(1U));

	/* close i2c */
	base->I2CR &= ~IEN;

	/* config i2c bus clk 100kHz */
	base->IFDR = (0x37 << 0);

	/* enable i2c */
	base->I2CR |= IEN;
}

struct rt_i2c_bus_device bus;

int imx6ull_i2c_register(void)
{
	rt_err_t ret;

	imx6ull_i2c_init();

	bus.ops = &imx6ull_i2c_ops;
	ret = rt_i2c_bus_device_register(&bus, "i2c2");
	if (ret != RT_EOK)
	{
		LOG_E("imx6ull i2c bus register failed");

		return -RT_EIO;
	}

	return RT_EOK;
}
INIT_BOARD_EXPORT(imx6ull_i2c_register);
