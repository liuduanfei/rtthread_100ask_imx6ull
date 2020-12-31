/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2012-11-20     Bernard    the first version
 */

#include <rthw.h>
#include <rtthread.h>

#include "board.h"
#include <mmu.h>

struct mem_desc platform_mem_desc[] = {
    {0x00000000, 0x80000000, 0x00000000, DEVICE_MEM},
    {0x80000000, 0xFFF00000, 0x80000000, NORMAL_MEM}
};

const rt_uint32_t platform_mem_desc_size = sizeof(platform_mem_desc)/sizeof(platform_mem_desc[0]);

typedef struct {
    rt_uint32_t CR;                                /**< Control register, offset: 0x0 */
    rt_uint32_t SR;                                /**< Status register, offset: 0x4 */
    rt_uint32_t LR;                                /**< Load register, offset: 0x8 */
    rt_uint32_t CMPR;                              /**< Compare register, offset: 0xC */
    rt_uint32_t CNR;                               /**< Counter register, offset: 0x10 */
} EPIT_Type;

EPIT_Type *epit1 = (EPIT_Type *)(0x020D0000);

static void rt_hw_timer_isr(int vector, void *param)
{
    rt_tick_increase();
    epit1->SR |= (1 << 0);
}

int rt_hw_timer_init(void)
{
	epit1->CR = 0;

	/* software reset  
	 * bit16
	 */
	epit1->CR |= (1 << 16);
	/* wait for software reset self clear*/
	while((epit1->CR) & (1 << 16))
		;
		  			 		  						  					  				 	   		  	  	 	  
	/*
	 * EPIT_CR
	 * bit21 stopen; bit19 waiten; bit18 debugen
	 * bit17 overwrite enable; bit3 reload
	 * bit2 compare interrupt enable; bit1 enable mode
	 */
	epit1->CR |= (1 << 21) | (1 << 19) | (1 << 3) | (1 << 1);

	/*
	 * EPIT_CR
	 * bit25-24: 00 off, 01 peripheral clock(ipg clk), 10 high, 11 low
	 * bit15-4: prescaler value, divide by n+1
	 */
	epit1->CR &= ~((0x3 << 24) | (0xFFF << 4));
	epit1->CR |= (1 << 24);

	/* EPIT_CMPR: compare register */
	epit1->CMPR = 0;

	/* assume use ipc clk which is 66MHz, 1us against to 66 count */
	#define USEC_TO_COUNT(us) (us * 66 - 1)
	/* EPIT_LR: load register , assue use ipc clk 66MHz*/
	epit1->LR = USEC_TO_COUNT(1000);

	epit1->CR |= (1 << 2);


    rt_hw_interrupt_install(88, rt_hw_timer_isr, RT_NULL, "tick");
    rt_hw_interrupt_umask(88);

	epit1->CR |= (1 << 0);

    return 0;
}
INIT_BOARD_EXPORT(rt_hw_timer_init);

/**
 * This function will initialize hardware board
 */
void rt_hw_board_init(void)
{
	extern void enable_neon_fpu(void);

    enable_neon_fpu();

	rt_hw_interrupt_init();

#ifdef RT_USING_HEAP
    rt_system_heap_init(HEAP_BEGIN, HEAP_END);
#endif

    rt_components_board_init();
    rt_console_set_device(RT_CONSOLE_DEVICE_NAME);
}

/*@}*/
