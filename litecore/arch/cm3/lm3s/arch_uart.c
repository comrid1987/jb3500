#if UART_ENABLE


//In litecore.c
#if IO_BUF_TYPE == BUF_T_DQUEUE
extern dque dqueue;
#endif

//Private Defines
#define LM3S_UART_QTY		3

//Private Typedef



//Private Const
static uint_t const lm3s_tblUartBase[LM3S_UART_QTY] = {
	UART0_BASE,
	UART1_BASE,
	UART2_BASE
};
static uint_t const lm3s_tblUartSysCtl[LM3S_UART_QTY] = {
	SYSCTL_PERIPH_UART0,
	SYSCTL_PERIPH_UART1,
	SYSCTL_PERIPH_UART2
};
static uint_t const lm3s_tblUartInt[LM3S_UART_QTY] = {
	INT_UART0,
	INT_UART1,
	INT_UART2
};

//Private Variables
static p_dev_uart lm3s_uart_dev[LM3S_UART_QTY];




//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
void arch_UartInit(p_dev_uart p)
{
	t_uart_para xUart;

	lm3s_uart_dev[p->def->id] = p;

	/* enable UART clock */
    MAP_SysCtlPeripheralEnable(lm3s_tblUartSysCtl[p->def->id]);

	/* enable UART interrupt */
	MAP_IntEnable(lm3s_tblUartInt[p->def->id]);

	/* set UART pin */
	arch_GpioClockEnable(p->def->rxport);
	arch_GpioClockEnable(p->def->txport);
	switch (p->def->id) {
	case 1:
		switch (p->def->rxport) {
		case GPIO_P1:
			if (p->def->rxpin == 0) {
				MAP_GPIOPinConfigure(GPIO_PB0_U1RX);
				MAP_GPIOPinConfigure(GPIO_PB1_U1TX);
			} else {
				MAP_GPIOPinConfigure(GPIO_PB4_U1RX);
				MAP_GPIOPinConfigure(GPIO_PB5_U1TX);
			}
			break;
		case GPIO_P2:
			MAP_GPIOPinConfigure(GPIO_PC6_U1RX);
			MAP_GPIOPinConfigure(GPIO_PC7_U1TX);
			break;
		case GPIO_P3:
			if (p->def->rxpin == 0) {
				MAP_GPIOPinConfigure(GPIO_PD0_U1RX);
				MAP_GPIOPinConfigure(GPIO_PD1_U1TX);
			} else {
				MAP_GPIOPinConfigure(GPIO_PD2_U1RX);
				MAP_GPIOPinConfigure(GPIO_PD3_U1TX);
			}
			break;
		default:
			break;
		}
		break;
	case 2:
		switch (p->def->rxport) {
		case GPIO_P3:
#if ARCH_TYPE == ARCH_T_LM3S5X
			MAP_GPIOPinConfigure(GPIO_PD0_U2RX);
			MAP_GPIOPinConfigure(GPIO_PD1_U2TX);
#elif ARCH_TYPE == ARCH_T_LM3S9X
			if (p->def->rxpin == 0) {
				MAP_GPIOPinConfigure(GPIO_PD0_U2RX);
				MAP_GPIOPinConfigure(GPIO_PD1_U2TX);
			} else {
				MAP_GPIOPinConfigure(GPIO_PD5_U2RX);
				MAP_GPIOPinConfigure(GPIO_PD6_U2TX);
			}
#endif
			break;
#if ARCH_TYPE == ARCH_T_LM3S9X
		case GPIO_P6:
			MAP_GPIOPinConfigure(GPIO_PG0_U2RX);
			MAP_GPIOPinConfigure(GPIO_PG1_U2TX);
			break;
#endif
		default:
			break;
		}
		break;
	default:
		break;
	}
	MAP_GPIOPinTypeUART(arch_GpioPortBase(p->def->rxport), BITMASK(p->def->rxpin));
	MAP_GPIOPinTypeUART(arch_GpioPortBase(p->def->txport), BITMASK(p->def->txpin));
	xUart.baud = 4800;
	xUart.pari = UART_PARI_NO;
	xUart.data = UART_DATA_8D;
	xUart.stop = UART_STOP_1D;
	arch_UartOpen(p->def->id, &xUart);
}


