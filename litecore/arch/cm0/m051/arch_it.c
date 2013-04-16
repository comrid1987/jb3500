



//Private functions




//External Functions
int arch_ExtIrqRegister(uint_t nPort, uint_t nPin, uint_t nTriggerMode)
{

}

void arch_ExtIrqEnable(uint_t nPort, uint_t nPin, uint_t nMode)
{

}

void arch_ExtIrqDisable(uint_t nPort, uint_t nPin)
{

}


//Interrupt Functions
void WDT_IRQHandler()
{
}

void EINT0_IRQHandler()
{
}

void EINT1_IRQHandler()
{
}

void TMR0_IRQHandler()
{

	os_irq_Enter();

	os_irq_Leave();
}

void TMR1_IRQHandler()
{

	os_irq_Enter();

	os_irq_Leave();
}

void TMR2_IRQHandler()
{

	os_irq_Enter();

	os_irq_Leave();
}

void TMR3_IRQHandler()
{

	os_irq_Enter();

	os_irq_Leave();
}

void UART0_IRQHandler()
{
	
	os_irq_Enter();

#ifdef RT_USING_FINSH
	arch_Uart0Handler();
#else
	arch_UartISR(0);
#endif

	os_irq_Leave();
}

void UART1_IRQHandler()
{

	os_irq_Enter();

	arch_UartISR(1);

	os_irq_Leave();
}



