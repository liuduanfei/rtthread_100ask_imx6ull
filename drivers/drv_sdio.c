/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author        Notes
 * 2021-05-09     liuduanfei    first version
 */

#include <rtdevice.h>
#include <board.h>

#include "drv_sdio.h"

//#define DRV_BUG 

#define DBG_TAG               "drv_sdio"
#ifdef DRV_BUG
#define DBG_LVL               DBG_LOG
#else
#define DBG_LVL               DBG_INFO
#endif
#include <rtdbg.h>

/** USDHC - Register Layout Typedef */
struct uSDHC{
  volatile rt_uint32_t DS_ADDR;                           /**< DMA System Address, offset: 0x0 */
  volatile rt_uint32_t BLK_ATT;                           /**< Block Attributes, offset: 0x4 */
  volatile rt_uint32_t CMD_ARG;                           /**< Command Argument, offset: 0x8 */
  volatile rt_uint32_t CMD_XFR_TYP;                       /**< Command Transfer Type, offset: 0xC */
  volatile rt_uint32_t CMD_RSP0;                          /**< Command Response0, offset: 0x10 */
  volatile rt_uint32_t CMD_RSP1;                          /**< Command Response1, offset: 0x14 */
  volatile rt_uint32_t CMD_RSP2;                          /**< Command Response2, offset: 0x18 */
  volatile rt_uint32_t CMD_RSP3;                          /**< Command Response3, offset: 0x1C */
  volatile rt_uint32_t DATA_BUFF_ACC_PORT;                /**< Data Buffer Access Port, offset: 0x20 */
  volatile rt_uint32_t PRES_STATE;                        /**< Present State, offset: 0x24 */
  volatile rt_uint32_t PROT_CTRL;                         /**< Protocol Control, offset: 0x28 */
  volatile rt_uint32_t SYS_CTRL;                          /**< System Control, offset: 0x2C */
  volatile rt_uint32_t INT_STATUS;                        /**< Interrupt Status, offset: 0x30 */
  volatile rt_uint32_t INT_STATUS_EN;                     /**< Interrupt Status Enable, offset: 0x34 */
  volatile rt_uint32_t INT_SIGNAL_EN;                     /**< Interrupt Signal Enable, offset: 0x38 */
  volatile rt_uint32_t AUTOCMD12_ERR_STATUS;              /**< Auto CMD12 Error Status, offset: 0x3C */
  volatile rt_uint32_t HOST_CTRL_CAP;                     /**< Host Controller Capabilities, offset: 0x40 */
  volatile rt_uint32_t WTMK_LVL;                          /**< Watermark Level, offset: 0x44 */
  volatile rt_uint32_t MIX_CTRL;                          /**< Mixer Control, offset: 0x48 */
  rt_uint8_t           RESERVED_0[4];
  volatile rt_uint32_t FORCE_EVENT;                       /**< Force Event, offset: 0x50 */
  volatile rt_uint32_t ADMA_ERR_STATUS;                   /**< ADMA Error Status Register, offset: 0x54 */
  volatile rt_uint32_t ADMA_SYS_ADDR;                     /**< ADMA System Address, offset: 0x58 */
  rt_uint8_t           RESERVED_1[4];
  volatile rt_uint32_t DLL_CTRL;                          /**< DLL (Delay Line) Control, offset: 0x60 */
  volatile rt_uint32_t DLL_STATUS;                        /**< DLL Status, offset: 0x64 */
  volatile rt_uint32_t CLK_TUNE_CTRL_STATUS;              /**< CLK Tuning Control and Status, offset: 0x68 */
  rt_uint8_t           RESERVED_2[84];
  volatile rt_uint32_t VEND_SPEC;                         /**< Vendor Specific Register, offset: 0xC0 */
  volatile rt_uint32_t MMC_BOOT;                          /**< MMC Boot Register, offset: 0xC4 */
  volatile rt_uint32_t VEND_SPEC2;                        /**< Vendor Specific 2 Register, offset: 0xC8 */
  volatile rt_uint32_t TUNING_CTRL;                       /**< Tuning Control Register, offset: 0xCC */
};

