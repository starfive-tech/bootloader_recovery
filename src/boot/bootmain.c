/* SPDX-License-Identifier: GPL-2.0-or-later */
/**
  ******************************************************************************
  * @file  bootmain.c
  * @author  StarFive Technology
  * @version  V1.0
  * @date  07/24/2020
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
#include <sys.h>
#include <uart.h>
#include <spi.h>
#include <spi_flash.h>
#include <clkgen_ctrl_macro.h>
#include <syscon_sysmain_ctrl_macro.h>
#include <ezGPIO_fullMux_ctrl_macro.h>
#include <rstgen_ctrl_macro.h>
#include <syscon_iopad_ctrl_macro.h>

#define PRINT(fmt, ...)  serial_printf(fmt, ##__VA_ARGS__)
#define ERROR(fmt, ...)  serial_printf("ERROR %s() ln %d:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

extern unsigned int receive_count;
int spi_flash_erases(struct spi_flash *spi_flash_local, u32 start_addr, int num_block)
{
	int ret;
	int l = 0;;
	int k, j, page_par_block = 0;
	u32 len = 0, offset_new = start_addr;
	u32 i, block_size,page_size;

	block_size = spi_flash_local->block_size;
	page_size = spi_flash_local->page_size;
	len = page_size;
	page_par_block = block_size/page_size;
	for(l = 0; l < num_block; l++)
	{
		ret = spi_flash_local->erase(spi_flash_local, offset_new, block_size, 64);
		if(ret != 0)
		{
	        return ret;
		}
		offset_new += block_size;

	}
	return ret;
	
}

int spi_flash_writes(struct spi_flash* spi_flash_local, u32 start_addr,u8 *data_buff, u32 page_num)
{
	int k = 0,i = 0,j = 0,ret = 0;
	u32 offset_new = start_addr;
	unsigned char *data = (unsigned char *)data_buff;
	int spi_write = 1; // or 4

	int len = spi_flash_local->page_size;

	for(k = 0; k < page_num; k++)
    	{
		ret = spi_flash_local->write(spi_flash_local, offset_new, len, (void*)data, spi_write);
		if(ret != 0)
		{
			return -1;
		}
		offset_new = offset_new + len;
		data = data + len;

   	 }	
	return ret;
}
int updata_flash(u32 flash_addr)
{
    int ret = 0;
    struct spi_flash* flash;
    u32 len = 0;
    int erase_block = 0;
    unsigned int page_count;
    unsigned int file_size = 0;
//    receive_count = 0;
    rlSendString("send a file by xmodem\r\n");
    // replace xmodem_recv_file, which may have too many errors to receive the file
    ret = xmodemReceive((unsigned char *)DEFAULT_BOOT_LOAD_ADDR, 192*1024);
    if(ret <= 0)
        return -1;
    file_size = ret;

    cadence_qspi_init(0, 1);
    flash = spi_flash_probe(0, 0, 10000000, 0, (u32)SPI_DATAMODE_8);
    if (!flash) {
        rlSendString("spi_flash_probe fail\r\n");
        return -1;
    }
    erase_block = (file_size + flash->block_size - 1) / flash->block_size;
    page_count = (file_size + flash->page_size - 1) / flash->page_size;

    ret = spi_flash_erases(flash, flash_addr, erase_block);
    if(ret < 0)
    {
        rlSendString("spi_flash_erases fail\r\n");
        return -1;
    }

    ret = spi_flash_writes(flash, flash_addr, (u8 *)DEFAULT_BOOT_LOAD_ADDR, page_count);
    if(ret < 0)
    {
        rlSendString("spi_flash_writes fail\r\n");    
        return -1;
    }
    else
    {
        rlSendString("updata flash ok\r\n");    
    }
    return 0;
}

static int updata_flash_code(unsigned int updata_num)
{
    int ret = 0;
    switch (updata_num){
        case 0:
            ret = updata_flash(FLASH_SECONDBOOT_START_ADDR);
            break;
        case 1:
            ret = updata_flash(FLASH_DDRINIT_START_ADDR);
            break;
        default:
            break;
            
    }

    return ret;
        
}
static void chip_clk_init() 
{
	_SWITCH_CLOCK_clk_cpundbus_root_SOURCE_clk_pll0_out_;
	_SWITCH_CLOCK_clk_dla_root_SOURCE_clk_pll1_out_;
	_SWITCH_CLOCK_clk_dsp_root_SOURCE_clk_pll2_out_;
	_SWITCH_CLOCK_clk_perh0_root_SOURCE_clk_pll0_out_;
}
void BootMain(void)
{	
	int boot_mode = 0;
	s32 usel;
	int i;
	char str[128];
	int ret = 0;
	int bootdelay = SECONDBOOTDELAY;
	unsigned long ts;
	int abort = 0;

	chip_clk_init();

	_SET_SYSCON_REG_register50_SCFG_funcshare_pad_ctrl_18(0x00c000c0);

	_CLEAR_RESET_rstgen_rstn_usbnoc_axi_;
	_CLEAR_RESET_rstgen_rstn_hifi4noc_axi_;

	_ENABLE_CLOCK_clk_x2c_axi_;
	_CLEAR_RESET_rstgen_rstn_x2c_axi_;

	_CLEAR_RESET_rstgen_rstn_dspx2c_axi_;
	_CLEAR_RESET_rstgen_rstn_dma1p_axi_;

	_ENABLE_CLOCK_clk_msi_apb_;
	_CLEAR_RESET_rstgen_rstn_msi_apb_;

	_ASSERT_RESET_rstgen_rstn_x2c_axi_;
	_CLEAR_RESET_rstgen_rstn_x2c_axi_;

	uart_init(3);
	
	PRINT("\r\nVIC second boot, version:%s %s\n", VERSION, CONFIGURATION);
	while(1)
	{
		rlSendString("***************************************************\r\n");
		rlSendString("***************JH7100 recovery boot ***************\r\n");
		rlSendString("***************************************************\r\n");  

		rlSendString("0:updata bootloader\r\n");  
		rlSendString("1:updata ddr init\r\n");  
		again:
		rlSendString("Select the function to test : ");
		serial_gets(str);

		if(str[0] == 0)
		    goto again;

		usel = atoi(str);
		if(usel > 2)
		{
		    rlSendString("error select,try again\r\n");
		    goto again;
		}
		PRINT("\r\nselect %d\n", usel);
		ret = updata_flash_code(usel);
		if(ret < 0)
		    rlSendString("updata fail\r\n");
		else
		{
		    rlSendString("updata success\r\n"); 
		}
             
	}
}
