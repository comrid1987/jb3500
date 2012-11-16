#if INTFLASH_ENABLE


//Private Defines
#define STM32_INTF_LOCK_ENABLE		0

#define ADDR_FLASH_SECTOR_0     ((uint32_t)0x08000000) /* Base @ of Sector 0, 16 Kbyte */
#define ADDR_FLASH_SECTOR_1     ((uint32_t)0x08004000) /* Base @ of Sector 1, 16 Kbyte */
#define ADDR_FLASH_SECTOR_2     ((uint32_t)0x08008000) /* Base @ of Sector 2, 16 Kbyte */
#define ADDR_FLASH_SECTOR_3     ((uint32_t)0x0800C000) /* Base @ of Sector 3, 16 Kbyte */
#define ADDR_FLASH_SECTOR_4     ((uint32_t)0x08010000) /* Base @ of Sector 4, 64 Kbyte */
#define ADDR_FLASH_SECTOR_5     ((uint32_t)0x08020000) /* Base @ of Sector 5, 128 Kbyte */
#define ADDR_FLASH_SECTOR_6     ((uint32_t)0x08040000) /* Base @ of Sector 6, 128 Kbyte */
#define ADDR_FLASH_SECTOR_7     ((uint32_t)0x08060000) /* Base @ of Sector 7, 128 Kbyte */
#define ADDR_FLASH_SECTOR_8     ((uint32_t)0x08080000) /* Base @ of Sector 8, 128 Kbyte */
#define ADDR_FLASH_SECTOR_9     ((uint32_t)0x080A0000) /* Base @ of Sector 9, 128 Kbyte */
#define ADDR_FLASH_SECTOR_10    ((uint32_t)0x080C0000) /* Base @ of Sector 10, 128 Kbyte */
#define ADDR_FLASH_SECTOR_11    ((uint32_t)0x080E0000) /* Base @ of Sector 11, 128 Kbyte */


//Private Macros
#if STM32_INTF_LOCK_ENABLE
#define stm32_intf_Lock()			os_thd_Lock()
#define stm32_intf_Unlock()			os_thd_Unlock()
#else
#define stm32_intf_Lock()
#define stm32_intf_Unlock()
#endif
uint32_t GetSector(uint32_t Address)
{
  uint32_t sector = 0;
  
  if((Address < ADDR_FLASH_SECTOR_1) && (Address >= ADDR_FLASH_SECTOR_0))
  {
    sector = FLASH_Sector_0;  
  }
  else if((Address < ADDR_FLASH_SECTOR_2) && (Address >= ADDR_FLASH_SECTOR_1))
  {
    sector = FLASH_Sector_1;  
  }
  else if((Address < ADDR_FLASH_SECTOR_3) && (Address >= ADDR_FLASH_SECTOR_2))
  {
    sector = FLASH_Sector_2;  
  }
  else if((Address < ADDR_FLASH_SECTOR_4) && (Address >= ADDR_FLASH_SECTOR_3))
  {
    sector = FLASH_Sector_3;  
  }
  else if((Address < ADDR_FLASH_SECTOR_5) && (Address >= ADDR_FLASH_SECTOR_4))
  {
    sector = FLASH_Sector_4;  
  }
  else if((Address < ADDR_FLASH_SECTOR_6) && (Address >= ADDR_FLASH_SECTOR_5))
  {
    sector = FLASH_Sector_5;  
  }
  else if((Address < ADDR_FLASH_SECTOR_7) && (Address >= ADDR_FLASH_SECTOR_6))
  {
    sector = FLASH_Sector_6;  
  }
  else if((Address < ADDR_FLASH_SECTOR_8) && (Address >= ADDR_FLASH_SECTOR_7))
  {
    sector = FLASH_Sector_7;  
  }
  else if((Address < ADDR_FLASH_SECTOR_9) && (Address >= ADDR_FLASH_SECTOR_8))
  {
    sector = FLASH_Sector_8;  
  }
  else if((Address < ADDR_FLASH_SECTOR_10) && (Address >= ADDR_FLASH_SECTOR_9))
  {
    sector = FLASH_Sector_9;  
  }
  else if((Address < ADDR_FLASH_SECTOR_11) && (Address >= ADDR_FLASH_SECTOR_10))
  {
    sector = FLASH_Sector_10;  
  }
  else/*(Address < FLASH_END_ADDR) && (Address >= ADDR_FLASH_SECTOR_11))*/
  {
    sector = FLASH_Sector_11;  
  }
    return sector;
}



void arch_IntfInit()
{

	FLASH_Unlock();
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | 
                  FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR);
}

sys_res arch_ErasseRegion(adr_t nStarAdr,adr_t nEndAdr)
{
	uint_t StartSector = 0, EndSector = 0,SectorCounter = 0;	//Address = 0,
	FLASH_Status res = FLASH_COMPLETE;
	stm32_intf_Lock();	
	FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | 
                  FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR); 
	StartSector = GetSector(nStarAdr);
	EndSector = GetSector(nEndAdr);
 	for (SectorCounter = StartSector; SectorCounter < EndSector; SectorCounter += 8){
        if (FLASH_EraseSector(SectorCounter, VoltageRange_3) != FLASH_COMPLETE) { 
             while (1){
 		}
 		}
   }		 		
   FLASH_Lock(); 
   stm32_intf_Unlock();
	if (res == FLASH_COMPLETE)
		return SYS_R_OK;
	return SYS_R_TMO;
}

sys_res arch_IntfErase(adr_t nAdr)//
{  uint32_t StartSector = 0;
	FLASH_Status res = FLASH_COMPLETE;
	adr_t nCur;//, nEndAdr
	stm32_intf_Lock();	
	nCur = nAdr;
	FLASH_Unlock();   
    StartSector = GetSector(nCur);
    if (FLASH_EraseSector(StartSector, VoltageRange_3) != FLASH_COMPLETE)
    { 
      /* Error occurred while sector erase. 
         User can add here some code to deal with this error  */
      while (1)
      {
      }
    }
// 	 //FLASH_Lock(); 
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
	FLASH_Unlock();  
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

