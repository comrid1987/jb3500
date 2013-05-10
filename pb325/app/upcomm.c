#include <stdio.h>
#include <string.h>
#include <litecore.h>
#include "system.h"
#include "para.h"
#include "meter.h"


//Private Variables
t_gw3761 rcp_aGw3761[5];




//Internal Functions
static sys_res upcom_Wait(p_gw3761 p, uint_t nTmo)
{

	for (nTmo /= OS_TICK_MS; nTmo; nTmo--) {
		if (gw3761_Handler(p) == SYS_R_OK)
			return SYS_R_OK;
	}
	return SYS_R_TMO;
}



//External Functions
sys_res rcp_IsLogin()
{

	if (rcp_aGw3761[0].parent.ste == DLRCP_S_READY)
		return SYS_R_OK;
	return SYS_R_ERR;
}


void tsk_Upcom1(void *args)
{
	p_gw3761 p, pM, pS;
	time_t tTime;
	uint_t nCnt;
	t_afn04_f1 xF1;
	t_afn04_f3 xF3;
	t_afn04_f85 xF85;

	pM = &rcp_aGw3761[0];
	pS = &rcp_aGw3761[1];
	gw3761_Init(pM);
	gw3761_Init(pS);
	for (nCnt = 0; ; ) {
		if (tTime != rtc_GetTimet()) {
			tTime = rtc_GetTimet();
			if ((nCnt & 0x3F) == 0) {
				icp_ParaRead(4, 85, TERMINAL, &xF85, sizeof(t_afn04_f85));
#if 0
				icp_ParaRead(4, 1, TERMINAL, &xF1, sizeof(t_afn04_f1));
				if (xF1.span < 1)
					xF1.span = 1;
#else
				xF1.span = 2;
#endif
				for (p = pM; p <= pS; p++) {
					p->rtua = xF85.area;
					p->terid = xF85.addr;
					p->parent.tmo = 5;
					p->parent.retry = 3;
					p->parent.refresh = xF1.span * 60;
				}
				icp_ParaRead(4, 3, TERMINAL, &xF3, sizeof(t_afn04_f3));
				modem_Config(xF3.apn, xF1.span * 60, 5);
				dlrcp_SetChl(&pM->parent, CHL_T_SOC_TC, xF3.port1, xF3.ip1[0], xF3.ip1[1], xF3.ip1[2], xF3.ip1[3]);
				dlrcp_SetChl(&pS->parent, CHL_T_RS232, 0, 9600, UART_PARI_EVEN, UART_DATA_8D, UART_STOP_1D);
			}
			nCnt += 1;
		}
		if (gw3761_Handler(pM) == SYS_R_OK) {
			SETBIT(g_sys_status, SYS_STATUS_LOGIN);
			if (pM->rmsg.c.dir == GW3761_DIR_RECV) {
				if (gw3761_RecvCheck(pM)) {
					gw3761_Response(pM);
				} else {
					//级联转发
					if (ecl_485_Wait(20000) == SYS_R_OK) {
						gw3761_Transmit(pM, pS);
						if (upcom_Wait(pS, 3000) == SYS_R_OK)
							gw3761_Transmit(pS, pM);
						ecl_485_Release();
					}
				}
			}
		}
		if (ecl_485_Wait(OS_TICK_MS) == SYS_R_OK) {
			if (gw3761_Handler(pS) == SYS_R_OK) {
				if (pS->rmsg.c.dir == GW3761_DIR_RECV) {
					if (gw3761_RecvCheck(pS))
						gw3761_Response(pS);
				}
			}
			ecl_485_Release();
		}
	}
}

void dbg_trace(const char *str)
{

	if (rcp_aGw3761[2].parent.chl->ste == CHL_S_READY) {
		chl_Send(rcp_aGw3761[2].parent.chl, dbg_header, sizeof(dbg_header));
		chl_Send(rcp_aGw3761[2].parent.chl, str, strlen(str));
	}
}

static t_gd5100 rcp_GD5100;
void tsk_Upcom2(void *args)
{
	p_gw3761 p;
	t_afn04_f85 xF85;

	icp_ParaRead(4, 85, TERMINAL, &xF85, sizeof(t_afn04_f85));
	for (p = &rcp_aGw3761[2]; p < ARR_ENDADR(rcp_aGw3761); p++) {
		gw3761_Init(p);
		p->rtua = xF85.area;
		p->terid = xF85.addr;
		p->parent.tmo = 5;
		p->parent.retry = 3;
		p->parent.refresh = 3 * 60;
	}
	//远程调试端口9000
	dlrcp_SetChl(&rcp_aGw3761[2].parent, CHL_T_SOC_TS, 9000, 0, 0, 0, 0);
	//远程GW3761规约端口8000
	dlrcp_SetChl(&rcp_aGw3761[3].parent, CHL_T_SOC_TS, 8000, 0, 0, 0, 0);
	//红外GW3761规约
	dlrcp_SetChl(&rcp_aGw3761[4].parent, CHL_T_RS232, 3, 1200, UART_PARI_EVEN, UART_DATA_8D, UART_STOP_1D);

	gd5100_Init(&rcp_GD5100);
	rcp_GD5100.rtua = xF85.area;
	rcp_GD5100.terid = xF85.addr;
	rcp_GD5100.parent.tmo = 5;
	rcp_GD5100.parent.retry = 3;
	rcp_GD5100.parent.refresh = 3 * 60;
	//远程南网GD5100规约端口777
	dlrcp_SetChl(&rcp_GD5100.parent, CHL_T_SOC_TS, 777, 0, 0, 0, 0);

	for (; ; ) {
		for (p = &rcp_aGw3761[2]; p < ARR_ENDADR(rcp_aGw3761); p++) {
			if (gw3761_Handler(p) == SYS_R_OK) {
				SETBIT(g_sys_status, SYS_STATUS_LOGIN);
				gw3761_Response(p);
			}
		}
		if (gd5100_Handler(&rcp_GD5100) == SYS_R_OK) {
			SETBIT(g_sys_status, SYS_STATUS_LOGIN);
			gd5100_Response(&rcp_GD5100);
		}
	}
}


