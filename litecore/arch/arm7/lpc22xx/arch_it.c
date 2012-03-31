



//Private Functions
static void lpc22xx_ExtIrqISR()
{

}

static void lpc22xx_TimerISR(uint_t nId)
{

	arch_TimerIntClear(nId);
	irq_TimerISR(nId);
}



//External Functions
int arch_ExtIrqRegister(uint_t nPort, uint_t nPin, uint_t nTriggerMode)
{

	NVIC_EnableIRQ(EINT3_IRQn);

	if (nPort)
		return (ARCH_EXTIRQ_QTY / 2 + nPin);
	return nPin;
}

void arch_ExtIrqClear(uint_t nPort, uint_t nPin)
{

	if (nPort)
		SETBIT(LPC_GPIOINT->IO2IntClr, nPin);
	else
		SETBIT(LPC_GPIOINT->IO0IntClr, nPin);
}

void arch_ExtIrqEnable(uint_t nPort, uint_t nPin, uint_t nMode)
{

	if (nPort) {
		SETBIT(LPC_GPIOINT->IO2IntClr, nPin);
		if (nMode == IRQ_TRIGGER_FALLING)
			SETBIT(LPC_GPIOINT->IO2IntEnF, nPin);
		else
			SETBIT(LPC_GPIOINT->IO2IntEnR, nPin);
	} else {
		SETBIT(LPC_GPIOINT->IO0IntClr, nPin);
		if (nMode == IRQ_TRIGGER_FALLING)
			SETBIT(LPC_GPIOINT->IO0IntEnF, nPin);
		else
			SETBIT(LPC_GPIOINT->IO0IntEnR, nPin);
	}
}

void arch_ExtIrqDisable(uint_t nPort, uint_t nPin, uint_t nMode)
{

	if (nPort) {
		if (nMode == IRQ_TRIGGER_FALLING)
			CLRBIT(LPC_GPIOINT->IO2IntEnF, nPin);
		else
			CLRBIT(LPC_GPIOINT->IO2IntEnR, nPin);
	} else {
		if (nMode == IRQ_TRIGGER_FALLING)
			CLRBIT(LPC_GPIOINT->IO0IntEnF, nPin);
		else
			CLRBIT(LPC_GPIOINT->IO0IntEnR, nPin);
	}
}


//Interrupt Functions
void WDT_IRQHandler(int vector)
{
}

void TIMER0_IRQHandler(int vector)
{

	lpc22xx_TimerISR(0);
}

void TIMER1_IRQHandler(int vector)
{

	lpc22xx_TimerISR(1);
}

void UART0_IRQHandler(int vector)
{
	
	arch_UartISR(0);
}

void UART1_IRQHandler(int vector)
{

	arch_UartISR(1);
}

void EINT0_IRQHandler(int vector)
{
}

void EINT1_IRQHandler(int vector)
{
}

void EINT2_IRQHandler(int vector)
{
}

void EINT3_IRQHandler(int vector)
{

	lpc22xx_ExtIrqISR();
}



