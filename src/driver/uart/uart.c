/* SPDX-License-Identifier: GPL-2.0-or-later */
/**
  ******************************************************************************
  * @file  uart.c
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

#include <stdarg.h>
#include <comdef.h>
#include <uart.h>
#include <sys.h>
#include <ezGPIO_fullMux_ctrl_macro.h>
#include <clkgen_ctrl_macro.h>
#include <rstgen_ctrl_macro.h>
#include <stdarg.h>
#include <stdio.h>
//#include <plic.h>

#define UART_CLK	(100000000UL)///(64000000UL)///

/* CLK 32M */
#define UART_BUADRATE_32MCLK_9600	9600
#define UART_BUADRATE_32MCLK_57600	57600
#define UART_BUADRATE_32MCLK_115200	115200
#define UART_BUADRATE_32MCLK_125000	125000
#define UART_BUADRATE_32MCLK_222222	222222	///230400
#define UART_BUADRATE_32MCLK_333333	333333	///380400
#define UART_BUADRATE_32MCLK_400000	400000	///460800
#define UART_BUADRATE_32MCLK_500000	500000 
#define UART_BUADRATE_32MCLK_666666	666666	///921600
#define UART_BUADRATE_32MCLK_1M	1000000
#define UART_BUADRATE_32MCLK_2M	2000000


/* CLK 64M */
#define UART_BUADRATE_64MCLK_38400	38400
#define UART_BUADRATE_64MCLK_57600	57600
#define UART_BUADRATE_64MCLK_115200	115200
#define UART_BUADRATE_64MCLK_230400	230400
#define UART_BUADRATE_64MCLK_380400	380400
#define UART_BUADRATE_64MCLK_444444	444444	///460800
#define UART_BUADRATE_64MCLK_500000	500000
#define UART_BUADRATE_64MCLK_571428	571428
#define UART_BUADRATE_64MCLK_666666	666666	///921600

#define UART_BUADRATE_64MCLK_800000	800000	///921600
#define UART_BUADRATE_64MCLK_1M	1000000
#define UART_BUADRATE_64MCLK_2M	2000000

////#define UART_BUADRATE_2_5M	2500000	///
////#define UART_BUADRATE_3M	3000000	///
#define UART_BUADRATE_64MCLK_4M	4000000	///shiboqi

volatile int uart_txfifo_available;

#define UART_PORT 3

static const u32 uart_base[4] = {
    UART0_BASE_ADDR,
    UART1_HS_BASE_ADDR,
    UART2_BASE_ADDR,
    UART3_BASE_ADDR,
        
};

static unsigned int serial_in(int id, int offset)
{
	offset <<= 2;
	return readw(uart_base[id] + offset);
}

static void serial_out(int id, int offset, int value)
{
	offset <<= 2;
	writew(value, uart_base[id] + offset);
}
static void wait_for_xmitr()
{
	unsigned int status;

	do {
		status = serial_in(UART_PORT, UART_USR);
	} while (!(status & UART_USR_Tx_FIFO_NFUL));
}

void uart_handler() {
	uart_txfifo_available = 1;
    u32 value = readl(uart_base[UART_PORT] + UART_IER_REG);
    value &= (~IER_TBE);
    writel(value, uart_base[UART_PORT] + UART_IER_REG);
}


void uart_init(int id)
{
	unsigned int divisor;
	unsigned char lcr_cache;
	unsigned char val;
    
            
	_ENABLE_CLOCK_clk_uart3_apb_;
	_ENABLE_CLOCK_clk_uart3_core_; 

	_ASSERT_RESET_rstgen_rstn_uart3_apb_;
	_ASSERT_RESET_rstgen_rstn_uart3_core_;
	_CLEAR_RESET_rstgen_rstn_uart3_core_;
	_CLEAR_RESET_rstgen_rstn_uart3_apb_;
	SET_GPIO_14_dout_uart3_pad_sout;
	//SET_GPIO_6_doen_uart0_pad_sout;
	SET_GPIO_14_doen_LOW;
	SET_GPIO_13_doen_HIGH;
	SET_GPIO_uart3_pad_sin(13);

 
	divisor = (UART_CLK / 9600) >> 4;

	lcr_cache = readl(uart_base[id] + UART_LCR_REG);
	writel((LCR_DLAB | lcr_cache), uart_base[id] + UART_LCR_REG);
	// writel(0, uart_base[id] + REG_MCR);
	writel((unsigned char)(divisor & 0xff), uart_base[id] + UART_BRDL_REG);
	writel((unsigned char)((divisor >> 8) & 0xff), uart_base[id] + UART_BRDH_REG);

	/* restore the DLAB to access the baud rate divisor registers */
	writel(lcr_cache, uart_base[id] + UART_LCR_REG);

	/* 8 data bits, 1 stop bit, no parity, clear DLAB */
	writel((LCR_CS8 | LCR_1_STB | LCR_PDIS), uart_base[id] + UART_LCR_REG);
	writel(0, uart_base[id] + UART_MDC_REG);

	/*
	 * Program FIFO: enabled, mode 0 (set for compatibility with quark),
	 * generate the interrupt at 8th byte
	 * Clear TX and RX FIFO
	 */
	writel((FCR_FIFO | FCR_MODE1 | /*FCR_FIFO_1*/FCR_FIFO_8 | FCR_RCVRCLR | FCR_XMITCLR), uart_base[id] + UART_FCR_REG);

}

