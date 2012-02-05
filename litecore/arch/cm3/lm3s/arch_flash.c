

//Private Defines
#define LM3S_INTF_LOCK_ENABLE		0


//Private Macros
#if LM3S_INTF_LOCK_ENABLE
#define lm3s_intf_Lock()			os_thd_Lock()
#define lm3s_intf_Unlock()			os_thd_Unlock()
#else
#define lm3s_intf_Lock()
#define lm3s_intf_Unlock()
#endif



void arch_IntfInit()
{

	MAP_FlashUsecSet(MAP_SysCtlClockGet() / 1000000);
}

sys_res arch_IntfErase(adr_t adr)
{
	int res = 0;
	adr_t nEndAdr;

	lm3s_intf_Lock();
	nEndAdr = adr + INTFLASH_BLK_SIZE;
	for (; adr < nEndAdr; adr += 4)
		if (*(volatile uint32_t *)adr != 0xFFFFFFFF)
			break;
	if (adr < nEndAdr)
		res = MAP_FlashErase(adr);
	lm3s_intf_Unlock();
	if (res)
		return SYS_R_TMO;
	return SYS_R_OK;
}


#if 1
sys_res arch_IntfProgram(adr_t adr, const void *pData, uint_t nLen)
{
	adr_t aEnd;
	uint32_t *p;

	lm3s_intf_Lock();
	p = (uint32_t *)pData;
	aEnd = adr + nLen;
	for (; adr < aEnd; adr += 4) {
		while (__raw_readl(FLASH_FMC) & FLASH_FMC_WRITE);
		__raw_writel(*p++, FLASH_FMD);
		__raw_writel(adr, FLASH_FMA);
		__raw_writel(FLASH_FMC_WRKEY | FLASH_FMC_WRITE, FLASH_FMC);
	}
	lm3s_intf_Unlock();
	return SYS_R_OK;
}
#else
sys_res arch_IntfProgram(adr_t adr, const void *pData, uint_t nLen)
{
	int res;
	
	lm3s_intf_Lock();
	res = MAP_FlashProgram((unsigned long *)pData, adr, nLen);
	lm3s_intf_Unlock();
	if (res)
		return SYS_R_TMO;
	return SYS_R_OK;
}
#endif



