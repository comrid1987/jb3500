


//Internal Functions
static void lpc176x_RccInit()
{

	SystemInit();

	/* Set the Vector Table base location */
	SCB->VTOR  = (BOOTLOADER_SIZE & 0x3FFFFF80);

	/* set pend exception priority */
	NVIC_SetPriority(PendSV_IRQn, (1<<__NVIC_PRIO_BITS) - 1);

	/* init systick */
	SysTick_Config(__CORE_CLK / RT_TICK_PER_SECOND - 1);
}

static void lpc176x_IrqInit()
{

}

static void lpc176x_GpioIdleInit()
{

	//将所有没有使用的IO置为模拟输入
	//可以降低功耗和改善EMC/EMI性能
}


#if DEBUG_ENABLE
static void lpc176x_DbgInit()
{

}
#endif

//External Functions
void arch_Init()
{

	//时钟系统初始化
	lpc176x_RccInit();
#if DEBUG_ENABLE
	lpc176x_DbgInit();
#endif
	//中断初始化
	lpc176x_IrqInit();
	//GPIO初始化
	lpc176x_GpioIdleInit();
}


#if IDLE_ENABLE
void arch_IdleEntry()
{

	PWR_EnterSTANDBYMode();
}
#endif


void arch_Reset()
{

	__raw_writel(0x05FA0000 | BITMASK(2), 0xE000ED0C);
}


