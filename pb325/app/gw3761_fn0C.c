#include <string.h>
#include <litecore.h>
#include "data.h"
#include "meter.h"
#include "alarm.h"
#include "acm.h"


//Private Defines
#define ECL_RTR_T_DEFAULT			0
#define ECL_RTR_T_ENERGY			1
#define ECL_RTR_T_MAXPOWER			2
#define ECL_RTR_T_POWER				3
#define ECL_RTR_T_VOL				4
#define ECL_RTR_T_CUR				5
#define ECL_RTR_T_CUR0				6
#define ECL_RTR_T_PF				7


//Private Typedefs
#define XBCount                     21


//Private Consts
static t_ecl_rtdi tbl_Di_Afn0CF25[] = {
	0,	ECL_RTR_T_POWER,		0x901F,	0x0203FF00,	//有功功率
	0,	ECL_RTR_T_POWER,		0x901F,	0x0204FF00,	//无功功率
	0,	ECL_RTR_T_PF,			0x901F,	0x0206FF00,	//功率因数
	0,	ECL_RTR_T_VOL,			0x901F,	0x0201FF00,	//电压
	0,	ECL_RTR_T_CUR,			0x901F,	0x0202FF00,	//电流
	0,	ECL_RTR_T_CUR0,			0x901F,	0x02080001,	//零序电流
	0,	ECL_RTR_T_POWER,		0x901F,	0x0205FF00,	//视在功率
};

static t_ecl_rtdi tbl_Di_Afn0CF33[] = {
	1,	ECL_RTR_T_ENERGY,		0x901F,	0x0001FF00,
	0,	ECL_RTR_T_DEFAULT,		0x911F,	0x0003FF00,
	0,	ECL_RTR_T_DEFAULT,		0x913F,	0x0005FF00,
	0,	ECL_RTR_T_DEFAULT,		0x914F,	0x0008FF00,
};

static t_ecl_rtdi tbl_Di_Afn0CF34[] = {
	1,	ECL_RTR_T_ENERGY,		0x902F,	0x0002FF00,
	0,	ECL_RTR_T_DEFAULT,		0x912F,	0x0004FF00,
	0,	ECL_RTR_T_DEFAULT,		0x915F,	0x0006FF00,
	0,	ECL_RTR_T_DEFAULT,		0x916F,	0x0007FF00,
};

static t_ecl_rtdi tbl_Di_Afn0CF35[] = {
	0,	ECL_RTR_T_MAXPOWER,		0xA01F,	0x0001FF00,
	0,	ECL_RTR_T_DEFAULT,		0xB01F, 0x0003FF00,
	0,	ECL_RTR_T_MAXPOWER,		0xA11F,	0x0003FF00,
	0,	ECL_RTR_T_DEFAULT,		0xB11F, 0x0003FF00,
};

static t_ecl_rtdi tbl_Di_Afn0CF36[] = {
	0,	ECL_RTR_T_MAXPOWER,		0xA02F,	0x0001FF00,
	0,	ECL_RTR_T_DEFAULT,		0xB02F, 0x0003FF00,
	0,	ECL_RTR_T_MAXPOWER,		0xA12F,	0x0003FF00,
	0,	ECL_RTR_T_DEFAULT,		0xB12F, 0x0003FF00,
};



//Internal Functions
static int gw3761_Afn0C_RealRead(buf b, t_afn04_f10 *pF10, t_ecl_rtdi *p)
{
	sys_res res;
	int nRet;
	uint_t i, nDiLen, nCode, nBaud, nLen;
	uint32_t nDi;
	uint8_t *pTemp;
	buf bTx = {0};

	if (pF10->prtl == ECL_PRTL_DLT645_97) {
		nDi = p->di97;
		nDiLen = 2;
		nCode = DLT645_CODE_READ97;
		nBaud = 1200;
	} else {
		nDi = p->di07;
		nDiLen = 4;
		nCode = DLT645_CODE_READ07;
		nBaud = 2400;
	}
	switch (p->type) {
	case ECL_RTR_T_ENERGY:
		nLen = (ECL_RATE_QTY + 1) * 5;
		break;
	case ECL_RTR_T_MAXPOWER:
		nLen = (ECL_RATE_QTY + 1) * 3;
		break;
	case ECL_RTR_T_POWER:
		nLen = 12;
		break;
	case ECL_RTR_T_VOL:
		nLen = 6;
		break;
	case ECL_RTR_T_CUR:
		nLen = 9;
		break;
	case ECL_RTR_T_CUR0:
		nLen = 3;
		break;
	case ECL_RTR_T_PF:
		nLen = 8;
		break;
	default:
		nLen = (ECL_RATE_QTY + 1) * 4;
		break;
	}
	dlt645_Packet2Buf(bTx, pF10->madr, nCode, &nDi, nDiLen);
	res = ecl_485_RealRead(bTx, nBaud, 2);
	if ((res == SYS_R_OK) && (bTx->p[8] == (nCode | BITMASK(7)))) {
		if (p->type == ECL_RTR_T_ENERGY) {
			pTemp = &bTx->p[10 + nDiLen];
			for (i = 0; i <= ECL_RATE_QTY; i++, pTemp += 4) {
				buf_PushData(b, 0x00, 1);
				buf_Push(b, pTemp, 4);
			}
		} else
			buf_Push(b, &bTx->p[10 + nDiLen], nLen);
		nRet = 1;
	} else {
		buf_Fill(b, GW3761_DATA_INVALID, nLen);
		nRet = 0;
	}
	buf_Release(bTx);
	return nRet;
}






