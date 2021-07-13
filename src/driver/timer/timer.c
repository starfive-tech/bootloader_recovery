/* SPDX-License-Identifier: GPL-2.0-or-later */
/**
  ******************************************************************************
  * @file  timer.c
  * @author  StarFive Technology
  * @version  V1.0
  * @date  07/24/2020
  * @brief
  ******************************************************************************
  * @copy
  *
  * THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STARFIVE SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  *  COPYRIGHT 2020 Shanghai StarFive Technology Co., Ltd.
  */

#include <comdef.h>
#include <sys.h>
#include <timer.h>
#include <clkgen_ctrl_macro.h>
#include <uart.h>
#include <div64.h>

#define TIMER_RATE_HZ				(TIMER_CLK_HZ /1000)  

#include "platform.h"

unsigned long long get_ticks(void)
{
	return readq(CLINT_CTRL_MTIME);
}

u32 get_timer(unsigned int base)
{
	return lldiv(get_ticks(), TIMER_RATE_HZ) - base;
}
#if 0
u32 usec_to_tick(u32 usec)
{
    u32 value = usec*(timer_clk/1000000);
    //printf("value = 0x%x\r\n", value);
    
    return usec*(timer_clk/1000000);
}

int udelay(u32 usec)
{
    UINT64 tmp;
    UINT64 tmo;

    tmo = usec_to_tick(usec);
    tmp = get_ticks(0) + tmo;	/* get current timestamp */

    while (get_ticks(0) < tmp)/* loop till event */
    	 /*NOP*/
    {
    }   

    return 0;
}

#endif

u64 usec_to_tick(u64 usec)
{
    u64 value;
    value = usec*(TIMER_CLK_HZ/1000)/1000;
    return value;
}

/* delay x useconds */
int udelay(unsigned int usec)
{
	unsigned long  tmp;

	tmp = readq((volatile void *)CLINT_CTRL_MTIME) + usec_to_tick(usec);	/* get current timestamp */
    
	while (readq((volatile void *)CLINT_CTRL_MTIME) < tmp);
}

void mdelay(unsigned int ms)
{
	udelay(1000*ms);
}

void sdelay(unsigned int s)
{
	mdelay(1000*s);
}

