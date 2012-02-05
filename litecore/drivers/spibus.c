#include <drivers/spibus.h>



//Private Definds
#define SPIBUS_LOCK_ENABLE		(0 && OS_TYPE)


//Private Macros
#if SPIBUS_LOCK_ENABLE
#define spibus_Lock()			os_thd_Lock()
#define spibus_Unlock()			os_thd_Unlock()
#else
#define spibus_Lock()
#define spibus_Unlock()
#endif




static void spibus_Delay(p_dev_spi p)
{

	if (p->speed)
		sys_Delay(MCU_CLOCK / p->speed / 2);
}

static void spibus_Sck(p_dev_spi p, uint_t nOut)
{
	p_gpio_def pIO = tbl_bspSpiDef[p->parent->id];

	if (p->sckmode == SPI_SCKIDLE_HIGH) {
		if (nOut)
			nOut = 0;
		else
			nOut = 1;
	}
	arch_GpioSet(pIO->port, pIO->pin, nOut);
	spibus_Delay(p);
}

static void spibus_Mosi(p_dev_spi p, uint_t nHL)
{
	p_gpio_def pIO = tbl_bspSpiDef[p->parent->id] + 1;

	arch_GpioSet(pIO->port, pIO->pin, nHL);
}

static uint_t spibus_Miso(p_dev_spi p)
{
	p_gpio_def pIO = tbl_bspSpiDef[p->parent->id] + 2;

	return arch_GpioRead(pIO->port, pIO->pin);
}

static void spibus_CeLow(p_dev_spi p)
{
	p_gpio_def pIO = tbl_bspSpiDef[p->parent->id] + 3;

	arch_GpioSet(pIO->port, pIO->pin, 0);
}

static void spibus_CeHigh(p_dev_spi p)
{
	p_gpio_def pIO = tbl_bspSpiDef[p->parent->id] + 3;

	arch_GpioSet(pIO->port, pIO->pin, 1);
}


static void _spibus_Recv(p_dev_spi p, uint8_t *pRec, uint_t nLen)
{
	uint_t i, nData;

	for (; nLen; nLen--) {
		nData = 0;
		for (i = 8; i; i--) {
			nData <<= 1;
			if (p->latchmode == SPI_LATCH_1EDGE)
				if (spibus_Miso(p))
					nData |= 1;
			spibus_Sck(p, 1);
			if (p->latchmode == SPI_LATCH_2EDGE)
				if (spibus_Miso(p))
					nData |= 1;
			spibus_Sck(p, 0);
		}
		*pRec++ = nData;
	}
}










//-------------------------------------------------------------------
//External Functions
//-------------------------------------------------------------------

sys_res spibus_Init(p_dev_spi p)
{
	tbl_gpio_def pSck = tbl_bspSpiDef[p->parent->id];

	sys_GpioConf(pSck);
	sys_GpioConf(pSck + 1);
	sys_GpioConf(pSck + 2);
#if SPI_CE_ENABLE
	sys_GpioConf(pSck + 3);
#endif
	return SYS_R_OK;
}

sys_res spibus_Config(p_dev_spi p)
{

	spibus_Sck(p, 0);
	return SYS_R_OK;
}


void spibus_Start(p_dev_spi p)
{

	spibus_Lock();
#if SPI_CE_ENABLE
	spibus_CeLow(p);
#endif
}

void spibus_SendChar(p_dev_spi p, uint_t nData)
{
	uint_t i;

	for (i = 8; i; i--, nData <<= 1) {
		if (p->latchmode == SPI_LATCH_1EDGE)
			spibus_Mosi(p, nData & 0x80);
		spibus_Sck(p, 1);
		if (p->latchmode == SPI_LATCH_2EDGE)
			spibus_Mosi(p, nData & 0x80);
		spibus_Sck(p, 0);
	}
}

void spibus_End(p_dev_spi p)
{

#if SPI_CE_ENABLE
	spibus_CeHigh(p);
#endif
	spibus_Unlock();
}


sys_res spibus_Send(p_dev_spi p, const void *pData, uint_t nLen)
{
	uint8_t *pBuf = (uint8_t *)pData;

	spibus_Start(p);
	for (; nLen; nLen--)
		spibus_SendChar(p, *pBuf++);
	spibus_End(p);
	return SYS_R_OK;
}

sys_res spibus_Recv(p_dev_spi p, void *pRec, uint_t nLen)
{

	spibus_Start(p);
	_spibus_Recv(p, pRec, nLen);
	spibus_End(p);
	return SYS_R_OK;
}

sys_res spibus_Transce(p_dev_spi p, const void *pCmd, uint_t nCmdLen, void *pRec, uint_t nRecLen)
{
	uint8_t *pBuf = (uint8_t *)pCmd;

	spibus_Start(p);
	for (; nCmdLen; nCmdLen--)
		spibus_SendChar(p, *pBuf++);
	_spibus_Recv(p, pRec, nRecLen);
	spibus_End(p);
	return SYS_R_OK;
}




