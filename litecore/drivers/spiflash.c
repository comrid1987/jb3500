#include <drivers/spiflash.h>


//Private Defines
#define SPIF_LOCK_ENABLE		(0 && OS_TYPE)

#define SPIF_PAGE_SIZE			256

#define SPIF_JEDEC_MX25L16		0x1520C2
#define SPIF_JEDEC_MX25L32		0x1620C2
#define SPIF_JEDEC_MX25L64		0x1720C2
#define SPIF_JEDEC_SST25VF03	0x4A25BF

#define SPIF_T_NULL				0
#define SPIF_T_MX25LXX			1
#define SPIF_T_SST25VFXX		2


//Command Defines
#define SPIF_CMD_READ_LS		0x03
#define SPIF_CMD_READ_HS		0x0B

#define SPIF_CMD_PAGE_PROG		0x02

#define SPIF_CMD_ERASE_4KB		0x20
#define SPIF_CMD_ERASE_32KB		0x52
#define SPIF_CMD_ERASE_64KB		0xD8

#define SPIF_CMD_WRITE			0xAD

#define SPIF_CMD_READ_STATUS	0x05
#define SPIF_CMD_WRITE_STATUS	0x01
#define SPIF_CMD_WRSR_ENABLE	0x50

#define SPIF_CMD_WRITE_EN		0x06
#define SPIF_CMD_WRITE_DIS		0x04

#define SPIF_CMD_JEDECID		0x9F


//Status Register Bits Defines
#define SPIF_SR_BUSY			0x01
#define SPIF_SR_WRITE_EN		0x02





//Private Typedefs
typedef struct {
	int		size;
	uint8_t	type[BSP_SPIF_QTY];
#if SPI_SEL_ENABLE
	uint8_t	csid[BSP_SPIF_QTY];
#endif
}t_spif_info, *p_spif_info;



//Private Variables
#if SPIF_LOCK_ENABLE
static os_sem spif_sem;
#endif
static t_spif_info spif_info;
static uint32_t spif_dbgJedec[BSP_SPIF_QTY];

//Private Macros
#if SPIF_LOCK_ENABLE
#define spif_Lock()				rt_sem_take(&spif_sem, RT_WAITING_FOREVER)
#define spif_Unlock()			rt_sem_release(&spif_sem)
#else
#define spif_Lock()
#define spif_Unlock()
#endif




//Internal Functions
static p_dev_spi _spif_SpiGet()
{
	p_dev_spi p;
	
	p = spi_Get(SPIF_COMID, OS_TMO_FOREVER);
	spi_Config(p, SPI_SCKIDLE_LOW, SPI_LATCH_1EDGE, 0);
	return p;
}

static void _spif_WaitIdle(p_dev_spi p)
{
	uint_t nTmo, nSte = SPIF_SR_BUSY, nCmd = SPIF_CMD_READ_STATUS;

	for (nTmo = 50000; (nSte & SPIF_SR_BUSY) && nTmo; nTmo--)
		spi_Transce(p, &nCmd, 1, &nSte, 1);
}

static void _spif_WriteEnable(p_dev_spi p)
{
	uint_t nCmd = SPIF_CMD_WRITE_EN;

	spi_Send(p, &nCmd, 1);
}

static void _spif_WriteDisable(p_dev_spi p)
{
	uint_t nCmd = SPIF_CMD_WRITE_DIS;

	spi_Send(p, &nCmd, 1);
}


static uint32_t _spif_Probe(p_dev_spi p)
{
	uint_t nCmd = SPIF_CMD_JEDECID;
	uint32_t nId = 0;

	_spif_WriteDisable(p);
	if (spi_Transce(p, &nCmd, 1, &nId, 3) == SYS_R_OK) {
		nCmd = SPIF_CMD_WRSR_ENABLE;
		spi_Send(p, &nCmd, 1);
		nCmd = SPIF_CMD_WRITE_STATUS;
		spi_Send(p, &nCmd, 2);
		return nId;
	}
	return 0;
}

static sys_res _spif_Read(p_dev_spi p, adr_t adr, void *pData, uint_t nLen)
{
	sys_res res;
	uint8_t aBuf[5];

	aBuf[0] = SPIF_CMD_READ_HS;
	aBuf[1] = adr >> 16;
	aBuf[2] = adr >> 8;
	aBuf[3] = adr;
	spi_Transce(p, aBuf, sizeof(aBuf), pData, nLen);
	return res;
}

static sys_res _spif_Erase(p_dev_spi p, adr_t adr)
{
	sys_res res;
	uint8_t aBuf[4];

	_spif_WriteEnable(p);
	aBuf[0] = SPIF_CMD_ERASE_4KB;
	aBuf[1] = adr >> 16;
	aBuf[2] = adr >> 8;
	aBuf[3] = adr;
	spi_Send(p, aBuf, sizeof(aBuf));
	_spif_WaitIdle(p);
	return res;
}

static sys_res _spif_Program_AAI(p_dev_spi p, adr_t adr, const void *pData, uint_t nLen)
{
	uint8_t aBuf[6], *pBuf, *pEnd;

	_spif_WriteEnable(p);
	pBuf = (uint8_t *)pData;
	pEnd = pBuf + nLen;
	aBuf[0] = SPIF_CMD_WRITE;
	aBuf[1] = adr >> 16;
	aBuf[2] = adr >> 8;
	aBuf[3] = adr;
	aBuf[4] = pBuf[0];
	aBuf[5] = pBuf[1];
	spi_Send(p, aBuf, sizeof(aBuf));
	_spif_WaitIdle(p);
	pBuf += 2;
	for (; pBuf < pEnd; pBuf += 2) {
		aBuf[1] = pBuf[0];
		aBuf[2] = pBuf[1];
		spi_Send(p, aBuf, 3);
		_spif_WaitIdle(p);
	}
	_spif_WriteDisable(p);
	return SYS_R_OK;
}

