

//In litecore.c
#if IO_BUF_TYPE == BUF_T_DQUEUE
extern dque dqueue;
#endif


//Private Defines



//Private Variables
static t_dev_uart dev_Uart[BSP_UART_QTY];



//-------------------------------------------------------------------------
//Internal Functions
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
static int uart_IsRxBufNE(p_dev_uart p)
{
#if IO_BUF_TYPE == BUF_T_BUFFER

	if (p->bufrx->len)
		return 1;
	return 0;
#elif IO_BUF_TYPE == BUF_T_DQUEUE

	if (dque_IsNotEmpty(dqueue, p->parent->id | UART_DQUE_RX_CHL))
		return 1;
	return 0;
#endif
}


//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
static int uart_IsTxBufNE(p_dev_uart p)
{
#if IO_BUF_TYPE == BUF_T_BUFFER
	
		if (p->buftx->len)
			return 1;
		return 0;
#elif IO_BUF_TYPE == BUF_T_DQUEUE
	
		if (dque_IsNotEmpty(dqueue, p->parent->id | UART_DQUE_TX_CHL))
			return 1;
		return 0;
#endif
}




//-------------------------------------------------------------------------
//External Functions
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
void uart_Init(p_dev_uart p)
{

#if IO_BUF_TYPE == BUF_T_BUFFER
	bzero(p->bufrx, sizeof(buf));
	bzero(p->buftx, sizeof(buf));
#endif
	switch (p->def->type) {
#if SC16IS7X_ENABLE
	case UART_T_SC16IS7X:
		break;
#endif
#if VK321X_ENABLE
	case UART_T_VK321X:
		break;
#endif
#if SWUART_ENABLE
	case UART_T_TIMER:
		swuart_Init(p);
		break;
#endif
	default:
		arch_UartInit(p);
		break;
	}
}

//-------------------------------------------------------------------------
//Function Name  : 
//Description    : 获得串口指针,串口号,超时
//Input          : - GPIOx:
//                - GPIO_InitStruct: 
//Output         : None
//Return         : None
//-------------------------------------------------------------------------
p_dev_uart uart_Get(uint_t nId, int nTmo)
{
	p_dev_uart p;

	if (nId < BSP_UART_QTY) {
		p = &dev_Uart[nId];
		if (dev_Get(p->parent, nTmo) == SYS_R_OK)
			return p;
	}
	return NULL;
}

//-------------------------------------------------------------------------
//Function Name  : 
//Description    : 
//Input          : - GPIOx:
//                - GPIO_InitStruct: 
//Output         : None
//Return         : None
//-------------------------------------------------------------------------
sys_res uart_Release(p_dev_uart p)
{

	return dev_Release(p->parent);
}


//-------------------------------------------------------------------------
//Function Name  : 
//Description    : 
//Input          : - GPIOx:
//                - GPIO_InitStruct: 
//Output         : None
//Return         : None
//-------------------------------------------------------------------------
sys_res uart_Config(p_dev_uart p, uint_t nBaud, uint_t nPari, uint_t nData, uint_t nStop)
{
	sys_res res;
	t_uart_para xPara;
	p_uart_def pDef;
	
	xPara.baud = nBaud;
	xPara.pari = nPari;
	xPara.data = nData;
	xPara.stop = nStop;

	if (memcmp(&p->para, &xPara, sizeof(p->para))) {
		memcpy(&p->para, &xPara, sizeof(p->para));
		pDef = p->def;
		switch (pDef->type) {
#if SC16IS7X_ENABLE
		case UART_T_SC16IS7X:
			res = sc16is7x_UartConfig(pDef->id, &p->para);
			break;
#endif
#if VK321X_ENABLE
		case UART_T_VK321X:
			res = vk321x_UartConfig(pDef->id, &p->para);
			break;
#endif
#if SWUART_ENABLE
		case UART_T_TIMER:
			res = swuart_Open(pDef->id, &p->para);
			break;
#endif
		default:
			res = arch_UartOpen(pDef->id, &p->para);
			break;
		}
		return res;
	}
	return SYS_R_OK;
}


//-------------------------------------------------------------------------
//Function Name  : 
//Description    : 
//Input          : - GPIOx:
//                - GPIO_InitStruct: 
//Output         : None
//Return         : None
//-------------------------------------------------------------------------
sys_res uart_Send(p_dev_uart p, const void *pData, uint_t nLen)
{
	const uint8_t *pBuf;

	switch (p->def->type) {
#if SC16IS7X_ENABLE
	case UART_T_SC16IS7X:
		sc16is7x_UartSend(p->def->id, pData, nLen);
		break;
#endif
#if VK321X_ENABLE
	case UART_T_VK321X:
		vk321x_UartSend(p->def->id, pData, nLen);
		break;
#endif
#if SWUART_ENABLE
	case UART_T_TIMER:
#if IO_BUF_TYPE == BUF_T_BUFFER
		buf_Push(p->buftx, pData, nLen);
#elif IO_BUF_TYPE == BUF_T_DQUEUE
		dque_Push(dqueue, p->parent->id | UART_DQUE_TX_CHL, pData, nLen);
#endif
		swuart_TxStart(p->def->id);
		break;
#endif
	default:
		if (p->def->txmode == UART_MODE_IRQ) {
#if IO_BUF_TYPE == BUF_T_BUFFER
			buf_Push(p->buftx, pData, nLen);
#elif IO_BUF_TYPE == BUF_T_DQUEUE
			dque_Push(dqueue, p->parent->id | UART_DQUE_TX_CHL, pData, nLen);
#endif
			arch_UartTxIEnable(p->def->id);
		} else {
			pBuf = (const uint8_t *)pData;
			for (; nLen; nLen--) {
				arch_UartSendChar(p->def->id, *pBuf++);
			}
		}
		break;
	}	
	return SYS_R_OK;
}


