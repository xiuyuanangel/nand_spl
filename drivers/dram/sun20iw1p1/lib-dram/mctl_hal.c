// SPDX-License-Identifier: GPL-2.0+

#include <common.h>
#include <arch/dram_v2.h>

#include "sdram.h"

#if 1
//typedef long          intptr_t;
#undef readl
#undef writel

//#define readl(x)	rv_readl((const volatile void __iomem *)(intptr_t)(x))
//#define writel(x, v)	rv_writel(v, (volatile void __iomem *)(intptr_t)(x))
#define readl(x)	(*(volatile unsigned int __iomem *)(x))
#define writel(x, v)	(*(volatile unsigned int __iomem *)(x) = (v))
#else
#define writel(x, v)	writel(v, x)
#endif

int set_ddr_voltage(int val)
{
	return val;
}

void handler_super_standby(void)
{
	return;
}

int get_pmu_exists(void)
{
	return -1;
}

char* memcpy_self(char* dst, char* src, int len)
{
	int i;
	for(i=0; i!=len; i++) {
		dst[i] = src[i];
	}
	return dst;
}

// Not used ??
//
void dram_udelay(unsigned int d1)
{
	unsigned long long d2 = d1;
	sdelay(d2);
	return;
}

void dram_vol_set(dram_para_t *para)
{
	int reg, vol = 0;

	switch( para->dram_type ) {
	case 2:  vol = 47; break;
	case 3:  vol = 25; break;
	default: vol = 0;
	}
vol = 25; // XXX
	reg = readl(0x3000150);
	reg &= ~(0xff00);
	reg |= vol << 8;
	reg &= ~(0x200000);
	writel(0x3000150, reg);

	sdelay(1);
}

void paraconfig(unsigned int *para, unsigned int mask, unsigned int value)
{
	*para &= ~(mask);
	*para |= value;
}


void dram_enable_all_master(void)
{
	writel(0x3102020, -1);
	writel(0x3102024, 0xff);
	writel(0x3102028, 0xffff);
	sdelay(10);
}


void dram_disable_all_master(void)
{
	writel(0x3102020, 1);
	writel(0x3102024, 0);
	writel(0x3102028, 0);
	sdelay(10);
}


void eye_delay_compensation(dram_para_t *para) // s1
{
	unsigned int val, ptr;

	// DATn0IOCR, n =  0...7
	for (ptr = 0x3103310; ptr != 0x3103334; ptr += 4) {
		val  = readl(ptr);
		val |= (para->dram_tpr11 << 9) & 0x1e00;
		val |= (para->dram_tpr12 << 1) & 0x001e;
		writel(ptr, val);
	}

	// DATn1IOCR, n =  0...7
	for (ptr = 0x3103390; ptr != 0x31033b4; ptr += 4) {
		val = readl(ptr);
		val |= ((para->dram_tpr11 >> 4) << 9) & 0x1e00;
		val |= ((para->dram_tpr12 >> 4) << 1) & 0x001e;
		writel(ptr, val);
	}

	// PGCR0: assert AC loopback FIFO reset
	val  = readl(0x3103100);
	val &= 0xfbffffff;
	writel(0x3103100, val);

	// ??
	val  = readl(0x3103334);
	val |= ((para->dram_tpr11 >> 16) << 9) & 0x1e00;
	val |= ((para->dram_tpr12 >> 16) << 1) & 0x001e;
	writel(0x3103334, val);

	val  = readl(0x3103338);
	val |= ((para->dram_tpr11 >> 16) << 9) & 0x1e00;
	val |= ((para->dram_tpr12 >> 16) << 1) & 0x001e;
	writel(0x3103338, val);

	val  = readl(0x31033b4);
	val |= ((para->dram_tpr11 >> 20) << 9) & 0x1e00;
	val |= ((para->dram_tpr12 >> 20) << 1) & 0x001e;
	writel(0x31033b4, val);

	val  = readl(0x31033b8);
	val |= ((para->dram_tpr11 >> 20) << 9) & 0x1e00;
	val |= ((para->dram_tpr12 >> 20) << 1) & 0x001e;
	writel(0x31033b8, val);

	val  = readl(0x310333c);
	val |= ((para->dram_tpr11 >> 16) << 25) & 0x1e000000;
	writel(0x310333c, val);

	val  = readl(0x31033bc);
	val |= ((para->dram_tpr11 >> 20) << 25) & 0x1e000000;
	writel(0x31033bc, val);

	// PGCR0: release AC loopback FIFO reset
	val  = readl(0x3103100);
	val |= 0x04000000;
	writel(0x3103100, val);

	sdelay(1);

	for (ptr = 0x3103240; ptr != 0x310327c; ptr += 4) {
		val = readl(ptr);
		val |= ((para->dram_tpr10 >> 4) << 8) & 0x0f00;
		writel(ptr, val);
	}

	for (ptr = 0x3103228; ptr != 0x3103240; ptr += 4) {
		val = readl(ptr);
		val |= ((para->dram_tpr10 >> 4) << 8) & 0x0f00;
		writel(ptr, val);
	}

	val  = readl(0x3103218);
	val |= (para->dram_tpr10 << 8) & 0x0f00;
	writel(0x3103218, val);

	val  = readl(0x310321c);
	val |= (para->dram_tpr10 << 8) & 0x0f00;
	writel(0x310321c, val);

	val  = readl(0x3103280);
	val |= ((para->dram_tpr10 >> 12) << 8) & 0x0f00;
	writel(0x3103280, val);
}


// Not used ??
//
void bit_delay_compensation(void)
{
	const unsigned int data0[44] = {
		0, 1, 2, 3, 2, 3, 3, 3, 0, 0, 0,
		6, 6, 6, 5, 5, 5, 5, 5, 0, 0, 0,
		0, 2, 4, 2, 6, 5, 5, 5, 0, 0, 0,
		3, 3, 3, 2, 2, 1, 1, 1, 0, 0, 0,
	};
	const unsigned int data1[44] = {
		0, 1, 3, 3, 3, 3, 3, 4, 3, 3, 3,
		3, 3, 4, 4, 3, 3, 3, 3, 3, 3, 3,
		0, 3, 3, 1, 6, 6, 5, 6, 3, 3, 3,
		5, 5, 6, 6, 4, 5, 3, 3, 3, 3, 3,
	};

	unsigned int *start = (unsigned int *)0x3102310; // DATX0IOCR
	unsigned int *end   = (unsigned int *)0x3102510; // DATX0IOCR x + 4 * size
	unsigned int *datxiocr;
	unsigned int i, j, k, rval;

	rval = readl(0x3102100) & 0x03ffffff;
	writel(0x3102100, rval);

	// Fill DATX0IOCR - DATX3IOCR, 11 registers per block, blocks 0x20 words apart
	for(i = 0, datxiocr = start; datxiocr != end; i += 11, datxiocr += 0x20) {
		for(j = 0, k = i; j != 11; j++, k++) {
			rval  = readl((unsigned int)datxiocr[j]);
			rval += data1[k] << 8;
			rval += data0[k];
			writel((unsigned int)datxiocr[j], rval);
		}
	}

	rval = readl(0x3102100) | 0x04000000;
	writel(0x3102100, rval);
}

