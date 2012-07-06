



//Private Const
static LPC_TIM_TypeDef * const lpc22xx_tblTimerBase[] = {
	LPC_TIM0,
	LPC_TIM1,
};




void arch_TimerInit(uint_t nId)
{

	switch (nId) {
	case 0:
		rt_hw_interrupt_install(TIMER0_INT, TIMER0_IRQHandler, RT_NULL);
		rt_hw_interrupt_umask(TIMER0_INT);
		break;
	case 1:
		rt_hw_interrupt_install(TIMER1_INT, TIMER1_IRQHandler, RT_NULL);
		rt_hw_interrupt_umask(TIMER1_INT);
		break;
	default:
		break;
	}
}

void arch_TimerIntClear(uint_t nId)
{
	LPC_TIM_TypeDef *pTimer = lpc22xx_tblTimerBase[nId];

	pTimer->IR = 0xFF;
}

void arch_TimerStart(uint_t nId, uint_t nValue)
{
	LPC_TIM_TypeDef *pTimer = lpc22xx_tblTimerBase[nId];

	pTimer->TCR = BITMASK(1);
	pTimer->TC = 0;
	pTimer->IR = 0xFF;
	pTimer->PR = 0;
	pTimer->PC = 0;
	pTimer->MR0 = nValue;
	pTimer->MCR = BITMASK(0) | BITMASK(1);
	pTimer->CCR0 = 0x0C00;
	pTimer->TCR = BITMASK(0);
}

void arch_TimerStop(uint_t nId)
{
	LPC_TIM_TypeDef *pTimer = lpc22xx_tblTimerBase[nId];

	pTimer->TCR = 0;
}

uint_t arch_TimerClockGet()
{

	return PERI_CLOCK;
}

void arch_TimerCapConf(uint_t nPort, uint_t nPin)
{

	arch_GpioSel(nPort, nPin, 2);
}

void arch_TimerCapStart(uint_t nId)
{
	LPC_TIM_TypeDef *pTimer = lpc22xx_tblTimerBase[nId];

	pTimer->TCR = BITMASK(1);
	pTimer->TC = 0;
	pTimer->IR = 0xFF;
	pTimer->PR = 0;
	pTimer->PC = 0;
	pTimer->CCR0 = 0x0C00;
	pTimer->TCR = BITMASK(0);
}


