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
static t_stat acm_stat;



//Internal Functions






void stat_Clear()
{
	uint_t i;
	p_stat p = &acm_stat;

	memset(p, 0, sizeof(t_stat));
	evt_StatWrite(p);
}

  

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

void stat_Handler(p_stat ps, t_afn04_f26 *pF26, t_afn04_f28 *pF28, time_t tTime)
{
	uint_t i;
	uint32_t nData;
	float fData, fLow, fUp, fUnder, fOver;
	t_acm_rtdata *pa = &acm_rtd;
	
	ps->run += 1;
	//电压
	fLow = (float)bcd2bin16(pF26->ulow) / 10.0f;
	fUnder = (float)bcd2bin16(pF26->uunder) / 10.0f;
	fUp = (float)bcd2bin16(pF26->uup) / 10.0f;
	fOver = (float)bcd2bin16(pF26->uover) / 10.0f;
	nData = 1;
	for (i = 0; i < 3; i++) {
		fData = pa->u[i];
		ps->usum[i] += fData;
		if (fData < fLow) {
			ps->ulow[i] += 1;
			nData = 0;
		}
		if (fData > fUp) {
			ps->uup[i] += 1;
			nData = 0;
		}
		if (fData < fUnder) {
			ps->uunder[i] += 1;
			nData = 0;
		}
		if (fData > fOver) {
			ps->uover[i] += 1;
			nData = 0;
		}
		if (ps->tumin[i] == 0)
			ps->umin[i] = fData;
		if (fData <= ps->umin[i]) {
			ps->umin[i] = fData;
			ps->tumin[i] = tTime;
		}
		if (fData >= ps->umax[i]) {
			ps->umax[i] = fData;
			ps->tumax[i] = tTime;
		}
	}		
	if (nData)
		ps->uok += 1;
	//电流
	nData = 0;
	memcpy(&nData, pF26->iup, 3);
	fUp = (float)bcd2bin32(nData) / 1000.0f;
	memcpy(&nData, pF26->iover, 3);
	fOver = (float)bcd2bin32(nData) / 1000.0f;
	for (i = 0; i < 3; i++) {
		fData = pa->i[i];
		if (fData > fUp)
			ps->iup[i] += 1;
		if (fData > fOver)
			ps->iover[i] += 1;
		if (fData >= ps->imax[i]) {
			ps->imax[i] = fData;
			ps->timax[i] = tTime;
		}
	}
	fData = pa->i[3];
	if (fData > fUp)
		ps->iup[3] += 1;
	if (fData >= ps->imax[3]) {
		ps->imax[3] = fData;
		ps->timax[3] = tTime;
	}
	//不平衡
	fUp = (float)bcd2bin16(pF26->ubalance) / 1000.0f;
	fData = pa->ub;
	if (fData > fUp)
		ps->ubalance += 1;
	if (fData >= ps->ubmax) {
		ps->ubmax = fData;
		ps->tubmax = tTime;
	}
	fUp = (float)bcd2bin16(pF26->ibalance) / 1000.0f;
	fData = pa->ib;
	if (fData > fUp)
		ps->ibalance += 1;
	if (fData >= ps->ibmax) {
		ps->ibmax = fData;
		ps->tibmax = tTime;
	}
	//视在功率
	memcpy(&nData, pF26->uiup, 3);
	fUp = (float)bcd2bin32(nData) / 10000.0f;
	memcpy(&nData, pF26->uiover, 3);
	fOver = (float)bcd2bin32(nData) / 10000.0f;
	for (i = 0; i < 4; i++) {
		fData = pa->ui[i];
		//总视在功率越限
		if (i == 3) {
			if (fData > fUp)
				ps->uiup += 1;
			if (fData > fOver)
				ps->uiover+= 1;
		}
		//功率为零时间
		if (fData < 0.1f)
			ps->p0[i] += 1;
		if (fData >= ps->pmax[i]) {
			ps->pmax[i] = fData;
			ps->tpmax[i] = tTime;
		}
	}
	//视在不平衡度
	ps->uibsum += pa->uib;
	//功率因数分段统计
	fLow = (float)bcd2bin16(pF28->low) / 1000.0f;
	fUp = (float)bcd2bin16(pF28->up) / 1000.0f;
	fData = pa->cos[3];
	if (fData < fLow) {
		ps->cos[0] += 1;
	} else {
		if (fData > fUp)
			ps->cos[2] += 1;
		else
			ps->cos[1] += 1;
	}
}



void tsk_Meter(void *args)
{
	sys_res res;
	chl chlRS485;
	time_t tTime;
	int nMin= -1, nDay = -1;
	uint_t nCnt, nCode, nBaud;
	uint32_t nRecDI, nData1, nData2;
	uint8_t *pTemp, aBuf[6];
	p_stat ps = &acm_stat;
	t_ecl_energy xEnergy;
	t_ecl_task *p = &ecl_Task485;
	t_afn04_f26 xF26;
	t_afn04_f28 xF28;

	acm_Init();

	memset(p, 0, sizeof(t_ecl_task));
	if (g_sys_status & BITMASK(0)) {
		p->chl = chlRS485;
		chl_Init(chlRS485);
		chl_Bind(chlRS485, CHL_T_RS232, 0, OS_TMO_FOREVER);
	}

	//统计恢复
	if (evt_StatRead(ps) == 0)
		stat_Clear();
	nMin = rtc_pTm()->tm_min;
	nDay = rtc_pTm()->tm_mday;

	for (nCnt = 0; ; os_thd_Slp1Tick()) {
		//秒count
		if (tTime == rtc_GetTimet())
			continue;
		tTime = rtc_GetTimet();
		if ((nCnt & 0x3F) == 0) {
			icp_ParaRead(4, 26, TERMINAL, &xF26, sizeof(t_afn04_f26));
			icp_ParaRead(4, 28, TERMINAL, &xF28, sizeof(t_afn04_f28));
		}
 		if ((nCnt & 0x0F) == 0)
            acm_XBRead();
		if ((nCnt & 0x1F) == 0) {
            acm_JLRead();
			evt_Terminal(&xF26);
		}
		nCnt += 1;
		//分钟
		if (nMin != rtc_pTm()->tm_min) {
			nMin = rtc_pTm()->tm_min;
			evt_RunTimeWrite(tTime);
			timet2array(tTime, aBuf, 1);
			acm_MinSave(aBuf);
			if ((nMin % 15) == 0)
				acm_QuarterSave(aBuf);

			stat_Handler(ps, &xF26, &xF28, tTime);
			//统计保存
			if ((nMin & 3) == 0)
				evt_StatWrite(ps);
			//跨日
			if (nDay != rtc_pTm()->tm_mday) {
				nDay = rtc_pTm()->tm_mday;
				day4timet(tTime, -1, aBuf, 1);
				data_DayWrite(aBuf, ps);
				stat_Clear();
			}
		}
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


