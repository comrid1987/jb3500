



//Private Functions
static void lpc22xx_ExtIrqISR(uint_t nId)
{

	irq_ExtISR(nId);
	SETBIT(EXTINT, nId);
}

static void lpc22xx_TimerISR(uint_t nId)
{

	irq_TimerISR(nId);
	arch_TimerIntClear(nId);
}

static uint_t lpc22xx_ExtIrqId(uint_t nPin)
{

	return 0;
}


//External Functions
int arch_ExtIrqRegister(uint_t nPort, uint_t nPin, uint_t nTriggerMode)
{

	rt_hw_interrupt_install(EINT0_INT, EINT0_IRQHandler, RT_NULL);
	return 0;
}

void arch_ExtIrqEnable(uint_t nPort, uint_t nPin, uint_t nMode)
{
	uint_t nId;
	
	nId = lpc22xx_ExtIrqId(nPin);
	SETBIT(EXTMODE, nId);
	if (nMode == IRQ_TRIGGER_FALLING)
		CLRBIT(EXTPOLAR, nId);
	else
		SETBIT(EXTPOLAR, nId);
	SETBIT(EXTINT, nId);
	rt_hw_interrupt_umask(EINT0_INT);
}

void arch_ExtIrqDisable(uint_t nPort, uint_t nPin, uint_t nMode)
{

	rt_hw_interrupt_mask(EINT0_INT);
}

void arch_ExtIrqRxConf(uint_t nPort, uint_t nPin)
{
	
	arch_GpioSel(nPort, nPin, 1);
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

	lpc22xx_ExtIrqISR(0);
}

void EINT1_IRQHandler(int vector)
{

	lpc22xx_ExtIrqISR(1);
}

void EINT2_IRQHandler(int vector)
{

	lpc22xx_ExtIrqISR(2);
}

void EINT3_IRQHandler(int vector)
{

	lpc22xx_ExtIrqISR(3);
}



