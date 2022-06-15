#include <common.h>
#include <private_boot0.h>
#include <private_uboot.h>
#include <private_toc.h>
#include <arch/clock.h>
#include <arch/uart.h>
#include <arch/dram.h>
#include <arch/rtc.h>
#include <arch/gpio.h>

void p_boot_file_head(const boot_file_head_t *p)
{
    printf("jump_instruction=%x\n",p->jump_instruction);
    printf("magic=%c%c%c%c%c%c%c%c\n",p->magic[0],p->magic[1],p->magic[2],p->magic[3],p->magic[4],p->magic[5],p->magic[6],p->magic[7]);
    printf("check_sum=%x\n",p->check_sum);
    printf("length=%x\n",p->length);
    printf("pub_head_size=%x\n",p->pub_head_size);
    printf("pub_head_vsn=%x\n",(__u64)&p->pub_head_vsn[0]);
    printf("ret_addr=%x\n",p->ret_addr);
    printf("run_addr=%x\n",p->run_addr);
    printf("run_addr=%x\n",p->boot_cpu);
    printf("platform0=%x\n",(__u64)&p->platform[0]);
    printf("platform1=%x\n",(__u64)&p->platform[4]);
}

void p_normal_gpio_cfg(const normal_gpio_cfg *p)
{
    printf("port=%x\n",p->port);            /* port : PA/PB/PC ... */
    printf("port_num=%x\n",p->port_num);        /* internal port num: PA00/PA01 ... */
    printf("mul_sel=%x\n",p->mul_sel);        /* function num: input/output/io-disalbe ...*/
    printf("pull=%x\n",p->pull);            /* pull-up/pull-down/no-pull */
    printf("drv_level=%x\n",p->drv_level);       /* driver level: level0-3*/
    printf("data=%x\n",p->data);            /* pin state when the port is configured as input or output*/
    printf("reserved[2]%x %x\n",p->reserved[0],p->reserved[1]);
}

void p_boot0_private_head(const boot0_private_head_t *p)
{
	int i;
    printf("prvt_head_size=%x\n",p->prvt_head_size);
    /*debug_mode = 0 : do not print any message,debug_mode = 1 ,print debug message*/
    printf("debug_mode=%x\n",p->debug_mode);
    /*0:axp, 1: no axp  */
    printf("power_mode=%x\n",p->power_mode);
    printf("reserve=%x %x\n",p->reserve[0],p->reserve[1]);
    /*DRAM patameters for initialising dram. Original values is arbitrary*/
	printf("dram_para:\n");
	for(i=0;i<32;i++)printf("\t\t%x\n", p->dram_para[i]);
	printf("\n");
    /*uart: num & uart pin*/
    printf("uart_port=%x\n",p->uart_port);
	
	p_normal_gpio_cfg(&p->uart_ctrl[0]);
	p_normal_gpio_cfg(&p->uart_ctrl[1]);
	
    /* jtag: 1 : enable,  0 : disable */
    printf("enable_jtag=%x\n",p->enable_jtag);
    printf("jtag_gpio[5]=%lx\n",(__u64)&p->jtag_gpio[0]);
    /* nand/mmc pin*/
    printf("storage_gpio[32]=%lx\n",(__u64)&p->storage_gpio[0]);
    /*reserve data*/
    printf("storage_data[512 - sizeof(normal_gpio_cfg) * 32]=%lx\n",(__u64)&p->storage_data[0]);
}

void p_head(const boot0_file_head_t *p)
{
    p_boot_file_head(&p->boot_head);
    p_boot0_private_head(&p->prvt_head);
}
