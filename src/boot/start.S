/* SPDX-License-Identifier: GPL-2.0-or-later */
/**
  ******************************************************************************
  * @file  start.S
  * @author  StarFive Technology
  * @version  V1.0
  * @date  07/29/2020
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
  * COPYRIGHT 2020 Shanghai StarFive Technology Co., Ltd.
  */
#include "encoding.h"

/* This is defined in sifive/platform.h, but that can't be included from
 * assembly. */
	.section .init
	.globl _start
	.type _start,@function

_start:
	.cfi_startproc
	.cfi_undefined ra
.option push
.option norelax
	la gp, __global_pointer$
.option pop
	la sp, _sp

    csrr a0, mhartid
    li a1, 0
    li a2, 0
  /* Increment by hartid number of stack sizes */
  li t0, 0
  la t1, __stack_size
1:
  beq t0, a0, 1f
  add sp, sp, t1
  addi t0, t0, 1
  j 1b
1:
  andi sp, sp, -16

    csrr a0, mhartid
    bnez a0, hart1_loop
    j start_boot_main

hart1_loop:
/*    j hart1_loop */
  li a2, MIP_MSIP
  csrw mie, a2
  wfi
  /*call second_main */

start_boot_main:
#if 0
	/* Load data section */
	la t0, _data_lma
	la t1, _data
	la t2, _edata
	bgeu t1, t2, 2f
1:
  ld   a0, 0(t0)
  addi t0, t0, 8
  sd   a0, 0(t1)
  addi t1, t1, 8
  blt  t1, t2, 1b
2:
#endif
	/* Clear bss section */
	la t1, __bss_start
	la t2, _end
	bgeu t1, t2, 2f
1:
  sd   x0, 0(t1)
  addi t1, t1, 8
  blt  t1, t2, 1b
2:

	/* Call global constructors */
	//la a0, __libc_fini_array
	//call atexit
	//call __libc_init_array

#ifndef __riscv_float_abi_soft
	/* Enable FPU */
	li t0, MSTATUS_FS
	csrs mstatus, t0
	csrr t1, mstatus
	and t1, t1, t0
	beqz t1, 1f
	fssr x0
1:
#endif

#if defined(ENABLE_SMP)
	smp_resume(t0, t1)

	csrr a0, mhartid
	bnez a0, 2f
#endif

	auipc ra, 0
	addi sp, sp, -16
#if __riscv_xlen == 32
	sw ra, 8(sp)
#else
	sd ra, 8(sp)
#endif

	/* argc = argv = 0 */
	li a0, 0
	li a1, 0
	call BootMain
	//tail exit
1:
	j 1b

	.cfi_endproc