// Not used ??
//
void set_master_priority_pad(dram_para_t *para)
{
	unsigned int val;

	val = readl(0x310200c) & 0xfffff000;
	val |= (para->dram_clk >> 1) - 1;
	writel(0x310200c, val);

	writel(0x3102200, 0x00001000);
	writel(0x3102210, 0x01000009);
	writel(0x3102214, 0x00500100);
	writel(0x3102230, 0x0200000d);
	writel(0x3102234, 0x00600100);
	writel(0x3102240, 0x01000009);
	writel(0x3102244, 0x00500100);
	writel(0x3102260, 0x00640209);
	writel(0x3102264, 0x00200040);
	writel(0x3102290, 0x01000009);
	writel(0x3102294, 0x00400080);
	writel(0x3102470, 0);
	writel(0x3102474, 0);

	writel(0x31031c0, 0x0f802f05);
	writel(0x31031c8, 0x0f0000ff);
	writel(0x31031d0, 0x3f00005f);
}

int auto_cal_timing(unsigned int time, unsigned int freq)
{
	unsigned int t = time*freq;
	return t/1000 + ( ((t%1000) != 0) ? 1 : 0);
}

// Main purpose of the auto_set_timing routine seems to be to calculate all
// timing settings for the specific type of sdram used. Read together with
// an sdram datasheet for context on the various variables.
//
void auto_set_timing_para(dram_para_t *para) // s5
{
	unsigned int   freq;	// s4
	unsigned int   type;	// s8
	unsigned int   tpr13;	// 80(sp)
	unsigned int   reg_val;

	unsigned char  tccd;	// 88(sp)
	unsigned char  trrd;	// s7
	unsigned char  trcd;	// s3
	unsigned char  trc;	// s9
	unsigned char  tfaw;	// s10
	unsigned char  tras;	// s11
	unsigned char  trp;	// 0(sp)
	unsigned char  twtr;	// s1
	unsigned char  twr;	// s6
	unsigned char  trtp;	// 64(sp)
	unsigned char  txp;	// a6
	unsigned short trefi;	// s2
	unsigned short trfc;	// a5 / 8(sp)

	freq  = para->dram_clk;
	type  = para->dram_type;
	tpr13 = para->dram_tpr13;

	//printf("type  = %d\n", type);
	//printf("tpr13 = %p\n", tpr13);

	if (para->dram_tpr13 & 0x2)
	{
		//dram_tpr0
		tccd = ( (para->dram_tpr0 >> 21) & 0x7  ); // [23:21]
		tfaw = ( (para->dram_tpr0 >> 15) & 0x3f ); // [20:15]
		trrd = ( (para->dram_tpr0 >> 11) & 0xf  ); // [14:11]
		trcd = ( (para->dram_tpr0 >>  6) & 0x1f ); // [10:6 ]
		trc  = ( (para->dram_tpr0 >>  0) & 0x3f ); // [ 5:0 ]
		//dram_tpr1
		txp =  ( (para->dram_tpr1 >> 23) & 0x1f ); // [27:23]
		twtr = ( (para->dram_tpr1 >> 20) & 0x7  ); // [22:20]
		trtp = ( (para->dram_tpr1 >> 15) & 0x1f ); // [19:15]
		twr =  ( (para->dram_tpr1 >> 11) & 0xf  ); // [14:11]
		trp =  ( (para->dram_tpr1 >>  6) & 0x1f ); // [10:6 ]
		tras = ( (para->dram_tpr1 >>  0) & 0x3f ); // [ 5:0 ]
		//dram_tpr2
		trfc  = ( (para->dram_tpr2 >> 12)& 0x1ff); // [20:12]
		trefi = ( (para->dram_tpr2 >> 0) & 0xfff); // [11:0 ]
	}
	else {
		unsigned int frq2 = freq >> 1; // s0

		if (type == 3) {
			// DDR3
			trfc  = auto_cal_timing( 350, frq2);
			trefi = auto_cal_timing(7800, frq2) / 32 + 1; // XXX
			twr   = auto_cal_timing(   8, frq2);
			trcd  = auto_cal_timing(  15, frq2);
			twtr  = twr + 2; // + 2 ? XXX
			if (twr < 2) twtr = 2;
			twr = trcd;
			if (trcd < 2) twr = 2;
			if (freq <= 800) {
				tfaw = auto_cal_timing(50, frq2);
				trrd = auto_cal_timing(10, frq2);
				if (trrd < 2) trrd = 2;
				trc  = auto_cal_timing(53, frq2);
				tras = auto_cal_timing(38, frq2);
				txp  = trrd; // 10
				trp  = trcd; // 15
			}
			else {
				tfaw = auto_cal_timing(35, frq2);
				trrd = auto_cal_timing(10, frq2);
				if (trrd < 2) trrd = 2;
				trcd = auto_cal_timing(14, frq2);
				trc  = auto_cal_timing(48, frq2);
				tras = auto_cal_timing(34, frq2);
				txp  = trrd; // 10
				trp  = trcd; // 14
			}
		}
		else if (type == 2) {
			// DDR2
			tfaw  = auto_cal_timing(  50, frq2);
			trrd  = auto_cal_timing(  10, frq2);
			trcd  = auto_cal_timing(  20, frq2);
			trc   = auto_cal_timing(  65, frq2);
			twtr  = auto_cal_timing(   8, frq2);
			trp   = auto_cal_timing(  15, frq2);
			tras  = auto_cal_timing(  45, frq2);
			trefi = auto_cal_timing(7800, frq2) / 32;
			trfc  = auto_cal_timing( 328, frq2);
			txp   = 2;
			twr   = trp; // 15
		}
		else if (type == 6) {
			// LPDDR2
			tfaw  = auto_cal_timing(  50, frq2);
			if (tfaw < 4) tfaw = 4;
			trrd  = auto_cal_timing(  10, frq2);
			if (trrd == 0) trrd = 1;
			trcd  = auto_cal_timing(  24, frq2);
			if (trcd < 2) trcd = 2;
			trc   = auto_cal_timing(  70, frq2);
			txp   = auto_cal_timing(   8, frq2);
			if (txp == 0) {
				txp  = 1;
				twtr = 2;
			}
			else {
				twtr = txp;
				if (txp < 2) {
					txp  = 2;
					twtr = 2;
				}
			}
			twr   = auto_cal_timing(  15, frq2);
			if (twr < 2) twr = 2;
			trp   = auto_cal_timing(  17, frq2);
			tras  = auto_cal_timing(  42, frq2);
			trefi = auto_cal_timing(3900, frq2) / 32;
			trfc  = auto_cal_timing( 210, frq2);
		}
		else if (type == 7) {
			// LPDDR3
			tfaw  = auto_cal_timing(  50, frq2);
			if (tfaw < 4) tfaw = 4;
			trrd  = auto_cal_timing(  10, frq2);
			if (trrd == 0) trrd = 1;
			trcd  = auto_cal_timing(  24, frq2);
			if (trcd < 2) trcd = 2;
			trc   = auto_cal_timing(  70, frq2);
			twtr   = auto_cal_timing(  8, frq2);
			if (twtr < 2) twtr = 2;
			twr   = auto_cal_timing(  15, frq2);
			if (twr < 2) twr = 2;
			trp   = auto_cal_timing(  17, frq2);
			tras  = auto_cal_timing(  42, frq2);
			trefi = auto_cal_timing(3900, frq2) / 32;
			trfc  = auto_cal_timing( 210, frq2);
			txp = twtr;
		}
		else {
			// default
			trfc  = 128;
			trp   =   6;
			trefi =  98;
			txp   =  10;
			twr   =   8;
			twtr  =   3;
			tras  =  14;
			tfaw  =  16;
			trc   =  20;
			trcd  =   6;
			trrd  =   3;
		}
		//assign the value back to the DRAM structure
		tccd = 2;
		trtp = 4; // not in .S ?
		para->dram_tpr0 = (trc<<0) | (trcd<<6) | (trrd<<11) | (tfaw<<15) | (tccd<<21);
		para->dram_tpr1 = (tras<<0) | (trp<<6) | (twr<<11) | (trtp<<15) | (twtr<<20) | (txp<<23);
		para->dram_tpr2 = (trefi<<0) | (trfc<<12);
	}

	unsigned int tcksrx;	// t1
	unsigned int tckesr;	// t4;
	unsigned int trd2wr;	// t6
	unsigned int trasmax;	// t3;
	unsigned int twtp;	// s6 (was twr!)
	unsigned int tcke;	// s8
	unsigned int tmod;	// t0
	unsigned int tmrd;	// t5
	unsigned int tmrw;	// a1
	unsigned int t_rdata_en;// a4 (was tcwl!)
	unsigned int tcl;	// a0
	unsigned int wr_latency;// a7
	unsigned int tcwl;	// first a4, then a5
	unsigned int mr3;	// s0
	unsigned int mr2;	// t2
	unsigned int mr1;	// s1
	unsigned int mr0;	// a3
	unsigned int dmr3;	// 72(sp)
	//unsigned int trtp;	// 64(sp)
	unsigned int dmr1;	// 56(sp)
	unsigned int twr2rd;	// 48(sp)
	unsigned int tdinit3;	// 40(sp)
	unsigned int tdinit2;	// 32(sp)
	unsigned int tdinit1;	// 24(sp)
	unsigned int tdinit0;	// 16(sp)

	dmr1 = para->dram_mr1;
	dmr3 = para->dram_mr3;

	switch (type) {

	case 2:	// DDR2
		{
		trasmax = freq / 30;
		if (freq < 409) {
			tcl        = 3;
			t_rdata_en = 1;
			mr0        = 0x06a3;
		}
		else {
			t_rdata_en = 2;
			tcl        = 4;
			mr0        = 0x0e73;
		}
		tmrd       =  2;
		twtp       =  twr + 5;
		tcksrx     =  5;
		tckesr     =  4;
		trd2wr     =  4;
		tcke       =  3;
		tmod       = 12;
		wr_latency =  1;
		mr3        =  0;
		mr2        =  0;
		tdinit0    = 200*freq + 1;
		tdinit1    = 100*freq / 1000 + 1;
		tdinit2    = 200*freq + 1;
		tdinit3    =   1*freq + 1;
		tmrw       =  0;
		twr2rd     = twtr + 5;
		tcwl       =  0;
		mr1        = dmr1;
		break;
	}

	case 3:	// DDR3
		{
		trasmax = freq / 30;
		if (freq <= 800) {
			mr0        = 0x1c70;
			tcl        =  6;
			wr_latency =  2;
			tcwl       =  4;
			mr2        = 24;
		}
		else {
			mr0        = 0x1e14;
			tcl        =  7;
			wr_latency =  3;
			tcwl       =  5;
			mr2        = 32;
		}

		twtp    = tcwl + 2 + twtr;	// WL+BL/2+tWTR
		trd2wr  = tcwl + 2 + twr;	// WL+BL/2+tWR
		twr2rd  = tcwl + twtr;		// WL+tWTR

		tdinit0 = 500*freq + 1;		// 500 us
		tdinit1 = 360*freq / 1000 + 1;	// 360 ns
		tdinit2 = 200*freq + 1;		// 200 us
		tdinit3 =   1*freq + 1;		//   1 us

		if (((tpr13>>2) & 0x03) == 0x01 || freq < 912) {
			mr1    = dmr1;
			t_rdata_en = tcwl;	// a5 <- a4
			tcksrx = 5;
			tckesr = 4;
			trd2wr = 5;
		}
		else {
			mr1    = dmr1;
			t_rdata_en = tcwl;	// a5 <- a4
			tcksrx = 5;
			tckesr = 4;
			trd2wr = 6;
		}
		tcke =  3; // not in .S ?
		tmod = 12;
		tmrd =  4;
		tmrw =  0;
		mr3  =  0;
		break;
		}

	case 6:	// LPDDR2
		{
		trasmax	   = freq / 60;
		mr3	   = dmr3;
		twtp       =  twr + 5;
		mr2	   =  6;
		mr1	   =  5;
		tcksrx     =  5;
		tckesr	   =  5;
		trd2wr	   = 10;
		tcke	   =  2;
		tmod	   =  5;
		tmrd	   =  5;
		tmrw	   =  3;
		tcl        =  4;
		wr_latency =  1;
		t_rdata_en =  1;
		tdinit0    = 200*freq + 1;
		tdinit1    = 100*freq / 1000 + 1;
		tdinit2    =  11*freq + 1;
		tdinit3    =   1*freq + 1;
		twr2rd	   = twtr + 5;
		tcwl       =   2;
		mr1	   = 195;
		mr0	   =   0;
		break;
	}

	case 7:	// LPDDR3
		{
		trasmax = freq / 60;
		if (freq < 800) {
			tcwl       =  4;
			wr_latency =  3;
			t_rdata_en =  6;
			mr2        = 12;
		}
		else {
			tcwl       =  3;
			tcke	   =  6;
			wr_latency =  2;
			t_rdata_en =  5;
			mr2        = 10;
		}
		twtp    = tcwl + 5;
		tcl     = 7;
		mr3     = dmr3;
		tcksrx  = 5;
		tckesr  = 5;
		trd2wr  = 13;
		tcke    = 3;
		tmod    = 12;
		tdinit0 = 400*freq + 1;
		tdinit1 = 500*freq / 1000 + 1;
		tdinit2 =  11*freq + 1;
		tdinit3 =   1*freq + 1;
		tmrd	= 5;
		tmrw    = 5;
		twr2rd  = tcwl + twtr + 5;
		mr1	= 195;
		mr0	= 0;
		break;
	}

	default:
		twr2rd		= 8;	// 48(sp)
		tcksrx		= 4;	// t1
		tckesr		= 3;	// t4
		trd2wr		= 4;	// t6
		trasmax		= 27;	// t3
		twtp		= 12;	// s6
		tcke		= 2;	// s8
		tmod		= 6;	// t0
		tmrd		= 2;	// t5
		tmrw		= 0;	// a1
		tcwl		= 3;	// a5
		tcl		= 3;	// a0
		wr_latency	= 1;	// a7
		t_rdata_en	= 1;	// a4
		mr3		= 0;	// s0
		mr2		= 0;	// t2
		mr1		= 0;	// s1
		mr0		= 0;	// a3
		tdinit3		= 0;	// 40(sp)
		tdinit2		= 0;	// 32(sp)
		tdinit1		= 0;	// 24(sp)
		tdinit0		= 0;	// 16(sp)
		break;
	}
	if (trtp < tcl - trp + 2) {
		trtp = tcl - trp + 2;
	}
	trtp = 4;

	// Update mode block when permitted
	if ((para->dram_mr0 & 0xffff0000) == 0) para->dram_mr0 = mr0;
	if ((para->dram_mr1 & 0xffff0000) == 0) para->dram_mr1 = mr1;
	if ((para->dram_mr2 & 0xffff0000) == 0) para->dram_mr2 = mr2;
	if ((para->dram_mr3 & 0xffff0000) == 0) para->dram_mr3 = mr3;

	// Set mode registers
	writel(0x3103030, para->dram_mr0);
	writel(0x3103034, para->dram_mr1);
	writel(0x3103038, para->dram_mr2);
	writel(0x310303c, para->dram_mr3);
	writel(0x310302c, (para->dram_odt_en >> 4) & 0x3); // ??

	// Set dram timing DRAMTMG0 - DRAMTMG5
	reg_val= (twtp<<24) | (tfaw<<16) | (trasmax<<8) | (tras<<0);
	writel(0x3103058, reg_val);
	reg_val= (txp<<16) | (trtp<<8) | (trc<<0);
	writel(0x310305c, reg_val);
	reg_val= (tcwl<<24) | (tcl<<16) | (trd2wr<<8) | (twr2rd<<0);
	writel(0x3103060, reg_val);
	reg_val= (tmrw<<16) | (tmrd<<12) | (tmod<<0);
	writel(0x3103064, reg_val);
	reg_val= (trcd<<24) | (tccd<<16) | (trrd<<8) | (trp<<0);
	writel(0x3103068, reg_val);
	reg_val= (tcksrx<<24) | (tcksrx<<16) | (tckesr<<8) | (tcke<<0);
	writel(0x310306c, reg_val);

	// Set two rank timing
	reg_val  = readl(0x3103078);
	reg_val &= 0x0fff0000;
	reg_val |= (para->dram_clk < 800) ? 0xf0006600 : 0xf0007600;
	reg_val |= 0x10;
	writel(0x3103078, reg_val);

	// Set phy interface time PITMG0, PTR3, PTR4
	reg_val = (0x2<<24) | (t_rdata_en<<16) | (0x1<<8) | (wr_latency<<0);
	writel(0x3103080, reg_val);
	writel(0x3103050, ((tdinit0<<0)|(tdinit1<<20)));
	writel(0x3103054, ((tdinit2<<0)|(tdinit3<<20)));

	// Set refresh timing and mode
	reg_val = (trefi<<16) | (trfc<<0);
	writel(0x3103090, reg_val);
	reg_val = 0x0fff0000 & (trefi<<15);
	writel(0x3103094, reg_val);
}

