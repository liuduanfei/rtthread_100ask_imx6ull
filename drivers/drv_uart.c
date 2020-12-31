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

/*UART1的寄存器的基地址*/
#define UART1_BASE          (0x2020000u)

#define UART1    ((UART_Type *)UART1_BASE)

/*根据IMX6ULL芯片手册<<55.15 UART Memory Map/Register Definition>>的3608页，定义UART的结构体,*/
typedef struct {
  volatile unsigned int  URXD;               /**< UART Receiver Register, offset: 0x0 	           串口接收寄存器，偏移地址0x0     */
  		   unsigned char RESERVED_0[60];		
  volatile unsigned int  UTXD;               /**< UART Transmitter Register, offset: 0x40          串口发送寄存器，偏移地址0x40*/
  		   unsigned char RESERVED_1[60];		
  volatile unsigned int  UCR1;               /**< UART Control Register 1, offset: 0x80 	       串口控制寄存器1，偏移地址0x80*/
  volatile unsigned int  UCR2;               /**< UART Control Register 2, offset: 0x84 	       串口控制寄存器2，偏移地址0x84*/
  volatile unsigned int  UCR3;               /**< UART Control Register 3, offset: 0x88            串口控制寄存器3，偏移地址0x88*/
  volatile unsigned int  UCR4;               /**< UART Control Register 4, offset: 0x8C            串口控制寄存器4，偏移地址0x8C*/
  volatile unsigned int  UFCR;               /**< UART FIFO Control Register, offset: 0x90         串口FIFO控制寄存器，偏移地址0x90*/
  volatile unsigned int  USR1;               /**< UART Status Register 1, offset: 0x94             串口状态寄存器1，偏移地址0x94*/
  volatile unsigned int  USR2;               /**< UART Status Register 2, offset: 0x98             串口状态寄存器2，偏移地址0x98*/
  volatile unsigned int  UESC;               /**< UART Escape Character Register, offset: 0x9C     串口转义字符寄存器，偏移地址0x9C*/
  volatile unsigned int  UTIM;               /**< UART Escape Timer Register, offset: 0xA0         串口转义定时器寄存器 偏移地址0xA0*/
  volatile unsigned int  UBIR;               /**< UART BRM Incremental Register, offset: 0xA4      串口二进制倍率增加寄存器 偏移地址0xA4*/
  volatile unsigned int  UBMR;               /**< UART BRM Modulator Register, offset: 0xA8 	   串口二进制倍率调节寄存器 偏移地址0xA8*/
  volatile unsigned int  UBRC;               /**< UART Baud Rate Count Register, offset: 0xAC      串口波特率计数寄存器 偏移地址0xAC*/
  volatile unsigned int  ONEMS;              /**< UART One Millisecond Register, offset: 0xB0      串口一毫秒寄存器 偏移地址0xB0*/
  volatile unsigned int  UTS;                /**< UART Test Register, offset: 0xB4                 串口测试寄存器 偏移地址0xB4*/		
  volatile unsigned int  UMCR;               /**< UART RS-485 Mode Control Register, offset: 0xB8  串口485模式控制寄存器 偏移地址0xB8*/
} UART_Type;

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

    rt_hw_interrupt_install(58, rt_hw_uart_isr, serial, "uart");
    rt_hw_interrupt_umask(58);

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