static void gw3761_Afn0C_F3(buf b)
{
	uint8_t	aBuf[31] = {0};
	uint_t i;

	setbit(aBuf, 9, 1);
	setbit(aBuf, 24, 1);
	for (i = 0; i < 83; i++) {
		if (icp_ParaRead(4, i + 1, TERMINAL, NULL, 0) >= 0)
			setbit(aBuf, i, 1);
	}
	buf_Push(b, aBuf, sizeof(aBuf));
}

static void gw3761_Afn0C_F25(buf b, uint_t nDa)
{
	t_afn04_f10 xF10;
	uint_t i;
	uint8_t aBuf[5];
	t_ecl_rtdi *pR;
	t_acm_rtdata *pD;

	if (icp_Meter4Tn(nDa, &xF10) == 0)
		return;
	switch (xF10.port) {
	case ECL_PORT_ACM:
		if (acm_IsReady()) {
			pD = &acm_rtd;
			gw3761_ConvertData_15(aBuf, rtc_GetTimet());
			buf_Push(b, aBuf, 5);
			for (i = 0; i < 4; i++) {
				gw3761_ConvertData_09(aBuf, FLOAT2FIX(pD->pp[i]), 1);
				buf_Push(b, aBuf, 3);
			}
			for (i = 0; i < 4; i++) {
				gw3761_ConvertData_09(aBuf, FLOAT2FIX(pD->pq[i]), 1);
				buf_Push(b, aBuf, 3);
			}
			for (i = 0; i < 4; i++) {
				gw3761_ConvertData_05_Percent(aBuf, FLOAT2FIX(pD->cos[i]), 1);
				buf_Push(b, aBuf, 2);
			}
			for (i = 0; i < 3; i++) {
				gw3761_ConvertData_07(aBuf, FLOAT2FIX(pD->vol[i]));
				buf_Push(b, aBuf, 2);
			}
			for (i = 0; i < 4; i++) {
				gw3761_ConvertData_25(aBuf, FLOAT2FIX(pD->cur[i]), 0);
				buf_Push(b, aBuf, 3);
			}
			for (i = 0; i < 4; i++) {
				gw3761_ConvertData_09(aBuf, FLOAT2FIX(pD->vi[i]), 0);
				buf_Push(b, aBuf, 3);
			}
		}
		break;
	default:
		gw3761_ConvertData_15(aBuf, rtc_GetTimet());
		buf_Push(b, aBuf, 5);
		for (pR = tbl_Di_Afn0CF25; pR < ARR_ENDADR(tbl_Di_Afn0CF25); pR++)
			gw3761_Afn0C_RealRead(b, &xF10, pR);
		break;
	}
}

static void gw3761_Afn0C_F33(buf b, uint_t nDa)
{
	t_afn04_f10 xF10;
	uint8_t aBuf[5];
	t_ecl_rtdi *pR;

	if (icp_Meter4Tn(nDa, &xF10) == 0)
		return;
	switch (xF10.port) {
	case ECL_PORT_ACM:
		break;
	default:
		gw3761_ConvertData_15(aBuf, rtc_GetTimet());
		buf_Push(b, aBuf, 5);
		buf_PushData(b, ECL_RATE_QTY, 1);
		for (pR = tbl_Di_Afn0CF33; pR < ARR_ENDADR(tbl_Di_Afn0CF33); pR++)
			gw3761_Afn0C_RealRead(b, &xF10, pR);
		break;
	}
}

