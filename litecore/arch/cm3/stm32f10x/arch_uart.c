#if UART_ENABLE





//In litecore.c
#if IO_BUF_TYPE == BUF_T_DQUEUE
extern dque dqueue;
#endif

//Private Variables
static p_dev_uart stm32_uart_dev[5];
static USART_TypeDef * const stm32_tblUartId[] = {USART1, USART2, USART3, UART4, UART5};



//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
void arch_UartInit(p_dev_uart p)
{
	t_uart_para xUart;
	p_uart_def pDef = p->def;
	NVIC_InitTypeDef xNVIC;
	GPIO_InitTypeDef xGpio;
	USART_ClockInitTypeDef xUartClock;
	USART_TypeDef *pUart;

	stm32_uart_dev[pDef->id] = p;

	pUart = stm32_tblUartId[pDef->id];
	switch (pDef->id) {
	case 0:
		/* Enalbe the USART1 Clock */
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
		/* Enable the USART1 Interrupt */
		xNVIC.NVIC_IRQChannel = USART1_IRQn;
		if (pDef->txport == GPIO_P1)
			GPIO_PinRemapConfig(GPIO_Remap_USART1, ENABLE);
		break;
	case 1:		
		/* Enalbe the USART2 Clock */
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
		/* Enable the USART2 Interrupt */
		xNVIC.NVIC_IRQChannel = USART2_IRQn;
		if (pDef->txport == GPIO_P3)
			GPIO_PinRemapConfig(GPIO_Remap_USART2, ENABLE);
		break;
	case 2:
		/* Enalbe the USART3 Clock */
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
		/* Enable the USART3 Interrupt */
		xNVIC.NVIC_IRQChannel = USART3_IRQn;
		break;
	case 3:
		/* Enalbe the USART4 Clock */
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);
		/* Enable the USART4 Interrupt */
		xNVIC.NVIC_IRQChannel = UART4_IRQn;
		break;
	case 4:
		/* Enalbe the USART5 Clock */
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE);
		/* Enable the USART5 Interrupt */
		xNVIC.NVIC_IRQChannel = UART5_IRQn;
		break;
	default:
		break;
	}
	xNVIC.NVIC_IRQChannelPreemptionPriority = 0;
	xNVIC.NVIC_IRQChannelSubPriority = 0;
	xNVIC.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&xNVIC);

	xGpio.GPIO_Speed = GPIO_Speed_50MHz;
	//Tx
	xGpio.GPIO_Pin = BITMASK(pDef->txpin);
#if SMARTCARD_ENABLE
	if (pDef->fun == UART_FUN_SC) {
		xGpio.GPIO_Mode = GPIO_Mode_AF_OD;
	} else {
#endif
		if (pDef->pinmode == DEV_PIN_OD)
			xGpio.GPIO_Mode = GPIO_Mode_AF_OD;
		else
			xGpio.GPIO_Mode = GPIO_Mode_AF_PP;
#if SMARTCARD_ENABLE
	}
#endif
	stm32_GpioClockEnable(pDef->txport);
	GPIO_Init(arch_GpioPortBase(pDef->txport), &xGpio);
	//Rx
	xGpio.GPIO_Pin = BITMASK(pDef->rxpin);
#if SMARTCARD_ENABLE
	if (pDef->fun == UART_FUN_SC) {
		if (pDef->pinmode == DEV_PIN_OD)
			xGpio.GPIO_Mode = GPIO_Mode_Out_OD;
		else
			xGpio.GPIO_Mode = GPIO_Mode_Out_PP;
	} else {
#endif
		if (pDef->pinmode == DEV_PIN_OD)
			xGpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		else
			xGpio.GPIO_Mode = GPIO_Mode_IPU;
#if SMARTCARD_ENABLE
	}
#endif
	stm32_GpioClockEnable(pDef->rxport);
	GPIO_Init(arch_GpioPortBase(pDef->rxport), &xGpio);
	//Clock
#if SMARTCARD_ENABLE
	if (pDef->fun == UART_FUN_SC)
		xUartClock.USART_Clock = USART_Clock_Enable;
	else
#endif
		xUartClock.USART_Clock = USART_Clock_Disable;
	xUartClock.USART_CPOL = USART_CPOL_Low;
	xUartClock.USART_CPHA = USART_CPHA_1Edge;
	xUartClock.USART_LastBit = USART_LastBit_Enable;
	USART_ClockInit(pUart, &xUartClock);
	//Fun
#if SMARTCARD_ENABLE
	if (pDef->fun == UART_FUN_SC) {
		xGpio.GPIO_Pin = BITMASK(pDef->fpin);
		stm32_GpioClockEnable(pDef->fport);
		if (pDef->pinmode == DEV_PIN_OD)
			xGpio.GPIO_Mode = GPIO_Mode_AF_OD;
		else
			xGpio.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIO_Init(arch_GpioPortBase(pDef->fport), &xGpio);
	}
#endif
	xUart.baud = 9677;
	xUart.pari = UART_PARI_EVEN;
	xUart.data = UART_DATA_8D;
	xUart.stop = UART_STOP_1_5D;
	arch_UartOpen(pDef->id, &xUart);
