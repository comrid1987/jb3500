

//Private Variables
static t_dev_spi dev_Spi[BSP_SPI_QTY];




//-------------------------------------------------------------------------
//Function Name  : 
//Description    : 
//Input          : - GPIOx:
//                - GPIO_InitStruct: 
//Output         : None
//Return         : None
//-------------------------------------------------------------------------
p_dev_spi spi_Get(uint_t nId, int nTmo)
{
	p_dev_spi p = &dev_Spi[nId];

#if OS_TYPE
	if (dev_Get(p->parent, nTmo) != SYS_R_OK)
		return NULL;
#endif
	return p;
}

//-------------------------------------------------------------------------
//Function Name  : 
//Description    : 
//Input          : - GPIOx:
//                - GPIO_InitStruct: 
//Output         : None
//Return         : None
//-------------------------------------------------------------------------
sys_res spi_Release(p_dev_spi p)
{

#if OS_TYPE
	return dev_Release(p->parent);
#else
	return SYS_R_OK;
#endif
}

//-------------------------------------------------------------------------
//Function Name  : 
//Description    : 
//Input          : - GPIOx:
//                - GPIO_InitStruct: 
//Output         : None
//Return         : None
//-------------------------------------------------------------------------
sys_res spi_Config(p_dev_spi p, uint_t nSckMode, uint_t nLatch, uint_t nSpeed)
{

	p->sckmode = nSckMode;
	p->latchmode = nLatch;
	p->speed = nSpeed;
#if SPI_SOFTWARE
	spibus_Config(p);
#else
	arch_SpiConfig(p);
#endif
	return SYS_R_OK;
}

#if SPI_SEL_ENABLE
void spi_CsSel(p_dev_spi p, uint_t nId)
{

	if (p->csid != nId) {
		p->csid = nId;
#if HC138_ENABLE
		hc138_Set(p->csid);
#endif
	}
}
#endif


sys_res spi_Start(p_dev_spi p)
{

#if OS_TYPE
	while (p->ste != SPI_S_IDLE)
		os_thd_Slp1Tick();
#endif
	p->ste = SPI_S_BUSY;
#if SPI_SOFTWARE
	spibus_Start(p);
#else
#endif
	return SYS_R_OK;
}

sys_res spi_SendChar(p_dev_spi p, uint_t nData)
{

#if SPI_SOFTWARE
	spibus_SendChar(p, nData);
#else
#endif
	return SYS_R_OK;
}

sys_res spi_End(p_dev_spi p)
{

	if (p->ste == SPI_S_BUSY) {
		p->ste = SPI_S_IDLE;
#if SPI_SOFTWARE
		spibus_End(p);
#else
#endif
		return SYS_R_OK;
	}
	return SYS_R_ERR;
}



//-------------------------------------------------------------------------
//Function Name  : 
//Description    : 
//Input          : - GPIOx:
//                - GPIO_InitStruct: 
//Output         : None
//Return         : None
//-------------------------------------------------------------------------
sys_res spi_Send(p_dev_spi p, const void *pData, uint_t nLen)
{
	sys_res res;

#if OS_TYPE
	while (p->ste != SPI_S_IDLE) {
		os_thd_Slp1Tick();
	}
#endif
	p->ste = SPI_S_BUSY;
#if SPI_SOFTWARE
	res = spibus_Send(p, pData, nLen);
#else
	res = arch_SpiSend(p, pData, nLen);
#endif
	p->ste = SPI_S_IDLE;
	return res;
}

//-------------------------------------------------------------------------
//Function Name  : 
//Description    : 
//Input          : - GPIOx:
//                - GPIO_InitStruct: 
//Output         : None
//Return         : None
//-------------------------------------------------------------------------
sys_res spi_Recv(p_dev_spi p, void *pRec, uint_t nLen)
{
 	sys_res res;

#if OS_TYPE
	while (p->ste != SPI_S_IDLE) {
		os_thd_Slp1Tick();
	}
#endif
	p->ste = SPI_S_BUSY;
#if SPI_SOFTWARE
	res = spibus_Recv(p, pRec, nLen);
#else
	res = arch_SpiRecv(p, pRec, nLen);
#endif
	p->ste = SPI_S_IDLE;
	return res;
}

//-------------------------------------------------------------------------
//Function Name  : 
//Description    : 
//Input          : - GPIOx:
//                - GPIO_InitStruct: 
//Output         : None
//Return         : None
//-------------------------------------------------------------------------
sys_res spi_Transce(p_dev_spi p, const void *pCmd, uint_t nCmdLen, void *pRec, uint_t nRecLen)
{
 	sys_res res;

#if OS_TYPE
	while (p->ste != SPI_S_IDLE) {
		os_thd_Slp1Tick();
	}
#endif
	p->ste = SPI_S_BUSY;
#if SPI_SOFTWARE
	res = spibus_Transce(p, pCmd, nCmdLen, pRec, nRecLen);
#else
	res = arch_SpiTransce(p, pCmd, nCmdLen, pRec, nRecLen);
#endif
	p->ste = SPI_S_IDLE;
	return res;
}

//-------------------------------------------------------------------------
//Function Name  : 
//Description    : 
//Input          : - GPIOx:
//                - GPIO_InitStruct: 
//Output         : None
//Return         : None
//-------------------------------------------------------------------------
sys_res spi_TranChar(p_dev_spi p, uint_t nSend, void *pRec)
{
 	sys_res res;

#if OS_TYPE
	while (p->ste != SPI_S_IDLE) {
		os_thd_Slp1Tick();
	}
#endif
	p->ste = SPI_S_BUSY;
#if SPI_SOFTWARE
	res = spibus_TranChar(p, nSend, pRec);
#else
#endif
	p->ste = SPI_S_IDLE;
	return res;
}


