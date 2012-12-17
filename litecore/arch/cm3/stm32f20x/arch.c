






//Internal Functions
#if OS_TYPE
static void stm32_OsTickInit()
{

	SysTick_Config(MCU_CLOCK / RT_TICK_PER_SECOND);
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);
}
#endif

static void stm32_RccInit()
{

	SystemInit();

#if OS_TYPE
	//SysTick Initialize
	stm32_OsTickInit();
#endif
}

static void stm32_IrqInit()
{
    NVIC_InitTypeDef xNVIC;

	NVIC_SetVectorTable(NVIC_VectTab_FLASH, BOOTLOADER_SIZE);
	/* Configure one bit for preemption priority */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

	xNVIC.NVIC_IRQChannelPreemptionPriority = 0;
	xNVIC.NVIC_IRQChannelSubPriority = 0;
	xNVIC.NVIC_IRQChannelCmd = ENABLE;

	xNVIC.NVIC_IRQChannel = EXTI0_IRQn;
	NVIC_Init(&xNVIC);
	xNVIC.NVIC_IRQChannel = EXTI1_IRQn;
	NVIC_Init(&xNVIC);
	xNVIC.NVIC_IRQChannel = EXTI2_IRQn;
	NVIC_Init(&xNVIC);
	xNVIC.NVIC_IRQChannel = EXTI3_IRQn;
	NVIC_Init(&xNVIC);
	xNVIC.NVIC_IRQChannel = EXTI4_IRQn;
	NVIC_Init(&xNVIC);
	xNVIC.NVIC_IRQChannel = EXTI9_5_IRQn;
	NVIC_Init(&xNVIC);
	xNVIC.NVIC_IRQChannel = EXTI15_10_IRQn;
	NVIC_Init(&xNVIC);
#if ETH_INT_ENABLE
	xNVIC.NVIC_IRQChannel = ETH_IRQn;
	NVIC_Init(&xNVIC);
#endif
#if USB_ENABLE == 0
	xNVIC.NVIC_IRQChannelCmd = DISABLE;
	xNVIC.NVIC_IRQChannel = OTG_FS_IRQn;
	NVIC_Init(&xNVIC);
#endif
}

static void stm32_GpioIdleInit()
{
	GPIO_InitTypeDef xGpio;

	//将所有没有使用的IO置为模拟输入
	//可以降低功耗和改善EMC/EMI性能
	RCC_AHB1PeriphClockCmd(	RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB |
						   	RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD |
							RCC_AHB1Periph_GPIOE | RCC_AHB1Periph_GPIOF |
							RCC_AHB1Periph_GPIOG, ENABLE);
	xGpio.GPIO_Pin = GPIO_Pin_All;
	xGpio.GPIO_Mode = GPIO_Mode_AIN;
	xGpio.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &xGpio);
	GPIO_Init(GPIOB, &xGpio);
	GPIO_Init(GPIOC, &xGpio);
	GPIO_Init(GPIOD, &xGpio);
	GPIO_Init(GPIOE, &xGpio);
	GPIO_Init(GPIOF, &xGpio);
	GPIO_Init(GPIOG, &xGpio);
	RCC_AHB1PeriphClockCmd(	RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB |
						   	RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD |
							RCC_AHB1Periph_GPIOE | RCC_AHB1Periph_GPIOF |
							RCC_AHB1Periph_GPIOG, DISABLE);
}


#if DEBUG_ENABLE
static void stm32_DbgInit()
{

#if 0
	DBGMCU_Config(0x00000127, ENABLE);
	*(vu32 *)0xE0000FB0 = 0xC5ACCE55;
	*(vu32 *)0xE0000E80 = 0x00010017;
	*(vu32 *)0xE0000E40 = 0x00000081;
	*(vu32 *)0xE0000E00 = 0x80000001;
	*(vu32 *)0xE00400F0 = 0x00000002;
	*(vu32 *)0xE0040304 = 0x00000102;
#else
	*(vu32 *)0xE0042008 = 0x00001000;
#endif
}
#endif

//External Functions
void arch_Init()
{

	//时钟系统初始化
	stm32_RccInit();
#if DEBUG_ENABLE
	stm32_DbgInit();
#endif
	//中断初始化
	stm32_IrqInit();
	//GPIO初始化
	//stm32_GpioIdleInit();
#if EPI_ENABLE && !EPI_SOFTWARE
	stm32_FsmcInit();
#endif
}


#if IDLE_ENABLE
void arch_IdleEntry()
{

	PWR_EnterSTANDBYMode();
}
#endif


void arch_Reset()
{

	NVIC_SystemReset();
}

