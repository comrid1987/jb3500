#include <hi/keypad.h>

void key_Init()
{
	p_gpio_def p;
	
	for (p = tbl_bspKeypad[0]; p < tbl_bspKeypad[1]; p++)
		sys_GpioConf(p);
}

uint_t key_Read()
{
	p_gpio_def p;
	uint_t i = 0, n = 0;
	static uint_t nSaved = 0;
	
	for (p = tbl_bspKeypad[0]; p < tbl_bspKeypad[1]; p++, i++) {
		if (sys_GpioRead(p)) {
			//有按键被按下
			if (nSaved & BITMASK(i)) {
				CLRBIT(nSaved, i);
				SETBIT(n, i);
			}
		} else
			SETBIT(nSaved, i);
	}
	return n;
}


