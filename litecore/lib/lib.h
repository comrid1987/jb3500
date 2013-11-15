#ifndef __LIB_H__
#define __LIB_H__

#include <lib/bcd.h>
#include <lib/ecc.h>
#include <lib/mathlib.h>
#include <lib/timelib.h>


#if OS_TYPE
#include <lib/memory.h>
#include <lib/buffer.h>
#include <lib/dqueue.h>
#endif


//-------------------------------------------------------------------------
//General External Functions
//-------------------------------------------------------------------------
#define GETBIT(n, bit)  		(((n) >> (bit)) & 1)

#define BITMASK(bit)			(1 << (bit))
#define BITANTI(bit)			(~(BITMASK(bit)))

#define SETBIT(n, bit)			((n) |= BITMASK(bit))
#define CLRBIT(n, bit)			((n) &= BITANTI(bit))

#define MAX(a, b)				(((a) > (b)) ? (a) : (b))
#define MIN(a, b)				(((a) < (b)) ? (a) : (b))

#define ALIGN2(n)				(((n) + 1) & ~1)
#define ALIGN4(n)				(((n) + 3) & ~3)

#define FPOS(type, field)		((uint_t)&((type *)0)->field)
#define FSIZ(type, field)		(sizeof(((type *)0)->field))

#define ARR_SIZE(a)				(sizeof(a) / sizeof(a[0]))
#define ARR_ENDADR(a)			(&a[ARR_SIZE(a)])

#define bzero(a, size)			memset(a, 0, size)

typedef struct {
	uint8_t		ip[4];
	uint16_t	port;
}t_ip4_adr;

typedef union {
	uint32_t	n;
	uint8_t		byte[4];
}u_byte4;

typedef union {
	uint32_t	n;
	uint16_t	word[2];
}u_word2;



uint32_t int_getbits(uint32_t nData, uint_t nStartBit, uint_t nBits);
uint32_t int_setbits(uint32_t nData, uint_t nStartBit, uint_t nBits);

uint_t getbit(void *pData, uint_t i);
void setbit(void *pData, uint_t i);
void clrbit(void *pData, uint_t i);
uint_t bits1(uint_t i);
uint_t invert_bits(uint_t nData, uint_t nBits);
void reverse(void *pData, uint_t nLen);
void byteadd(void *pData, int nValue, uint_t nLen);
uint_t cycle(uint_t n, uint_t nStart, uint_t nEnd, int nStepping);

void bcd2str32(uint32_t n, char *pc);
void int2str32(uint32_t n, char *pc);
void bcd2str16(uint_t n, char *pc);
void bcd2str8(uint_t n, char *pc);
int memtest(const void *s, uint8_t c, int len);
int memcnt(const void *s, uint8_t c, uint_t len);




#endif