//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
sys_res arch_UartOpen(uint_t nId, p_uart_para pPara)
{
	uint_t nBase, nMode = 0;
	p_dev_uart p;

	p = lm3s_uart_dev[nId];
	nBase = lm3s_tblUartBase[p->def->id];
	if (memcmp(&p->para, pPara, sizeof(p->para))) {
		memcpy(&p->para, pPara, sizeof(p->para));
		switch (pPara->stop) {
		case UART_STOP_2D:
			nMode |= UART_CONFIG_STOP_TWO;
			break;
		default:
			nMode |= UART_CONFIG_STOP_ONE;
			break;
		}
		switch (pPara->pari) {
		case UART_PARI_EVEN:
			nMode |= UART_CONFIG_PAR_EVEN;
			break;
		case UART_PARI_ODD:
			nMode |= UART_CONFIG_PAR_ODD;
			break;
		default:
			nMode |= UART_CONFIG_PAR_NONE;
			break;
		}
		switch (pPara->data) {
		case UART_DATA_7D:
			nMode |= UART_CONFIG_WLEN_7;
			break;
		default:
			nMode |= UART_CONFIG_WLEN_8;
			break;
		}
		MAP_UARTConfigSetExpClk(nBase, MAP_SysCtlClockGet(), pPara->baud, nMode);
		MAP_UARTFIFOLevelSet(nBase, UART_FIFO_TX2_8, UART_FIFO_RX6_8);
		if (p->def->rxmode == UART_MODE_IRQ)
			MAP_UARTIntEnable(nBase, UART_INT_RX | UART_INT_RT);
		MAP_UARTEnable(nBase);
	}
	return SYS_R_OK;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
void arch_UartTxIEnable(uint_t nId)
{

	MAP_UARTIntEnable(lm3s_tblUartBase[nId], UART_INT_TX);
	arch_UartISR(nId);
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
void arch_UartScReset(uint_t nId, uint_t nHL)
{

#if 0
	if (nHL)
		GPIO_SetBits(arch_GpioPortBase(p->def->rxport), BITMASK(p->def->rxpin));
	else
		GPIO_ResetBits(arch_GpioPortBase(p->def->rxport), BITMASK(p->def->rxpin));
#endif
}


//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
void arch_UartSend(uint_t nId, const void *pData, uint_t nLen)
{
	uint_t nBase = lm3s_tblUartBase[nId];
	uint8_t *pBuf = (uint8_t *)pData;

	for (; nLen; nLen--)
		MAP_UARTCharPut(nBase, *pBuf++);
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
int arch_UartGetChar(uint_t nId)
{

	return MAP_UARTCharGet(lm3s_tblUartBase[nId]);
}



//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
void arch_UartISR(uint_t nId)
{
	uint_t nBase = lm3s_tblUartBase[nId];
	p_dev_uart p;
	uint8_t aTemp[16], *pTemp = aTemp;
	int i;

	p = lm3s_uart_dev[nId];

	MAP_UARTRxErrorClear(nBase);

	while (MAP_UARTCharsAvail(nBase) == true) {
        *pTemp++ = MAP_UARTCharGetNonBlocking(nBase);
		if (pTemp >= ARR_ENDADR(aTemp)) {
#if IO_BUF_TYPE == BUF_T_BUFFER
			buf_Push(p->bufrx, aTemp, sizeof(aTemp));
#elif IO_BUF_TYPE == BUF_T_DQUEUE
			dque_Push(dqueue, p->parent->id | UART_DQUE_RX_CHL, aTemp, sizeof(aTemp));
#endif
			pTemp = aTemp;
		}
    }
	if (pTemp > aTemp) {
#if IO_BUF_TYPE == BUF_T_BUFFER
		buf_Push(p->bufrx, aTemp, pTemp - aTemp);
#elif IO_BUF_TYPE == BUF_T_DQUEUE
		dque_Push(dqueue, p->parent->id | UART_DQUE_RX_CHL, aTemp, pTemp - aTemp);
#endif
	}

	if (__raw_readl(nBase + UART_O_IM) & UART_INT_TX) {
#if IO_BUF_TYPE == BUF_T_BUFFER
		while (MAP_UARTSpaceAvail(nBase) == true) {
			if (p->buftx->len == 0) {
				MAP_UARTIntDisable(nBase, UART_INT_TX);
				break;
			}
			for (i = 0; i < p->buftx->len; i++) {
				if (MAP_UARTSpaceAvail(nBase) == false)
					break;
				MAP_UARTCharPutNonBlocking(nBase, p->buftx->p[i]);
			}
			buf_Remove(p->buftx, i);
		}
#elif IO_BUF_TYPE == BUF_T_DQUEUE
		while (MAP_UARTSpaceAvail(nBase) == true) {
			i = dque_PopChar(dqueue, p->parent->id | UART_DQUE_TX_CHL);
			if (i < 0) {
				MAP_UARTIntDisable(nBase, UART_INT_TX);
				break;
			}
			MAP_UARTCharPutNonBlocking(nBase, i);
		}
#endif
	}
}

#endif
