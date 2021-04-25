
#include <rtdevice.h>
#include <board.h>

#include "drv_sdio.h"

#define DRV_BUG 

#define DBG_TAG               "drv_sdio"
#ifdef DRV_BUG
#define DBG_LVL               DBG_LOG
#else
#define DBG_LVL               DBG_INFO
#endif
#include <rtdbg.h>

static USDHC_Type *sdhc1_base = (USDHC_Type *)(0x02190000u);

#define REQ_ST_INIT (1U << 0)
#define REQ_ST_CMD  (1U << 1)
#define REQ_ST_STOP (1U << 2)

struct imx6ull_sdhc
{
  struct rt_mmcsd_host *host;
  struct rt_mmcsd_req *req;
  struct rt_mmcsd_cmd *cmd;
  rt_uint32_t current_status;
};

struct imx6ull_sdhc *sdhc1;

static void imx6ull_sdhc_process_next(struct imx6ull_sdhc *sdhc);

/* ADMA2 descriptor table
 * |----------------|---------------|-------------|--------------------------|
 * | Address field  |     Length    | Reserved    |         Attribute        |
 * |----------------|---------------|-------------|--------------------------|
 * |63            32|31           16|15         06|05  |04  |03|02 |01 |00   |
 * |----------------|---------------|-------------|----|----|--|---|---|-----|
 * | 32-bit address | 16-bit length | 0000000000  |Act2|Act1| 0|Int|End|Valid|
 * |----------------|---------------|-------------|----|----|--|---|---|-----|
 *
 *
 * | Act2 | Act1 |     Comment     | Operation                                                         |
 * |------|------|-----------------|-------------------------------------------------------------------|
 * |   0  |   0  | No op           | Don't care                                                        |
 * |------|------|-----------------|-------------------------------------------------------------------|
 * |   0  |   1  | Reserved        | Read this line and go to next one                                 |
 * |------|------|-----------------|-------------------------------------------------------------------|
 * |   1  |   0  | Transfer data   | Transfer data with address and length set in this descriptor line |
 * |------|------|-----------------|-------------------------------------------------------------------|
 * |   1  |   1  | Link descriptor | Link to another descriptor                                        |
 * |------|------|-----------------|-------------------------------------------------------------------|
 */

struct adma2_descriptor
{
    rt_uint32_t attribute;
    const rt_uint32_t *address;
};

/* should align 4 */
struct adma2_descriptor *adma2_tablbe;
static rt_uint8_t sd_buf[4096] __attribute__ ((aligned(1024)));

/* The bit shift for LENGTH field in ADMA2's descriptor */
#define ADMA2_DESCRIPTOR_LENGTH_SHIFT   (16U)

/* The bit mask for LENGTH field in ADMA2's descriptor */
#define ADMA2_DESCRIPTOR_LENGTH_MASK    (0xFFFFU)

/* The maximum value of LENGTH field in ADMA2's descriptor */
#define ADMA2_DESCRIPTOR_MAX_LENGTH_PER_ENTRY (ADMA2_DESCRIPTOR_LENGTH_MASK - 3U)

#define ADMA2_DESCRIPTOR_TYPE_TRANSFER  ((0x1 << 5) | (0x1 << 0))

#define ADMA2_DESCRIPTOR_END_FLAG       (0x1 << 1)

