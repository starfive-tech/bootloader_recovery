/* SPDX-License-Identifier: GPL-2.0-or-later */
/**
  ******************************************************************************
  * @file  spi_probe.c
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

#include <comdef.h>
#include <sys.h>
#include <spi_flash.h>
#include <spi_flash_internal.h>
#include <spi.h>
#include <uart.h>

#define IDCODE_CONT_LEN 0
#define IDCODE_PART_LEN 3
#define IDCODE_LEN (IDCODE_CONT_LEN + IDCODE_PART_LEN)

#define CMD_READ_ID			0x9f

#define NOR 			0
#define GIGANAND 		1

//#define CONFIG_SPI_FLASH_ATMEL
//#define CONFIG_SPI_FLASH_GIGADEVICE
//#define CONFIG_SPI_FLASH_EON
//#define CONFIG_SPI_FLASH_MACRONIX
//#define CONFIG_SPI_FLASH_SPANSION
//#define CONFIG_SPI_FLASH_STMICRO
//#define CONFIG_SPI_FLASH_SST
//#define CONFIG_SPI_FLASH_WINBOND
//#define TEST_GD25Q64B
//#define TEST_GD25Q64C
//#define TEST_GD25LB64C

static struct spi_flash g_spi_flash[1];

static const struct spi_flash_params spi_flash_table[] =
{
	 {"Common_flash", 0x534654, 8,16,16,256, NOR},
};

struct spi_flash *spi_flash_probe_nor(struct spi_slave *spi, u8 *idcode)
{
	struct spi_flash_params *params;
	struct spi_flash *flash;
	u32 id = 0;
	static int i = 0;

#if 0
	id = ((idcode[2] << 16) | (idcode[1] << 8) | idcode[0]);
    if(id == 0x0)
    {
        return NULL;
    }

	params = spi_flash_table;
	for (i = 0; spi_flash_table[i].name != NULL; i++)
	{
		if ((spi_flash_table[i].id & 0xFFFFFF) == id)
		{
			break;
		}
	}
#endif
	flash = &g_spi_flash[0];
	if (!flash)
	{
		//uart_printf("SF: Failed to allocate memory\r\n");
		return NULL;
	}

	flash->name = spi_flash_table[i].name;
	if(spi_flash_table[i].flags == NOR)
	{
		/* Assuming power-of-two page size initially. */
		flash->write = spi_flash_cmd_write_mode;
		flash->erase = spi_flash_erase_mode;
		flash->read = spi_flash_read_mode;
		flash->page_size = 1 << spi_flash_table[i].l2_page_size;
		flash->sector_size = flash->page_size * spi_flash_table[i].pages_per_sector;
		flash->block_size = flash->sector_size * spi_flash_table[i].sectors_per_block;
		flash->size = flash->page_size * spi_flash_table[i].pages_per_sector
						* spi_flash_table[i].sectors_per_block
						* spi_flash_table[i].nr_blocks;
	}

	//uart_printf("spi probe complete\r\n");

	return flash;
}
#if 1
static int print_id(u8 *idcode, int len)
{
	int i;

	uart_printf("idcode:0x");
	for (i=len-1; i>=0; i--)
		print_ubyte_hex(idcode[i]);
	uart_printf("\r\n");
	return 0;
}
#endif
static int spi_read_id(struct spi_slave *spi, unsigned char cmd, void *response, u32 len)
{
	int ret = -1,ret1 = -1,ret2 = -1;
	unsigned char buf[4] = {0};// = {(u8)cmd, 0x00, 0x00, 0x00};
	unsigned char buf_nor[1]; // = {(u8)cmd};
	u8 *idcode = (u8 *)response;

	buf[0] = cmd;
	buf[1] = 0;
	buf[2] = 0;
	buf[3] = 0;

	buf_nor[0] = cmd;

	ret1 = spi_xfer(spi, 1*8, &buf[0], NULL, SPI_XFER_BEGIN, 8);
	ret2 = spi_xfer(spi, len*8, NULL, response, SPI_XFER_END, 8);
    return 0;
}

static struct spi_flash aic_flash;

struct spi_flash *spi_flash_probe(unsigned int bus, unsigned int cs,
		unsigned int max_hz, unsigned int mode, unsigned int bus_width)
{
	struct spi_slave *spi;
	struct spi_flash *flash = &aic_flash;
	int ret = 0;
	u8 idcode[IDCODE_LEN];

	spi = spi_setup_slave(bus, cs, max_hz, mode, bus_width);
	if (!spi) {
		//uart_printf("SF: Failed to set up slave\n");
		return NULL;
	}

	/* Read the ID codes */
 	ret = spi_read_id(spi, CMD_READ_ID, idcode, sizeof(idcode));
	if (ret)
	{
		//uart_printf("SF: Failed to read ID : %d\n", ret);
		goto err_read_id;
	}

	print_id(idcode, sizeof(idcode));

	flash = spi_flash_probe_nor(spi,idcode);
	if (!flash)
	{
		goto err_manufacturer_probe;
	}

	flash->spi = spi;
	return flash;

err_manufacturer_probe:
err_read_id:

	return NULL;
}
