#ifndef __APP_ALARM_H__
#define __APP_ALARM_H__

#ifdef __cplusplus
extern "C" {
#endif


//Public Defines





//External Functions
void evt_ERC3(uint_t nMSA, u_word2 *pDu);
void evt_RunTimeWrite(time_t tTime);
int evt_RunTimeRead(time_t *pTime);

void evt_Init(void);
void evt_Clear(void);
uint_t evt_GetCount(void);
void evt_GetFlag(void *pBuf);
int evt_Read(buf b, uint_t nPm, uint_t nPn, uint_t nIsImp);




#ifdef __cplusplus
}
#endif

#endif

