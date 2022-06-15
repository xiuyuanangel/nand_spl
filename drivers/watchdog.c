/*
 * Copyright (C) 2019 Allwinner Tech
* SPDX-License-Identifier:	GPL-2.0+
 * frank <frank@allwinnertech.com>
 */

#include <arch/watchdog.h>
#include <asm/io.h>
#include <common.h>

static const int wdt_timeout_map[] = {
	[1]  = 0x1,  /* 1s  */
	[2]  = 0x2,  /* 2s  */
	[3]  = 0x3,  /* 3s  */
	[4]  = 0x4,  /* 4s  */
	[5]  = 0x5,  /* 5s  */
	[6]  = 0x6,  /* 6s  */
	[8]  = 0x7,  /* 8s  */
	[10] = 0x8, /* 10s */
	[12] = 0x9, /* 12s */
	[14] = 0xA, /* 14s */
	[16] = 0xB, /* 16s */
};

void wdt_stop(void)
{
	struct sunxi_wdog *wdt = (struct sunxi_wdog *)SUNXI_WDOG_BASE;
	unsigned int wtmode;

	wtmode = readl(&wdt->mode);
	wtmode &= ~WDT_MODE_EN;

	writel((0x16AA<<16)|wtmode, &wdt->mode);
}

void wdt_start(unsigned int timeout)
{
	struct sunxi_wdog *wdt = (struct sunxi_wdog *)SUNXI_WDOG_BASE;
	unsigned int wtmode;

	wdt_stop();

	if (wdt_timeout_map[timeout] == 0)
		timeout++;

	writel((0x16AA<<16)|1, 0x020500a8);

	wtmode = wdt_timeout_map[timeout] << 4 | WDT_MODE_EN;

	writel((0xA57<<1)|WDT_CFG_RESET, &wdt->cfg);
//    printf("%s %d -- %x\n",__FILE__,__LINE__,readl(&wdt->cfg));
	writel((0x16AA<<16)|wtmode, &wdt->mode);
// printf("%s %d -- %x %x %x %x\n",__FILE__,__LINE__,SUNXI_WDOG_BASE,timeout,wtmode,WDT_CFG_RESET);   
}
