#ifndef __RWLOCK_H__
#define __RWLOCK_H__




#define RWL_S_IDLE					0
#define RWL_S_READ					1
#define RWL_S_TRYLOCK				2
#define RWL_S_LOCK					3


typedef struct {
	uint8_t	ste;
	uint8_t	rcnt;
	os_sem	sem;
}os_rwl[1];


//rwlock operate
void os_rwl_Init(os_rwl rwl, const char *name);
sys_res os_rwl_Trylock(os_rwl rwl, int nTmo);
void os_rwl_Unlock(os_rwl rwl);
sys_res os_rwl_Wait(os_rwl rwl, int nTmo);
void os_rwl_Release(os_rwl rwl);




#endif

