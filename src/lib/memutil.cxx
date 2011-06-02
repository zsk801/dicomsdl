/* -----------------------------------------------------------------------
 *
 * $Id$
 *
 * Copyright 2010, Kim, Tae-Sung. All rights reserved.
 * See copyright.txt for details
 *
 * -------------------------------------------------------------------- */

#include "memutil.h"
#include "errormsg.h"

namespace dicom { //------------------------------------------------------

#if !defined(_MSC_VER) || !defined(_M_IX86) || \
	defined(_M_X64) || defined(_M_IA64)
// swap routines; no-asm ver. --------------------------------------------

uint32 __ntohl (uint32 x)
{
    return  (((uint32)x >> 24) & 0x000000FF) |
            (((uint32)x >> 8)  & 0x0000FF00) |
            (((uint32)x << 8)  & 0x00FF0000) |
            (((uint32)x << 24) & 0xFF000000) ;
}

uint16 __ntohs (uint16 x)
{
    return  (((uint16)x >> 8)  & 0x00FF) |
            (((uint16)x << 8)  & 0xFF00) ;

}

void swap16(void *src)
{
	*(uint16 *)src = __ntohs(*(uint16 *)src);
}

void swap16(void *src, int srclen)
{
	uint16 *p = (uint16 *)src;
	srclen /= 2;
	while (srclen--)
		*p++ = __ntohs(*p);
}

void swap32(void *src)
{
	*(uint32 *)src = __ntohl(*(uint32 *)src);
}

void swap32(void *src, int srclen)
{
	uint32 *p = (uint32 *)src;
	srclen /= 4;
	while (srclen--)
		*p++ = __ntohl(*p);
}

void swap64(void *src)
{
	uint32 *p = (uint32 *)src, c;
	c = p[0];
	p[0] = __ntohl(p[1]);
	p[1] = __ntohl(c);
}

void swap64(void *src, int srclen)
{
	uint32 *p = (uint32 *)src, c;
	srclen /= 8;
	while (srclen--) {
		c = *p;
		*p++ = __ntohl(p[1]);
		*p++ = __ntohl(c);
	}
}

#else
// swap routines; asm ver. -----------------------------------------------

uint32 __ntohl (uint32 x)
{
	__asm {
		mov eax, x
		bswap eax
	}
}

uint16 __ntohs (uint16 x)
{
	__asm {
		mov ax, x
		xchg al, ah
	}
}

void swap16(void *src)
{
	__asm {
		mov esi, src
		mov ax, [esi]
		xchg al, ah
		mov [esi], ax
	}
}

void swap16(void *src, int srclen)
{
	__asm {
		mov edi, src
		mov ecx, srclen
		shr ecx, 1
		test ecx, ecx
		je short $EXITSWAP2N

	$LOOPSWAP2N:
		mov ax, [edi]
		xchg al, ah
		stosw
		loop $LOOPSWAP2N

	$EXITSWAP2N:
	}
}

void swap32(void *src)
{
	__asm {
		mov esi, src
		mov eax, [esi]
		bswap eax
		mov [esi], eax
	}
}

void swap32(void *src, int srclen)
{
	__asm {
		mov edi, src
		mov ecx, srclen
		shr ecx, 2
		test ecx, ecx
		je short $EXITSWAP4N

	$LOOPSWAP4N:
		mov eax, [edi]
		bswap eax
		stosd
		loop $LOOPSWAP4N

	$EXITSWAP4N:
	}
}

void swap64(void *src)
{
	__asm {
		mov esi, src
		mov eax, [esi]
		bswap eax
		xchg eax, [esi+4]
		bswap eax
		mov [esi], eax
	}
}

void swap64(void *src, int srclen)
{
	__asm {
		mov edi, src
		mov ecx, srclen
		shr ecx, 3
		test ecx, ecx
		je short $EXITSWAP8N

	$LOOPSWAP8N:
		mov eax, [edi]
		bswap eax
		xchg eax, [edi+4]
		bswap eax
		mov [edi], eax
		add edi, 8
		loop short $LOOPSWAP8N

	$EXITSWAP8N:
	}
}

#endif

} // namespace dicom -----------------------------------------------------
