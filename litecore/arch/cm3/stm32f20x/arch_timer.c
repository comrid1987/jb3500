//Private Const
static TIM_TypeDef * const stm32_tblTimBase[ARCH_TIMER_QTY] = {
	TIM2,
	TIM3,
	TIM4,
	TIM5,
	TIM6,
	TIM7,
};


void arch_TimerInit(uint_t nId)
{
	NVIC_InitTypeDef xNVIC;

	switch(nId) {
	default:
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
		xNVIC.NVIC_IRQChannel = TIM2_IRQn;
		break;
	case 1:
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
		xNVIC.NVIC_IRQChannel = TIM3_IRQn;
		break;
	case 2:
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
		xNVIC.NVIC_IRQChannel = TIM4_IRQn;
		break;
	case 3:
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
		xNVIC.NVIC_IRQChannel = TIM5_IRQn;
		break;
	case 4:
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);
		xNVIC.NVIC_IRQChannel = TIM6_IRQn;
		break;
	case 5:
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);
		xNVIC.NVIC_IRQChannel = TIM7_IRQn;
		break;
	}
	xNVIC.NVIC_IRQChannelPreemptionPriority = 0;
	xNVIC.NVIC_IRQChannelSubPriority = 1;
	xNVIC.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&xNVIC);
}

void arch_TimerIntClear(uint_t nId)
{

	TIM_ClearITPendingBit(stm32_tblTimBase[nId], TIM_IT_Update);
}

void arch_TimerStart(uint_t nId, uint_t nValue)
{
	TIM_TimeBaseInitTypeDef  xTIM_TimeBase;
	
	TIM_InternalClockConfig(stm32_tblTimBase[nId]);//内部时钟源
    TIM_DeInit(stm32_tblTimBase[nId]);
    xTIM_TimeBase.TIM_Period = nValue;          //ARR
    xTIM_TimeBase.TIM_Prescaler = 0;			//PSC
    xTIM_TimeBase.TIM_ClockDivision = TIM_CKD_DIV1; 
    xTIM_TimeBase.TIM_CounterMode = TIM_CounterMode_Up; //计数方式
	TIM_TimeBaseInit(stm32_tblTimBase[nId], &xTIM_TimeBase); 
	TIM_ClearFlag(stm32_tblTimBase[nId], TIM_FLAG_Update);   
	TIM_Cmd(stm32_tblTimBase[nId], ENABLE); //使能
	TIM_ITConfig(stm32_tblTimBase[nId], TIM_IT_Update | TIM_IT_Trigger, ENABLE);
}

void arch_TimerStop(uint_t nId)
{

	TIM_ClearFlag(stm32_tblTimBase[nId], TIM_FLAG_Update);  
	TIM_Cmd(stm32_tblTimBase[nId], DISABLE); //
	TIM_ITConfig(stm32_tblTimBase[nId], TIM_IT_Update | TIM_IT_Trigger, DISABLE);
}

uint_t arch_TimerClockGet()
{

	return 60000000;
}