// Not used ?
//
void ccm_get_sscg(void)
{
	// NOTE: function is present in the assembly, but was not translated.
}

// Not used ?
//
void ccm_set_pll_sscg(void)
{
	// NOTE: function is present in the assembly, but was not translated.
}

// Purpose of this routine seems to be to initialize the PLL driving
// the MBUS and sdram.
//
int ccm_set_pll_ddr_clk(int index, dram_para_t *para)
{
	unsigned int val, clk, n;

	clk = (para->dram_tpr13 & (1 << 6)) ? para->dram_tpr9
					    : para->dram_clk;

	// set VCO clock divider
	n = (clk * 2) / 24;
	val = readl(0x2001010);
	val &= 0xfff800fc;   // clear dividers
	val |= (n - 1) << 8; // set PLL division
	val |= 0xc0000000;   // enable PLL and LDO
	writel(0x2001010, val);

	// Restart PLL locking
	val &= 0xdfffffff;   // disbable lock
	val |= 0xc0000000;   // enable PLL and LDO
	writel(0x2001010, val);
	val |= 0xe0000000;   // re-enable lock
	writel(0x2001010, val);

	// wait for PLL to lock
	while ((readl(0x2001010) & 0x10000000) == 0) {;}

	sdelay(20);

	// enable PLL output
	val = readl(0x2001000);
	val |= 0x08000000;
	writel(0x2001000, val);

	// turn clock gate on
	val = readl(0x2001800);
	val &= 0xfcfffcfc; // select DDR clk source, n=1, m=1
	val |= 0x80000000; // turn clock on
	writel(0x2001800, val);

	return n * 24;
}

