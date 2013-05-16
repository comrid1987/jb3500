#ifndef __SYS_FLASH_H__
#define __SYS_FLASH_H__


#ifdef __cplusplus
extern "C" {
#endif

//Header Files
#if INTFLASH_ENABLE
#include <drivers/intflash.h>
#endif
#if NANDFLASH_ENABLE
#include <drivers/nandflash.h>
#endif	  
#if NORFLASH_ENABLE
#include <drivers/norflash.h>
#endif	  
#if SPIFLASH_ENABLE
#include <drivers/spiflash.h>
#endif



//Public Defines
#define FLASH_DEV_NULL			0
#define FLASH_DEV_INT			1
#define FLASH_DEV_EXTNOR		2
#define FLASH_DEV_SPINOR		3



//Public Typedefs
typedef const struct {
	uint16_t	dev;
	uint16_t	blk;
	adr_t		start;
}t_flash_dev;



//External Functions
int flash_BlkSize(uint_t nDev);
sys_res flash_nolockErase(uint_t nDev, adr_t nAdr);
sys_res flash_nolockProgram(uint_t nDev, adr_t nAdr, const void *pData, uint_t nLen);

sys_res flash_Erase(uint_t nDev, adr_t nAdr);
sys_res flash_Program(uint_t nDev, adr_t nAdr,const void *pData, uint_t nLen);

void flash_Init(void);

void intf_Read(uint_t nAdr, void *pBuf, uint_t nLen);
void intf_Write(uint_t nAdr, const void *pBuf, uint_t nLen);

void spif_SecRead(uint_t nSec, void *pData);
void spif_SecWrite(uint_t nSec, const void *pBuf);
void spif_Read(uint_t nAdr, void *pData, uint_t nLen);
void spif_Write(uint_t nAdr, const void *pBuf, uint_t nLen);
void spif_Fill(uint_t nAdr, uint_t nVal, uint_t nLen);

void flash_Flush(uint_t nDelay);




#ifdef __cplusplus
}
#endif

#endif

