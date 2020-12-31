/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2013-07-06     Bernard    the first version
 */

#ifndef __BOARD_H__
#define __BOARD_H__

#include <rthw.h>

#if defined(__CC_ARM)
extern int Image$$RW_IRAM1$$ZI$$Limit;
#define HEAP_BEGIN           ((void*)&Image$$RW_IRAM1$$ZI$$Limit)
#elif defined(__GNUC__)
extern int __bss_end;
#define HEAP_BEGIN           ((void*)&__bss_end)
#endif

#define HEAP_END             (void*)(0x80000000 + 512 * 1024 * 1024)


#define GIC_IRQ_START        0
#define GIC_ACK_INTID_MASK   0x000003ff

/* the maximum number of gic */
#define ARM_GIC_MAX_NR       1

/* the maximum number of interrupts */
#define ARM_GIC_NR_IRQS      160

/* the maximum number of exception and interrupt handler table */
#define MAX_HANDLERS         160

enum
{
    GIC_DIST_OFFSET = 0x1000,    //!< GIC distributor offset.
    GIC_CPU_OFFSET  = 0x2000     //!< GIC CPU interface offset.  
};


/* the basic constants needed by gic */
rt_inline rt_uint32_t platform_get_gic_dist_base(void)
{
    rt_uint32_t gic_base;
    asm volatile ("mrc p15, 4, %0, c15, c0, 0" : "=r"(gic_base));
    return gic_base + GIC_DIST_OFFSET;
}

rt_inline rt_uint32_t platform_get_gic_cpu_base(void)
{
    rt_uint32_t gic_base;
    asm volatile ("mrc p15, 4, %0, c15, c0, 0" : "=r"(gic_base));
    return gic_base + GIC_CPU_OFFSET;
}

/* the definition needed by gic.c */
#define __REG32(x)  (*((volatile unsigned int *)(x)))

typedef void (*irq_hdlr_t) (void);
void rt_hw_board_init(void);

#endif
