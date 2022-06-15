#ifndef __public_t113_s3_u_boot_nand_spl_include_config_h_
#define __public_t113_s3_u_boot_nand_spl_include_config_h_
#include<sun20iw1p1.h>
#define CFG_ARCH_ARM 1
#define CFG_BOOT0_RUN_ADDR 0x20000
#define CFG_FES1_RUN_ADDR 0x28000
#define CFG_SBOOT_RUN_ADDR 0x20480
#define CFG_SUNXI_CHIPID 1
#define CFG_SUNXI_DMA 1
#define CFG_SUNXI_GPIO_V2 1
#define CFG_SUNXI_GUNZIP 1
#define CFG_SUNXI_LZ4 1
#define CFG_SUNXI_LZMA 1
#define CFG_SUNXI_MEMOP 1
#define CFG_SUNXI_NAND 1
#define CFG_SUNXI_POWER 1
#define CFG_SUNXI_SPINAND 1
#define CFG_SYS_INIT_RAM_SIZE 0x18000
#define CFG_USE_DCACHE 1
#define PLATFORM_LIBGCC -L /data/allwinner/tina-d1-open/lichee/brandy-2.0/tools/toolchain/gcc-linaro-7.2.1-2017.11-x86_64_arm-linux-gnueabi/bin/../lib/gcc/arm-linux-gnueabi/7.2.1 -lgcc
#endif
