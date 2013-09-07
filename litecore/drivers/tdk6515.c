#if TDK6515_ENABLE
#include <drivers/tdk6515.h>





//Private Defines
#define TDK6515_JL				sys_GpioRead(gpio_node(tbl_bspTdk6515, 1))
#define TDK6515_XB				sys_GpioRead(gpio_node(tbl_bspTdk6515, 2))




void tdk6515_Init()
{
	p_gpio_def p;
	
	for (p = tbl_bspTdk6515[0]; p < tbl_bspTdk6515[1]; p++)
		sys_GpioConf(p);
}

void tdk6515_Reset(t_tdk6515 *p)
{

	sys_GpioSet(gpio_node(tbl_bspTdk6515, 0), 0);
	os_thd_Sleep(500);
	sys_GpioSet(gpio_node(tbl_bspTdk6515, 0), 1);
	os_thd_Sleep(3000);
}

int tdk6515_IsJLReady()
{

	if (TDK6515_JL)
		return 0;
	return 1;
}

int tdk6515_IsXBReady()
{

	if (TDK6515_XB)
		return 0;
	return 1;
}

//===========================================================
// 名   称： tdk6515_CmdSend()
// 功   能： 处理发送命令
//入口参数： 读/写命令个数 + 寄存器地址
//出口参数： 
//===========================================================
void tdk6515_CmdSend(t_tdk6515 *p, uint_t nWrite, uint_t nAdr, uint_t nLen)
{	
	uint8_t aBuf[3];

	aBuf[0] = nLen * 2;
	if (nWrite)
		aBuf[0] += 1;
	aBuf[1] = nAdr >> 8;
	aBuf[2] = nAdr;
	uart_Send(p->uart, aBuf, sizeof(aBuf));
}






#endif

