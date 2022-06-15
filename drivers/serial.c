/*
 * (C) Copyright 2013-2016
* SPDX-License-Identifier:	GPL-2.0+
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 *
 */
/*
 * COM1 NS16550 support
 * originally from linux source (arch/powerpc/boot/ns16550.c)
 * modified to use CONFIG_SYS_ISA_MEM and new defines
 */

#include <common.h>
#include <asm/io.h>
#include <arch/uart.h>
#include <arch/gpio.h>
#include <arch/clock.h>

#define thr rbr
#define dll rbr
#define dlh ier
#define iir fcr


static serial_hw_t *serial_ctrl_base;


void sunxi_serial_init(int uart_port, void *gpio_cfg, int gpio_max)
{
	u32 uart_clk;
	void __iomem *uart0_base = sunxi_get_iobase(SUNXI_UART0_BASE);

#ifdef FPGA_PLATFORM
	normal_gpio_set_t fpga_uart_gpio[2] = {
	    /* 		{ 1, 5, 2, 1, 1, 0, {0}},// PA5: 2--RX */
	    /* 		{ 1, 4, 2, 1, 1, 0, {0}},// PA4: 2--TX */
	    {2, 9, 3, 1, 1, 0, {0} }, /* PB9: 3--RX */
	    {2, 8, 3, 1, 1, 0, {0} }, /* PB8: 3--TX */
	};
#endif
	sunxi_clock_init_uart(uart_port);
	/* gpio */
#ifdef FPGA_PLATFORM
	boot_set_gpio(fpga_uart_gpio, gpio_max, 1);
#else
	boot_set_gpio(gpio_cfg, gpio_max, 1);
#endif
	/* uart init */
	serial_ctrl_base = (serial_hw_t *)(uart0_base + uart_port * CCM_UART_ADDR_OFFSET);

	serial_ctrl_base->mcr = 0x3;
	uart_clk = (24000000 + 8 * UART_BAUD)/(16 * UART_BAUD);
	serial_ctrl_base->lcr |= 0x80;
	serial_ctrl_base->dlh = uart_clk>>8;
	serial_ctrl_base->dll = uart_clk&0xff;
	serial_ctrl_base->lcr &= ~0x80;
	serial_ctrl_base->lcr = ((PARITY&0x03)<<3) | ((STOP&0x01)<<2) | (DLEN&0x03);
	serial_ctrl_base->fcr = 0x7;

	return;
}

void sunxi_serial_putc (char c)
{
	while ((serial_ctrl_base->lsr & (1 << 6)) == 0)
		;
	serial_ctrl_base->thr = c;
}

char sunxi_serial_getc (void)
{
	while ((serial_ctrl_base->lsr & 1) == 0)
		;
	return serial_ctrl_base->rbr;
}

int sunxi_serial_tstc (void)
{
	return serial_ctrl_base->lsr & 1;
}

#if 0

#define virtual_addr_t unsigned long long
#define u32_t unsigned int

static inline __attribute__((__always_inline__)) u32_t read32(virtual_addr_t addr)
{
    return (*((volatile u32_t *)(addr)));
}

static inline __attribute__((__always_inline__)) void write32(virtual_addr_t addr, u32_t value)
{
    *((volatile u32_t *)(addr)) = value;
}

void sys_uart_init(void)
{
	virtual_addr_t addr;
	u32_t val;

	/* Config GPIOE2 and GPIOE3 to txd0 and rxd0 */
	addr = 0x020000c0 + 0x0;
	val = read32(addr);
	val &= ~(0xf << ((2 & 0x7) << 2));
	val |= ((0x6 & 0xf) << ((2 & 0x7) << 2));
	write32(addr, val);

	val = read32(addr);
	val &= ~(0xf << ((3 & 0x7) << 2));
	val |= ((0x6 & 0xf) << ((3 & 0x7) << 2));
	write32(addr, val);

	/* Open the clock gate for uart0 */
	addr = 0x0200190c;
	val = read32(addr);
	val |= 1 << 0;
	write32(addr, val);

	/* Deassert uart0 reset */
	addr = 0x0200190c;
	val = read32(addr);
	val |= 1 << 16;
	write32(addr, val);

	/* Config uart0 to 115200-8-1-0 */
	addr = 0x02500000;
	write32(addr + 0x04, 0x0);
	write32(addr + 0x08, 0xf7);
	write32(addr + 0x10, 0x0);
	val = read32(addr + 0x0c);
	val |= (1 << 7);
	write32(addr + 0x0c, val);
	write32(addr + 0x00, 0xd & 0xff);
	write32(addr + 0x04, (0xd >> 8) & 0xff);
	val = read32(addr + 0x0c);
	val &= ~(1 << 7);
	write32(addr + 0x0c, val);
	val = read32(addr + 0x0c);
	val &= ~0x1f;
	val |= (0x3 << 0) | (0 << 2) | (0x0 << 3);
	write32(addr + 0x0c, val);
}

void sys_uart_putc(char c)
{
	virtual_addr_t addr = 0x02500000;

	while((read32(addr + 0x7c) & (0x1 << 1)) == 0);
	write32(addr + 0x00, c);
}

void sunxi_serial_putc (char c)
{
    sys_uart_putc(c);
}

void sys_uart_puts(char *s)
{
    while(*s)sys_uart_putc(*s++);
}
#endif