//-------------------------------------------------------------------------
//Function Name  : 
//Description    : 
//Input          : - GPIOx:
//                - GPIO_InitStruct: 
//Output         : None
//Return         : None
//-------------------------------------------------------------------------
sys_res uart_RecData(p_dev_uart p, buf b, int nTmo)
{
 
	for (nTmo /= OS_TICK_MS; ; nTmo--) {
		if (uart_GetRxData(p, b) > 0)
			return SYS_R_OK;
		if (nTmo == 0)
			return SYS_R_TMO;
		os_thd_Slp1Tick();
	}
}


//-------------------------------------------------------------------------
//Function Name  : 
//Description    : 
//Input          : - GPIOx:
//                - GPIO_InitStruct: 
//Output         : None
//Return         : None
//-------------------------------------------------------------------------
sys_res uart_RecLength(p_dev_uart p, buf b, uint_t nLen, int nTmo)
{

	for (nTmo /= OS_TICK_MS; ; nTmo--) {
		uart_GetRxData(p, b);
		if (b->len >= nLen)
			return SYS_R_OK;
		if (nTmo == 0)
			return SYS_R_TMO;
		os_thd_Slp1Tick();
	}
}


//-------------------------------------------------------------------------
//Function Name  : 
//Description    : 
//Input          : - GPIOx:
//                - GPIO_InitStruct: 
//Output         : None
//Return         : None
//-------------------------------------------------------------------------
int uart_GetRxData(p_dev_uart p, buf b)
{
#if IO_BUF_TYPE == BUF_T_BUFFER
	uint_t nLen = p->bufrx->len;

	if (nLen) {
		buf_Push(b, p->bufrx->p, nLen);
		buf_Remove(p->bufrx, nLen);
	}
	return nLen;
#elif IO_BUF_TYPE == BUF_T_DQUEUE

	return dque_Pop(dqueue, p->parent->id | UART_DQUE_RX_CHL, b);
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
int uart_GetRxChar(p_dev_uart p)
{
#if IO_BUF_TYPE == BUF_T_BUFFER
	int nData;

	if (p->bufrx->len) {
		nData = p->bufrx->p[0];
		buf_Remove(p->bufrx, 1);
		return nData;
	}
	return -1;
#elif IO_BUF_TYPE == BUF_T_DQUEUE

	return dque_PopChar(dqueue, p->parent->id | UART_DQUE_RX_CHL);
#endif
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
#if SMARTCARD_ENABLE
sys_res uart_ScReset(p_dev_uart p, uint_t nHL)
{
	p_uart_def pDef;

	pDef = p->def;
	if (pDef->fun != UART_FUN_SC)
		return SYS_R_ERR;
	arch_GpioSet(pDef->fport, pDef->fpin, nHL);
	return SYS_R_OK;
}
#endif

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
void uart_Maintain()
{
	p_dev_uart p;
	p_uart_def pDef;
	
	for (p = dev_Uart; p < ARR_ENDADR(dev_Uart); p++) {
		if (uart_IsTxBufNE(p)) {
			pDef = p->def;
			if (pDef->txmode == UART_MODE_IRQ) {
				switch (pDef->type) {
#if SWUART_ENABLE
#if SWUART_RX_MODE == SWUART_RX_M_EINT
				case UART_T_TIMER:
					swuart_TxStart(pDef->id);
					break;
#endif
#endif
				default:
					arch_UartTxIEnable(pDef->id);
					break;
				}
			}
		}
		if (uart_IsRxBufNE(p)) {
			p->parent->idle += 1;
			if (p->parent->idle > 100) {
				p->parent->idle = 0;
#if IO_BUF_TYPE == BUF_T_BUFFER
				buf_Release(p->bufrx);
#elif IO_BUF_TYPE == BUF_T_DQUEUE
				dque_Clear(dqueue, p->parent->id | UART_DQUE_RX_CHL);
#endif
			}
		} else {
			p->parent->idle = 0;
		}
	}
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
void uart_Reopen()
{
	p_dev_uart p;
	t_uart_para xPara;

	for (p = dev_Uart; p < ARR_ENDADR(dev_Uart); p++) {
		memcpy(&xPara, &p->para, sizeof(t_uart_para));
		memset(&p->para, 0, sizeof(t_uart_para));
		uart_Config(p, xPara.baud, xPara.pari, xPara.data, xPara.stop);
	}
}


