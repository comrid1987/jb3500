

//In litecore.c
extern t_dev_uart dev_Uart[BSP_UART_QTY];
#if IO_BUF_TYPE == BUF_T_DQUEUE
extern dque dqueue;
#endif


//Private Defines


//-------------------------------------------------------------------------
//Internal Functions
//-------------------------------------------------------------------------
#if UART_TXLOCK_ENABLE
static __inline void uart_Lock(p_dev_uart p)
{

	rt_sem_take(&p->semtx, RT_WAITING_FOREVER);
}

static __inline void uart_Unlock(p_dev_uart p)
{

	rt_sem_release(&p->semtx);
}
#else
#define uart_Lock(...)
#define uart_Unlock(...)
#endif



//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
static void uart_Init(p_dev_uart p)
{

#if UART_TXLOCK_ENABLE
	rt_sem_init(&p->semtx, "uart_tx", 1, RT_IPC_FLAG_FIFO);
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
#if IO_BUF_TYPE == BUF_T_BUFFER
	bzero(p->bufrx, sizeof(buf));
	bzero(p->buftx, sizeof(buf));
#endif
}








//-------------------------------------------------------------------------
//External Functions
//-------------------------------------------------------------------------
p_dev_uart uart_Dev(uint_t nId)
{

	if (nId < BSP_UART_QTY)
		return &dev_Uart[nId];
	return NULL;
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

	p = uart_Dev(nId);
	if (p != NULL) {
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
	
	xPara.baud = nBaud;
	xPara.pari = nPari;
	xPara.data = nData;
	xPara.stop = nStop;

	switch (p->def->type) {
#if SC16IS7X_ENABLE
	case UART_T_SC16IS7X:
		res = sc16is7x_UartConfig(p->def->id, &xPara);
		break;
#endif
#if VK321X_ENABLE
	case UART_T_VK321X:
		res = vk321x_UartConfig(p->def->id, &xPara);
		break;
#endif
#if SWUART_ENABLE
	case UART_T_TIMER:
		res = swuart_Open(p->def->id, &xPara);
		break;
#endif
	default:
		res = arch_UartOpen(p->def->id, &xPara);
		break;
	}
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
sys_res uart_Send(p_dev_uart p, const void *pData, uint_t nLen)
{

	uart_Lock(p);
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
		} else
			arch_UartSend(p->def->id, pData, nLen);
		break;
	}	
	uart_Unlock(p);
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
 
	nTmo /= OS_TICK_MS;
	do {
		if (uart_GetRxData(p, b) > 0)
			return SYS_R_OK;
		else
			os_thd_Slp1Tick();
	} while (nTmo--);
	return SYS_R_TMO;
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

	nTmo /= OS_TICK_MS;
	do {
		uart_GetRxData(p, b);
		if (b->len >= nLen)
			return SYS_R_OK;
		else
			os_thd_Slp1Tick();
	} while (nTmo--);
	return SYS_R_TMO;
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
void uart_RxBufClear(p_dev_uart p)
{
#if IO_BUF_TYPE == BUF_T_BUFFER
	
		buf_Release(p->bufrx);
#elif IO_BUF_TYPE == BUF_T_DQUEUE
	
		dque_Clear(dqueue, p->parent->id | UART_DQUE_RX_CHL);
#endif
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
sys_res uart_IsRxBufNE(p_dev_uart p)
{
#if IO_BUF_TYPE == BUF_T_BUFFER

	if (p->bufrx->len)
		return SYS_R_OK;
	return SYS_R_EMPTY;
#elif IO_BUF_TYPE == BUF_T_DQUEUE

	if (dque_IsNotEmpty(dqueue, p->parent->id | UART_DQUE_RX_CHL))
		return SYS_R_OK;
	return SYS_R_EMPTY;
#endif
}


//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
sys_res uart_IsTxBufNE(p_dev_uart p)
{
#if IO_BUF_TYPE == BUF_T_BUFFER
	
		if (p->buftx->len)
			return SYS_R_OK;
		return SYS_R_EMPTY;
#elif IO_BUF_TYPE == BUF_T_DQUEUE
	
		if (dque_IsNotEmpty(dqueue, p->parent->id | UART_DQUE_TX_CHL))
			return SYS_R_OK;
		return SYS_R_EMPTY;
#endif
}


//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
void uart_TxStart(p_uart_def p)
{

	if (p->txmode == UART_MODE_IRQ) {
		switch (p->type) {
#if SWUART_ENABLE
		case UART_T_TIMER:
			swuart_TxStart(p->id);
			break;
#endif
		default:
			arch_UartTxIEnable(p->id);
			break;
		}	
	}
}


//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
#if SMARTCARD_ENABLE
sys_res uart_ScReset(p_dev_uart p, uint_t nHL)
{

	if (p->def->fun != UART_FUN_SC)
		return SYS_R_ERR;
	arch_GpioSet(p->def->fport, p->def->fpin, nHL);
	return SYS_R_OK;
}
#endif


