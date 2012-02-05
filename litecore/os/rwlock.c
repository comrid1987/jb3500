#include <string.h>



void os_rwl_Init(os_rwl rwl, const char *name)
{

	bzero(rwl, sizeof(os_rwl));
	rt_sem_init(&rwl->sem, name, 1, RT_IPC_FLAG_FIFO);
}

//----------------------------------------------------------------------------
//Function Name  : os_rwl_Trylock
//Description    :  试图锁定读写锁
//Input            : -rwl:指定要锁的读写寄存器
//                   -nTmo:超时时间
//Output         : None
//Return          :-SYS_R_OK  :成功
//                  -SYS_R_TMO:超时
//----------------------------------------------------------------------------
sys_res os_rwl_Trylock(os_rwl rwl, int nTmo)
{
	sys_res res = SYS_R_TMO;

	nTmo /= OS_TICK_MS;
	do {
		if (rwl->rcnt == 0) {
			if (rt_sem_take(&rwl->sem, nTmo) == RT_EOK) {
				rwl->ste = RWL_S_LOCK;
				res = SYS_R_OK;
			} else {
				rwl->ste = RWL_S_IDLE;
				res = SYS_R_TMO;
			}
			break;
		} else
			os_thd_Slp1Tick();
	} while (nTmo--);
	return res;
}


//----------------------------------------------------------------------------
//Function Name  : os_rwl_Unlock
//Description    :   解锁读写锁，释放互斥量
//Input            : -rwl:指定要解锁的读写锁
//                   -nTmo:超时时间
//Output         : None
//Return          :-SYS_R_OK  :成功
//                  -SYS_R_TMO:超时
//----------------------------------------------------------------------------
void os_rwl_Unlock(os_rwl rwl)
{

	if (rwl->ste == RWL_S_LOCK) {
		rt_sem_release(&rwl->sem);
		rwl->ste = RWL_S_IDLE;
	}
}


//----------------------------------------------------------------------------
//Function Name  : os_rwl_Wait
//Description    :   等待函数os_rwl_Unlock()释放读写锁
//Input            : -rwl:指定要等的读写锁
//                   -nTmo:超时时间
//Output         : None
//Return          :-SYS_R_OK  :成功
//                  -SYS_R_TMO:超时
//----------------------------------------------------------------------------
sys_res os_rwl_Wait(os_rwl rwl, int nTmo)
{

	nTmo /= OS_TICK_MS;
	do {
		if (rwl->ste == RWL_S_IDLE) {
			rwl->rcnt += 1;
			return SYS_R_OK;
		}
		os_thd_Slp1Tick();
	} while (nTmo--);
	return SYS_R_TMO;
}


//----------------------------------------------------------------------------
//Function Name  : os_rwl_Release
//Description    :  释放读写锁
//Input            : -rwl:指定释放的读写寄存器
//Output         : None
//Return          :None
//----------------------------------------------------------------------------
void os_rwl_Release(os_rwl rwl)
{

	if (rwl->rcnt)
		rwl->rcnt -= 1;
}