static struct uSDHC *sdhc1_base = (struct uSDHC *)(0x02190000u);

void imx6ull_dump_regs(struct uSDHC *base)
{
	rt_kprintf("-name----------------addr---------value---------\n");
	rt_kprintf("DS_ADDR--------------0x%08x---0x%08X----\n", &base->DS_ADDR, base->DS_ADDR);
	rt_kprintf("BLK_ATT--------------0x%08x---0x%08X----\n", &base->BLK_ATT, base->BLK_ATT);
	rt_kprintf("CMD_ARG--------------0x%08x---0x%08X----\n", &base->CMD_ARG, base->CMD_ARG);
	rt_kprintf("CMD_XFR_TYP----------0x%08x---0x%08X----\n", &base->CMD_XFR_TYP, base->CMD_XFR_TYP);
	rt_kprintf("CMD_RSP0-------------0x%08x---0x%08X----\n", &base->CMD_RSP0, base->CMD_RSP0);
	rt_kprintf("CMD_RSP1-------------0x%08x---0x%08X----\n", &base->CMD_RSP1, base->CMD_RSP1);
	rt_kprintf("CMD_RSP2-------------0x%08x---0x%08X----\n", &base->CMD_RSP2, base->CMD_RSP2);
	rt_kprintf("CMD_RSP3-------------0x%08x---0x%08X----\n", &base->CMD_RSP3, base->CMD_RSP3);
	rt_kprintf("DATA_BUFF_ACC_PORT---0x%08x---0x%08X----\n", &base->DATA_BUFF_ACC_PORT, base->DATA_BUFF_ACC_PORT);
	rt_kprintf("PRES_STATE-----------0x%08x---0x%08X----\n", &base->PRES_STATE, base->PRES_STATE);
	rt_kprintf("PROT_CTRL------------0x%08x---0x%08X----\n", &base->PROT_CTRL, base->PROT_CTRL);
	rt_kprintf("SYS_CTRL-------------0x%08x---0x%08X----\n", &base->SYS_CTRL, base->SYS_CTRL);
	rt_kprintf("INT_STATUS-----------0x%08x---0x%08X----\n", &base->INT_STATUS, base->INT_STATUS);
	rt_kprintf("INT_STATUS_EN--------0x%08x---0x%08X----\n", &base->INT_STATUS_EN, base->INT_STATUS_EN);
	rt_kprintf("INT_SIGNAL_EN--------0x%08x---0x%08X----\n", &base->INT_SIGNAL_EN, base->INT_SIGNAL_EN);
	rt_kprintf("AUTOCMD12_ERR_STATUS-0x%08x---0x%08X----\n", &base->AUTOCMD12_ERR_STATUS, base->AUTOCMD12_ERR_STATUS);
	rt_kprintf("HOST_CTRL_CAP--------0x%08x---0x%08X----\n", &base->HOST_CTRL_CAP, base->HOST_CTRL_CAP);
	rt_kprintf("WTMK_LVL-------------0x%08x---0x%08X----\n", &base->WTMK_LVL, base->WTMK_LVL);
	rt_kprintf("MIX_CTRL-------------0x%08x---0x%08X----\n", &base->MIX_CTRL, base->MIX_CTRL);
	rt_kprintf("FORCE_EVENT----------0x%08x---0x%08X----\n", &base->FORCE_EVENT, base->FORCE_EVENT);
	rt_kprintf("ADMA_ERR_STATUS------0x%08x---0x%08X----\n", &base->ADMA_ERR_STATUS, base->ADMA_ERR_STATUS);
	rt_kprintf("ADMA_SYS_ADDR--------0x%08x---0x%08X----\n", &base->ADMA_SYS_ADDR, base->ADMA_SYS_ADDR);
	rt_kprintf("DLL_CTRL-------------0x%08x---0x%08X----\n", &base->DLL_CTRL, base->DLL_CTRL);
	rt_kprintf("DLL_STATUS-----------0x%08x---0x%08X----\n", &base->DLL_STATUS, base->DLL_STATUS);
	rt_kprintf("CLK_TUNE_CTRL_STATUS-0x%08x---0x%08X----\n", &base->CLK_TUNE_CTRL_STATUS, base->CLK_TUNE_CTRL_STATUS);
	rt_kprintf("VEND_SPEC------------0x%08x---0x%08X----\n", &base->VEND_SPEC, base->VEND_SPEC);
	rt_kprintf("MMC_BOOT-------------0x%08x---0x%08X----\n", &base->MMC_BOOT, base->MMC_BOOT);
	rt_kprintf("VEND_SPEC2-----------0x%08x---0x%08X----\n", &base->VEND_SPEC2, base->VEND_SPEC2);
	rt_kprintf("TUNING_CTRL----------0x%08x---0x%08X----\n", &base->TUNING_CTRL, base->TUNING_CTRL);
	rt_kprintf("------------------------------------------------\n");
}