static void adma2_sdhc_create_desctable(rt_uint8_t *data, rt_uint32_t len)
{
	rt_int32_t i;
	rt_uint32_t dma_len, entries;

	/* calc num of descriptor */		 		  						  					  				 	   		  	  	 	  
	entries = len / ADMA2_DESCRIPTOR_MAX_LENGTH_PER_ENTRY;
	if ((len % ADMA2_DESCRIPTOR_MAX_LENGTH_PER_ENTRY) != 0U)
		entries++;

	for (i = 0; i < entries; i++)
	{
		if (len > ADMA2_DESCRIPTOR_MAX_LENGTH_PER_ENTRY)
		{
			dma_len = ADMA2_DESCRIPTOR_MAX_LENGTH_PER_ENTRY;
			len -= ADMA2_DESCRIPTOR_MAX_LENGTH_PER_ENTRY;
		}
		else
		{
			dma_len = len;
		}

		/* each descriptor for ADMA2 is 64-bit in length */
		adma2_tablbe[i].address = (rt_uint32_t *)data;
		adma2_tablbe[i].attribute = (dma_len << ADMA2_DESCRIPTOR_LENGTH_SHIFT);
		adma2_tablbe[i].attribute |= ADMA2_DESCRIPTOR_TYPE_TRANSFER;
		data += dma_len;
	}
	adma2_tablbe[entries - 1].attribute |= ADMA2_DESCRIPTOR_END_FLAG;
}

void imx6ull_sdhc_get_response(struct imx6ull_sdhc *sdhc)
{
	struct rt_mmcsd_cmd *cmd = sdhc->cmd;

    if (resp_type(cmd) != RESP_NONE)
    {
        if (resp_type(cmd) == RESP_R2)
        {
			cmd->resp[0] = (sdhc1_base->CMD_RSP3 << 8) | (sdhc1_base->CMD_RSP2 >> 24);
			cmd->resp[1] = (sdhc1_base->CMD_RSP2 << 8) | (sdhc1_base->CMD_RSP1 >> 24);
			cmd->resp[2] = (sdhc1_base->CMD_RSP1 << 8) | (sdhc1_base->CMD_RSP0 >> 24);
			cmd->resp[3] = (sdhc1_base->CMD_RSP0 << 8);
        }
		else
		{
			cmd->resp[0U] = sdhc1_base->CMD_RSP0;
		}
		LOG_D("rspesp = [0x%x 0x%x 0x%x 0x%x]", cmd->resp[0], cmd->resp[1], cmd->resp[2], cmd->resp[3]);
    }
}

static void imx6ull_sdhc_check_errors(struct imx6ull_sdhc *sdhc, rt_uint32_t status)
{
	struct rt_mmcsd_cmd *cmd = sdhc->cmd;
	struct rt_mmcsd_data *data = cmd->data;
	int data_len = data->blks * data->blksize;
	rt_uint8_t *buf;

	rt_kprintf("data_len %d\n", data_len);

	if (data)
	{
		if (status & SDHC_DATA_ERRORS)
		{
			LOG_D("data trans error\n");
            data->err = -RT_ERROR;
		}
		else
		{
			LOG_D("adma2 trans done");
			if (data->flags == DATA_DIR_READ)
			{
				LOG_D("read from sd buf");

				rt_memcpy(data->buf, sd_buf, data->blks * data->blksize);

				int i;

				rt_kprintf("\nsd_buf:");
				for(i = 0; i< data_len; i++)
				{
					rt_kprintf("0x%02x ", sd_buf[i]);
				}
				buf = (rt_uint8_t *)data->buf;
				rt_kprintf("\nda_buf:");
				for(i = 0; i< data_len; i++)
				{
					rt_kprintf("0x%02x ", buf[i]);
				}
				rt_kprintf("\n");

			}
			data->err = RT_EOK;
		}
	}
	else
	{
		if (status & SDHC_CMD_ERRORS)
		{
		    if ((status & CCE) && (resp_type(cmd) & (RESP_R3 | RESP_R4)))
			{
				cmd->err = 0;
			}
			else
			{
				LOG_D("cmd trans error\n");
				cmd->err = -RT_ERROR;
			}
		}
		else
		{
			cmd->err = 0;
		}
	}
}

static void imx6ull_sdhc_completed_command(struct imx6ull_sdhc *sdhc, rt_uint32_t status)
{
	imx6ull_sdhc_check_errors(sdhc, status);
	imx6ull_sdhc_get_response(sdhc);
	imx6ull_sdhc_process_next(sdhc);
}