// Main purpose of sys_init seems to be to initalise the clocks for
// the sdram controller.
//
void mctl_sys_init(dram_para_t *para)
{
	unsigned int val;

	// s1 = 0x02001000

	// assert MBUS reset
	val  = readl(0x2001540);
	val &= 0xbfffffff;
	writel(0x2001540, val);

	// turn off sdram clock gate, assert sdram reset
	val  = readl(0x200180c);
	val &= 0xfffffffe;
	writel(0x200180c, val);
	val &= 0xfffefffe;
	writel(0x200180c, val);

	// turn of bit 30 [??]
	val  = readl(0x2001800);
	writel(0x2001800, val & 0xbfffffff);
	// and toggle dram clock gating off + trigger update
	val &= 0x7fffffff;
	writel(0x2001800, val);
	val |= 0x08000000;
	writel(0x2001800, val);
	sdelay(10);

	// set ddr pll clock
	val = ccm_set_pll_ddr_clk(0, para);
	para->dram_clk = val >> 1;
	sdelay(100);
	dram_disable_all_master();

	// release sdram reset
	val  = readl(0x200180c);
	val |= 0x00010000;
	writel(0x200180c, val);

	// release MBUS reset
	val  = readl(0x2001540);
	val |= 0x40000000;
	writel(0x2001540, val);

	// turn bit 30 back on [?]
	val  = readl(0x2001800);
	val |= 0x40000000;
	writel(0x2001800, val);
	sdelay(5);

	// turn on sdram clock gate
	val  = readl(0x200180c);
	val |= 0x0000001; // (1<<0)
	writel(0x200180c, val);

	// turn dram clock gate on, trigger sdr clock update
	val  = readl(0x2001800);
	val |= 0x80000000;
	writel(0x2001800, val);
	val |= 0x88000000;
	writel(0x2001800, val);
	sdelay(5);

	// mCTL clock enable
	writel(0x310300c, 0x00008000);
	sdelay(10);
}

// The main purpose of this routine seems to be to copy an address configuration
// from the dram_para1 and dram_para2 fields to the PHY configuration registers
// (0x3102000, 0x3102004).
//
void mctl_com_init(dram_para_t *para)
{
	unsigned int val, end, ptr;
	int i;

	// purpose ??
	val = readl(0x3102008) & 0xffffc0ff;
	val |= 0x2000;
	writel(0x3102008, val);

	// Set sdram type and word width
	val  = readl(0x3102000) & 0xff000fff;
	val |= (para->dram_type &0x7) << 16;			// DRAM type
	val |= (~para->dram_para2 & 0x1) << 12;			// DQ width
	if((para->dram_type) != 6 && (para->dram_type) != 7) {
		val |= ((para->dram_tpr13 >> 5) & 0x1) << 19;	// 2T or 1T
		val |= 0x400000;
	}
	else {
		val |= 0x480000;	// type 6 and 7 must use 1T
	}
	writel(0x3102000, val);

	// init rank / bank / row for single/dual or two different ranks
	val = para->dram_para2;
	end = ((val & 0x100) && (((val >> 12) & 0xf) != 1)) ? 32 : 16;
	ptr = 0x3102000;

	for (i = 0 ; i != end; i += 16) {

		val = readl(ptr) & 0xfffff000;

		val |=   (para->dram_para2 >> 12) & 0x3;		   // rank
		val |=  ((para->dram_para1 >> (i + 12)) << 2) & 0x4;	   // bank - 2
		val |= (((para->dram_para1 >> (i +  4)) - 1) << 4) & 0xff; // row - 1

		// convert from page size to column addr width - 3
		switch ((para->dram_para1 >> i) & 0xf) {
		case 8:  val |= 0xa00; break;
		case 4:  val |= 0x900; break;
		case 2:  val |= 0x800; break;
		case 1:  val |= 0x700; break;
		default: val |= 0x600; break;
		}
		writel(ptr, val);
		ptr += 4;
	}

	// set ODTMAP based on number of ranks in use
	val = (readl(0x3102000) & 0x1) ? 0x303 : 0x201;
	writel(0x3103120, val);

	// set mctl reg 3c4 to zero when using half DQ
	if (para->dram_para2 & (1 << 0)) {
		writel(0x31033c4, 0);
	}

	// purpose ??
	if (para->dram_tpr4) {
		val  = readl(0x3102000);
		val |= (para->dram_tpr4 << 25) & 0x06000000;
		writel(0x3102000, val);

		val  = readl(0x3102004);
		val |= ((para->dram_tpr4 >> 2) << 12) & 0x001ff000;
		writel(0x3102004, val);
	}
}