void dump_sdhc1(void)
{
	imx6ull_dump_regs(sdhc1_base);
}
MSH_CMD_EXPORT(dump_sdhc1, ...);

#define REQ_ST_INIT (1U << 0)
#define REQ_ST_CMD  (1U << 1)
#define REQ_ST_STOP (1U << 2)

struct imx6ull_sdhc
{
	struct rt_mmcsd_host *host;
	struct rt_mmcsd_req *req;
	struct rt_mmcsd_cmd *cmd;
	rt_uint32_t status;
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
ALIGN(4);
struct adma2_descriptor adma2_tablbe[8];

ALIGN(1024);
static rt_uint8_t sd_buf[4096];

/* The bit shift for LENGTH field in ADMA2's descriptor */
#define ADMA2_DESCRIPTOR_LENGTH_SHIFT   (16U)

/* The bit mask for LENGTH field in ADMA2's descriptor */
#define ADMA2_DESCRIPTOR_LENGTH_MASK    (0xFFFFU)

/* The maximum value of LENGTH field in ADMA2's descriptor */
#define ADMA2_DESCRIPTOR_MAX_LENGTH_PER_ENTRY (ADMA2_DESCRIPTOR_LENGTH_MASK - 3U)

#define ADMA2_DESCRIPTOR_TYPE_TRANSFER  ((0x1 << 5) | (0x1 << 0))
#define ADMA2_DESCRIPTOR_END_FLAG       (0x1 << 1)

static void adma2_create_desctable(rt_uint8_t *dma_base, rt_uint32_t len)
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
		adma2_tablbe[i].address = (rt_uint32_t *)dma_base;
		adma2_tablbe[i].attribute = (dma_len << ADMA2_DESCRIPTOR_LENGTH_SHIFT);
		adma2_tablbe[i].attribute |= ADMA2_DESCRIPTOR_TYPE_TRANSFER;
		dma_base += dma_len;
	}
	adma2_tablbe[entries - 1].attribute |= ADMA2_DESCRIPTOR_END_FLAG;
}

static void imx6ull_sdhc_get_response(struct imx6ull_sdhc *sdhc)
{
	struct rt_mmcsd_cmd *cmd = sdhc->cmd;

    if (resp_type(cmd) != RESP_NONE)
    {
	    cmd->resp[0] = sdhc1_base->CMD_RSP0;
        if (resp_type(cmd) == RESP_R2)
        {
			cmd->resp[0] = (sdhc1_base->CMD_RSP3 << 8) | (sdhc1_base->CMD_RSP2 >> 24);
			cmd->resp[1] = (sdhc1_base->CMD_RSP2 << 8) | (sdhc1_base->CMD_RSP1 >> 24);
			cmd->resp[2] = (sdhc1_base->CMD_RSP1 << 8) | (sdhc1_base->CMD_RSP0 >> 24);
			cmd->resp[3] = (sdhc1_base->CMD_RSP0 << 8);
        }
		LOG_D("rspesp = [0x%x 0x%x 0x%x 0x%x]", cmd->resp[0], cmd->resp[1], cmd->resp[2], cmd->resp[3]);
    }
}

