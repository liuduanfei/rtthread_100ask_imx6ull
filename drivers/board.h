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
#include <registers.h>

#define CONFIG_MX6
#define CONFIG_MX6UL

#if defined(__CC_ARM)
extern int Image$$RW_IRAM1$$ZI$$Limit;
#define HEAP_BEGIN      ((void*)&Image$$RW_IRAM1$$ZI$$Limit)
#elif defined(__GNUC__)
extern int __bss_end;
#define HEAP_BEGIN      ((void*)&__bss_end)
#endif

#define HEAP_END        (void*)(0x80000000 + 32 * 1024 * 1024)

/* the maximum number of gic */
#define ARM_GIC_MAX_NR 1

/* the maximum number of interrupts */
#define ARM_GIC_NR_IRQS 160

/* the maximum entries of the interrupt table */
#define MAX_HANDLERS 160

enum _gic_base_offsets
{
    kGICDBaseOffset = 0x1000,   //!< GIC distributor offset.
#if defined(CHIP_MX6UL)
    kGICCBaseOffset = 0x2000     //!< GIC CPU interface offset.
#else    
    kGICCBaseOffset = 0x100     //!< GIC CPU interface offset.
#endif    
};


/* the basic constants needed by gic */
rt_inline rt_uint32_t platform_get_gic_dist_base(void)
{
    rt_uint32_t gic_base;
    asm volatile ("mrc p15, 4, %0, c15, c0, 0" : "=r"(gic_base));
    return gic_base + kGICDBaseOffset;
}

rt_inline rt_uint32_t platform_get_gic_cpu_base(void)
{
    rt_uint32_t gic_base;
    asm volatile ("mrc p15, 4, %0, c15, c0, 0" : "=r"(gic_base));
    return gic_base + kGICCBaseOffset;
}

#define GIC_IRQ_START   0

#define GIC_ACK_INTID_MASK              0x000003ff

/* the definition needed by gic.c */
#define __REG32(x)  (*((volatile unsigned int *)(x)))

typedef void (*irq_hdlr_t) (void);

extern void rt_hw_interrupt_mask(int vector);
extern void rt_hw_interrupt_umask(int vector);
extern rt_isr_handler_t rt_hw_interrupt_install(int vector, rt_isr_handler_t handler,
    void *param, const char *name);

rt_inline void register_interrupt_routine(uint32_t irq_id, irq_hdlr_t isr)
{
    rt_hw_interrupt_install(irq_id, (rt_isr_handler_t)isr, NULL, "unknown");
}

rt_inline void enable_interrupt(uint32_t irq_id, uint32_t cpu_id, uint32_t priority)
{
    rt_hw_interrupt_umask(irq_id);
}

rt_inline void disable_interrupt(uint32_t irq_id, uint32_t cpu_id)
{
    rt_hw_interrupt_mask(irq_id);
}

/* keep compatible with platform SDK */
typedef enum {
    CPU_0,
    CPU_1,
    CPU_2,
    CPU_3,
} cpuid_e;

enum _gicd_sgi_filter
{
    //! Forward the interrupt to the CPU interfaces specified in the @a target_list parameter.
    kGicSgiFilter_UseTargetList = 0,

    //! Forward the interrupt to all CPU interfaces except that of the processor that requested
    //! the interrupt.
    kGicSgiFilter_AllOtherCPUs = 1,

    //! Forward the interrupt only to the CPU interface of the processor that requested the
    //! interrupt.
    kGicSgiFilter_OnlyThisCPU = 2
};




void rt_hw_board_init(void);

#endif
