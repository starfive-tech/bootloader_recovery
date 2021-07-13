/* SPDX-License-Identifier: GPL-2.0-or-later */
/**
  ******************************************************************************
  * @file  sys.h
  * @author  StarFive Technology
  * @version  V1.0
  * @date  07/22/2020
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

#ifndef SYS_H_
#define SYS_H_

#include "comdef.h"
#include "serial_printf.h"

//#define DEBUG
#ifdef DEBUG
#define DEBUG_INFO      serial_printf
#else
#define DEBUG_INFO      
#endif

#define uart_printf(...)    serial_printf(__VA_ARGS__)

// Pheripheral register access
#define REGW(a)             *((volatile UINT32 *)(a))
#define REGH(a)             *((volatile UINT16 *)(a))
#define REGB(a)             *((volatile UINT8 *)(a))
#define writew(v,a)			(*(volatile unsigned short *)(a) = (v))
#define readw(a)			(*(volatile unsigned short *)(a))
/*********************************** sys clock************************/
//#define FOR_FPGA           1
#define SYS_BOOT_CLK        6250000

//#if(FOR_FPGA == 1)
#define SECONDBOOTDELAY     1
#define UART_BAUD_DIV       20
#define UART0_CLK           SYS_BOOT_CLK
#define SYS_APB_CLK			SYS_BOOT_CLK
#define SD_CARD_CLK         SYS_BOOT_CLK
#define QSPI_CLK            10000000
#define QSPI_REF_CLK        250000000
#define TIMER_CLK_HZ		SYS_BOOT_CLK


#define DEFAULT_BOOT_LOAD_ADDR  0x18080000
#define DEFAULT_SD_BOOT_LOAD_BLOCK  1000
#define SECONDBOOT_RUN_ADDR         0x18000000
#define DEFAULT_SPI_BOOT_START_ADDR 1
#define DEFAULT_SPI_BOOT_SIZE_ADDR 0x0

#define DEFAULT_SPI_BOOT_LOAD_PAGES     512
#define FLASH_SECONDBOOT_START_ADDR     0
#define FLASH_DDRINIT_START_ADDR        0x10000
#define     SPIBOOT_LOAD_ADDR_OFFSET    252
// QSPI
#define QSPI_BASE_ADDR		0x11860000
//CADENCE QSPI AHB
#define QSPI_BASE_AHB_ADDR  0x20000000

// TIMER --WDT
#define TIMER_BASE_ADDR         0x12480000

// GPIO
#define GPIO_BASE_ADDR		0x11910000///0xA9060000
#define EZGPIO_FULLMUX_BASE_ADDR		0x11910000
#define RSTGEN_BASE_ADDR                0x11840000
#define CLKGEN_BASE_ADDR                0x11800000
// CLKGEN
#define VIC_CLKGEN_TOP_SV_BASE_ADDR 0x11800000///0xA9090000

// RSTGEN
#define VIC_RSTGEN_BASE_ADDR 0x11840000///0xA90A0000
#define SYSCON_IOPAD_CTRL_BASE_ADDR     0x11858000
// SYSCON
#define SYSCON_SYSMAIN_CTRL_BASE_ADDR	0x11850000

// HS-UART0
#define UART0_BASE_ADDR                 0x11870000

// HS-UART1
#define UART1_HS_BASE_ADDR              0x11880000

// UART2
#define UART2_BASE_ADDR                 0x12430000

// UART3
#define UART3_BASE_ADDR                 0x12440000


/*********************************** function ************************/

/* memory*/
extern void sys_memcpy(void *p_des,const void * p_src,unsigned long size);
extern void sys_memcpy_32(void *p_des,const void * p_src,unsigned long size);
extern void sys_memset(void *p_des,unsigned char c,unsigned long size);
extern void sys_memset32(void *p_des,int c,unsigned long size);

extern  int sys_memcmp(const void * cs,const void * ct,unsigned int count);

extern void * _memcpy(void * dest,const void *src,unsigned int count);

#endif
