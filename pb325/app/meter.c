#include <stdio.h>
#include <string.h>
#include <litecore.h>
#include "system.h"
#include "data.h"
#include "meter.h"
#include "alarm.h"
#include "acm.h"

//Private Defines
#define DLT645_ITEM_T_OTHER			0

#define DLT645_ITEM_T_SDEC_2		1
#define DLT645_ITEM_T_UDEC_2		2
#define DLT645_ITEM_T_UDEC_3		3
#define DLT645_ITEM_T_UDEC_4		4

#define DLT645_ITEM_T_UINT			5
#define DLT645_ITEM_T_BIN			6
#define DLT645_ITEM_T_TIME_YEAR		7
#define DLT645_ITEM_T_TIME_MON		8
#define DLT645_ITEM_T_TIME_HOUR		9

#define DLT645_METER_T_ALL			0xFF
#define DLT645_METER_T_1PHASE		BITMASK(0)
#define DLT645_METER_T_3PHASE		BITMASK(1)

#define ECL_USER_TYPE_S_1PHASE		1
#define ECL_USER_TYPE_S_3PHASE		2


#define ECL_TASK_S_IDLE				0
#define ECL_TASK_S_FAIL				1
#define ECL_TASK_S_HANDLE			2
#define ECL_TASK_S_AUTO				3
#define ECL_TASK_S_REAL				4
#define ECL_TASK_S_ERROR			5


//Private Typedefs
#define METER_DEBUG_ENABLE			1

//Private Consts


//Private Macors
#if METER_DEBUG_ENABLE
#define meter_DbgOut				dbg_trace
#else
#define meter_DbgOut(...)
#endif


//Private Variables
static t_ecl_task ecl_Task485;




//Internal Functions












//External Functions
sys_res ecl_485_RealRead(buf b, uint_t nBaud, uint_t nTmo)
{
	t_ecl_task *p = &ecl_Task485;

	if (p->ste == ECL_TASK_S_ERROR)
		return SYS_R_ERR;
	for (nTmo *= (1000 / OS_TICK_MS); nTmo; nTmo--) {
		if (p->ste == ECL_TASK_S_IDLE) {
			p->ste = ECL_TASK_S_REAL;
			break;
		}
		os_thd_Slp1Tick();
	}
	if (nTmo == 0)
		return SYS_R_TMO;

	chl_rs232_Config(p->chl, nBaud, UART_PARI_EVEN, UART_DATA_8D, UART_STOP_1D);
	for (nTmo = (nTmo / (1000 / OS_TICK_MS)) + 1; nTmo; nTmo--) {
		if (dlt645_Meter(p->chl, b, 1000) == SYS_R_OK)
			break;
	}
	p->ste = ECL_TASK_S_IDLE;
	if (nTmo == 0)
		return SYS_R_TMO;
	return SYS_R_OK;
}

void tsk_Meter(void *args)
{
	chl chlRS485;
	uint_t nTemp;
	uint8_t aBuf[12];
	t_afn04_f10 xPM;
	t_ecl_task *p = &ecl_Task485;
	buf b = {0};

	os_thd_Sleep(1000);
	if (g_sys_status & BITMASK(SYS_STATUS_UART)) {
		p->ste = ECL_TASK_S_ERROR;
		return;
	}
	memset(p, 0, sizeof(t_ecl_task));
	chl_Init(chlRS485);
	chl_Bind(chlRS485, CHL_T_RS232, 0, OS_TMO_FOREVER);
	p->chl = chlRS485;
	for (; ; os_thd_Sleep(10000)) {
		for (p->sn = 1; p->sn < ECL_SN_MAX; p->sn++) {
			if (icp_MeterRead(p->sn, &xPM) <= 0)
				continue;
			if (xPM.prtl != ECL_PRTL_DLQ_QL)
				continue;
			nTemp = 0xC040;
			dlt645_Packet2Buf(b, xPM.madr, DLT645_CODE_READ97, &nTemp, 2);
			if (ecl_485_RealRead(b, 1200, 2) == SYS_R_OK) {
				meter_DbgOut("<Meter%d> Read %X", p->sn, b->p[4]);
				nTemp = 1;
				if (evt_DlqQlStateGet(p->sn, aBuf) == SYS_R_OK) {
					meter_DbgOut("<Meter%d> State %X", p->sn, aBuf[0]);
					if (aBuf[0] != b->p[4])
						evt_ERC5(p->sn, aBuf, &b->p[4]);
					else
						nTemp = 0;
				}
				if (nTemp)
					evt_DlqQlStateSet(p->sn, &b->p[4]);
			}
			buf_Release(b);
			nTemp = 0xC04F;
			dlt645_Packet2Buf(b, xPM.madr, DLT645_CODE_READ97, &nTemp, 2);
			if (ecl_485_RealRead(b, 1200, 2) == SYS_R_OK) {
				nTemp = 1;
				if (evt_DlqQlParaGet(p->sn, aBuf) == SYS_R_OK) {
					if (memcmp(aBuf, &b->p[5], 11))
						evt_ERC8(p->sn, aBuf, &b->p[5]);
					else
						nTemp = 0;
				}
				if (nTemp) {
					memcpy(aBuf, &b->p[5], 11);
					evt_DlqQlParaSet(p->sn, aBuf);
				}
			}
			buf_Release(b);
		}
	}
}


