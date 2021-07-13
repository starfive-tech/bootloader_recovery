/* SPDX-License-Identifier: GPL-2.0-or-later */
/**
  ******************************************************************************
  * @file  sys.c
  * @author  StarFive Technology
  * @version  V1.0
  * @date  07/10/2021
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
#include "sys.h"
#include "comdef.h"
//unsigned int funcpll_432 = 0;
//unsigned int osc_clk = 0;

/*===========================================================================
FUNCTION:  sys_memcpy

DESCRIPTION:
	 copy one block memory to another position

INPUT PARAMETER:
	void *p_des:	destination address
	const void * p_src: source address
	unsigned long size: length of memory to copy (unit is byte)

OUTPUT PARAMETER:

RETURN VALUE:
===========================================================================*/
void sys_memcpy(void *dest,const void * src,unsigned long count)
{
	unsigned long *dl = (unsigned long *)dest, *sl = (unsigned long *)src;
	char *d8, *s8;

	if (src == dest)
		return dest;

	/* while all data is aligned (common case), copy a word at a time */
	if ( (((unsigned long)dest | (unsigned long)src) & (sizeof(*dl) - 1)) == 0) {
		while (count >= sizeof(*dl)) {
			*dl++ = *sl++;
			count -= sizeof(*dl);
		}
	}
	/* copy the reset one byte at a time */
	d8 = (char *)dl;
	s8 = (char *)sl;
	while (count--)
		*d8++ = *s8++;
}
 int sys_memcmp(const void * cs,const void * ct,unsigned int count)
{
	const unsigned char *su1, *su2;
	int res = 0;

	for( su1 = cs, su2 = ct; 0 < count; ++su1, ++su2, count--)
		if ((res = *su1 - *su2) != 0)
			break;
	return res;
}
void * _memcpy(void * dest,const void *src,unsigned int count)
{
	char *tmp = (char *) dest;
	const char *s = (char *) src;

	while (count--)
		*tmp++ = *s++;
	return dest;
}
/*===========================================================================
FUNCTION:  sys_memcpy_32

DESCRIPTION:
	 copy one block memory to another position
	 caller guarantee the src/des address are DWORD allign

INPUT PARAMETER:
	void *p_des:	destination address
	const void * p_src: source address
	unsigned long size: length of memory to copy (unit is DWORD)

OUTPUT PARAMETER:

RETURN VALUE:
===========================================================================*/
void sys_memcpy_32(void *p_des,const void * p_src,unsigned long size)
{
	unsigned long i;
	for (i=0;i<size;i++)
		*((unsigned long*)p_des+i) = *((unsigned long*)p_src+i);
}

/*===========================================================================
FUNCTION:  sys_memset

DESCRIPTION:
	 fill memory with specifed value

INPUT PARAMETER:
	void *p_des:	destination address
	int c:	value to set
	unsigned long size: length of memory

OUTPUT PARAMETER:

RETURN VALUE:
===========================================================================*/
void sys_memset(void *p_des,unsigned char c,unsigned long size)
{
	unsigned long i;
	for (i=0;i<size;i++)
		*((char*)p_des+i) = c;
}

/*===========================================================================
FUNCTION:  sys_memset32

DESCRIPTION:
	 fill memory with specifed value

INPUT PARAMETER:
	void *p_des:	destination address
	int c:	value to set
	unsigned long size: length of memory in word(32bit)

OUTPUT PARAMETER:

RETURN VALUE:
===========================================================================*/
void sys_memset32(void *p_des,int c,unsigned long size)
{
	unsigned long i;
	for(i=0; i< size; i++)
		((unsigned long*)p_des)[i] = c;
}
