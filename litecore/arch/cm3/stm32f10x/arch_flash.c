#if INTFLASH_ENABLE


//Private Defines
#define STM32_INTF_LOCK_ENABLE		0



//Private Macros
#if STM32_INTF_LOCK_ENABLE
#define stm32_intf_Lock()			os_thd_Lock()
#define stm32_intf_Unlock()			os_thd_Unlock()
#else
#define stm32_intf_Lock()
#define stm32_intf_Unlock()
#endif



void arch_IntfInit()
{

	FLASH_Unlock();
}

sys_res arch_IntfErase(adr_t adr)
{
	FLASH_Status res = FLASH_COMPLETE;
	adr_t nEndAdr;

	stm32_intf_Lock();
	nEndAdr = adr + INTFLASH_BLK_SIZE;
	for (; adr < nEndAdr; adr += 4)
		if (*(volatile uint32_t *)adr != 0xFFFFFFFF)
			break;
	if (adr < nEndAdr)
		res = FLASH_ErasePage(adr);
	stm32_intf_Unlock();
	if (res == FLASH_COMPLETE)
		return SYS_R_OK;
	return SYS_R_TMO;
}

sys_res arch_IntfProgram(adr_t adr, const void *pData, uint_t nLen)
{
	adr_t nEndAdr;
	uint_t nData;
	__packed uint16_t *p = (__packed uint16_t *)pData;
	
	stm32_intf_Lock();
	nEndAdr = adr + nLen;
	for (; adr < nEndAdr; adr += 2) {
		nData = *p++;
		if (*(volatile uint16_t *)adr != nData)
			if (FLASH_ProgramHalfWord(adr, nData) != FLASH_COMPLETE)
				break;
	}
	stm32_intf_Unlock();
	if (adr < nEndAdr)
		return SYS_R_TMO;
	return SYS_R_OK;
}


#endif

