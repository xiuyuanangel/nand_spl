/*
 * (C) Copyright 2018
* SPDX-License-Identifier:	GPL-2.0+
 * wangwei <wangwei@allwinnertech.com>
 */

#include <config.h>
#include <asm/mode.h>

.globl _start
_start: b	reset

reset:
	mrs r0, cpsr
	bic r0, r0, #ARMV7_MODE_MASK
	orr r0, r0, #ARMV7_SVC_MODE
	orr r0, r0, #(ARMV7_IRQ_MASK | ARMV7_FIQ_MASK)
	bic r0, r0, #(1<<9)     @set little-endian
	msr cpsr_c, r0

	mrc	p15, 0, r0, c1, c0, 0
	bic	r0, r0, #0x00002000	@ clear bits 13 (--V-)
	bic	r0, r0, #0x00000007	@ clear bits 2:0 (-CAM)
	orr	r0, r0, #0x00000800	@ set bit 11 (Z---) BTB
	bic	r0, r0, #0x00001000	@ clear bit 12 (I) I-cache
	mcr	p15, 0, r0, c1, c0, 0

	ldr sp, =CONFIG_NBOOT_STACK
	bl  clear_bss

    /* Copy ddr bin to 0x00028000 */
/*
    ldr r0, _ddr_bin_start
    ldr r2, _ddr_bin_end
    sub r2, r2, r0
    ldr r1, _image_start
    sub r0, r0, r1
    adr r1, _start
    add r1, r1, r0
    ldr r0, =0x00028000
    bl memcpy
*/
    /* Copy ddr param to 0x00028038 */
/*
    ldr r0, _ddr_param_start
    ldr r2, _ddr_param_end
    sub r2, r2, r0
    ldr r1, _image_start
    sub r0, r0, r1
    adr r1, _start
    add r1, r1, r0
    ldr r0, =0x00028038
    bl memcpy
*/    
	bl  main

clear_bss:
	ldr	r0, =__bss_start
	ldr	r1, =__bss_end
	mov	r2, #0

clbss_1:
	stmia   r0!, {r2}
	cmp r0, r1
	blt clbss_1

	mov pc, lr

_image_start:
    .long __image_start
_image_end:
    .long __image_end

_ddr_param_start:
    .long __ddr_param_start
_ddr_param_end:
    .long __ddr_param_end
_ddr_bin_start:
    .long __ddr_bin_start
_ddr_bin_end:
    .long __ddr_bin_end