static void imx6ull_sdhc_check_errors(struct imx6ull_sdhc *sdhc, rt_uint32_t status)
{
	struct rt_mmcsd_cmd *cmd = sdhc->cmd;
	struct rt_mmcsd_data *data = cmd->data;

	if (data)
	{
		if (status & DATA_ERRORS)
		{
			LOG_E("data trans error\n");
            data->err = -RT_ERROR;
		}
		else
		{
			if (data->flags == DATA_DIR_READ)
			{
				LOG_E("read from sd buf");
				rt_memcpy(data->buf, sd_buf, data->blks * data->blksize);
				rt_hw_cpu_dcache_ops(RT_HW_CACHE_INVALIDATE, sd_buf, data->blks * data->blksize);
			}
			data->err = RT_EOK;
		}
	}

	if (status & CMD_ERRORS)
	{
	    if ((status & CCE) && (resp_type(cmd) & (RESP_R3 | RESP_R4)))
		{
			cmd->err = RT_EOK;
		}
		else
		{
			LOG_D("cmd %d cmd error\n", cmd->cmd_code);
			cmd->err = -RT_ERROR;
		}
	}
	else
	{
		cmd->err = RT_EOK;
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

	intenable = DMAESEN | DEBESEN | DCESEN | DTOESEN | DINTSEN |CIESEN |\
				CEBESEN | CCESEN | CTOESEN | TCSEN | CCSEN;

	if (data)
	{
		LOG_D("set data irq");
		intenable &= ~CCSEN;
	}

	if (resp_type(cmd) & (RESP_R2 | RESP_R3))
	{
		intenable &=~(CIEIEN | CCESEN);
	}

	sdhc1_base->INT_STATUS_EN = intenable; // 0x157F51FFUL
	sdhc1_base->INT_SIGNAL_EN = intenable;
}

static void imx6ull_sdhc_send_command(struct imx6ull_sdhc *sdhc, struct rt_mmcsd_cmd *cmd)
{
	rt_int32_t data_len;
	rt_uint32_t xfr_typ;
	rt_uint32_t sys_ctl;
	struct rt_mmcsd_data *data = cmd->data;

	sdhc1_base->INT_STATUS_EN = 0; //0x157F51FFUL
	sdhc1_base->INT_SIGNAL_EN = 0;

	xfr_typ = (CICEN | CCCEN);
	sdhc->cmd = cmd;

#if 1
	//when enable will stack
	while(sdhc1_base->PRES_STATE & CIHB)
	{
		sdhc1_base->SYS_CTRL |= RSTC;
	}
#endif

	if (data)
	{
		while(sdhc1_base->PRES_STATE & CDIHB)
		{
			sdhc1_base->SYS_CTRL |= RSTD;
		}
	
		data_len = data->blks * data->blksize;
		LOG_E("data dir 0x%x, blks %d, blksize %d", data->flags, data->blks, data->blksize);

		adma2_create_desctable(sd_buf, data_len);

		sdhc1_base->DS_ADDR = RT_NULL;
		sdhc1_base->VEND_SPEC2 |= (1 << 23);

		sdhc1_base->BLK_ATT = ((data->blks << BLKCNT_SHIFT) | (data->blksize << BLKSIZE_SHIFT));
				
		sdhc1_base->ADMA_SYS_ADDR = (rt_uint32_t)adma2_tablbe;

		if (data->flags == DATA_DIR_READ)
		{
			sdhc1_base->MIX_CTRL |= (DTDSEL | DMAEN);
		}
		else if (data->flags == DATA_DIR_WRITE)
		{
			LOG_E("write to sd buf");
			rt_hw_cpu_dcache_ops(RT_HW_CACHE_FLUSH, sd_buf, data_len);

     		rt_memcpy(sd_buf, (rt_uint8_t *)data->buf, data_len);
			
			sdhc1_base->MIX_CTRL |= DMAEN;
		}
		else
		{
			LOG_E("not support DATA_STREAM now");
		}

		xfr_typ |= DPSEL;		
	}

	if (resp_type(cmd) == RESP_NONE)
	{
		xfr_typ |= RSPTYP_NONE;
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

	imx6ull_sdhc_set_intrrupts(sdhc);

	LOG_E("send cmd xfr_typ 0x%x", xfr_typ);
	/* send cmd */
	sdhc1_base->CMD_ARG = cmd->arg;
	sdhc1_base->CMD_XFR_TYP = xfr_typ;

}

static void imx6ull_sdhc_process_next(struct imx6ull_sdhc *sdhc)
{
	if (sdhc->status == REQ_ST_INIT)
	{
		sdhc->status = REQ_ST_CMD;
		imx6ull_sdhc_send_command(sdhc, sdhc->req->cmd);
	}
	else if ((sdhc->status == REQ_ST_CMD) && sdhc->req->stop)
	{
		sdhc->status = REQ_ST_STOP;
		imx6ull_sdhc_send_command(sdhc, sdhc->req->stop);
	}
	else
	{
		mmcsd_req_complete(sdhc->host);
	}
}

static void imx6ull_sdhc_request(struct rt_mmcsd_host *host, struct rt_mmcsd_req *req)
{
	LOG_D("-----------------------");
	LOG_E("cmd %d, cmd args 0x%x, data %d stop %d", req->cmd->cmd_code, req->cmd->arg, req->data != RT_NULL, req->stop != RT_NULL);

	struct imx6ull_sdhc *sdhc = host->private_data;
	sdhc->req = req;
	sdhc->status = REQ_ST_INIT;

	imx6ull_sdhc_process_next(sdhc);
}

static void im6ull_sdhc_reset(void)
{
	rt_uint32_t port_ctrl;
	rt_uint32_t sys_ctl;

	sdhc1_base->SYS_CTRL |= RSTA;
	while(sdhc1_base->SYS_CTRL & RSTA);

	sdhc1_base->SYS_CTRL |= RSTC;
	while(sdhc1_base->SYS_CTRL & RSTC);

	sdhc1_base->SYS_CTRL |= RSTD;
	while(sdhc1_base->SYS_CTRL & RSTD);

	port_ctrl = sdhc1_base->PROT_CTRL;
	port_ctrl &= ~EMODE_MASK;
	port_ctrl |= EMODE_LITTLE_ENDIAN;
	sdhc1_base->PROT_CTRL = port_ctrl;

	sdhc1_base->VEND_SPEC &= ~EXT_DMA_EN;
	sdhc1_base->PROT_CTRL &= ~(DMASEL_MASK | BURST_LEN_EN_MASK);
	sdhc1_base->PROT_CTRL |= (DMASEL_ADMA2 | BURST_LEN_INCR);
	
	sdhc1_base->WTMK_LVL = ((0x8 << WR_BRST_LEN_SHIFT) | (0x80 << WR_WML_SHIFT) | (0x8 << RD_BRST_LEN_SHIFT) | (0x80 << RD_WML_SHIFT));
	
	sys_ctl = sdhc1_base->SYS_CTRL;
	sys_ctl &= ~DTOCV_MASK;
	sys_ctl |= (0xC << DTOCV_SHIFT);
	sdhc1_base->SYS_CTRL = sys_ctl;


	//dump_sdhc1();
}

static void im6ull_sdhc_active(void)
{
	sdhc1_base->SYS_CTRL |= INITA;
	while(sdhc1_base->SYS_CTRL & INITA);
}

static void im6ull_sdhc_set_clock(rt_uint32_t srcClock_Hz, rt_uint32_t busClock_Hz)
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
	while (!(sdhc1_base->PRES_STATE & SDSTB));

	LOG_D("set sd bus clk %d", nearestFrequency);
}


static void imx6ull_sdhc_set_iocfg(struct rt_mmcsd_host *host, struct rt_mmcsd_io_cfg *io_cfg)
{
	int prot_ctrl = 0;
	if(io_cfg->clock > host->freq_max)
	{
		io_cfg->clock = host->freq_max;
	}

	LOG_E("io_cfg->clock %d", io_cfg->clock);

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
	LOG_D("%s", __func__);
	return RT_EOK;
}
static void imx6ull_sdhc_enable_sdio_irq(struct rt_mmcsd_host *host, rt_int32_t en)
{
	LOG_D("%s", __func__);
}

void rt_hw_usdhc1_irq(int vector, void *param)
{ 
	rt_uint32_t intsta; 

	/* enter interrupt */
	rt_interrupt_enter();

	intsta = sdhc1_base->INT_STATUS;

	LOG_E("uSDHC1 irq 0x%x", intsta);
	
	sdhc1_base->INT_STATUS    = intsta;
	//sdhc1_base->INT_SIGNAL_EN &= ~intsta;
	//sdhc1_base->INT_STATUS_EN &= ~intsta;

	imx6ull_sdhc_completed_command(sdhc1, intsta);

	/* leave interrupt */
	rt_interrupt_leave();
}

struct rt_mmcsd_host_ops imx6ull_sdio_ops =
{
	imx6ull_sdhc_request,
	imx6ull_sdhc_set_iocfg,
	imx6ull_sdhc_get_card_status,
	imx6ull_sdhc_enable_sdio_irq,
};

void imx6ull_sdio_low_init(void);

void imx6ull_sdio(void)
{
	struct rt_mmcsd_host *host;

	imx6ull_sdio_low_init();

	rt_kprintf("sdbuf 0x%x\n", sd_buf);
	rt_kprintf("adma2_tablbe 0x%x\n", adma2_tablbe);

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

	/* config host */
	host->ops           = &imx6ull_sdio_ops;
	host->freq_min      = 400000;
	host->freq_max      = 50000000;
	host->valid_ocr     = VDD_32_33 | VDD_33_34;

    /* bus width 1 */
	host->flags         = MMCSD_MUTBLKWRITE;
	host->max_seg_size  = 4096;
	host->max_blk_size  = 512;

	host->private_data  = sdhc1;

	sdhc1->host = host;

	mmcsd_change(host);
}
MSH_CMD_EXPORT(imx6ull_sdio, ...);

#include <dfs_posix.h>

void mount_test(void)
{
	if(dfs_mount("sd0", "/", "elm", 0, RT_NULL) < 0)
	{
		rt_kprintf("mount fatfs failed\n");
	}
}
MSH_CMD_EXPORT(mount_test, ...);


void imx6ull_sdio_low_init(void)
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

/*sd card read/write*/
void sd_block(void)
{
	int i;
	rt_device_t sd;
	struct rt_device_blk_geometry sd_geometry;
	rt_uint8_t *read_buf;

	sd = rt_device_find("sd0");

	if (sd == RT_NULL)
	{
		LOG_E("can not find sd0");
		return;
	}

	rt_device_control(sd, RT_DEVICE_CTRL_BLK_GETGEOME, &sd_geometry);

	LOG_I("sector_count %d, bytes_per_sector %d, block_size %d", sd_geometry.sector_count, sd_geometry.bytes_per_sector, sd_geometry.block_size);

	read_buf = rt_calloc(1, sd_geometry.bytes_per_sector);

	rt_device_read(sd, 2, read_buf, sd_geometry.bytes_per_sector);

	for(i = 0; i < sd_geometry.bytes_per_sector; i++)
	{
		rt_kprintf("0x%02x ", read_buf[i]);
		if((i + 1) % 32 == 0)
		{
			rt_kprintf("\n");
		}
	}
	rt_kprintf("\n");
}
MSH_CMD_EXPORT(sd_block, ...);

