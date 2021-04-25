/*
 * COPYRIGHT (C) 2018, Real-Thread Information Technology Ltd
 * 
 * SPDX-License-Identifier: Apache-2.0
 * Change Logs:
 * Date           Author       Notes
 * 2013-03-30     Bernard      the first verion
 */

#include <rthw.h>
#include <rtdevice.h>
#include <board.h>

#define UART1_BASE          (0x2020000u)

#define UART1    ((UART_Type *)UART1_BASE)

static void rt_hw_uart_isr(int irqno, void *param)
{
    struct rt_serial_device *serial = (struct rt_serial_device *)param;

    rt_hw_serial_isr(serial, RT_SERIAL_EVENT_RX_IND);
}

static rt_err_t uart_configure(struct rt_serial_device *serial, struct serial_configure *cfg)
{
    UART1->UCR1 |= (1 << 0);		/*关闭当前串口*/ 
	
	/* 
	 *  设置UART传输格式：
	 *  UART1中的UCR2寄存器关键bit如下
	 *  [14]:	1：忽略RTS引脚
	 *  [8] :	0: 关闭奇偶校验 默认为0，无需设置
	 *  [6] :	0: 停止位1位	    默认为0，无需设置
	 *  [5] :	1: 数据长度8位
	 *  [2] :	1: 发送数据使能
	 *  [1] :	1: 接收数据使能
	 */
	
	UART1->UCR2 |= (1<<14) |(1<<5) |(1<<2)|(1<<1);

	/*
	 *  UART1中的UCR3寄存器关键bit如下
	 *  [2]:  1:根据官方文档表示，IM6ULL的UART用了这个MUXED模型，提示要设置	
	 */
	
	UART1->UCR3 |= (1<<2);
	
	/*
	 * 设置波特率
	 * 根据芯片手册得知波特率计算公式:
	 * Baud Rate = Ref Freq / (16 * (UBMR + 1)/(UBIR+1))
	 * 当我们需要设置 115200的波特率
	 * UART1_UFCR [9:7]=101，表示不分频，得到当前UART参考频率Ref Freq ：80M ，
	 * 带入公式：115200 = 80000000 /(16*(UBMR + 1)/(UBIR+1))
	 * 
	 * 选取一组满足上式的参数：UBMR、UBIR即可
	 *	
	 * UART1_UBIR = 71
	 * UART1_UBMR = 3124  
	 */
	 
    UART1->UFCR = 5 << 7;     /* Uart的时钟clk：80MHz */
    UART1->UBIR = 71;
    UART1->UBMR = 3124;
	UART1->UCR4 |= (1 << 0);
	UART1->UCR1 |= (1 << 0);  /*使能当前串口*/

    rt_hw_interrupt_install(UART1_IRQn, rt_hw_uart_isr, serial, "uart");
    rt_hw_interrupt_umask(UART1_IRQn);

    return RT_EOK;
}

static rt_err_t uart_control(struct rt_serial_device *serial, int cmd, void *arg)
{
    return RT_EOK;
}

static int uart_putc(struct rt_serial_device *serial, char c)
{
	while (!((UART1->USR2) & (1<<3))); /*等待上个字节发送完毕*/
	UART1->UTXD = (unsigned char)c;

    return 1;
}

static int uart_getc(struct rt_serial_device *serial)
{
    int ch = -1;

	if (UART1->USR2 & (1<<0)) /*等待接收数据*/
	{
		return (int)UART1->URXD;
	}
    return ch;
}

static const struct rt_uart_ops _uart_ops =
{
    uart_configure,
    uart_control,
    uart_putc,
    uart_getc,
};

void uart_iomux(void)
{
#ifdef BSP_USING_UART1

#if 0
	static volatile unsigned int *IOMUXC_SW_MUX_CTL_PAD_UART1_TX_DATA;
	static volatile unsigned int *IOMUXC_SW_MUX_CTL_PAD_UART1_RX_DATA;
	static volatile unsigned int *IOMUXC_UART1_RX_DATA_SELECT_INPUT;
	IOMUXC_SW_MUX_CTL_PAD_UART1_TX_DATA 	= (volatile unsigned int *)(0x20E0084);
	IOMUXC_SW_MUX_CTL_PAD_UART1_RX_DATA 	= (volatile unsigned int *)(0x20E0088);
	IOMUXC_UART1_RX_DATA_SELECT_INPUT		= (volatile unsigned int *)(0x20E0624);

	*IOMUXC_SW_MUX_CTL_PAD_UART1_RX_DATA = 0;
	*IOMUXC_UART1_RX_DATA_SELECT_INPUT = 3;
	*IOMUXC_SW_MUX_CTL_PAD_UART1_TX_DATA = 0;
#endif

    IOMUXC_SetPinMux(IOMUXC_UART1_RX_DATA_UART1_RX, 0U);
    IOMUXC_SetPinMux(IOMUXC_UART1_RX_DATA_UART1_TX, 0U);

#endif
}

struct rt_serial_device _uart1;

int rt_hw_uart_init(void)
{
    struct serial_configure config;

	uart_iomux();

    config.baud_rate = BAUD_RATE_115200;
    config.bit_order = BIT_ORDER_LSB;
    config.data_bits = DATA_BITS_8;
    config.parity    = PARITY_NONE;
    config.stop_bits = STOP_BITS_1;
    config.invert    = NRZ_NORMAL;
    config.bufsz     = RT_SERIAL_RB_BUFSZ;

#ifdef BSP_USING_UART1
    _uart1.ops = &_uart_ops;
    _uart1.config = config;

    /* register UART1 device */
    rt_hw_serial_register(&_uart1, "uart1",
                          RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX, RT_NULL);
#endif

    return 0;
}
INIT_BOARD_EXPORT(rt_hw_uart_init);
