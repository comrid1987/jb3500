



//Private Const
static LPC_TIM_TypeDef * const lpc22xx_tblTimerBase[] = {
	LPC_TIM0,
	LPC_TIM1,
};




void arch_TimerInit(uint_t nId)
{

	switch (nId) {
	case 0:
		SETBIT(LPC_SC->PCONP, 1);
		NVIC_EnableIRQ(TIMER0_IRQn);
		break;
	case 1:
		SETBIT(LPC_SC->PCONP, 2);
		NVIC_EnableIRQ(TIMER1_IRQn);
		break;
	case 2:
		SETBIT(LPC_SC->PCONP, 22);
		NVIC_EnableIRQ(TIMER2_IRQn);
		break;
	case 3:
		SETBIT(LPC_SC->PCONP, 23);
		NVIC_EnableIRQ(TIMER3_IRQn);
		break;
	default:
		break;
	}
}

void arch_TimerIntClear(uint_t nId)
{
	LPC_TIM_TypeDef *pTimer = lpc22xx_tblTimerBase[nId];

	pTimer->IR = BITMASK(0);
}

void arch_TimerStart(uint_t nId, uint_t nValue)
{
	LPC_TIM_TypeDef *pTimer = lpc22xx_tblTimerBase[nId];

	pTimer->TCR = BITMASK(1);
	pTimer->TC = 0;
	pTimer->IR = BITMASK(0);
	pTimer->CTCR = 0;
	pTimer->PR = 0;
	pTimer->PC = 0;
	pTimer->MR0 = nValue;
	pTimer->MCR = BITMASK(0) | BITMASK(1);
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


