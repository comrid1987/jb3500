#include <stdio.h>
#include <string.h>
#include <litecore.h>
#include "system.h"
#include "data.h"
#include "meter.h"
#include "alarm.h"
#include "acm.h"

//Private Defines
#define METER_DEBUG_ENABLE			0

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
#define ECL_TASK_S_BUSY				1


//Private Typedefs

//Private Consts


//Private Macors
#if METER_DEBUG_ENABLE
#define meter_DbgOut				dbg_printf
#else
#define meter_DbgOut(...)
#endif


//Private Variables
t_ecl_task ecl_Task485;




//Internal Functions












//External Functions
sys_res ecl_485_Wait(uint_t nTmo)
{
	t_ecl_task *p = &ecl_Task485;

	for (nTmo /= OS_TICK_MS; nTmo; nTmo--) {
		if (p->ste == ECL_TASK_S_IDLE) {
			p->ste = ECL_TASK_S_BUSY;
			break;
		}
		os_thd_Slp1Tick();
	}
	if (nTmo == 0)
		return SYS_R_TMO;
	return SYS_R_OK;
}

void ecl_485_Release()
{
	t_ecl_task *p = &ecl_Task485;

	chl_rs232_Config(p->chl, 9600, UART_PARI_EVEN, UART_DATA_8D, UART_STOP_1D);
	p->ste = ECL_TASK_S_IDLE;
}

sys_res ecl_485_RealRead(buf b, uint_t nBaud, uint_t nTmo)
{
	sys_res res;
	t_ecl_task *p = &ecl_Task485;

	res = ecl_485_Wait(nTmo);
	if (res != SYS_R_OK)
		return res;
	
	chl_rs232_Config(p->chl, nBaud, UART_PARI_EVEN, UART_DATA_8D, UART_STOP_1D);
	res = dlt645_Meter(p->chl, b, nTmo);
	ecl_485_Release();
	return res;
}

//In upcomm.c
extern t_gw3761 rcp_aGw3761[5];

void tsk_Meter(void *args)
{
	uint_t nTemp;
	uint8_t aBuf[12];
	t_afn04_f10 xPM;
	t_ecl_task *p = &ecl_Task485;
	buf b = {0};

	os_thd_Sleep(1000);
	memset(p, 0, sizeof(t_ecl_task));
	p->chl = rcp_aGw3761[1].parent.chl;
	for (; ; os_thd_Sleep(10000)) {
		for (p->sn = 1; p->sn < ECL_SN_MAX; p->sn++) {
			if (icp_MeterRead(p->sn, &xPM) <= 0)
				continue;
			if (xPM.prtl == ECL_PRTL_DLQ_QL) {
				nTemp = 0xC040;
				dlt645_Packet2Buf(b, xPM.madr, DLT645_CODE_READ97, &nTemp, 2);
				if (ecl_485_RealRead(b, 1200, 2000) == SYS_R_OK) {
					meter_DbgOut("<Meter%d> Read %X", p->sn, b->p[4]);
					nTemp = 1;
					if (evt_DlqStateGet(p->sn, aBuf) == SYS_R_OK) {
						meter_DbgOut("<Meter%d> State %X", p->sn, aBuf[0]);
						if (aBuf[0] != b->p[4])
							evt_ERC5(p->sn, aBuf, &b->p[4]);
						else
							nTemp = 0;
					}
					if (nTemp)
						evt_DlqStateSet(p->sn, &b->p[4]);
				}
				buf_Release(b);
				nTemp = 0xC04F;
				dlt645_Packet2Buf(b, xPM.madr, DLT645_CODE_READ97, &nTemp, 2);
				if (ecl_485_RealRead(b, 1200, 2000) == SYS_R_OK) {
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
			if (xPM.prtl == ECL_PRTL_DLQ_SY) {
				nTemp = 0xA0000000;
				dlt645_Packet2Buf(b, xPM.madr, DLT645_CODE_READ07, &nTemp, 4);
				if (ecl_485_RealRead(b, 2400, 2000) == SYS_R_OK) {
					meter_DbgOut("<Meter%d> Read %X", p->sn, b->p[6]);
					nTemp = 1;
					if (evt_DlqStateGet(p->sn, aBuf) == SYS_R_OK) {
						meter_DbgOut("<Meter%d> State %X", p->sn, aBuf[0]);
						if (aBuf[0] != b->p[6])
							evt_ERC5(p->sn, aBuf, &b->p[6]);
						else
							nTemp = 0;
					}
					if (nTemp) {
						memcpy(aBuf, &b->p[6], 4);
						evt_DlqStateSet(p->sn, aBuf);
					}
				}
				buf_Release(b);
			}
		}
	}
}




