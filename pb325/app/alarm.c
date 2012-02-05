#include <stdio.h>
#include <string.h>
#include <litecore.h>
#include "alarm.h"
#include "para.h"



//Private Defines
#define EVT_LOCK_ENABLE			0
#define EVT_MAGIC_WORD			0x67527920

#define EVT_SIZE				256
#define EVT_QTY					2048
#define EVT_DATA_BASE			(1024 * 0x1000)


//Private Variables
#if EVT_LOCK_ENABLE
static os_sem evt_sem;
#endif


//Private Macros
#if EVT_LOCK_ENABLE
#define evt_Lock()				rt_sem_take(&evt_sem, RT_WAITING_FOREVER)
#define evt_Unlock()			rt_sem_release(&evt_sem)
#else
#define evt_Lock()
#define evt_Unlock()
#endif





//External Functions
//参数变更事件
void evt_ERC3(uint_t nMSA, u_word2 *pDu)
{

}

void evt_Init()
{

#if EVT_LOCK_ENABLE
	rt_sem_init(&evt_sem, "sem_evt", 1, RT_IPC_FLAG_FIFO);
#endif
}

void evt_Clear()
{

	evt_Lock();
	evt_Unlock();
}


uint_t evt_GetCount()
{

	return 0;
}

void evt_GetFlag(void *pBuf)
{

}

int evt_Read(buf b, uint_t nPm, uint_t nPn, uint_t nIsImp)
{

	return 0;
}


