#include <hi/keypad.h>

void key_Init()
{
	p_gpio_def p;
	
	for (p = tbl_bspKeypad[0]; p < tbl_bspKeypad[1]; p++)
		sys_GpioConf(p);
}

#if 1
uint_t key_Read()
{
	p_gpio_def p;
	uint_t i, n = 0;
	static uint_t nSaved = ~0;
	
	for (p = tbl_bspKeypad[0], i = 0; p < tbl_bspKeypad[1]; p++, i++) {
		if (sys_GpioRead(p)) {
			//有按键被按下
			if (GETBIT(nSaved, i) == 0) {
				SETBIT(nSaved, i);
				SETBIT(n, i);
			}
		} else
			CLRBIT(nSaved, i);
	}
	return n;
}
#else
#define KEY_LN_MASK			(0x00180000)		//P0.19,P0.20 做为行检测,需要中断
#define KEY_ROW_MASK		(0x1e000000)		//P0.25~28 做为列检测,不需要中断

#define KEY_LN_GPO			(LPC_GPIO0->FIODIR |= KEY_LN_MASK)
#define KEY_LN_GPI			(LPC_GPIO0->FIODIR &= ~KEY_LN_MASK)
#define KEY_LN_SET			(LPC_GPIO0->FIOSET = KEY_LN_MASK)
#define KEY_LN_CLR			(LPC_GPIO0->FIOCLR = KEY_LN_MASK)

#define KEY_ROW_GPO			(LPC_GPIO0->FIODIR |= KEY_ROW_MASK)
#define KEY_ROW_GPI			(LPC_GPIO0->FIODIR &= ~KEY_ROW_MASK)
#define KEY_ROW_SET			(LPC_GPIO0->FIOSET = KEY_ROW_MASK)
#define KEY_ROW_CLR			(LPC_GPIO0->FIOCLR = KEY_ROW_MASK)

#define KEY_LN_VAL			((~(LPC_GPIO0->FIOPIN>>19)) & (KEY_LN_MASK>>19))		//P0.19开始
#define KEY_ROW_VAL			((~(LPC_GPIO0->FIOPIN>>25)) & (KEY_ROW_MASK>>25))	//P0.25开始

uint_t key_Read()
{
	uint_t i, nLn, nRow=0;

	// 1.读H线编码
	KEY_ROW_GPO;
	KEY_ROW_CLR;
	KEY_LN_GPI;

	nLn = KEY_LN_VAL;
	
	if (nLn) {
		KEY_ROW_SET;  		// 2.读V线编码	
		KEY_ROW_GPI;
		KEY_LN_CLR;
		KEY_LN_GPO;

		for (i = 0; i < 30; i++) {
			nRow = KEY_ROW_VAL;
			if (nRow == 0x0A)
				break;
		}
		if (nRow) 	  		// 3.转换合并H-V编码
			nRow = ((nLn & 0x02) << 4) | nRow;

		KEY_LN_SET;
		KEY_LN_GPI;
		KEY_ROW_GPO;
		KEY_ROW_CLR;
		sys_Delay(10);
	}	

 	return nRow;
}
#endif