#if SMARTCARD_ENABLE
	switch (pDef->fun) {
	case UART_FUN_SC:
		//SmartCard时钟372倍波特率,1为PCLK2分频,2为4分频......
#if MCU_FREQUENCY == MCU_SPEED_LOW
		while (1);					//PCLK=8M,不能分频出SmartCard时钟
#elif MCU_FREQUENCY == MCU_SPEED_HALF
		USART_SetPrescaler(pUart, 5);//PCLK=36M
#else
		USART_SetPrescaler(pUart, 10);//PCLK=72M
#endif
		USART_SetGuardTime(pUart, 16);
		USART_SmartCardNACKCmd(pUart, ENABLE);
		USART_SmartCardCmd(pUart, ENABLE);
		break;
	default:
		break;
	}
#endif
}


//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
sys_res arch_UartOpen(uint_t nId, p_uart_para pPara)
{
	USART_InitTypeDef xUartPara;
	USART_TypeDef *pUart;
	p_dev_uart p;

	p = stm32_uart_dev[nId];
	pUart = stm32_tblUartId[nId];
	if (memcmp(&p->para, pPara, sizeof(p->para))) {
		memcpy(&p->para, pPara, sizeof(p->para));
		switch (pPara->stop) {
		case UART_STOP_0_5D:
			xUartPara.USART_StopBits = USART_StopBits_0_5;
			break;
		case UART_STOP_1_5D:
			xUartPara.USART_StopBits = USART_StopBits_1_5;
			break;
		case UART_STOP_2D:
			xUartPara.USART_StopBits = USART_StopBits_2;
			break;
		case UART_STOP_1D:
		default:
			xUartPara.USART_StopBits = USART_StopBits_1;
			break;
		}
		switch (pPara->pari) {
		case UART_PARI_EVEN:
			xUartPara.USART_Parity = USART_Parity_Even;
			break;
		case UART_PARI_ODD:
			xUartPara.USART_Parity = USART_Parity_Odd;
			break;
		case UART_PARI_NO:
		default:
			xUartPara.USART_Parity = USART_Parity_No;
			break;
		}
		switch (pPara->data) {
		case UART_DATA_7D:
			xUartPara.USART_WordLength = USART_WordLength_8b;
			break;
		case UART_DATA_8D:
		default:
			if (pPara->pari == UART_PARI_NO)
				xUartPara.USART_WordLength = USART_WordLength_8b;
			else
				xUartPara.USART_WordLength = USART_WordLength_9b;
			break;
		}
		xUartPara.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
		xUartPara.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
		xUartPara.USART_BaudRate = pPara->baud;
		USART_Init(pUart, &xUartPara);
		if (p->def->rxmode == UART_MODE_IRQ)
			USART_ITConfig(pUart, USART_IT_RXNE, ENABLE);
		USART_Cmd(pUart, ENABLE);
	}
	return SYS_R_OK;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
void arch_UartTxIEnable(uint_t nId)
{

	stm32_tblUartId[nId]->CR1 |= USART_FLAG_TXE | USART_FLAG_TC;
	arch_UartISR(nId);
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
void arch_UartSend(uint_t nId, const void *pData, uint_t nLen)
{
	USART_TypeDef *pUart = stm32_tblUartId[nId];
	uint8_t *pBuf = (uint8_t *)pData;

	for (; nLen; nLen--) {
		while ((pUart->SR & USART_FLAG_TXE)== 0);
		pUart->DR = *pBuf++;
	}
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
int arch_UartGetChar(uint_t nId)
{
	USART_TypeDef *pUart = stm32_tblUartId[nId];

	while ((pUart->SR & USART_FLAG_RXNE) == 0);
	return ((int)(pUart->DR & 0x1FF));
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
void arch_UartISR(uint_t nId)
{
	USART_TypeDef *pUart = stm32_tblUartId[nId];
	p_dev_uart p;
	uint8_t aTemp[16], *pTemp = aTemp;
	int i;

	p = stm32_uart_dev[nId];
	while (pUart->SR & (USART_FLAG_RXNE | USART_FLAG_ORE)) {
		if (p->para.data == UART_DATA_7D)
			*pTemp++ = pUart->DR & 0x7F;
		else
			*pTemp++ = pUart->DR;
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
	if (pUart->CR1 & USART_FLAG_TXE) {
#if IO_BUF_TYPE == BUF_T_BUFFER
		while (pUart->SR & USART_FLAG_TXE) {
			if (p->buftx->len == 0) {
				pUart->CR1 &= ~(USART_FLAG_TXE | USART_FLAG_TC);
				break;
			}
			for (i = 0; i < p->buftx->len; i++) {
				if ((pUart->SR & USART_FLAG_TXE) == 0)
					break;
				pUart->DR = p->buftx->p[i];
			}
			buf_Remove(p->buftx, i);
		}
#elif IO_BUF_TYPE == BUF_T_DQUEUE
		while (pUart->SR & USART_FLAG_TXE) {
			i = dque_PopChar(dqueue, p->parent->id | UART_DQUE_TX_CHL);
			if (i < 0) {
				pUart->CR1 &= ~(USART_FLAG_TXE | USART_FLAG_TC);
				break;
			}
			pUart->DR = i;
		}
#endif
	}
}


#endif

