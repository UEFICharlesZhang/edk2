/* mem_clr.c */

#include <string.h>
#include "openssl/crypto.h"

unsigned char cleanse_ctr = 0;

void OPENSSL_cleanse(void *ptr, UINTN len)
	{
	unsigned char *p = (unsigned char *)ptr;
	UINTN loop = len;
	while(loop--)
		{
		*(p++) = cleanse_ctr;
		cleanse_ctr += (17 + (unsigned char)((UINTN)p & 0xF));
		}
	if(memchr(ptr, cleanse_ctr, len))
		cleanse_ctr += 63;
	}
