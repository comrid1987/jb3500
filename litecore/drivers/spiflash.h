#ifndef __SPIFLASH_H__
#define __SPIFLASH_H__



#ifdef __cplusplus
extern "C" {
#endif





//Public Defines
#define SPIF_SEC_SIZE			0x1000
#define SPIF_SEC_QTY			1024


//Public Typedefs



//External Functions
void spif_Init(void);
void spif_ReadLen(uint_t nSec, uint_t nOffset, void *pData, uint_t nLen);
void spif_Program(uint_t nSec, const void *pData);
int spif_GetSize(void);




#ifdef __cplusplus
}
#endif

#endif