// This routine seems to have several remapping tables for 22 lines.
// It is unclear which lines are being remapped. It seems to pick
// table cfg7 for the Nezha board.
//
void mctl_phy_ac_remapping(dram_para_t *para)
{
	char cfg0[] = {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 };
	static char cfg1[] = {  1,  9,  3,  7,  8, 18,  4, 13,  5,  6, 10,
			 2, 14, 12,  0,  0, 21, 17, 20, 19, 11, 22 };
	static char cfg2[] = {  4,  9,  3,  7,  8, 18,  1, 13,  2,  6, 10,
			 5, 14, 12,  0,  0, 21, 17, 20, 19, 11, 22 };
	static char cfg3[] = {  1,  7,  8, 12, 10, 18,  4, 13,  5,  6,  3,
			 2,  9,  0,  0,  0, 21, 17, 20, 19, 11, 22 };
	static char cfg4[] = {  4, 12, 10,  7,  8, 18,  1, 13,  2,  6,  3,
			 5,  9,  0,  0,  0, 21, 17, 20, 19, 11, 22 };
	static char cfg5[] = { 13,  2,  7,  9, 12, 19,  5,  1,  6,  3,  4,
			 8, 10,  0,  0,  0, 21, 22, 18, 17, 11, 20 };
	static char cfg6[] = {  3, 10,  7, 13,  9, 11,  1,  2,  4,  6,  8,
			 5, 12,  0,  0,  0, 20,  1,  0, 21, 22, 17 };
	static char cfg7[] = {  3,  2,  4,  7,  9,  1, 17, 12, 18, 14, 13,
			 8, 15,  6, 10,  5, 19, 22, 16, 21, 20, 11 };

	unsigned int fuse, val;

	// read SID info @ 0x228
	fuse = (readl(0x3002228) >> 8) & 0x4;
	printf("ddr_efuse_type: 0x%x\n", fuse);

	if ((para->dram_tpr13 >> 18) & 0x3) {
		memcpy_self(cfg0, cfg7, 22);
	}
	else {
		switch (fuse) {
		case  8: memcpy_self(cfg0, cfg2, 22); break;
		case  9: memcpy_self(cfg0, cfg3, 22); break;
		case 10: memcpy_self(cfg0, cfg5, 22); break;
		case 11: memcpy_self(cfg0, cfg4, 22); break;
		default:
		case 12: memcpy_self(cfg0, cfg1, 22); break;
		case 13:
		case 14: break;
		}
	}
	if (para->dram_type == 2) {
		if (fuse == 15) return;
		memcpy_self(cfg0, cfg6, 22);
	}
	if ( para->dram_type == 2 || para->dram_type == 3) {

		val = (cfg0[ 4] << 25) | (cfg0[ 3] << 20) | (cfg0[ 2] << 15) |
		      (cfg0[ 1] << 10) | (cfg0[ 0] <<  5);
		writel(0x3102500, val);

		val = (cfg0[10] << 25) | (cfg0[ 9] << 20) | (cfg0[ 8] << 15) |
		      (cfg0[ 7] << 10) | (cfg0[ 6] <<  5) |  cfg0[ 5];
		writel(0x3102504, val);

		val =                    (cfg0[15] << 20) | (cfg0[14] << 15) |
		      (cfg0[13] << 10) | (cfg0[12] <<  5) |  cfg0[11];
		writel(0x3102508, val);

		val = (cfg0[21] << 25) | (cfg0[20] << 20) | (cfg0[19] << 15) |
		      (cfg0[18] << 10) | (cfg0[17] <<  5) |  cfg0[16];
		writel(0x310250c, val);

		val = (cfg0[ 4] << 25) | (cfg0 [3] << 20) | (cfg0[2] << 15) |
		      (cfg0[ 1] << 10) | (cfg0 [0] <<  5) |  1;
		writel(0x3102500, val);
	}
}

