#ifndef __BKP_H__
#define __BKP_H__


#ifdef __cplusplus
extern "C" {
#endif



//Public Defines
#define BKP_T_NULL			0
#define BKP_T_INT			1
#define BKP_T_EEPROM		2
#define BKP_T_SFS			3




//External Functions
void bkp_Init(void);
sys_res bkp_Write(uint_t nAdr, const void *pBuf, uint_t nLen);
sys_res bkp_WriteData(uint_t nAdr, const uint64_t nData, uint_t nLen);
void bkp_Fill(uint_t nAdr, uint_t nEnd, uint_t nVal);
sys_res bkp_Read(uint_t nAdr, void *pBuf, uint_t nLen);


#ifdef __cplusplus
}
#endif

#endif

