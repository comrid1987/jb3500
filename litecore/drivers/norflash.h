#ifndef __NORFLASH_H__
#define __NORFLASH_H__




#ifdef __cplusplus
extern "C" {
#endif









//×ÜÏß²Ù×÷
#define sysAddress(nOffset)		*(volatile uint16_t *)(NORFLASH_BASE_ADR + (nOffset << 1))
#define specAddress(nSpdAdr)	*(volatile uint16_t *)(nSpdAdr)


//External Functions
void norf_Init(void);
sys_res norf_EraseChip(void);
sys_res norf_nolockErase(adr_t adr);
sys_res norf_Erase(adr_t adr);
sys_res norf_nolockProgram(adr_t adr, void *pBuf, uint_t nSize);
sys_res norf_Program(adr_t adr, void *pBuf, uint_t nSize);
sys_res norf_Read(adr_t adr, void *pBuf, uint_t nLen);


#ifdef __cplusplus
}
#endif


#endif