static void imx6ull_sdhc_set_intrrupts(struct imx6ull_sdhc *sdhc)
{
	rt_uint32_t intenable;
	struct rt_mmcsd_cmd *cmd = sdhc->cmd;
	struct rt_mmcsd_data *data = cmd->data;

	intenable = DMAESEN | DEBESEN | DCESEN | DTOESEN | CIESEN |\
				CEBESEN | CCESEN | CTOESEN | DINTSEN | TCSEN | CCSEN;

	if (data)
	{
		rt_kprintf("set data irq\n");
		intenable &= ~CCSEN;
	}

	if (resp_type(cmd) & (RESP_R2 | RESP_R3))
	{
		intenable &=~(CIEIEN | CCESEN);
	}

	sdhc1_base->INT_STATUS_EN = intenable;
	sdhc1_base->INT_SIGNAL_EN = intenable;
}

static void imx6ull_sdhc_send_command(struct imx6ull_sdhc *sdhc, struct rt_mmcsd_cmd *cmd)
{
	rt_uint32_t xfr_typ;
	rt_uint32_t wtmk_lvl;
	rt_uint32_t sys_ctl;
	struct rt_mmcsd_data *data = cmd->data;

	xfr_typ = (CICEN | CCCEN);
	sdhc->cmd = cmd;

	if (data)
	{
		rt_kprintf("data dir 0x%x, blks %d, blksize %d\n", data->flags, data->blks, data->blksize);

		while(sdhc1_base->PRES_STATE & CDIHB);

		adma2_sdhc_create_desctable(sd_buf, data->blks * 512);
		sdhc1_base->DS_ADDR = RT_NULL;
		sdhc1_base->VEND_SPEC &= ~EXT_DMA_EN;
		sdhc1_base->PROT_CTRL &= ~(DMASEL_MASK | BURST_LEN_EN_MASK);
		sdhc1_base->PROT_CTRL |= (BURST_LEN_INCR | DMASEL_ADMA2);

		wtmk_lvl = sdhc1_base->WTMK_LVL;
		wtmk_lvl &= ~(WR_BRST_LEN_MASK | WR_WML_MASK | RD_BRST_LEN_MASK | RD_WML_MASK);
		wtmk_lvl |= ((0x8 << WR_BRST_LEN_SHIFT) | (0x80 << WR_WML_SHIFT) | (0x8 << RD_BRST_LEN_SHIFT) | (0x80 << RD_WML_SHIFT));
		sdhc1_base->WTMK_LVL = wtmk_lvl;

		sys_ctl = sdhc1_base->SYS_CTRL;
		sys_ctl &= ~DTOCV_MASK;
		sys_ctl |= (0xF << DTOCV_SHIFT);
		sdhc1_base->SYS_CTRL = sys_ctl;

		sdhc1_base->BLK_ATT &= ~(BLKCNT_MASK | BLKSIZE_MASK);
		sdhc1_base->BLK_ATT |= ((data->blks << BLKCNT_SHIFT) | (data->blksize << BLKSIZE_SHIFT));
				
		sdhc1_base->ADMA_SYS_ADDR = (rt_uint32_t)adma2_tablbe;
		
		/* use dma read data to data->buf */
		if (data->flags == DATA_DIR_READ)
		{
			sdhc1_base->MIX_CTRL |= (DTDSEL | DMAEN);
		}
		else if (data->flags == DATA_DIR_WRITE)
		{
			LOG_D("write to sd buf\n");
			rt_memcpy(sd_buf, data->buf, data->blks * data->blksize);
			sdhc1_base->MIX_CTRL &= ~DTDSEL;
			sdhc1_base->MIX_CTRL |= DMAEN;
			rt_hw_cpu_dcache_ops(RT_HW_CACHE_INVALIDATE, data->buf, data->blks * 512);
		}
		else
		{
			LOG_E("not support DATA_STREAM now");
		}

		xfr_typ |= DPSEL;
		rt_hw_cpu_dcache_ops(RT_HW_CACHE_INVALIDATE, sd_buf, data->blks * 512);
		rt_hw_cpu_dcache_ops(RT_HW_CACHE_INVALIDATE, data->buf, data->blks * 512);
			
	}

	if (resp_type(cmd) == RESP_NONE)
	{
		xfr_typ |= RSPTYP_0;
	}	
	else
	{
		/* set 136 bit response for R2, 48 bit response otherwise */
		if (resp_type(cmd) == RESP_R2)
		{
			xfr_typ |= RSPTYP_136;
		}
		else
		{
			xfr_typ |= RSPTYP_48;
		}
	}

	xfr_typ |= (cmd->cmd_code << CMDINX_SHIFT);

	rt_kprintf("cmd_arg 0x%x\n", cmd->arg);
	rt_kprintf("xfr_typ 0x%x\n", xfr_typ);

	imx6ull_sdhc_set_intrrupts(sdhc);

	/* send cmd */
	//while(sdhc1_base->PRES_STATE & CIHB);
	sdhc1_base->CMD_ARG = cmd->arg;
	sdhc1_base->CMD_XFR_TYP = xfr_typ;

}

