

#define hc595_OE(x)				sys_GpioSet(gpio_node(tbl_bspHC595, 0), x)
#define hc595_Clk(x)			sys_GpioSet(gpio_node(tbl_bspHC595, 1), x)
#define hc595_RCK(x)			sys_GpioSet(gpio_node(tbl_bspHC595, 2), x)
#define hc595_SER(x)			sys_GpioSet(gpio_node(tbl_bspHC595, 3), x)


//Private Variables
void hc595_Write(uint_t nPin, uint_t nHL)
{
	static uint_t hc595_nStatus = 0x00;
	uint_t i, nTemp;

	if (nHL)
		CLRBIT(hc595_nStatus, nPin);
	else
		SETBIT(hc595_nStatus, nPin);

	hc595_RCK(0);
	hc595_OE(0);
	nTemp = hc595_nStatus;
	for(i = 0; i < 8; i++)
	{
		hc595_Clk(0);
		if((nTemp&0x80)==0)
			hc595_SER(1);
		else
			hc595_SER(0);
		hc595_Clk(1);
		nTemp <<= 1;
	}
	hc595_RCK(1);
	hc595_OE(0);
}