// Init the controller channel. The key part is placing commands in the main
// command register (PIR, 0x3103000) and checking command status (PGSR0, 0x3103010).
//
unsigned int mctl_channel_init(unsigned int ch_index, dram_para_t *para)
{
	unsigned int val, dqs_gating_mode;

	dqs_gating_mode = (para->dram_tpr13 >> 2) & 0x3;

	// set DDR clock to half of CPU clock
	val  = readl(0x310200c) & 0xfffff000;
	val |= (para->dram_clk >> 1 ) - 1;
	writel(0x310200c, val);

	// MRCTRL0 nibble 3 undocumented
	val  = readl(0x3103108) & 0xfffff0ff;
	val |= 0x300;
	writel(0x3103108, val);

	// DX0GCR0
	val = readl(0x3103344) & 0xffffffcf;
	val |= ((~para->dram_odt_en) << 5) & 0x20;
	if (para->dram_clk > 672) {
		val &= 0xffff09f1;
		val |= 0x00000400;
	}
	else {
		val &= 0xffff0ff1;
	}
	writel(0x3103344, val);

	// DX1GCR0
	val  = readl(0x31033c4) & 0xffffffcf;
	val |= ((~para->dram_odt_en) << 5) & 0x20;
	if (para->dram_clk > 672) {
		val &= 0xffff09f1;
		val |= 0x00000400;
	}
	else {
		val &= 0xffff0ff1;
	}
	writel(0x31033c4, val);

	// 0x3103208 undocumented
	val  = readl(0x3103208);
	val |= 0x2;
	writel(0x3103208, val);

	eye_delay_compensation(para);

	//set PLL SSCG ?
	//
	val  = readl(0x3103108);
	if (dqs_gating_mode == 1) {

		val &= ~(0xc0);
		writel(0x3103108, val);

		val  = readl(0x31030bc);
		val &= 0xfffffef8;
		writel(0x31030bc, val);
	}
	else if (dqs_gating_mode == 2) {

		val &= ~(0xc0);
		val |= 0x80;
		writel(0x3103108, val);

		val  = readl(0x31030bc);
		val &= 0xfffffef8;
		val |= ((para->dram_tpr13 >> 16) & 0x1f) - 2;
		val |= 0x100;
		writel(0x31030bc, val);

		val  = readl(0x310311c) & 0x7fffffff;
		val |= 0x08000000;
		writel(0x310311c, val);
	}
	else {
		val &= ~(0x40);
		writel(0x3103108, val);

		sdelay(10);

		val  = readl(0x3103108);
		val |= 0xc0;
		writel(0x3103108, val);
	}

	if (para->dram_type == 6 || para->dram_type == 7) {
		val = readl(0x310311c);
		if (dqs_gating_mode == 1) {
			val &= 0xf7ffff3f;
			val |= 0x80000000;
		}
		else {
			val &= 0x88ffffff;
			val |= 0x22000000;
		}
		writel(0x310311c, val);
	}

	val  = readl(0x31030c0);
	val &= 0xf0000000;
	val |= (para->dram_para2 & (1 << 12)) ? 0x03000001 : 0x01000007; // 0x01003087 XXX
	writel(0x31030c0, val);

	if (readl(0x70005d4) & (1 << 16)) {
		val = readl(0x7010250);
		val &= 0xfffffffd;
		writel(0x7010250, val);

		sdelay(10);
	}

	// Set ZQ config
	val  = readl(0x3103140) & 0xfc000000;
	val |= para->dram_zq & 0x00ffffff;
	val |= 0x02000000;
	writel(0x3103140, val);

	// Initialise DRAM controller
	if (dqs_gating_mode == 1) {
		writel(0x3103000, 0x52); // prep PHY reset + PLL init + z-cal
		writel(0x3103000, 0x53); // Go

		while ((readl(0x3103010) & 0x1) == 0) {;} // wait for IDONE
		sdelay(10);

		// 0x520 = prep DQS gating + DRAM init + d-cal
		val = (para->dram_type == 3) ? 0x5a0	// + DRAM reset
					     : 0x520;
	}
	else {
		if ((readl(0x70005d4) & (1 << 16)) == 0) {
			// prep DRAM init + PHY reset + d-cal + PLL init + z-cal
			val = (para->dram_type == 3) ? 0x1f2	// + DRAM reset
						     : 0x172;
		}
		else {
			// prep PHY reset + d-cal + z-cal
			val = 0x62;
		}
	}

	writel(0x3103000, val); // Prep
	val |= 1;
	writel(0x3103000, val); // Go

	sdelay(10);
	while ((readl(0x3103010) & 0x1) == 0) {;} // wait for IDONE

	if (readl(0x70005d4) & (1 << 16)) {

		val  = readl(0x310310c);
		val &= 0xf9ffffff;
		val |= 0x04000000;
		writel(0x310310c, val);

		sdelay(10);

		val  = readl(0x3103004);
		val |= 0x1;
		writel(0x3103004, val);

		while ((readl(0x3103018) & 0x7) != 0x3) {;}

		val  = readl(0x7010250);
		val &= 0xfffffffe;
		writel(0x7010250, val);

		sdelay(10);

		val  = readl(0x3103004);
		val &= 0xfffffffe;
		writel(0x3103004, val);

		while ((readl(0x3103018) & 0x7) != 0x1) {;}

		sdelay(15);

		if (dqs_gating_mode == 1) {

			val  = readl(0x3103108);
			val &= 0xffffff3f;
			writel(0x3103108, val);

			val  = readl(0x310310c);
			val &= 0xf9ffffff;
			val |= 0x02000000;
			writel(0x310310c, val);

			sdelay(1);
			writel(0x3103000, 0x401);

			while ((readl(0x3103010) & 0x1) == 0) {;}
		}
	}

	// Check for training error
	val = readl(0x3103010);
	if (((val >> 20) & 0xff) && (val & 0x100000)) {
		printf("ZQ calibration error, check external 240 ohm resistor.\n");
		return 0;
	}

	// STATR = Zynq STAT? Wait for status 'normal'?
	while ((readl(0x3103018) & 0x1) == 0) {;}

	val  = readl(0x310308c);
	val |= 0x80000000;
	writel(0x310308c, val);

	sdelay(10);

	val  = readl(0x310308c);
	val &= 0x7fffffff;
	writel(0x310308c, val);

	sdelay(10);

	val  = readl(0x3102014);
	val |= 0x80000000;
	writel(0x3102014, val);

	sdelay(10);

	val  = readl(0x310310c);
	val &= 0xf9ffffff;
	writel(0x310310c, val);

	if (dqs_gating_mode == 1) {
		val  = readl(0x310311c);
		val &= 0xffffff3f;
		val |= 0x00000040;
		writel(0x310311c, val);
	}
	return 1;
}

// The below routine reads the dram config registers and extracts
// the number of address bits in each rank available. It then calculates
// total memory size in MB.
//
int DRAMC_get_dram_size(void)
{
	unsigned int rval, temp, size0, size1;

	rval = readl(0x3102000);	// MC_WORK_MODE0

	temp  = (rval>>8) & 0xf;	// page size - 3
	temp += (rval>>4) & 0xf;	// row width - 1
	temp += (rval>>2) & 0x3;	// bank count - 2
	temp -= 14;			// 1MB = 20 bits, minus above 6 = 14
	size0 = 1 << temp;

	temp = rval & 0x3;		// rank count = 0? -> done
	if (temp == 0) {
		return size0;
	}

	rval = readl(0x3102004);	// MC_WORK_MODE1

	temp = rval & 0x3;
	if (temp == 0) {		// two identical ranks
		return 2 * size0;
	}

	temp  = (rval>>8) & 0xf;	// page size - 3
	temp += (rval>>4) & 0xf;	// row width - 1
	temp += (rval>>2) & 0x3;	// bank number - 2
	temp -= 14;			// 1MB = 20 bits, minus above 6 = 14
	size1 = 1 << temp;

	return size0 + size1;		// add size of each rank
}

// The below routine reads the command status register to extract
// DQ width and rank count. This follows the DQS training command in
// channel_init. If error bit 22 is reset, we have two ranks and full DQ.
// If there was an error, figure out whether it was half DQ, single rank,
// or both. Set bit 12 and 0 in dram_para2 with the results.
//
int dqs_gate_detect(dram_para_t *para)
{
	unsigned int rval, dx0, dx1;

	if (readl(0x3103010) & (1 << 22)) {

		dx0 = (readl(0x3103348) >> 24) & 0x3;
		dx1 = (readl(0x31033c8) >> 24) & 0x3;

		if (dx0 == 2) {
			rval  = para->dram_para2;
			rval &= 0xffff0ff0;
			if (dx0 != dx1) {
				rval |= 0x1;
				para->dram_para2 = rval;
				printf("[AUTO DEBUG] single rank and half DQ!\n");
				return 1;
			}
			para->dram_para2 = rval;
			printf("[AUTO DEBUG] single rank and full DQ!\n");
			return 1;
		}
		else if (dx0 == 0) {
			rval  = para->dram_para2;
			rval &= 0xfffffff0;
			rval |= 0x00001001;
			para->dram_para2 = rval;
			printf("[AUTO DEBUG] dual rank and half DQ!\n");
			return 1;
		}
		else {
			if (para->dram_tpr13 & (1 << 29)) {
				printf("DX0 state:%d\n", dx0);
				printf("DX1 state:%d\n", dx1);
			}
			return 0;
		}
	}
	else {
		rval  = para->dram_para2;
		rval &= 0xfffffff0;
		rval |= 0x00001000;
		para->dram_para2 = rval;
		printf("[AUTO DEBUG] two rank and full DQ!\n");
		return 1;
	}
}


#define SDRAM_BASE	((unsigned int *)0x40000000)
#define uint		unsigned int

int dramc_simple_wr_test(uint mem_mb, int len)
{
	unsigned int  offs = (mem_mb >> 1) << 18; // half of memory size
	unsigned int  patt1 = 0x01234567;
	unsigned int  patt2 = 0xfedcba98;
	unsigned int *addr, v1, v2, i;

	addr = SDRAM_BASE;
	for (i = 0; i != len; i++, addr++) {
		writel(addr,        patt1 + i);
		writel(addr + offs, patt2 + i);
	}

	addr = SDRAM_BASE;
	for (i = 0; i != len; i++) {
		v1 = readl(addr+i);
		v2 = patt1 + i;
		if (v1 != v2) {
			printf("DRAM simple test FAIL.\n");
			printf("%x != %x at address %x\n", v1, v2, addr+i);
			return 1;
		}
		v1 = readl(addr+offs+i);
		v2 = patt2 + i;
		if (v1 != v2) {
			printf("DRAM simple test FAIL.\n");
			printf("%x != %x at address %x\n", v1, v2, addr+offs+i);
			return 1;
		}
	}
	printf("DRAM simple test OK.\n");
	return 0;
}