static void imx6ull_sdhc_process_next(struct imx6ull_sdhc *sdhc)
{
	if (sdhc->current_status == REQ_ST_INIT)
	{
		rt_kprintf("CMD\n");
		sdhc->current_status = REQ_ST_CMD;
		imx6ull_sdhc_send_command(sdhc, sdhc->req->cmd);
		rt_kprintf("END\n");
	}
	else if ((sdhc->current_status == REQ_ST_CMD) && sdhc->req->stop)
	{
		rt_kprintf("STOP\n");
		sdhc->current_status = REQ_ST_STOP;
		imx6ull_sdhc_send_command(sdhc, sdhc->req->stop);
		rt_kprintf("STOP END\n");
	}
	else
	{
		rt_kprintf("NO STOP\n");
		mmcsd_req_complete(sdhc->host);
	}
}

static void imx6ull_sdhc_request(struct rt_mmcsd_host *host, struct rt_mmcsd_req *req)
{
	rt_kprintf("-----------------------\n");
	rt_kprintf("cmd %d, data %d stop %d\n", req->cmd->cmd_code, req->data != RT_NULL, req->stop != RT_NULL);

	struct imx6ull_sdhc *sdhc = host->private_data;
	sdhc->req = req;
	sdhc->current_status = REQ_ST_INIT;

	imx6ull_sdhc_process_next(sdhc);
}

static void im6ull_sdhc_reset(void)
{
	rt_uint32_t port_ctrl;

	sdhc1_base->SYS_CTRL |= RSTA;
	while(sdhc1_base->SYS_CTRL & RSTA);

	port_ctrl = sdhc1_base->PROT_CTRL;
	port_ctrl &= ~EMODE_MASK;
	port_ctrl |= EMODE_LITTLE_ENDIAN;
	sdhc1_base->PROT_CTRL = port_ctrl;
}

static void im6ull_sdhc_active(void)
{
	sdhc1_base->SYS_CTRL |= INITA;
	while(sdhc1_base->SYS_CTRL & INITA);
}