static void gw3761_Afn0C_F34(buf b, uint_t nDa)
{
	t_afn04_f10 xF10;
	uint8_t aBuf[5];
	t_ecl_rtdi *pR;

	if (icp_Meter4Tn(nDa, &xF10) == 0)
		return;
	switch (xF10.port) {
	case ECL_PORT_ACM:
		break;
	default:
		gw3761_ConvertData_15(aBuf, rtc_GetTimet());
		buf_Push(b, aBuf, 5);
		buf_PushData(b, ECL_RATE_QTY, 1);
		for (pR = tbl_Di_Afn0CF34; pR < ARR_ENDADR(tbl_Di_Afn0CF34); pR++)
			gw3761_Afn0C_RealRead(b, &xF10, pR);
		break;
	}
}

static void gw3761_Afn0C_F35(buf b, uint_t nDa)
{
	t_afn04_f10 xF10;
	uint8_t aBuf[5];
	t_ecl_rtdi *pR;

	if (icp_Meter4Tn(nDa, &xF10) == 0)
		return;
	switch (xF10.port) {
	case ECL_PORT_ACM:
		break;
	default:
		gw3761_ConvertData_15(aBuf, rtc_GetTimet());
		buf_Push(b, aBuf, 5);
		buf_PushData(b, ECL_RATE_QTY, 1);
		for (pR = tbl_Di_Afn0CF35; pR < ARR_ENDADR(tbl_Di_Afn0CF35); pR++)
			gw3761_Afn0C_RealRead(b, &xF10, pR);
		break;
	}
}

static void gw3761_Afn0C_F36(buf b, uint_t nDa)
{
	t_afn04_f10 xF10;
	uint8_t aBuf[5];
	t_ecl_rtdi *pR;

	if (icp_Meter4Tn(nDa, &xF10) == 0)
		return;
	switch (xF10.port) {
	case ECL_PORT_ACM:
		break;
	default:
		gw3761_ConvertData_15(aBuf, rtc_GetTimet());
		buf_Push(b, aBuf, 5);
		buf_PushData(b, ECL_RATE_QTY, 1);
		for (pR = tbl_Di_Afn0CF36; pR < ARR_ENDADR(tbl_Di_Afn0CF36); pR++)
			gw3761_Afn0C_RealRead(b, &xF10, pR);
		break;
	}
}

static void gw3761_Afn0C_F49(buf b, uint_t nDa)
{
	t_afn04_f10 xF10;
	uint_t i;
	uint8_t aBuf[5];
	t_acm_rtdata *pD;

	if (icp_Meter4Tn(nDa, &xF10) == 0)
		return;
	switch (xF10.port) {
	case ECL_PORT_ACM:
		if (acm_IsReady()) {
			pD = &acm_rtd;
			for (i = 0; i < 3; i++) {
				gw3761_ConvertData_05(aBuf, FLOAT2FIX(pD->au[i]), 1);
				buf_Push(b, aBuf, 2);
			}
			for (i = 0; i < 3; i++) {
				gw3761_ConvertData_05(aBuf, FLOAT2FIX(pD->ai[i]), 1);
				buf_Push(b, aBuf, 2);
			}
		}
		break;
	default:
		break;
	}
}

static void gw3761_Afn0C_F57(buf b, uint_t nDa)
{
	t_afn04_f10 xF10;
	uint_t i, j;
	uint8_t aBuf[5];
	t_acm_xbdata *pD;

	if (icp_Meter4Tn(nDa, &xF10) == 0)
		return;
	switch (xF10.port) {
	case ECL_PORT_ACM:
		if (acm_IsReady()) {
			buf_PushData(b, 19, 1);
			for (i = 0; i < 6; i++) {
				if (i < 3)
					pD = &acm_uxb[i];
				else
					pD = &acm_ixb[i - 3];
				for (j = 1; j < 19; j++) {
					if (j & 1) {
						buf_Fill(b, 0, 2);
					} else {
						if (i < 3)
							gw3761_ConvertData_07(aBuf, FLOAT2FIX(pD->xbrate[j >> 1] * pD->base / 100.0f));
						else
							gw3761_ConvertData_06(aBuf, FLOAT2FIX(pD->xbrate[j >> 1] * pD->base / 100.0f), 1);
						buf_Push(b, aBuf, 2);
					}
				}
			}
		}
		break;
	default:
		break;
	}
}

