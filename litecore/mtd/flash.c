

#if INTFLASH_ENABLE
#include <drivers/intflash.c>
#endif
#if NANDFLASH_ENABLE
#include <drivers/nandflash.c>
#endif
#if NORFLASH_ENABLE
#include <drivers/norflash.c>
#endif	  
#if SPIFLASH_ENABLE
#include <drivers/spiflash.c>
#endif



int flash_BlkSize(uint_t nDev)
{

	switch (nDev) {
#if INTFLASH_ENABLE
	case FLASH_DEV_INT:
		return INTFLASH_BLK_SIZE;
#endif
#if NORFLASH_ENABLE
	case FLASH_DEV_EXTNOR:
		return NORFLASH_BLK_SIZE;
#endif
	default:
		return -1;
	}
}

sys_res flash_nolockErase(uint_t nDev, adr_t nAdr)
{
	sys_res res;

	switch (nDev) {
#if INTFLASH_ENABLE
	case FLASH_DEV_INT:
		res = intf_nolockErase(nAdr);
		break;
#endif
#if NORFLASH_ENABLE
	case FLASH_DEV_EXTNOR:
		res = norf_nolockErase(nAdr);
		break;
#endif
	default:
		res = SYS_R_EMPTY;
		break;
	}
	return res;
}

sys_res flash_nolockProgram(uint_t nDev, adr_t nAdr, const void *pData, uint_t nLen)
{
	sys_res res;

	switch (nDev) {
#if INTFLASH_ENABLE
	case FLASH_DEV_INT:
		res = intf_nolockProgram(nAdr, pData, nLen);
		break;
#endif
#if NORFLASH_ENABLE
	case FLASH_DEV_EXTNOR:
		res = norf_nolockProgram(nAdr, pData, nLen);
		break;
#endif
	default:
		res = SYS_R_EMPTY;
		break;
	}
	return res;
}


#if FLASH_ENABLE

sys_res flash_Erase(uint_t nDev, adr_t nAdr)
{
	sys_res res;

	switch (nDev) {
#if INTFLASH_ENABLE
	case FLASH_DEV_INT:
		res = intf_Erase(nAdr);
		break;
#endif
#if NORFLASH_ENABLE
	case FLASH_DEV_EXTNOR:
		res = norf_Erase(nAdr);
		break;
#endif
	default:
		res = SYS_R_EMPTY;
		break;
	}
	return res;
}

sys_res flash_Program(uint_t nDev, adr_t nAdr, const void *pData, uint_t nLen)
{
	sys_res res;

	switch (nDev) {
#if INTFLASH_ENABLE
	case FLASH_DEV_INT:
		res = intf_Program(nAdr, pData, nLen);
		break;
#endif
#if NORFLASH_ENABLE
	case FLASH_DEV_EXTNOR:
		res = norf_Program(nAdr, pData, nLen);
		break;
#endif
	default:
		res = SYS_R_EMPTY;
		break;
	}
	return res;
}




//Private Defines
#define FLASH_LOCK_ENABLE		(1 && OS_TYPE)

#define FLASH_BLOCK_INVALID		(-1)


//Private Typedefs
typedef struct {
	int		type;
	int		sector;
	time_t	dirty;
	uint8_t	fbuf[SPIF_SEC_SIZE];
}t_flash_buffer, *p_flash_buffer;



//Private Variables
#if FLASH_LOCK_ENABLE
static os_sem flash_sem;
#endif
static t_flash_buffer flash_buf;



//Private Macros
#if FLASH_LOCK_ENABLE
#define flash_Lock()				rt_sem_take(&flash_sem, RT_WAITING_FOREVER)
#define flash_Unlock()				rt_sem_release(&flash_sem)
#else
#define flash_Lock()
#define flash_Unlock()
#endif