// Set the Vref mode for the controller
//
void mctl_vrefzq_init(dram_para_t *para)
{
	unsigned int val;

	if ((para->dram_tpr13 & (1 << 17)) == 0) {
		val  = readl(0x3103110) & 0x80808080;	// IOCVR0
		val |= para->dram_tpr5;
		writel(0x3103110, val);

		if ((para->dram_tpr13 & (1 << 16)) == 0) {
			val  = readl(0x3103114) & 0xffffff80; // IOCVR1
			val |= para->dram_tpr6 & 0x7f;
			writel(0x3103114, val);
		}
	}
}

// Perform an init of the controller. This is actually done 3 times. The first
// time to establish the number of ranks and DQ width. The second time to
// establish the actual ram size. The third time is final one, with the final
// settings.
//
int mctl_core_init(dram_para_t *para)
{
	mctl_sys_init(para);
	mctl_vrefzq_init(para);
	mctl_com_init(para);
	mctl_phy_ac_remapping(para);
	auto_set_timing_para(para);
	return mctl_channel_init(0, para);
}


#define RAM_BASE	(0x40000000)

// Autoscan sizes a dram device by cycling through address lines and figuring
// out if it is connected to a real address line, or if the address is a mirror.
// First the column and bank bit allocations are set to low values (2 and 9 address
// lines. Then a maximum allocation (16 lines) is set for rows and this is tested.
// Next the BA2 line is checked. This seems to be placed above the column, BA0-1 and
// row addresses. Finally, the column address is allocated 13 lines and these are
// tested. The results are placed in dram_para1 and dram_para2.
//
int auto_scan_dram_size(dram_para_t *para) // s7
{
	unsigned int	rval, i, j, rank, maxrank, offs, mc_work_mode;
	unsigned int	chk, ptr, shft, banks;

	if (mctl_core_init(para) == 0) {
		printf("[ERROR DEBUG] DRAM initialisation error : 0!\n");
		return 0;
	}

	maxrank = (para->dram_para2 & 0xf000) ? 2 : 1;
	mc_work_mode = 0x3102000;
	offs = 0;

	// write test pattern
	for (i = 0, ptr = RAM_BASE; i < 64; i++, ptr += 4) {
		writel(ptr, (i & 1) ? ptr : ~ptr);
	}

	for (rank = 0; rank < maxrank; ) {

		// Set row mode
		rval  = readl(mc_work_mode);
		rval &= 0xfffff0f3;
		rval |= 0x000006f0;
		writel(mc_work_mode, rval);
		while (readl(mc_work_mode) != rval);

		// Scan per address line, until address wraps (i.e. see shadow)
		for(i = 11; i < 17; i++) {
			chk = RAM_BASE + (1 << (i + 11));
			ptr = RAM_BASE;
			for (j = 0; j < 64; j++) {
				if (readl(chk) != ((j & 1) ? ptr : ~ptr))
					goto out1;
				ptr += 4;
				chk += 4;
			}
			break;
		out1: ;
		}
		if (i > 16) i = 16;
		printf("[AUTO DEBUG] rank %d row = %d\n", rank, i);

		// Store rows in para 1
		shft = 4 + offs;
		rval = para->dram_para1;
		rval &= ~(0xff << shft);
		rval |= i << shft;
		para->dram_para1 = rval;

		if (rank == 1) {
			// Set bank mode for rank0
			rval  = readl(0x3102000);
			rval &= 0xfffff003;
			rval |= 0x000006a4;
			writel(0x3102000, rval);
		}

		// Set bank mode for current rank
		rval  = readl(mc_work_mode);
		rval &= 0xfffff003;
		rval |= 0x000006a4;
		writel(mc_work_mode, rval);
		while (readl(mc_work_mode) != rval);

		// Test if bit A23 is BA2 or mirror XXX A22?
		chk = RAM_BASE + (1 << 22);
		ptr = RAM_BASE;
		for (i = 0, j = 0; i < 64; i++) {
			if (readl(chk) != ((i & 1) ? ptr : ~ptr)) {
				j = 1;
				break;
			}
			ptr += 4;
			chk += 4;
		}
		banks = (j + 1) << 2; // 4 or 8
		printf("[AUTO DEBUG] rank %d bank = %d\n", rank, banks);

		// Store banks in para 1
		shft = 12 + offs;
		rval  = para->dram_para1;
		rval &= ~(0xf << shft);
		rval |= j << shft;
		para->dram_para1 = rval;

		if (rank == 1) {
			// Set page mode for rank0
			rval  = readl(0x3102000);
			rval &= 0xfffff003;
			rval |= 0x00000aa0;
			writel(0x3102000, rval);
		}

		// Set page mode for current rank
		rval  = readl(mc_work_mode);
		rval &= 0xfffff003;
		rval |= 0x00000aa0;
		writel(mc_work_mode, rval);
		while (readl(mc_work_mode) != rval);

		// Scan per address line, until address wraps (i.e. see shadow)
		for(i = 9; i < 14; i++) {
			chk = RAM_BASE + (1 << i);
			ptr = RAM_BASE;
			for (j = 0; j < 64; j++) {
				if (readl(chk) != ((j & 1) ? ptr : ~ptr))
					goto out2;
				ptr += 4;
				chk += 4;
			}
			break;
		out2:;
		}
		if (i > 13) i = 13;
		int pgsize = (i==9) ? 0 : (1 << (i-10));
		printf("[AUTO DEBUG] rank %d page size = %d KB\n", rank, pgsize);

		// Store page size
		shft = offs;
		rval  = para->dram_para1;
		rval &= ~(0xf << shft);
		rval |= pgsize << shft;
		para->dram_para1 = rval;

		// Move to next rank
		rank++;
		if (rank != maxrank) {
			if (rank == 1) {
				rval  = readl(0x3202000); // MC_WORK_MODE
				rval &= 0xfffff003;
				rval |= 0x000006f0;
				writel(0x3202000, rval);

				rval  = readl(0x3202004); // MC_WORK_MODE2
				rval &= 0xfffff003;
				rval |= 0x000006f0;
				writel(0x3202004, rval);
			}
			offs += 16; // store rank1 config in upper half of para1
			mc_work_mode +=  4; // move to MC_WORK_MODE2
		}
	}
	if (maxrank == 2) {
		para->dram_para2 &= 0xfffff0ff;
		// note: rval is equal to para->dram_para1 here
		if ((rval & 0xffff) == ((rval >> 16) & 0xffff)) {
			printf("rank1 config same as rank0\n");
		}
		else {
			para->dram_para2 |= 0x00000100;
			printf("rank1 config different from rank0\n");
		}
	}
	return 1;
}

// This routine sets up parameters with dqs_gating_mode equal to 1 and two
// ranks enabled. It then configures the core and tests for 1 or 2 ranks and
// full or half DQ width. it then resets the parameters to the original values.
// dram_para2 is updated with the rank & width findings.
//
int auto_scan_dram_rank_width(dram_para_t *para)
{
	unsigned int s1 = para->dram_tpr13;
	unsigned int s2 = para->dram_para1;
	unsigned int v;

	para->dram_para1 = 0x00b000b0;
	v = (para->dram_para2 & 0xfffffff0) | 0x1000;
	para->dram_para2 = v;

	v = (s1 & 0xfffffff7) | 0x5; // set DQS probe mode
	para->dram_tpr13 = v;

	mctl_core_init(para);
	if (readl(0x3103010) & (1 << 20)) {
		return 0;
	}
	if (dqs_gate_detect(para) == 0) {
		return 0;
	}

	para->dram_tpr13 = s1;
	para->dram_para1 = s2;
	return 1;
}

