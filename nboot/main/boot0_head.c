/*
 * (C) Copyright 2018
* SPDX-License-Identifier:	GPL-2.0+
 * wangwei <wangwei@allwinnertech.com>
 */

#include <config.h>
#include <private_boot0.h>
#include <commit_info.h>

#ifdef CFG_ARCH_RISCV
#define BROM_FILE_HEAD_SIZE		(sizeof(boot0_file_head_t) & 0x00FFFFF)
#define BROM_FILE_HEAD_BIT_10_1		((BROM_FILE_HEAD_SIZE & 0x7FE) >> 1)
#define BROM_FILE_HEAD_BIT_11		((BROM_FILE_HEAD_SIZE & 0x800) >> 11)
#define BROM_FILE_HEAD_BIT_19_12	((BROM_FILE_HEAD_SIZE & 0xFF000) >> 12)
#define BROM_FILE_HEAD_BIT_20		((BROM_FILE_HEAD_SIZE & 0x100000) >> 20)

#define BROM_FILE_HEAD_SIZE_OFFSET	((BROM_FILE_HEAD_BIT_20 << 31) | \
									(BROM_FILE_HEAD_BIT_10_1 << 21) | \
									(BROM_FILE_HEAD_BIT_11 << 20) | \
									(BROM_FILE_HEAD_BIT_19_12 << 12))
#define JUMP_INSTRUCTION		(BROM_FILE_HEAD_SIZE_OFFSET | 0x6f)
#else
#define BROM_FILE_HEAD_SIZE_OFFSET	(((sizeof(boot0_file_head_t) + sizeof(int) - 1) / sizeof(int) - 2))
#define JUMP_INSTRUCTION		(BROM_FILE_HEAD_SIZE_OFFSET | 0xEA000000)
#endif

const boot0_file_head_t  BT0_head = {
	{
		/* jump_instruction*/
		JUMP_INSTRUCTION,
		BOOT0_MAGIC,
		STAMP_VALUE,
#ifdef ALIGN_SIZE_8K
		0x2000,
#else
		0x4000,
#endif
		sizeof(boot_file_head_t),
		BOOT_PUB_HEAD_VERSION,
		CONFIG_BOOT0_RET_ADDR,
		CONFIG_BOOT0_RUN_ADDR,
		0,
		{
		/*brom modify: nand-4bytes sdmmc-2bytes*/
		0, 0, 0, 0, '4', '.', '0', 0
		},
	},

	{
		/*__u32 prvt_head_size;*/
		0,
		/*char prvt_head_vsn[4];*/
		1,
		0,/* power_mode */
		{0},/* reserver[2]  */
		/*unsigned int     dram_para[32] ;*/
		{
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
                0x0
        },
		/*__s32	uart_port;*/
		0,
		/*normal_gpio_cfg   uart_ctrl[2];*/
		{
		{5, 2, 6, 1, 1, 0, {0} },/*PB8: 4--RX*/
		{5, 3, 6, 1, 1, 0, {0} },/*PB9: 4--TX*/
		},
		/*__s32 enable_jtag;*/
		0,
		/*normal_gpio_cfg	 jtag_gpio[5];*/
		{{0}, {0}, {0}, {0}, {0} },
		/*normal_gpio_cfg   storage_gpio[32];*/
		{
			{0, 0, 2, 1, 2, 0, {0} },/*PF0-5: 2--SDC*/
			{0, 1, 2, 1, 2, 0, {0} },
			{0, 2, 2, 1, 2, 0, {0} },
			{0, 3, 2, 1, 2, 0, {0} },
			{0, 4, 2, 1, 2, 0, {0} },
			{0, 5, 2, 1, 2, 0, {0} },
		},
		/*char  storage_data[512 - sizeof(normal_gpio_cfg) * 32];*/
		{0}
	},

	{CI_INFO},
#ifdef CFG_SUNXI_SELECT_DRAM_PARA
	.fes_union_addr.extd_head.magic = DRAM_EXT_MAGIC,
#endif
};



/*******************************************************************************
*
*                  关于Boot_file_head中的jump_instruction字段
*
*  jump_instruction字段存放的是一条跳转指令：( B  BACK_OF_Boot_file_head )，此跳
*转指令被执行后，程序将跳转到Boot_file_head后面第一条指令。
*
*  ARM指令中的B指令编码如下：
*          +--------+---------+------------------------------+
*          | 31--28 | 27--24  |            23--0             |
*          +--------+---------+------------------------------+
*          |  cond  | 1 0 1 0 |        signed_immed_24       |
*          +--------+---------+------------------------------+
*  《ARM Architecture Reference Manual》对于此指令有如下解释：
*  Syntax :
*  B{<cond>}  <target_address>
*    <cond>    Is the condition under which the instruction is executed. If the
*              <cond> is ommitted, the AL(always,its code is 0b1110 )is used.
*    <target_address>
*              Specified the address to branch to. The branch target address is
*              calculated by:
*              1.  Sign-extending the 24-bit signed(wro's complement)immediate
*                  to 32 bits.
*              2.  Shifting the result left two bits.
*              3.  Adding to the contents of the PC, which contains the address
*                  of the branch instruction plus 8.
*
*  由此可知，此指令编码的最高8位为：0b11101010，低24位根据Boot_file_head的大小动
*态生成，所以指令的组装过程如下：
*  ( sizeof( boot_file_head_t ) + sizeof( int ) - 1 ) / sizeof( int )
*                                              求出文件头占用的“字”的个数
*  - 2                                         减去PC预取的指令条数
*  & 0x00FFFFFF                                求出signed-immed-24
*  | 0xEA000000                                组装成B指令
*
*******************************************************************************/

