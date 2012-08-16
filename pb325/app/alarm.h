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
void evt_ERC8(uint_t nTn, uint_t nFlag);
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




#ifdef __cplusplus
}
#endif

#endif

