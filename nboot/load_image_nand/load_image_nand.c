/*
 * (C) Copyright 2018-2020
* SPDX-License-Identifier:	GPL-2.0+
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * wangwei <wangwei@allwinnertech.com>
 *
 */

#include <common.h>
#include <spare_head.h>
#include <nand_boot0.h>
#include <private_toc.h>
#include <private_boot0.h>
#include <private_uboot.h>

void update_flash_para(phys_addr_t uboot_base)
{

	struct spare_boot_head_t  *bfh = (struct spare_boot_head_t *) uboot_base;
/*	bfh->boot_data.storage_type = (BT0_head.boot_head.platform[0]&0xf) == 4 ? \
		STORAGE_SPI_NAND : STORAGE_NAND ; */
    bfh->boot_data.storage_type = STORAGE_SPI_NAND;
}


int load_toc1_from_rawnand( void )
{
	__u32 i;
	__s32  status;
	__u32 length;
	__u32 read_blks;
	sbrom_toc1_head_info_t  *toc1_head;
	char *buffer = (void*)CONFIG_BOOTPKG_BASE;

	if(NF_open( ) == NF_ERROR)
	{
		printf("fail in opening nand flash\n");
		return -1;
	}

	printf("block from %d to %d\n", BOOT1_START_BLK_NUM, BOOT1_LAST_BLK_NUM);
	for( i = BOOT1_START_BLK_NUM;  i <= BOOT1_LAST_BLK_NUM;  i++ )
	{
		if( NF_read_status( i ) == NF_BAD_BLOCK )
		{
			printf("nand block %d is bad\n", i);
			continue;
		}
		/*read head*/
		if( NF_read( i * ( NF_BLOCK_SIZE >> NF_SCT_SZ_WIDTH ), (void *)buffer, 1 )  == NF_OVERTIME_ERR )
		{
			printf("the first data is error\n");
			continue;
		}
		/* check magic */
		toc1_head = (sbrom_toc1_head_info_t *) buffer;
		if(toc1_head->magic != TOC_MAIN_INFO_MAGIC)
		{
			printf("%s err: the toc1 head magic is invalid\n", __func__);
			continue;
		}
		/* check align */
		length =  toc1_head->valid_len;
		if( ( length & ( ALIGN_SIZE - 1 ) ) != 0 )
		{
			printf("the boot1 is not aligned by 0x%x\n", ALIGN_SIZE);
			continue;
		}
		if( 1==load_uboot_in_one_block_judge(length) )
		{
			/* load toc1 in one blk */
			status = load_and_check_in_one_blk( i, (void *)buffer, length, NF_BLOCK_SIZE );
			if( status == ADV_NF_OVERTIME_ERR )
			{
				continue;
			}
			else if( status == ADV_NF_OK )
			{
				printf("Check is correct.\n");
				NF_close( );
				return 0;
			}
		}
		else
		{
			/* load toc in many blks */
			status = load_in_many_blks( i, BOOT1_LAST_BLK_NUM,
			                          (void*)buffer,length,
			                          NF_BLOCK_SIZE, &read_blks );
			if( status == ADV_NF_LACK_BLKS )
			{
				printf("ADV_NF_LACK_BLKS\n");
				NF_close( );
				return -1;
			}
			else if( status == ADV_NF_OVERTIME_ERR )
			{
				printf("mult block ADV_NF_OVERTIME_ERR\n");
				continue;
			}
			if( verify_addsum( (__u32 *)buffer, length ) == 0 )
			{
				printf("The file stored in start block %u is perfect.\n", i );
				NF_close( );
				return 0;
			}
		}
	}

	printf("Can't find a good Boot1 copy in nand.\n");
	NF_close( );
	return -1;
}

#ifdef CFG_SUNXI_SPINAND
#include <spinand_boot0.h>
#include <arch/watchdog.h>

static int do_fel(void)
{
    __u64 GP_data_reg = 0x07090000;
    __u32 cnt = 0x100000;

    do {
        writel(0x5aa5a55a, GP_data_reg + 0x100 + 0x8);
        if(!cnt--)break;
    } while(readl(GP_data_reg + 0x100 + 0x8) != 0x5aa5a55a);

    wdt_start(1);

    return 0;
}

__s32 load_toc1_from_spinand( void )
{
	__u32 i;
	__s32  status;
	__u32 length;
	__u32 read_blks;
//	sbrom_toc1_head_info_t  *toc1_head;
	char *buffer = (void*)CONFIG_BOOTPKG_BASE;

	if(SpiNand_PhyInit( ) != 0)
	{
		printf("fail in opening nand flash\n");
		return -1;
	}

//	printf("%s %d block from %d to %d\n", __FILE__, __LINE__, UBOOT_START_BLK_NUM, UBOOT_LAST_BLK_NUM);
//	printf("%s %d .......... %d to %d\n", __FILE__, __LINE__, UBOOT_START_BLK_NUM, UBOOT_LAST_BLK_NUM);
	for( i = UBOOT_START_BLK_NUM;  i <= UBOOT_LAST_BLK_NUM;  i++ )
	{
		if( SpiNand_Check_BadBlock( i ) == SPINAND_BAD_BLOCK )
		{
			printf("spi nand block %d is bad\n", i);
		    continue;
		}
#if 0
        if( SpiNand_Read( i * ( SPN_BLOCK_SIZE >> NF_SCT_SZ_WIDTH ), (void *)buffer, 1 )  == NAND_OP_FALSE )
		{
		    printf("the first data is error\n");
			continue;
		}

        toc1_head = (sbrom_toc1_head_info_t *) buffer;
        {int idx;for(idx=0;idx<512;idx++){printf("%x ",buffer[idx]);}printf("\n");}
		if(toc1_head->magic != TOC_MAIN_INFO_MAGIC)
		{
				printf("%s err:  magic is invalid\n", __func__);
				continue;
		}

		//check align
		length =  toc1_head->valid_len;
		if( ( length & ( ALIGN_SIZE - 1 ) ) != 0 )
		{
			printf("the boot1 is not aligned by 0x%x\n", ALIGN_SIZE);
			continue;
		}
#else        
	    buffer = (void*)SDRAM_OFFSET(0x3000000);
        length = 0x100000;

//	printf("%s %d ..........blk_no %d offset %x length %x\n", __FILE__, __LINE__,i, buffer, length);
		status = Spinand_Load_Boot1_Copy( i ++, (void*)buffer, length, SPN_BLOCK_SIZE, &read_blks );
		if( status == NAND_OP_FALSE )
		{
			printf("SPI nand load uboot copy fail\n");
			continue;
		}
#endif
		SpiNand_PhyExit( );
#if 0
        do_fel();
//        boot0_jmp(FEL_BASE);
#endif
        return 0;
#if 0
        if( verify_addsum( buffer, length ) == 0 )
		{
			printf("Check is correct.\n");
		    SpiNand_PhyExit( );
		    return 0;
		}
#endif        
	}

	printf("Can't find a good Boot1 copy in spi nand.\n");
	SpiNand_PhyExit( );
	return -1;
}
#else
__s32 load_toc1_from_spinand( void )
{
	return -1;
}

#endif

int BOOT_NandGetPara(void *param, uint size)
{
	memcpy( (void *)param, BT0_head.prvt_head.storage_data, size);
	return 0;
}

int load_package(void)
{

	return load_toc1_from_spinand();
#if 0
    if((BT0_head.boot_head.platform[0]&0xf) == 4)
		return load_toc1_from_spinand();
	else
		return load_toc1_from_rawnand();
#endif
}
