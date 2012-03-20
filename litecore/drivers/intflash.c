#if INTFLASH_ENABLE
#include <drivers/intflash.h>


//Private Defines
#define INTF_LOCK_ENABLE		(0 && OS_TYPE)


//Internal Functions
#if INTF_LOCK_ENABLE
#define intf_Lock()				os_thd_Lock()
#define intf_Unlock()			os_thd_Unlock()
#else
#define intf_Lock()
#define intf_Unlock()
#endif



void intf_Init()
{

	arch_IntfInit();
}

sys_res intf_Erase(adr_t adr)
{
	sys_res res;

	intf_Lock();
	res = arch_IntfErase(adr);
	intf_Unlock();
	return res;
}

sys_res intf_Program(adr_t adr, const void *pData, uint_t nLen)
{
	sys_res res;

	intf_Lock();
	res = arch_IntfProgram(adr, pData, nLen);
	intf_Unlock();
	return res;
}


#endif

