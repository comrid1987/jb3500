#ifndef __APP_ALARM_H__
#define __APP_ALARM_H__

#ifdef __cplusplus
extern "C" {
#endif

//Header Files
#include "para.h"	

	
//Public Defines





//External Functions
void evt_ERC3(uint_t nMSA, u_word2 *pDu);
void evt_ERC5(uint_t nTn, const void *pOld, const void *pNew);
void evt_ERC8(uint_t nTn, const void *pOld, const void *pNew);
void evt_ERC35(uint_t nPort, const uint8_t *pAdr);
void evt_Terminal(t_afn04_f26 *pF26);
uint_t evt_YXRead(void);
int evt_RunTimeRead(time_t *pTime);
void evt_RunTimeWrite(time_t tTime);
void evt_Init(void);
void evt_Clear(void);
uint_t evt_GetCount(void);
void evt_GetFlag(void *pBuf);
int evt_Read(buf b, uint_t nPm, uint_t nPn, uint_t nIsNormal);

sys_res evt_DlqStateGet(uint_t nSn, void *pBuf);
void evt_DlqStateSet(uint_t nSn, const void *pBuf);
sys_res evt_DlqQlParaGet(uint_t nSn, void *pBuf);
void evt_DlqQlParaSet(uint_t nSn, const void *pBuf);



#ifdef __cplusplus
}
#endif

#endif

