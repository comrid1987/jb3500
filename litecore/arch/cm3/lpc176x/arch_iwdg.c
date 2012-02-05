#if WDT_INT_ENABLE










//External Functions
void arch_WdgReload(uint_t nIntEnable)
{

	if (nIntEnable)
		os_interrupt_Disable();
	LPC_WDT->WDFEED = 0xAA;
	LPC_WDT->WDFEED = 0x55;
	if (nIntEnable)
		os_interrupt_Enable();
}

void arch_WdgInit()
{

	LPC_WDT->WDCLKSEL = 0x80000001;
	LPC_WDT->WDTC = MCU_CLOCK / 4;
	LPC_WDT->WDMOD = 3;
	arch_WdgReload(1);
}

#endif