static void gw3761_Afn0C_F58(buf b, uint_t nDa)
{
	t_afn04_f10 xF10;
	uint_t i, j, k;
	uint8_t aBuf[5];
	t_acm_xbdata *pD;

	if (icp_Meter4Tn(nDa, &xF10) == 0)
		return;
	switch (xF10.port) {
	case ECL_PORT_ACM:
		if (acm_IsReady()) {
			buf_PushData(b, XBCount, 1);
			for (i = 0; i < 6; i++) {
				if (i < 3) {
					pD = &acm_uxb[i];
					k = 0;
				} else {
					pD = &acm_ixb[i - 3];
					k = 1;
				}
				for (j = k; j < XBCount; j++) {
					if (j & 1) {
						buf_Fill(b, 0, 2);
					} else {
						gw3761_ConvertData_05(aBuf, FLOAT2FIX(pD->xbrate[j >> 1]), 0);
						buf_Push(b, aBuf, 2);
					}
				}
			}
		}
		break;
	default:
		break;
	}
}


//External Functions
int gw3761_ResponseData1(p_gw3761 p)
{
#if GW3761_TYPE == GW3761_T_GWJC2009
	uint16_t aDa[8];
#else
	uint8_t aDa[64];
#endif
	int res = 0;
	u_word2 uDu;
	uint_t i, j, nDa, nDaQty, nFn;
	uint8_t *pData, *pEnd, aBuf[24];
	buf b = {0};

	pData = p->rmsg.data->p;
	pEnd = pData + p->rmsg.data->len;
	for (; (pData + 4) <= pEnd; ) {
		memcpy(&uDu, pData, 4);
		buf_Push(b, pData, 4);
		pData += 4;
		nDaQty = gw3761_ConvertDa2Map(uDu.word[0], aDa);
		for (i = 0; i < nDaQty; i++) {
			nDa = aDa[i];
			for (j = 0; j < 8; j++) {
				if ((uDu.word[1] & BITMASK(j)) == 0)
					continue;
				nFn = gw3761_ConvertDt2Fn((uDu.word[1] & 0xFF00) | BITMASK(j));
				switch (nFn) {
				case 2:
					//终端日历时钟
					gw3761_ConvertData_01(aBuf, rtc_GetTimet());
					buf_Push(b, aBuf, 6);
					break;
				case 3:
					//终端参数状态
					gw3761_Afn0C_F3(b);
					break;
				case 4:
					//终端上行通讯状态
					buf_PushData(b, 5, 1);
					break;
				case 7:
					//终端事件计数器
					buf_PushData(b, evt_GetCount(), 2);
					break;
				case 8:
					//终端事件标志状态
					evt_GetFlag(aBuf);
					buf_Push(b, aBuf, 8);
					break;
				case 9:
					//终端状态量及变位标志
					buf_PushData(b, 0, 2);
					break;
				case 10:
					//终端与主站当日月通信量
					buf_PushData(b, 0, 4);
					buf_PushData(b, 0, 4);
					break;
				case 16:
					//配电扩展,10次上电时间
					data_RuntimeRead(b);
					break;
				case 25:
					//当前电压电流功率
					gw3761_Afn0C_F25(b, nDa);
					break;
				case 33:
					//当前正向有无功电能示值
					gw3761_Afn0C_F33(b, nDa);
					break;
				case 34:
					//当前反向有无功电能示值
					gw3761_Afn0C_F34(b, nDa);
					break;
				case 35:
					//当前正向有无功最大需量
					gw3761_Afn0C_F35(b, nDa);
					break;
				case 36:
					//当前反向有无功最大需量
					gw3761_Afn0C_F36(b, nDa);
					break;
				case 49:
					//当前电压电流相位角
					gw3761_Afn0C_F49(b, nDa);
					break;
				case 57:
					//当前电压电流谐波有效值
					gw3761_Afn0C_F57(b, nDa);
					break;
				case 58:
					//当前电压电流谐波含有率
					gw3761_Afn0C_F58(b, nDa);
					break;
				default:
					break;
				}
			}
		}
		if (b->len > 4) {
			gw3761_TmsgSend(p, GW3761_FUN_RESPONSE, GW3761_AFN_DATA_L1, b, DLRCP_TMSG_RESPOND);
			res += 1;
		}
		buf_Release(b);
	}
	if (res == 0)
		gw3761_TmsgReject(p);
	return res;
}