void im6ull_sdhc_set_clock(rt_uint32_t srcClock_Hz, rt_uint32_t busClock_Hz)
{
	rt_uint32_t totalDiv = 0U;
	rt_uint32_t divisor = 0U;
	rt_uint32_t prescaler = 0U;
	rt_uint32_t sysctl = 0U;
	rt_uint32_t nearestFrequency = 0U;
	rt_uint32_t maxClKFS = 0x100;
	bool enDDR = false;
	/* DDR mode max clkfs can reach 512 */
	if ((sdhc1_base->MIX_CTRL & 0x8) != 0U)
	{
		enDDR = true;
		maxClKFS *= 2U;
	}
	/* calucate total divisor first */
	totalDiv = srcClock_Hz / busClock_Hz;

	if (totalDiv != 0U)
	{
		/* calucate the divisor (srcClock_Hz / divisor) <= busClock_Hz */
		if ((srcClock_Hz / totalDiv) > busClock_Hz)
		{
			totalDiv++;
		}

		/* divide the total divisor to div and prescaler */
		if (totalDiv > 0x10)
		{
			prescaler = totalDiv / 0x10;
			/* prescaler must be a value which equal 2^n and smaller than SDHC_MAX_CLKFS */
			while (((maxClKFS % prescaler) != 0U) || (prescaler == 1U))
			{
				prescaler++;
			}
			/* calucate the divisor */
			divisor = totalDiv / prescaler;
			/* fine tuning the divisor until divisor * prescaler >= totalDiv */
			while ((divisor * prescaler) < totalDiv)
			{
				divisor++;
			}
			nearestFrequency = srcClock_Hz / divisor / prescaler;
		}
		else
		{
			/* in this situation , divsior and SDCLKFS can generate same clock
			   use SDCLKFS*/
			if ((0x10 % totalDiv) == 0U)
			{
				divisor = 0U;
				prescaler = totalDiv;
			}
			else
			{
				divisor = totalDiv;
				prescaler = 0U;
			}
			nearestFrequency = srcClock_Hz / totalDiv;
		}
	}
	/* in this condition , srcClock_Hz = busClock_Hz, */
	else
	{
		/* in DDR mode , set SDCLKFS to 0, divisor = 0, actually the
		   totoal divider = 2U */
		divisor = 0U;
		prescaler = 0U;
		nearestFrequency = srcClock_Hz;
	}

	/* calucate the value write to register */
	if (divisor != 0U)
	{
		divisor -= 1;
	}
	/* calucate the value write to register */
	if (prescaler != 0U)
	{
		(prescaler) >>= ((enDDR ? 2U : 1U));
	}

	/* Set the SD clock frequency divisor, SD clock frequency select, data timeout counter value. */
	sysctl = sdhc1_base->SYS_CTRL;
	sysctl &= ~SDCLKFS_MASK;
	sysctl &= ~DVS_MASK;

	sysctl |= prescaler << SDCLKFS_SHIFT;
	sysctl |= divisor << DVS_SHIFT;
	sdhc1_base->SYS_CTRL = sysctl;

	/* Wait until the SD clock is stable. */
	while (!(sdhc1_base->PRES_STATE & USDHC_PRES_STATE_SDSTB_MASK))
	{
	}

	LOG_D("set sd bus clk %d", nearestFrequency);
}


static void imx6ull_sdhc_set_iocfg(struct rt_mmcsd_host *host, struct rt_mmcsd_io_cfg *io_cfg)
{
	int prot_ctrl = 0;
	if(io_cfg->clock > host->freq_max)
	{
		io_cfg->clock = host->freq_max;
	}

	if (io_cfg->clock)
	{
		im6ull_sdhc_set_clock(198000000U, io_cfg->clock);
	}
	
	switch (io_cfg->power_mode)
	{
	case MMCSD_POWER_UP:
		im6ull_sdhc_reset();
		break;
	case MMCSD_POWER_ON:
		im6ull_sdhc_active();
		break;
	case MMCSD_POWER_OFF:
		break;
	default:
		break;		
	}

	prot_ctrl = sdhc1_base->PROT_CTRL;
	prot_ctrl &= ~DTW_MASK;

	switch (io_cfg->bus_width)
	{
	case MMCSD_BUS_WIDTH_8:
		prot_ctrl |= DTW_8;
		break;
	case MMCSD_BUS_WIDTH_4:
		prot_ctrl |= DTW_4;
		break;
	case MMCSD_BUS_WIDTH_1:
		prot_ctrl |= DTW_1;
		break;
	default:
		break;		
	}

	sdhc1_base->PROT_CTRL = prot_ctrl;
}

static rt_int32_t imx6ull_sdhc_get_card_status(struct rt_mmcsd_host *host)
{
	rt_kprintf("%s\n", __func__);
	return RT_EOK;
}
static void imx6ull_sdhc_enable_sdio_irq(struct rt_mmcsd_host *host, rt_int32_t en)
{
	rt_kprintf("%s\n", __func__);
}

