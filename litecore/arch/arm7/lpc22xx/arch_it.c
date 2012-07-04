



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

	return 0;
}

void arch_ExtIrqClear(uint_t nPort, uint_t nPin)
{

}

void arch_ExtIrqEnable(uint_t nPort, uint_t nPin, uint_t nMode)
{

}

void arch_ExtIrqDisable(uint_t nPort, uint_t nPin, uint_t nMode)
{

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



