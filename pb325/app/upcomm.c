#include <stdio.h>
#include <string.h>
#include <litecore.h>
#include "system.h"
#include "para.h"


//Private Variables
static t_gw3761 rcp_aGw3761[5];


sys_res rcp_IsLogin()
{

	if (rcp_aGw3761[0].parent.ste == DLRCP_S_READY)
		return SYS_R_OK;
	return SYS_R_ERR;
}


void tsk_Upcom1(void *args)
{
	p_gw3761 p;
	time_t tTime;
	uint_t nCnt;
	t_afn04_f1 xF1;
	t_afn04_f3 xF3;
	t_afn04_f85 xF85;

	p = &rcp_aGw3761[0];
	gw3761_Init(p, 1);
	for (nCnt = 0; ; ) {
		if ((nCnt & 0x3F) == 0) {
			icp_ParaRead(4, 85, TERMINAL, &xF85, sizeof(t_afn04_f85));
			p->rtua = xF85.area;
			p->terid = xF85.addr;
			icp_ParaRead(4, 1, TERMINAL, &xF1, sizeof(t_afn04_f1));
			if (xF1.span < 1)
				xF1.span = 1;
			p->parent.tmo = 5;
			p->parent.retry = 3;
			p->parent.refresh = xF1.span * 60;
			p->parent.connect = 60;
			icp_ParaRead(4, 3, TERMINAL, &xF3, sizeof(t_afn04_f3));
			modem_Config(xF3.apn, xF1.span * 60, 5);
			dlrcp_SetChl(&p->parent, CHL_T_SOC_TC, xF3.port1, xF3.ip1[0], xF3.ip1[1], xF3.ip1[2], xF3.ip1[3]);
		}
		if (tTime != rtc_GetTimet()) {
			tTime = rtc_GetTimet();
			nCnt += 1;
		}
		gw3761_Handler(p);
	}
}

void dbg_trace(const char *fmt, ...)
{
	va_list args;
	char str[64];
	p_gw3761 p = &rcp_aGw3761[1];

	if (p->parent.chl->ste != CHL_S_READY)
		return;
	str[0] = '\r';
	str[1] = '\n';
	va_start(args, fmt);
	vsnprintf(&str[2], sizeof(str) - 2, fmt, args);
	va_end(args);
	chl_Send(p->parent.chl, str, strlen(str));
}

static t_gd5100 rcp_GD5100;
void tsk_Upcom2(void *args)
{
	p_gw3761 p, pEnd;
	t_afn04_f85 xF85;

	icp_ParaRead(4, 85, TERMINAL, &xF85, sizeof(t_afn04_f85));
	for (p = &rcp_aGw3761[1]; p < ARR_ENDADR(rcp_aGw3761); p++) {
		gw3761_Init(p, 1);
		p->rtua = xF85.area;
		p->terid = xF85.addr;
		p->parent.tmo = 5;
		p->parent.retry = 3;
		p->parent.refresh = 3 * 60;
		p->parent.connect = 60;
	}
	//远程调试端口9000
	dlrcp_SetChl(&rcp_aGw3761[1].parent, CHL_T_SOC_TS, 9000, 0, 0, 0, 0);
	//远程GW3761规约端口8000
	dlrcp_SetChl(&rcp_aGw3761[2].parent, CHL_T_SOC_TS, 8000, 0, 0, 0, 0);
	//红外GW3761规约
	dlrcp_SetChl(&rcp_aGw3761[3].parent, CHL_T_RS232, 3, 1200, UART_PARI_EVEN, UART_DATA_8D, UART_STOP_1D);
	//串口GW3761规约(开机按键启用)
	os_thd_Sleep(500);
	if ((g_sys_status & BITMASK(0))) {
		pEnd = &rcp_aGw3761[3];
	} else {
		pEnd = &rcp_aGw3761[4];
		dlrcp_SetChl(&pEnd->parent, CHL_T_RS232, 0, 9600, UART_PARI_EVEN, UART_DATA_8D, UART_STOP_1D);
	}

	gd5100_Init(&rcp_GD5100);
	p->rtua = xF85.area;
	p->terid = xF85.addr;
	rcp_GD5100.parent.tmo = 5;
	rcp_GD5100.parent.retry = 3;
	rcp_GD5100.parent.refresh = 3 * 60;
	rcp_GD5100.parent.connect = 60;
	//远程南网GD5100规约端口777
	dlrcp_SetChl(&rcp_GD5100.parent, CHL_T_SOC_TS, 777, 0, 0, 0, 0);

	for (; ; ) {
		for (p = &rcp_aGw3761[1]; p <= pEnd; p++)
			gw3761_Handler(p);
		if (gd5100_Handler(&rcp_GD5100) == SYS_R_OK)
			gd5100_Response(&rcp_GD5100);
	}
}


