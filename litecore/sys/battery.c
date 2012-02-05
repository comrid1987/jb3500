#include <litecore.h>

//Private Variables
static int bat_nPowerOff = 0;


//External Functions
void bat_On()
{

#if BAT_CTRL_HIGH_EN
	sys_GpioSet(gpio_node(tbl_bspBattery, 0), 1);
#else
	sys_GpioSet(gpio_node(tbl_bspBattery, 0), 0);
#endif
}

void bat_Off()
{

#if BAT_CTRL_HIGH_EN
	sys_GpioSet(gpio_node(tbl_bspBattery, 0), 0);
#else
	sys_GpioSet(gpio_node(tbl_bspBattery, 0), 1);
#endif
}

#if BAT_VOL_ENABLE
float bat_Voltage()
{

	return arch_AdcData();
}
#endif

int bat_IsPowerOn()
{

	return sys_GpioRead(gpio_node(tbl_bspBattery, 1));
}

int bat_GetPowerOffCnt()
{

	return bat_nPowerOff;
}

void bat_Maintain()
{

	if (bat_IsPowerOn())
		bat_nPowerOff = 0;
	else
		bat_nPowerOff += 1;
}