//Internal Functions
static void _flash_Flush(uint_t nDelay)
{
	p_flash_buffer p = &flash_buf;
	adr_t nAdr;
	time_t tTime;

	if (p->sector == FLASH_BLOCK_INVALID)
		return;
	if (p->dirty == 0)
		return;
	tTime = rtc_GetTimet();
	if (p->dirty > tTime) {
		nDelay = 0;
	} else {
		if ((tTime - p->dirty) >= nDelay)
			nDelay = 0;
	}
	if (nDelay == 0) {
		switch (p->type) {
#if INTFLASH_ENABLE
		case FLASH_DEV_INT:
			nAdr = p->sector * INTFLASH_BLK_SIZE;
			if ((nAdr >= INTFLASH_BASE_ADR) && (nAdr < (INTFLASH_BASE_ADR + INTFLASH_SIZE))) {
				intf_Erase(nAdr);
				intf_Program(nAdr, p->fbuf, INTFLASH_BLK_SIZE);
			}
			break;
#endif
#if NORFLASH_ENABLE
		case FLASH_DEV_EXTNOR:
			nAdr = p->sector * NORFLASH_BLK_SIZE;
			norf_Erase(nAdr);
			norf_Program(nAdr, p->fbuf, NORFLASH_BLK_SIZE);
			break;
#endif
#if SPIFLASH_ENABLE
		case FLASH_DEV_SPINOR:
#if SPIF_PROTECT_ENABLE
			spif_SecErase(SPIF_PROTECT_SEC);
			spif_Program(SPIF_PROTECT_SEC, p->fbuf);
			sfs_Write(&spif_IdxDev, 1, &p->sector, sizeof(p->sector));
#endif
			spif_SecErase(p->sector);
			spif_Program(p->sector, p->fbuf);
#if SPIF_PROTECT_ENABLE
			sfs_Delete(&spif_IdxDev, 1);
#endif
			break;
#endif
		default:
			break;
		}
		p->dirty = 0;
	}
}








void flash_Init()
{
	p_flash_buffer p = &flash_buf;

#if FLASH_LOCK_ENABLE
	rt_sem_init(&flash_sem, "sem_fls", 1, RT_IPC_FLAG_FIFO);
#endif
#if SPIF_PROTECT_ENABLE
	if (sfs_Read(&spif_IdxDev, 0, NULL) != SYS_R_OK) {
		sfs_Init(&spif_IdxDev);
		sfs_Write(&spif_IdxDev, 0, NULL, 0);
	}
	if (sfs_Read(&spif_IdxDev, 1, &p->sector) == SYS_R_OK) {
		spif_ReadLen(SPIF_PROTECT_SEC, 0, p->fbuf, SPIF_SEC_SIZE);
		spif_SecErase(p->sector);
		spif_Program(p->sector, p->fbuf);
		sfs_Delete(&spif_IdxDev, 1);
	}
#endif
	p->type = FLASH_DEV_NULL;
	p->sector = FLASH_BLOCK_INVALID;
	p->dirty = 0;
}

#if INTFLASH_ENABLE
void intf_Read(uint_t nAdr, void *pBuf, uint_t nLen)
{
	p_flash_buffer p = &flash_buf;
	uint8_t *pData, *pTemp;
	uint_t nSec, nOffset, nRead;

	flash_Lock();
	for (pData = (uint8_t *)pBuf; nLen; nLen -= nRead, pData += nRead, nAdr += nRead) {
		nSec = nAdr / INTFLASH_BLK_SIZE;
		nOffset = nAdr % INTFLASH_BLK_SIZE;
		nRead = MIN(INTFLASH_BLK_SIZE - nOffset, nLen);
		if ((p->type == FLASH_DEV_INT) && (nSec == p->sector)) {
			pTemp = p->fbuf + nOffset;
			memcpy(pData, pTemp, nRead);
		} else
			memcpy(pData, (void *)nAdr, nRead);
	}
	flash_Unlock();
}

void intf_Write(uint_t nAdr, const void *pBuf, uint_t nLen)
{
	p_flash_buffer p = &flash_buf;
	uint8_t *pData, *pTemp;
	uint_t nSec, nOffset, nWrite;

	flash_Lock();
	for (pData = (uint8_t *)pBuf; nLen; nLen -= nWrite, pData += nWrite, nAdr += nWrite) {
		nSec = nAdr / INTFLASH_BLK_SIZE;
		nOffset = nAdr % INTFLASH_BLK_SIZE;
		nWrite = MIN(INTFLASH_BLK_SIZE - nOffset, nLen);
		if ((p->type != FLASH_DEV_INT) || (nSec != p->sector)) {
			_flash_Flush(0);
			memcpy(p->fbuf, (void *)(nSec * INTFLASH_BLK_SIZE), INTFLASH_BLK_SIZE);
			p->sector = nSec;
			p->type = FLASH_DEV_INT;
		}
		pTemp = p->fbuf + nOffset;
		memcpy(pTemp, pData, nWrite);
		p->dirty = rtc_GetTimet();
	}
	flash_Unlock();
}
#endif



