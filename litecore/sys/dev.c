

sys_res dev_Get(dev p, int nTmo)
{
	uint_t nForever = 0;

	if (nTmo == OS_TMO_FOREVER) {
		nForever = 1;
	} else {
		nForever = 0;
		nTmo /= OS_TICK_MS;
	}
	do {
		if (p->ste == DEV_S_IDLE) {
			p->ste = DEV_S_READY;
			p->tid = os_thd_IdSelf();
			return SYS_R_OK;
		}
		os_thd_Slp1Tick();
	} while (nForever || nTmo--);
	return SYS_R_TMO;
}

sys_res dev_Release(dev p)
{

	p->ste = DEV_S_IDLE;
	return SYS_R_OK;
}

