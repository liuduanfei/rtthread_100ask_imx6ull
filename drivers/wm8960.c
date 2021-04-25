
#include <rthw.h>
#include <rtdevice.h>

#define DRV_BUG 

#define DBG_TAG               "wm8960"
#ifdef DRV_BUG
#define DBG_LVL               DBG_LOG
#else
#define DBG_LVL               DBG_INFO
#endif
#include <rtdbg.h>


/* i2c test wm8960 */

struct rt_i2c_bus_device *i2c2;

/* R15 (0Fh) reset register */

void i2c_test(void)
{
	rt_size_t ret;
	rt_uint8_t buf[2] = {0};
	struct rt_i2c_msg msg;

	buf[0] = 0xF<<1;
	buf[1] = 0xFF;

	msg.addr = 0x1a;
	msg.buf = buf;
	msg.flags = RT_I2C_WR;
	msg.len = 2;

	i2c2 = rt_i2c_bus_device_find("i2c2");
	if (i2c2 == RT_NULL)
	{
		LOG_E("find i2c bus failed");

		return;
	}

	ret = rt_i2c_transfer(i2c2, &msg, 1);
	if (ret != 1)
	{
		LOG_E("imx6ull i2c transfer failed");

		return;
	}
}
MSH_CMD_EXPORT(i2c_test, ....);

void ft54x6_get_point(rt_uint16_t *x, rt_uint16_t *y)
{
    struct rt_i2c_msg msg = {0};

    rt_uint8_t buf[4] = {0};
    rt_uint8_t reg = 0x3;

    msg.addr = (0x70>>1);
    msg.flags = RT_I2C_WR;
    msg.buf = &reg;
    msg.len = 1;
    rt_i2c_transfer(i2c2, &msg, 1);

    msg.addr = (0x70>>1);
    msg.flags = RT_I2C_RD;
    msg.buf = buf;
    msg.len = 4;
    rt_i2c_transfer(i2c2, &msg, 1);

    *x = ((buf[0]&0X0F)<<8)+buf[1];
    *y = ((buf[2]&0X0F)<<8)+buf[3];
}

static void ft54x6_write_reg(rt_uint8_t reg, rt_uint8_t *value)
{
    struct rt_i2c_msg msg = {0};
    rt_uint8_t buf[2];
    buf[0] = reg;
    buf[1] = *value;

    msg.addr = (0x70>>1);
    msg.flags = RT_I2C_WR;
    msg.buf = buf;
    msg.len = 2;
    rt_i2c_transfer(i2c2, &msg, 1);
}

static void timeout(void * parameter)
{
	rt_uint16_t x, y;
	ft54x6_get_point(&x, &y);
	LOG_D("point x:%d, y:%d", x, y);
}

void ft54x6_init(void)
{
	rt_uint8_t temp;
	rt_timer_t t;

	i2c2 = rt_i2c_bus_device_find("i2c2");
	if (i2c2 == RT_NULL)
	{
		LOG_E("find i2c bus failed");

		return;
	}

    temp = 0;
    ft54x6_write_reg(0x0, &temp);
    temp = 0;
    ft54x6_write_reg(0xA4, &temp);
    temp = 22;
    ft54x6_write_reg(0x80, &temp);
    temp = 12;
    ft54x6_write_reg(0x88, &temp);

	t = rt_timer_create("touch", timeout, RT_NULL, 30, RT_TIMER_FLAG_SOFT_TIMER | RT_TIMER_CTRL_SET_PERIODIC);
	if (t == RT_NULL)
	{
		LOG_D("can not create timer");
	}
	rt_timer_start(t);
}
MSH_CMD_EXPORT(ft54x6_init, ...);


