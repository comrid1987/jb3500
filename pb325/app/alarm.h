#ifndef __APP_ALARM_H__
#define __APP_ALARM_H__

#ifdef __cplusplus
extern "C" {
#endif


//Public Defines
#define EVT_BASE_ADDR				0

#define EVT_CNT_ADDR				16	// 4
#define EVT_FLAG_ADDR				20	// 8

#define EVT_RUNVER_ADDR				40	// 4
#define EVT_RUNTIME_ADDR			64	// 4

#define EVT_METERFLAG_ADDR			128	//





//External Functions
void evt_ERC3(uint_t nMSA, u_word2 *pDu);
void evt_Init(void);
void evt_Clear(void);
uint_t evt_GetCount(void);
void evt_GetFlag(void *pBuf);
int evt_Read(buf b, uint_t nPm, uint_t nPn, uint_t nIsImp);




#ifdef __cplusplus
}
#endif

#endif

