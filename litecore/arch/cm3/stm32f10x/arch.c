



//Internal Functions
#if OS_TYPE
static void stm32_OsTickInit()
{

	SysTick_Config(MCU_CLOCK / RT_TICK_PER_SECOND);
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);
}
#endif



#if ARCH_TYPE == ARCH_T_STM32F10X_HD
void SystemInit(void)
{

	//初始化系统时钟
	RCC_DeInit();
#if MCU_HSE_ENABLE
	//启用外部高速晶振
	RCC_HSEConfig(RCC_HSE_ON);
#else
	//启用内部高速时钟
	RCC_HSICmd(ENABLE);
#endif

#if MCU_HSE_ENABLE
	if (RCC_WaitForHSEStartUp() == SUCCESS) {
#endif
		/* HCLK = 72M Max */
		RCC_HCLKConfig(RCC_SYSCLK_Div1);
		/* PCLK2 72M Max */
		RCC_PCLK2Config(RCC_HCLK_Div1);
		/* PCLK1 36M Max */
#if MCU_FREQUENCY == MCU_SPEED_HALF
		RCC_PCLK1Config(RCC_HCLK_Div1);
#else
		RCC_PCLK1Config(RCC_HCLK_Div2);
#endif
		/* ADCCLK = PCLK2/6 */
		RCC_ADCCLKConfig(RCC_PCLK2_Div6);
		/* Enable Prefetch Buffer */
		FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
		/* Flash wait state */
#if MCU_FREQUENCY == MCU_SPEED_LOW
		FLASH_SetLatency(FLASH_Latency_0);
#elif MCU_FREQUENCY == MCU_SPEED_HALF
		FLASH_SetLatency(FLASH_Latency_1);
#else
		FLASH_SetLatency(FLASH_Latency_2);
#endif
		/* PLLCLK */
#if MCU_HSE_ENABLE
#if MCU_FREQUENCY == MCU_SPEED_LOW
		RCC_PLLConfig(RCC_PLLSource_HSE_Div2, RCC_PLLMul_2);
#elif MCU_FREQUENCY == MCU_SPEED_HALF
		RCC_PLLConfig(RCC_PLLSource_HSE_Div2, RCC_PLLMul_9);
#else
#if MCU_HSE_FREQ == 8000000
		RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);
#else
		RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_6);
#endif
#endif
#else
#if MCU_FREQUENCY == MCU_SPEED_LOW
		RCC_PLLConfig(RCC_PLLSource_HSI_Div2, RCC_PLLMul_2);
#elif MCU_FREQUENCY == MCU_SPEED_HALF
		RCC_PLLConfig(RCC_PLLSource_HSI_Div2, RCC_PLLMul_9);
#else
		RCC_PLLConfig(RCC_PLLSource_HSI_Div2, RCC_PLLMul_16);
#endif
#endif
		/* Enable PLL */ 
		RCC_PLLCmd(ENABLE);
		/* Wait till PLL is ready */
		while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);
		/* Select PLL as system clock source */
		RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
		/* Wait till PLL is used as system clock source */
		while (RCC_GetSYSCLKSource() != 0x08);
#if MCU_HSE_ENABLE
	}	
#endif
}
#endif


static void stm32_RccInit()
{

	SystemInit();

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

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
#if (ARCH_TYPE == ARCH_T_STM32F10X_CL) && (USB_ENABLE == 0)
	xNVIC.NVIC_IRQChannelCmd = DISABLE;
	xNVIC.NVIC_IRQChannel = OTG_FS_IRQn;
	NVIC_Init(&xNVIC);
#endif
}

static void stm32_GpioInit()
{
	GPIO_InitTypeDef xGpio;

	//将所有没有使用的IO置为模拟输入
	//可以降低功耗和改善EMC/EMI性能
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB |
						   	RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD |
							RCC_APB2Periph_GPIOE | RCC_APB2Periph_GPIOF |
							RCC_APB2Periph_GPIOG, ENABLE);
	xGpio.GPIO_Pin = GPIO_Pin_All;
	xGpio.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOA, &xGpio);
	GPIO_Init(GPIOB, &xGpio);
	GPIO_Init(GPIOC, &xGpio);
	GPIO_Init(GPIOD, &xGpio);
	GPIO_Init(GPIOE, &xGpio);
	GPIO_Init(GPIOF, &xGpio);
	GPIO_Init(GPIOG, &xGpio);
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB |
						   	RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD |
							RCC_APB2Periph_GPIOE | RCC_APB2Periph_GPIOF |
							RCC_APB2Periph_GPIOG, DISABLE);

	//使能SWD,禁用JTAG
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
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
	stm32_GpioInit();
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

