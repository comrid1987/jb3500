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
	t_ecl_task *p = &ecl_Task485;

	if (g_sys_status & BITMASK(SYS_STATUS_UART))
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
	time_t tTime;
	uint_t nCnt;
	t_ecl_task *p = &ecl_Task485;
	
	memset(p, 0, sizeof(t_ecl_task));
	if ((g_sys_status & BITMASK(SYS_STATUS_UART)) == 0) {
		chl_Init(chlRS485);
		chl_Bind(chlRS485, CHL_T_RS232, 0, OS_TMO_FOREVER);
		p->chl = chlRS485;
		for (nCnt = 0; ; os_thd_Slp1Tick()) {
			//秒count
			if (tTime == rtc_GetTimet())
				continue;
			tTime = rtc_GetTimet();
			for (p->sn = 1; p->sn < ECL_SN_MAX; p->sn++) {
				if (icp_MeterRead(p->sn, &p->f10) < 0)
					continue;


			}
		}
	}
}

#if 0
			if ((g_sys_status & BITMASK(SYS_STATUS_UART))) {
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


