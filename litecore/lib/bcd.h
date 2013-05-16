#ifndef __LIB_BCD_H__
#define __LIB_BCD_H__



#ifdef __cplusplus
extern "C" {
#endif

uint8_t bcd2bin8(uint8_t x);
uint8_t bin2bcd8(uint8_t x);
uint16_t bin2bcd16(uint16_t x);
uint16_t bcd2bin16(uint16_t x);
uint32_t bin2bcd32(uint32_t x);
uint32_t bcd2bin32(uint32_t x);
uint64_t bin2bcd64(uint64_t x);
uint64_t bcd2bin64(uint64_t x);
int isnotbcd(const void *pAdr, uint_t nLen);


#ifdef __cplusplus
}
#endif

#endif
	