void rt_hw_usdhc1_irq(int vector, void *param)
{ 
	rt_uint32_t intsta; 

	/* enter interrupt */
	rt_interrupt_enter();

	intsta = sdhc1_base->INT_STATUS;

	rt_kprintf("uSDHC1 irq 0x%x\n", intsta);
	
	sdhc1_base->INT_STATUS    = intsta;
	sdhc1_base->INT_SIGNAL_EN &= ~intsta;
	sdhc1_base->INT_STATUS_EN &= ~intsta;

	imx6ull_sdhc_completed_command(sdhc1, intsta);

	/* leave interrupt */
	rt_interrupt_leave();
}

struct rt_mmcsd_host_ops imx6ll_sdio_ops =
{
	imx6ull_sdhc_request,
	imx6ull_sdhc_set_iocfg,
	imx6ull_sdhc_get_card_status,
	imx6ull_sdhc_enable_sdio_irq,
};

void im6ull_sdio_low_init(void);


void imx6ull_sdio(void)
{
	struct rt_mmcsd_host *host;

	im6ull_sdio_low_init();
	host = mmcsd_alloc_host();
	if (host == RT_NULL)
	{
		LOG_E("can not alloc sdio host");
		return;
	}

	sdhc1 = rt_calloc(1, sizeof(struct imx6ull_sdhc));
	if (!sdhc1)
	{
		mmcsd_free_host(host);
		LOG_D("alloc stm32_sdmmc failed");
		return;
	}

	adma2_tablbe = rt_calloc(8, sizeof(struct adma2_descriptor));
	if (!adma2_tablbe)
	{
		mmcsd_free_host(host);
		rt_free(sdhc1);
		LOG_D("alloc adma tablbe failed");
		return;
	}

	LOG_D("adma2 tablbe addr 0x%x", adma2_tablbe);

	host->ops           = &imx6ll_sdio_ops;
	host->freq_min      = 400000;
	host->freq_max      = 50000000;
	host->valid_ocr     = VDD_32_33 | VDD_33_34;

    /* bus width 1 */
	host->flags         = MMCSD_MUTBLKWRITE;
	host->max_seg_size  = 4096;
	host->max_dma_segs  = 1;
	host->max_blk_size  = 512;
	host->max_blk_count = 8;

	host->private_data  = sdhc1;

	sdhc1->host = host;

	/* config host */
	mmcsd_change(host);
}
MSH_CMD_EXPORT(imx6ull_sdio, ...);

