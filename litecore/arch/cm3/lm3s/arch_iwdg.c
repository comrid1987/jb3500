#if WDT_INT_ENABLE










//External Functions
void arch_WdgReload(uint_t nIntEnable)
{

	MAP_WatchdogRunning();
}

void arch_WdgInit()
{

	MAP_WatchdogEnable();
	MAP_WatchdogReloadSet();
}

#endif