// This routine determines the sdram topology. It first establishes the number
// of ranks and the DQ width. Then it scans the sdram address lines to establish
// the size of each rank. It then updates dram_tpr13 to reflect that the sizes
// are now known: a re-init will not repeat the autoscan.
//
int auto_scan_dram_config(dram_para_t *para)
{
	if (((para->dram_tpr13 & (1 << 14)) == 0) &&
	    (auto_scan_dram_rank_width(para) == 0))
	{
		printf("[ERROR DEBUG] auto scan dram rank & width failed !\n");
		return 0;
	}
	if (((para->dram_tpr13 & (1 << 0)) == 0)  &&
	    (auto_scan_dram_size(para) == 0 ))
	{
		printf("[ERROR DEBUG] auto scan dram size failed !\n");
		return 0;
	}
	if ((para->dram_tpr13 & (1 << 15)) == 0) {
		para->dram_tpr13 |= 0x6003;
	}
	return 1;
}


signed int init_DRAM(int type, dram_para_t *para) // s0
{
	int rc, mem_size;

	// Test ZQ status
	if (para->dram_tpr13 & (1 << 16)) {
		printf("DRAM only have internal ZQ!!\n");
		writel(0x3000160, readl(0x3000160) |  0x100);
		writel(0x3000168, 0);
		sdelay(10);
	}
	else {
		writel(0x7010254, 0);
		writel(0x3000160, readl(0x3000160) & ~0x003);
		sdelay(10);
		writel(0x3000160, readl(0x3000160) & ~0x108);
		sdelay(10);
		writel(0x3000160, readl(0x3000160) |  0x001);
		sdelay(20);
		printf("ZQ value = 0x%x***********\n", readl(0x3000172));
	}

	// Set voltage
	rc = get_pmu_exists();
	printf("get_pmu_exist() = %d\n", rc);
	if ( rc<0 ) {
		dram_vol_set(para);
	}
	else {
		if (para->dram_type==2) set_ddr_voltage(1800); else
		if (para->dram_type==3) set_ddr_voltage(1500);
	}

	// Set SDRAM controller auto config
	if ( (para->dram_tpr13 & 0x1)==0 ) {
		if ( auto_scan_dram_config(para)==0 ) {
			return 0;
		}
	}

	// Print header message (too late)
	printf("DRAM BOOT DRIVE INFO: %s\n", "V0.24");
	printf("DRAM CLK = %d MHz\n", para->dram_clk);
	printf("DRAM Type = %d (2:DDR2,3:DDR3)\n", para->dram_type);
	if ( (para->dram_odt_en & 0x1) == 0 ) {
		printf("DRAMC read ODT  off.\n");
	}
	else {
		printf("DRAMC ZQ value: 0x%x\n", para->dram_zq);
	}

	// report ODT
	rc = para->dram_mr1;
	if ( (rc & 0x44)==0 ) {
		printf("DRAM ODT off.\n");
	}
	else {
		printf("DRAM ODT value: 0x%x.\n", rc);
	}

	// Init core, final run
	if ( mctl_core_init(para)==0 ) {
		printf("DRAM initialisation error : 1 !\n");
		return 0;
	}

	// Get sdram size
	rc = para->dram_para2;
	if ( rc<0 ) {
		rc = (rc & 0x7fff0000U) >> 16;
	}
	else {
		rc = DRAMC_get_dram_size();
		printf("DRAM SIZE =%d M\n", rc);
		para->dram_para2 = (para->dram_para2 & 0xffffu) | rc << 16;
	}
	mem_size = rc;

	// Purpose ??
	if ( para->dram_tpr13 & (1 << 30) ) {
		rc = readl(&para->dram_tpr8);
		if ( rc==0 ) {
			rc = 0x10000200;
		}
		writel(0x31030a0, rc);
		writel(0x310309c, 0x40a);
		writel(0x3103004, readl(0x3103004) | 1 );
		printf("Enable Auto SR");
	}
	else {
		writel(0x31030a0, readl(0x31030a0) & 0xffff0000);
		writel(0x3103004, readl(0x3103004) & (~0x1) );
	}

	// Pupose ??
	rc = readl(0x3103100) & ~(0xf000);
	if ( (para->dram_tpr13 & 0x200)==0 ) {
		if ( para->dram_type != 6 ) {
			writel(0x3103100, rc);
		}
	}
	else {
		writel(0x3103100, rc | 0x5000);
	}

	writel(0x3103140, readl(0x3103140) | (1 << 31));
	if (para->dram_tpr13 & (1 << 8)) {
		writel(0x31030b8, readl(0x3103140) | 0x300);
	}

	rc = readl(0x3103108);
	if (para->dram_tpr13 & (1 << 16)) {
		rc &= 0xffffdfff;
	}
	else {
		rc |= 0x00002000;
	}
	writel(0x3103108, rc);


	// Purpose ??
	if (para->dram_type == 7) {
		rc  = readl(0x310307c) & 0xfff0ffff;
		rc |= 0x0001000;
		writel(0x310307c, rc);
	}

	dram_enable_all_master();
	//if (dramc_simple_wr_test(mem_size, 64)) return 0;
	if (para->dram_tpr13 & (1 << 28)) {
		rc = readl(0x70005d4);
		if ( (rc & (1 << 16)) || dramc_simple_wr_test(mem_size, 4096) ) {
			return 0;
		}
	}

	handler_super_standby();
	return mem_size;
}

// Apparently only used for FPGA testing
//
unsigned int mctl_init(void)
{
	signed int ret_val = 0;

	dram_para_t dram_para;

	// Build parameter block
	dram_para.dram_clk	= 528;
	dram_para.dram_type	= 2;
	dram_para.dram_zq	= 0x7b7bf9;
	dram_para.dram_odt_en	= 0;
	dram_para.dram_para1	= 210;
	dram_para.dram_para2	= 0;
	dram_para.dram_mr0	= 0x0e73;
	dram_para.dram_mr1	= 0x2;
	dram_para.dram_mr2	= 0;
	dram_para.dram_mr3	= 0;
	dram_para.dram_tpr0	= 0x00471992;
	dram_para.dram_tpr1	= 0x0131a10c;
	dram_para.dram_tpr2	= 0x80000000;
	dram_para.dram_tpr3	= 0xffffffff;
	dram_para.dram_tpr4	= 0;
	dram_para.dram_tpr5	= 0x48484848;
	dram_para.dram_tpr6	= 0x48;
	dram_para.dram_tpr7	= 0x1621121e;
	dram_para.dram_tpr8	= 0;
	dram_para.dram_tpr9	= 0;
	dram_para.dram_tpr10	= 0;
	dram_para.dram_tpr11	= 0x40000000;
	dram_para.dram_tpr12	= 0x45;
	dram_para.dram_tpr13	= 0x34000000;

	ret_val = init_DRAM(0, &dram_para);
	return ret_val;
}
/*
                0x318,
                0x3,
                0x7b7bfb,
                0x0,
                0x10d2,
                0x0,
                0x1c70,
                0x42,
                0x18,
                0x0,
                0x4a2195,
                0x2423190,
                0x8b061,
                0xb4787896,
                0x0,
                0x48484848,
                0x48,
                0x1620121e,
                0x0,
                0x0,
                0x0,
                0x340000,
                0x46,
                0x34000100,
                0x0,
                0x0,
                0x0,
                0x0,
                0x0,
                0x0,
                0x0,
                0x0,
*/