void im6ull_sdio_low_init(void)
{
	/* uSDHC1 pins start*/
	IOMUXC_SetPinMux(IOMUXC_UART1_RTS_B_USDHC1_CD_B, 0U);
    IOMUXC_SetPinConfig(IOMUXC_UART1_RTS_B_USDHC1_CD_B, 
                        IOMUXC_SW_PAD_CTL_PAD_SRE_MASK |
                        IOMUXC_SW_PAD_CTL_PAD_DSE(1U) |
                        IOMUXC_SW_PAD_CTL_PAD_HYS_MASK);
	IOMUXC_SetPinMux(IOMUXC_SD1_CLK_USDHC1_CLK, 0U);
	IOMUXC_SetPinConfig(IOMUXC_SD1_CLK_USDHC1_CLK, 
						IOMUXC_SW_PAD_CTL_PAD_SRE_MASK |
						IOMUXC_SW_PAD_CTL_PAD_DSE(1U) |
						IOMUXC_SW_PAD_CTL_PAD_SPEED(1U) |
						IOMUXC_SW_PAD_CTL_PAD_PKE_MASK |
						IOMUXC_SW_PAD_CTL_PAD_PUE_MASK |
						IOMUXC_SW_PAD_CTL_PAD_PUS(1U) |
						IOMUXC_SW_PAD_CTL_PAD_HYS_MASK);
	IOMUXC_SetPinMux(IOMUXC_SD1_CMD_USDHC1_CMD, 0U);
	IOMUXC_SetPinConfig(IOMUXC_SD1_CMD_USDHC1_CMD, 
						IOMUXC_SW_PAD_CTL_PAD_SRE_MASK |
						IOMUXC_SW_PAD_CTL_PAD_DSE(1U) |
						IOMUXC_SW_PAD_CTL_PAD_SPEED(2U) |
						IOMUXC_SW_PAD_CTL_PAD_PKE_MASK |
						IOMUXC_SW_PAD_CTL_PAD_PUE_MASK |
						IOMUXC_SW_PAD_CTL_PAD_PUS(1U) |
						IOMUXC_SW_PAD_CTL_PAD_HYS_MASK);
	IOMUXC_SetPinMux(IOMUXC_SD1_DATA0_USDHC1_DATA0, 0U);
	IOMUXC_SetPinConfig(IOMUXC_SD1_DATA0_USDHC1_DATA0, 
						IOMUXC_SW_PAD_CTL_PAD_SRE_MASK |
						IOMUXC_SW_PAD_CTL_PAD_DSE(1U) |
						IOMUXC_SW_PAD_CTL_PAD_SPEED(2U) |
						IOMUXC_SW_PAD_CTL_PAD_PKE_MASK |
						IOMUXC_SW_PAD_CTL_PAD_PUE_MASK |
						IOMUXC_SW_PAD_CTL_PAD_PUS(1U) |
						IOMUXC_SW_PAD_CTL_PAD_HYS_MASK);
	IOMUXC_SetPinMux(IOMUXC_SD1_DATA1_USDHC1_DATA1, 0U);
	IOMUXC_SetPinConfig(IOMUXC_SD1_DATA1_USDHC1_DATA1, 
						IOMUXC_SW_PAD_CTL_PAD_SRE_MASK |
						IOMUXC_SW_PAD_CTL_PAD_DSE(1U) |
						IOMUXC_SW_PAD_CTL_PAD_SPEED(2U) |
						IOMUXC_SW_PAD_CTL_PAD_PKE_MASK |
						IOMUXC_SW_PAD_CTL_PAD_PUE_MASK |
						IOMUXC_SW_PAD_CTL_PAD_PUS(1U) |
						IOMUXC_SW_PAD_CTL_PAD_HYS_MASK);
	IOMUXC_SetPinMux(IOMUXC_SD1_DATA2_USDHC1_DATA2, 0U);
	IOMUXC_SetPinConfig(IOMUXC_SD1_DATA2_USDHC1_DATA2, 
						IOMUXC_SW_PAD_CTL_PAD_SRE_MASK |
						IOMUXC_SW_PAD_CTL_PAD_DSE(1U) |
						IOMUXC_SW_PAD_CTL_PAD_SPEED(2U) |
						IOMUXC_SW_PAD_CTL_PAD_PKE_MASK |
						IOMUXC_SW_PAD_CTL_PAD_PUE_MASK |
						IOMUXC_SW_PAD_CTL_PAD_PUS(1U) |
						IOMUXC_SW_PAD_CTL_PAD_HYS_MASK);
	IOMUXC_SetPinMux(IOMUXC_SD1_DATA3_USDHC1_DATA3, 0U);
	IOMUXC_SetPinConfig(IOMUXC_SD1_DATA3_USDHC1_DATA3, 
						IOMUXC_SW_PAD_CTL_PAD_SRE_MASK |
						IOMUXC_SW_PAD_CTL_PAD_DSE(1U) |
						IOMUXC_SW_PAD_CTL_PAD_SPEED(2U) |
						IOMUXC_SW_PAD_CTL_PAD_PKE_MASK |
						IOMUXC_SW_PAD_CTL_PAD_PUE_MASK |
						IOMUXC_SW_PAD_CTL_PAD_PUS(1U) |
						IOMUXC_SW_PAD_CTL_PAD_HYS_MASK);
	/* uSDHC1 pins end*/

	rt_hw_interrupt_install(USDHC1_IRQn, rt_hw_usdhc1_irq, RT_NULL, "uSDHC1");
	rt_hw_interrupt_umask(USDHC1_IRQn);
}