int _putc(char c) {
	int timecnt = 0;
	int ret = 0;

	do
	{
		//if(timecnt > 10000)
		//	return -1;
		//timecnt++;
	}while((readl((uart_base[UART_PORT] + UART_LSR_REG)) & LSR_THRE) == 0);

    writel(c, uart_base[UART_PORT] + UART_THR_REG);
    

	return 0;
}

void rlSendString(char *s)
{
	while (*s){
		_putc(*s++);
	}
}

static void sys_out_char(const char c)
{
	_putc( c);
}
int CtrlBreak( void )
{
	int retflag;

	do{
		retflag	= serial_getc( );
		if( retflag == 0x03 ){
			break;
		}
	}while( retflag );
	return retflag;
}
int serial_getc()
{
	unsigned int status;

    status = serial_in(UART_PORT, REG_LSR);
    while (!(status & (1 << 0)))
	status = serial_in(UART_PORT, REG_LSR);

	status = serial_in(UART_PORT, REG_RDR);
	return status;
}

void serial_gets(char *pstr)
{
	unsigned char c;
	unsigned char *pstrorg;
	
	pstrorg = (unsigned char *) pstr;
again:

		while ((c = serial_getc()) != '\r')
		{
			if (c == '\b'){
				if ((int) pstrorg < (int) pstr){
					rlSendString("\b \b");
					pstr--;
				}
			}else{
				*pstr++ = c;
				sys_out_char(c);
			}
		}

		*pstr = '\0';

		rlSendString("\r\n");
		
}

int _inbyte(unsigned short timeout) // msec timeout
{
    unsigned int c;
    unsigned int delay = timeout*20;

    while (!(serial_in(UART_PORT, REG_LSR) & LSR_RXRDY)) {
        udelay(50);
        if (timeout && (--delay == 0)) {
            return -1;
        }
    }
    c = serial_in(UART_PORT, REG_RDR);
    return c;
}

void _outbyte(int c)
{
    while(!(serial_in(UART_PORT, REG_LSR) & LSR_TEMT))
        ;

    serial_out(UART_PORT, REG_THR, c);
}

void _puts(const char * s) {
  while (*s != '\0'){
    _putc(*s++);
  }
}
int __serial_tstc()
{
	return ((serial_in(UART_PORT, REG_LSR)) & (1 << 0));
}
 int serial_tstc()
{
	return __serial_tstc();
}
void print_ubyte_hex(unsigned char bval)
{
	static const char digits[16] = "0123456789ABCDEF";
	char tmp[2];
	int dig=0;

	dig = ((bval&0xf0)>>4);
	tmp[0] = digits[dig];
	dig = (bval&0x0f);
	tmp[1] = digits[dig];
	_putc(tmp[0]);
	_putc(tmp[1]);
}
int serial_nowait_getc()
{
	unsigned int status;

	status = serial_in(UART_PORT, REG_LSR);
	if (!(status & (1 << 0))) {
		status = 0;//NO_POLL_CHAR;
		goto out;
	}
	status = serial_in(UART_PORT, REG_RDR);
out:
	return status;
}

void sys_itoa(int value, char* string, unsigned char radix)
{
	int i;
	int count = 0;
	unsigned long temp;
	unsigned long res;
	char ch[512];
	
	if (radix == 10 && value < 0){
		ch[0] = '-';count++;temp = 0 - value;
	}else{
		temp = value;
	}

	while (1)
	{
		res = temp % radix;
		temp = temp / radix;
		if (res < 10)
			ch[count++] = res + '0';
		else
			ch[count++] = res - 10 + 'a';
		if (temp == 0)
			break;
	}
	ch[count] = '\0';
	string[0] = '\0';
	for (i = 0; i < count; i++)
		string[count - i - 1] = ch[i];

	string[count] = '\0';
}
static void sys_console_out(const char* buf, unsigned int nbytes)
{
	while (nbytes--)
		sys_out_char(*buf++);
}
