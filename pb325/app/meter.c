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


//Private Typedefs


//Private Consts




//Private Variables
static t_ecl_task ecl_Task485;



//Internal Functions




#if 0
void ecl_DataHandler(uint_t nTn, const uint8_t *pAdr, const uint8_t *pTime, uint32_t nRecDI, const void *pData)
{
	t_ecl_energy xEnergy;

	switch (nRecDI) {
	case 0x901F:
	case 0x05060101:
		//日冻结
		data_DayRead(nTn, pAdr, pTime, &xEnergy);
		if (xEnergy.time == GW3761_DATA_INVALID) {
			xEnergy.time = rtc_GetTimet();
			memcpy(&xEnergy.data, pData, 20);
			data_DayWrite(nTn, pAdr, pTime, &xEnergy);
		}
		break;
	default:
		break;
	}
}
#endif


sys_res ecl_485_RealRead(buf b, uint_t nBaud, uint_t nTmo)
{
	sys_res res;
	t_ecl_task *p = &ecl_Task485;
	buf bTx = {0};

	if ((g_sys_status & BITMASK(0)) == 0)
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
	buf_Push(bTx, b->p, b->len);
	for (nTmo = (nTmo / (1000 / OS_TICK_MS)) + 1; nTmo; nTmo--) {
		res = dlt645_Transmit2Meter(p->chl, b, &bTx->p[1], bTx->p, bTx->len, 1000);
		if (res == SYS_R_OK)
			break;
	}
	buf_Release(bTx);
	p->ste = ECL_TASK_S_IDLE;
	return res;
}

void tsk_Meter(void *args)
{
	sys_res res;
	chl chlRS485;
	time_t tTime;
	int nMin= -1, nHour = -1, nDay = -1;
	uint_t nCnt, nLen, nCode, nBaud;
	uint32_t nRecDI, nData1, nData2;
	uint8_t *pTemp, aTime[6];
	t_ecl_energy xEnergy;
	t_ecl_task *p = &ecl_Task485;
	buf b = {0};

	acm_Init();

	data_RuntimeWrite();

	memset(p, 0, sizeof(t_ecl_task));
	if (g_sys_status & BITMASK(0)) {
		p->chl = chlRS485;
		chl_Init(chlRS485);
		chl_Bind(chlRS485, CHL_T_RS232, 0, OS_TMO_FOREVER);
	}
	for (nCnt = 0; ; nCnt++) {
		//秒count
		os_thd_Sleep(1000);
		timet2array(rtc_GetTimet(), aTime, 1);
 		if ((nCnt & 0x0F) == 0)
            acm_XBRead();
		if ((nCnt & 0x1F) == 0)
            acm_JLRead();
		//分钟
		if (nMin != aTime[1]) {
			nMin = aTime[1];
			icp_RunTimeWrite(rtc_GetTimet());
			acm_MinSave(aTime);
			if ((bcd2bin8(nMin) % 15) == 0)
				acm_QuarterSave(aTime);
		}
		//小时
		if (nHour != aTime[2]) {
			nHour = aTime[2];


		}
		//日
		if (nDay != aTime[3]) {
			nDay = aTime[3];
			//交采冻结
			if (acm_IsReady()) {
				day4timet(rtc_GetTimet(), -1, p->time, 1);
//				acm_DaySave(p->time);
			}
		}
#if 0
		if ((g_sys_status & BITMASK(0))) {
			switch (p->ste) {
			case ECL_TASK_S_IDLE:
				if ((rtc_GetTimet() - tTime) > 60) {
					if (p->ste == ECL_TASK_S_IDLE) {
						p->ste = ECL_TASK_S_AUTO;
						tTime = rtc_GetTimet();
					}
				}
				break;
			case ECL_TASK_S_AUTO:
				monthprev(tTime, p->time, 1);
				for (p->sn = 1; p->sn < ECL_SN_MAX; p->sn++) {
					//下一个有效电表
					if (icp_MeterRead(p->sn, &p->f10) == 0)
						continue;
					if (p->f10.port != ECL_PORT_RS485)
						continue;
					if (p->f10.prtl == ECL_PRTL_DLT645_97) {
						nLen = 2;
						nCode = DLT645_CODE_READ97;
						nBaud = 1200;
					} else {
						nLen = 4;
						nCode = DLT645_CODE_READ07;
						nBaud = 2400;
					}
					chl_rs232_Config(chlRS485, nBaud, UART_PARI_EVEN, UART_DATA_8D, UART_STOP_1D);
					data_DayRead(p->f10.tn, p->f10.madr, p->time, &xEnergy);
					if (xEnergy.time == GW3761_DATA_INVALID) {
						if (p->f10.prtl == ECL_PRTL_DLT645_97)
							p->di = 0x901F;
						else
							p->di = 0x05060101;
						dlt645_Packet2Buf(b, p->f10.madr, nCode, &p->di, nLen);
						res = dlt645_Transmit2Meter(chlRS485, b, p->f10.madr, b->p, b->len, 3000);
						if ((res == SYS_R_OK) && (b->p[8] == (nCode | BITMASK(7)))) {
							nRecDI = 0;
							pTemp = &b->p[10];
							memcpy(&nRecDI, pTemp, nLen);
							pTemp += nLen;
							ecl_DataHandler(p->f10.tn, p->f10.madr, p->time, nRecDI, pTemp);
						}
						buf_Release(b);
					}
				}
				p->ste = ECL_TASK_S_IDLE;
				break;
			default:
				break;
			}
		}
#endif
	}
}



