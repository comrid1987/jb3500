

#if PULSE_COL_ENABLE
void pulse_Init()
{
	p_gpio_def p;

	for (p = tbl_bspPulse[0]; p < tbl_bspPulse[1]; p++)
		sys_GpioConf(p);
}

uint_t pulse_Read()
{
	uint_t i, n = 0;
	p_gpio_def p;
	static uint_t nSaved = ~0;

	for (i = 0, p = tbl_bspPulse[0]; p < tbl_bspPulse[1]; i++, p++) {
		if (sys_GpioRead(p)) {
			if (GETBIT(nSaved, i) == 0) {
				SETBIT(nSaved, i);
				SETBIT(n, i);
			}
		} else {
			CLRBIT(nSaved, i);
		}
	}
	return n;
}
#endif