static sys_res _spif_Program_PP(p_dev_spi p, adr_t adr, const void *pData, uint_t nLen)
{
	uint8_t *pBuf, *pPageEnd, *pSecEnd;

	pBuf = (uint8_t *)pData;
	pSecEnd = pBuf + nLen;
	for (; pBuf < pSecEnd; adr += SPIF_PAGE_SIZE) {
		_spif_WriteEnable(p);
		spi_Start(p);
		spi_SendChar(p, SPIF_CMD_PAGE_PROG);
		spi_SendChar(p, adr >> 16);
		spi_SendChar(p, adr >> 8);
		spi_SendChar(p, adr);
		for (pPageEnd = pBuf + 256; pBuf < pPageEnd; )
			spi_SendChar(p, *pBuf++);
		spi_End(p);
		_spif_WaitIdle(p);
	}
	return SYS_R_OK;
}



//External Functions
void spif_Init()
{
	p_spif_info p = &spif_info;
	p_dev_spi pSpi;
	uint_t i, j, nRetry;
	uint32_t nJedecID;

#if SPIF_LOCK_ENABLE
	rt_sem_init(&spif_sem, "sem_sf0", 1, RT_IPC_FLAG_FIFO);
#endif
	spif_Lock();
	pSpi = _spif_SpiGet();
	p->size = 0;
	for (i = 0, j = 0; i < BSP_SPIF_QTY; i++) {
#if SPI_SEL_ENABLE
		spi_CsSel(pSpi, tbl_bspSpifCsid[i]);
#endif
		for (nRetry = 0; nRetry < 3; nRetry++) {
			nJedecID = _spif_Probe(pSpi);
			if (nJedecID == SPIF_JEDEC_MX25L32) {
				p->size += 1024;
#if SPI_SEL_ENABLE
				p->csid[j] = tbl_bspSpifCsid[i];
#endif
				p->type[j] = SPIF_T_MX25LXX;
				j += 1;
				break;
			}
			if (nJedecID == SPIF_JEDEC_MX25L64) {
				p->size += 2048;
#if SPI_SEL_ENABLE
				p->csid[j] = tbl_bspSpifCsid[i];
#endif
				p->type[j] = SPIF_T_MX25LXX;
				j += 1;
				break;
			}
			if (nJedecID == SPIF_JEDEC_SST25VF03) {
				p->size += 1024;
#if SPI_SEL_ENABLE
				p->csid[j] = tbl_bspSpifCsid[i];
#endif
				p->type[j] = SPIF_T_SST25VFXX;
				j += 1;
				break;
			}
#if OS_TYPE
			os_thd_Sleep(10);
#else
			sys_Delay(10000);
#endif
		}
		spif_dbgJedec[i] = nJedecID;
	}
	spi_Release(pSpi);
	spif_Unlock();
}

void spif_ReadLen(uint_t nSec, uint_t nOffset, void *pData, uint_t nLen)
{
	p_dev_spi pSpi;

	spif_Lock();
	pSpi = _spif_SpiGet();
#if SPI_SEL_ENABLE
	spi_CsSel(pSpi, spif_info.csid[nSec / SPIF_SEC_QTY]);
#endif
	_spif_Read(pSpi, (nSec & (SPIF_SEC_QTY - 1)) * SPIF_SEC_SIZE + nOffset, pData, nLen);
	spi_Release(pSpi);
	spif_Unlock();
}

void spif_SecErase(uint_t nSec)
{
	p_dev_spi pSpi;

	spif_Lock();
	pSpi = _spif_SpiGet();
#if SPI_SEL_ENABLE
	spi_CsSel(pSpi, spif_info.csid[nSec / SPIF_SEC_QTY]);
#endif
	_spif_Erase(pSpi, (nSec & (SPIF_SEC_QTY - 1)) * SPIF_SEC_SIZE);
	spi_Release(pSpi);
	spif_Unlock();
}

void spif_Program(uint_t nSec, const void *pData)
{
	p_spif_info p = &spif_info;
	p_dev_spi pSpi;

	spif_Lock();
	pSpi = _spif_SpiGet();
#if SPI_SEL_ENABLE
	spi_CsSel(pSpi, p->csid[nSec / SPIF_SEC_QTY]);
#endif
	switch (p->type[nSec / SPIF_SEC_QTY]) {
	case SPIF_T_MX25LXX:
		_spif_Program_PP(pSpi, (nSec & (SPIF_SEC_QTY - 1)) * SPIF_SEC_SIZE, pData, SPIF_SEC_SIZE);
		break;
	default:
		_spif_Program_AAI(pSpi, (nSec & (SPIF_SEC_QTY - 1)) * SPIF_SEC_SIZE, pData, SPIF_SEC_SIZE);
		break;
	}
	spi_Release(pSpi);
	spif_Unlock();
}

int spif_GetSize()
{

	return spif_info.size;
}

int dbg_SpifInfo(uint8_t *pBuf)
{
	p_spif_info p = &spif_info;
	uint_t nLen;
#if SPI_SEL_ENABLE
	uint_t i;
#endif

	nLen = sprintf((char *)pBuf, "\r\nSize: %d", p->size * SPIF_SEC_SIZE);
#if SPI_SEL_ENABLE
	for (i = 0; i < BSP_SPIF_QTY; i++)
		nLen += sprintf((char *)(pBuf + nLen), "\r\n%d %d %08X", p->csid[i], p->type[i], spif_dbgJedec[i]);
#endif
	return nLen;
}