#if SPIFLASH_ENABLE
#if SPIFS_ENABLE
void spif_SecRead(uint_t nSec, void *pBuf)
{
	p_flash_buffer p = &flash_buf;

	flash_Lock();
	if ((p->type == FLASH_DEV_SPINOR) && (p->sector == nSec))
		memcpy(pBuf, p->fbuf, SPIF_SEC_SIZE);
	else
		spif_ReadLen(nSec, 0, pBuf, SPIF_SEC_SIZE);
	flash_Unlock();
}

void spif_SecWrite(uint_t nSec, const void *pBuf)
{
	p_flash_buffer p = &flash_buf;
	uint32_t *pLast, *pData;

	flash_Lock();
	if ((p->type == FLASH_DEV_SPINOR) && (p->sector == nSec)) {
		memcpy(p->fbuf, pBuf, SPIF_SEC_SIZE);
		p->dirty = rtc_GetTimet();
	} else {
		pData = (uint32_t *)pBuf;
		pLast = pData + (SPIF_SEC_SIZE / 4);
		while (pData < pLast)
			if (*pData++ != 0xFFFFFFFF)
				break;
		if (pData < pLast) {
			_flash_Flush(0);
			memcpy(p->fbuf, pBuf, SPIF_SEC_SIZE);
			p->sector = nSec;
			p->type = FLASH_DEV_SPINOR;
			p->dirty = rtc_GetTimet();
		} else
			spif_SecErase(nSec);
	}
	flash_Unlock();
}
#endif

void spif_Read(uint_t nAdr, void *pBuf, uint_t nLen)
{
	p_flash_buffer p = &flash_buf;
	uint8_t *pData, *pTemp;
	uint_t nSec, nOffset, nRead;

	flash_Lock();
	for (pData = (uint8_t *)pBuf; nLen; nLen -= nRead, pData += nRead, nAdr += nRead) {
		nSec = nAdr / SPIF_SEC_SIZE;
		nOffset = nAdr % SPIF_SEC_SIZE;
		nRead = MIN(SPIF_SEC_SIZE - nOffset, nLen);
		if ((p->type == FLASH_DEV_SPINOR) && (nSec == p->sector)) {
			pTemp = p->fbuf + nOffset;
			memcpy(pData, pTemp, nRead);
		} else
			spif_ReadLen(nSec, nOffset, pData, nRead);
	}
	flash_Unlock();
}

void spif_Write(uint_t nAdr, const void *pBuf, uint_t nLen)
{
	p_flash_buffer p = &flash_buf;
	uint8_t *pData, *pTemp;
	uint_t nSec, nOffset, nWrite;

	flash_Lock();
	for (pData = (uint8_t *)pBuf; nLen; nLen -= nWrite, pData += nWrite, nAdr += nWrite) {
		nSec = nAdr / SPIF_SEC_SIZE;
		nOffset = nAdr % SPIF_SEC_SIZE;
		nWrite = MIN(SPIF_SEC_SIZE - nOffset, nLen);
		if ((p->type != FLASH_DEV_SPINOR) || (nSec != p->sector)) {
			_flash_Flush(0);
			spif_ReadLen(nSec, 0, p->fbuf, SPIF_SEC_SIZE);
			p->sector = nSec;
			p->type = FLASH_DEV_SPINOR;
		}
		pTemp = p->fbuf + nOffset;
		memcpy(pTemp, pData, nWrite);
		p->dirty = rtc_GetTimet();
	}
	flash_Unlock();
}

void spif_Fill(uint_t nAdr, uint_t nEnd, uint_t nVal)
{
	uint_t nLen, nFill;
	uint8_t aBuf[64];

	memset(aBuf, nVal, sizeof(aBuf));
	nLen = nEnd - nAdr;
	for (; nLen; nLen -= nFill, nAdr += nFill) {
		nFill = MIN(sizeof(aBuf), nLen);
		spif_Write(nAdr, aBuf, nFill);
	}
}
#endif


void flash_Flush(uint_t nDelay)
{

	flash_Lock();
	_flash_Flush(nDelay);
	flash_Unlock();
}


#endif

