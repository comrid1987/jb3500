#include <string.h>
#include <litecore.h>
#include "data.h"
#include "meter.h"
#include "alarm.h"
#include "acm.h"


//Private Defines
#define ECL_RTR_T_NONE				0
#define ECL_RTR_T_ENERGY_4			1
#define ECL_RTR_T_ENERGY_5			2
#define ECL_RTR_T_MAXPOWER			3
#define ECL_RTR_T_POWER				4
#define ECL_RTR_T_VOL				5
#define ECL_RTR_T_CUR				6
#define ECL_RTR_T_CUR0				7
#define ECL_RTR_T_PF				8
#define ECL_RTR_T_MAXPOWER07		9

#define ECL_RTR_T_DLQ_RT			16
#define ECL_RTR_T_DLQ_STATE			17


//Private Typedefs



//Private Consts
static t_ecl_rtdi07 tbl_Di07_Afn0CF25[] = {
	0,	ECL_RTR_T_POWER,		0x0203FF00,	//有功功率
	0,	ECL_RTR_T_POWER,		0x0204FF00,	//无功功率
	0,	ECL_RTR_T_PF,			0x0206FF00,	//功率因数
	1,	ECL_RTR_T_VOL,			0x0201FF00,	//电压
	1,	ECL_RTR_T_CUR,			0x0202FF00,	//电流
	0,	ECL_RTR_T_CUR0,			0x02080001,	//零序电流
	0,	ECL_RTR_T_POWER,		0x0205FF00,	//视在功率
};

static t_ecl_rtdi97 tbl_Di97_Afn0CF25[] = {
	0,	ECL_RTR_T_POWER,		0xB63F,	//有功功率
	0,	ECL_RTR_T_POWER,		0xB64F,	//无功功率
	0,	ECL_RTR_T_PF,			0xB65F,	//功率因数
	1,	ECL_RTR_T_VOL,			0xB61F,	//电压
	1,	ECL_RTR_T_CUR,			0xB62F,	//电流
	0,	ECL_RTR_T_CUR0,			0xB620,	//零序电流
	0,	ECL_RTR_T_NONE,			12,	//视在功率,97表无
};

static t_ecl_rtdi07 tbl_Di07_Afn0CF33[] = {
	1,	ECL_RTR_T_ENERGY_5,		0x0001FF00,
	0,	ECL_RTR_T_ENERGY_4,		0x0003FF00,
	0,	ECL_RTR_T_ENERGY_4,		0x0005FF00,
	0,	ECL_RTR_T_ENERGY_4,		0x0008FF00,
};

static t_ecl_rtdi97 tbl_Di97_Afn0CF33[] = {
	1,	ECL_RTR_T_ENERGY_5,		0x901F,
	0,	ECL_RTR_T_ENERGY_4,		0x911F,
	0,	ECL_RTR_T_ENERGY_4,		0x913F,
	0,	ECL_RTR_T_ENERGY_4,		0x914F,
};

static t_ecl_rtdi07 tbl_Di07_Afn0CF34[] = {
	1,	ECL_RTR_T_ENERGY_5,		0x0002FF00,
	0,	ECL_RTR_T_ENERGY_4,		0x0004FF00,
	0,	ECL_RTR_T_ENERGY_4,		0x0006FF00,
	0,	ECL_RTR_T_ENERGY_4,		0x0007FF00,
};

static t_ecl_rtdi97 tbl_Di97_Afn0CF34[] = {
	1,	ECL_RTR_T_ENERGY_5,		0x902F,
	0,	ECL_RTR_T_ENERGY_4,		0x912F,
	0,	ECL_RTR_T_ENERGY_4,		0x915F,
	0,	ECL_RTR_T_ENERGY_4,		0x916F,
};

static t_ecl_rtdi97 tbl_Di97_Afn0CF35[] = {
	0,	ECL_RTR_T_MAXPOWER,		0xA01F,
	0,	ECL_RTR_T_ENERGY_4,		0xB01F,
	0,	ECL_RTR_T_MAXPOWER,		0xA11F,
	0,	ECL_RTR_T_ENERGY_4,		0xB11F,
};

static t_ecl_rtdi07 tbl_Di07_Afn0CF35[] = {
	0,	ECL_RTR_T_MAXPOWER07,	0x0101FF00,
	0,	ECL_RTR_T_MAXPOWER07,	0x0103FF00,
};

static t_ecl_rtdi97 tbl_Di97_Afn0CF36[] = {
	0,	ECL_RTR_T_MAXPOWER,		0xA02F,
	0,	ECL_RTR_T_ENERGY_4,		0xB02F,
	0,	ECL_RTR_T_MAXPOWER,		0xA12F,
	0,	ECL_RTR_T_ENERGY_4,		0xB12F,
};

static t_ecl_rtdi07 tbl_Di07_Afn0CF36[] = {
	0,	ECL_RTR_T_MAXPOWER07,	0x0102FF00,
	0,	ECL_RTR_T_MAXPOWER07,	0x0104FF00,
};

static t_ecl_rtdi07 tbl_DiSY_Afn0CF64[] = {
	0,	ECL_RTR_T_DLQ_RT,		0x02FF0000, //电压电流
	0,	ECL_RTR_T_DLQ_STATE, 	0x02FF0000, //状态
};

static t_ecl_rtdi97 tbl_DiQL_Afn0CF64[] = {
	0,	ECL_RTR_T_DLQ_RT,		0xB66F,	//电压电流
	0,	ECL_RTR_T_DLQ_STATE,	0xC04F,	//状态
};



//Internal Functions
static int gw3761_Afn0C_Type(int nType)
{

	switch (nType) {
	case ECL_RTR_T_ENERGY_5:
		return (ECL_RATE_QTY + 1) * 5;
	case ECL_RTR_T_MAXPOWER:
		return (ECL_RATE_QTY + 1) * 3;
	case ECL_RTR_T_POWER:
		return 12;
	case ECL_RTR_T_VOL:
		return 6;
	case ECL_RTR_T_CUR:
		return 9;
	case ECL_RTR_T_CUR0:
		return 3;
	case ECL_RTR_T_PF:
		return 8;
	case ECL_RTR_T_MAXPOWER07:
		return 40;
	case ECL_RTR_T_DLQ_RT:
		return 19;
	case ECL_RTR_T_DLQ_STATE:
		return 11;
	default:
		return (ECL_RATE_QTY + 1) * 4;
	}
}

static int gw3761_Afn0C_07RealRead(buf b, t_afn04_f10 *pF10, t_ecl_rtdi07 *p, uint_t nIs3P)
{
	sys_res res;
	int nRet = 0;
	uint_t i, nLen;
	uint8_t *pTemp;
	buf bTx = {0};

	if (p->type == ECL_RTR_T_NONE) {
		nLen = p->di07;
	} else {
		nLen = gw3761_Afn0C_Type(p->type);
		dlt645_Packet2Buf(bTx, pF10->madr, DLT645_CODE_READ07, &p->di07, 4);
		res = ecl_485_RealRead(bTx, 2400, 2);
		if ((res == SYS_R_OK) && (bTx->p[0] == (DLT645_CODE_READ07 | BITMASK(7)))) {
			pTemp = &bTx->p[6];
			switch (p->type) {
			case ECL_RTR_T_ENERGY_5:
				for (i = 0; i <= ECL_RATE_QTY; i++, pTemp += 4) {
					buf_PushData(b, 0x00, 1);
					buf_Push(b, pTemp, 4);
				}
				break;
			case ECL_RTR_T_MAXPOWER07:
				for (i = 0; i <= ECL_RATE_QTY; i++, pTemp += 8)
					buf_Push(b, pTemp, 3);
				pTemp = &bTx->p[9];
				for (i = 0; i <= ECL_RATE_QTY; i++, pTemp += 8)
					buf_Push(b, pTemp, 4);
				break;
			case ECL_RTR_T_DLQ_RT:
				pTemp += 16;
				for (i = 0; i < 3; i++, pTemp -= 2)
					buf_Push(b, pTemp, 2);
				for (i = 0; i < 4; i++, pTemp -= 2) {
					buf_PushData(b, 0x00, 1);
					buf_Push(b, pTemp, 2);
				}
				buf_PushData(b, 0, 1);
				break;
			default:
				buf_Push(b, pTemp, nLen);
				break;
			}
			nRet = 1;
		}
		buf_Release(bTx);
	}
	if (nRet == 0)
		buf_Fill(b, GW3761_DATA_INVALID, nLen);
	return nRet;
}

static int gw3761_Afn0C_97RealRead(buf b, t_afn04_f10 *pF10, t_ecl_rtdi97 *p, uint_t nIs3P)
{
	sys_res res;
	int nRet = 0;
	uint_t i, nLen, nData;
	uint8_t *pTemp;
	buf bTx = {0};

	if (p->type == ECL_RTR_T_NONE) {
		nLen = p->di97;
	} else {
		nLen = gw3761_Afn0C_Type(p->type);
		dlt645_Packet2Buf(bTx, pF10->madr, DLT645_CODE_READ97, &p->di97, 2);
		res = ecl_485_RealRead(bTx, 1200, 2);
		if ((res == SYS_R_OK) && (bTx->p[0] == (DLT645_CODE_READ97 | BITMASK(7)))) {
			pTemp = &bTx->p[4];
			switch (p->type) {
			case ECL_RTR_T_ENERGY_5:
				for (i = 0; i <= ECL_RATE_QTY; i++, pTemp += 4) {
					buf_PushData(b, 0x00, 1);
					buf_Push(b, pTemp, 4);
				}
				break;
			case ECL_RTR_T_DLQ_RT:
				nData = *pTemp++;
				for (i = 0; i < 3; i++, pTemp += 2)
					buf_Push(b, pTemp, 2);
				for (i = 0; i < 4; i++, pTemp += 2) {
					buf_PushData(b, 0x00, 1);
					buf_Push(b, pTemp, 2);
				}
				buf_PushData(b, nData, 1);
				break;
			default:
				buf_Push(b, pTemp, nLen);
				break;
			}
			nRet = 1;
		}
		buf_Release(bTx);
	}
	if (nRet == 0)
		buf_Fill(b, GW3761_DATA_INVALID, nLen);
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

static int gw3761_Afn0C_F25(buf b, uint_t nDa)
{
	t_afn04_f10 xPM;
	uint_t i;
	uint8_t aBuf[5];
	t_acm_rtdata *pD;
	t_ecl_rtdi97 *pR97;
	t_ecl_rtdi07 *pR07;

	if (icp_Meter4Tn(nDa, &xPM) == 0)
		return 0;
	switch (xPM.port) {
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
				gw3761_ConvertData_07(aBuf, FLOAT2FIX(pD->u[i]));
				buf_Push(b, aBuf, 2);
			}
			for (i = 0; i < 4; i++) {
				gw3761_ConvertData_25(aBuf, FLOAT2FIX(pD->i[i]), 0);
				buf_Push(b, aBuf, 3);
			}
			for (i = 0; i < 4; i++) {
				gw3761_ConvertData_09(aBuf, FLOAT2FIX(pD->ui[i]), 0);
				buf_Push(b, aBuf, 3);
			}
		}
		break;
	default:
		gw3761_ConvertData_15(aBuf, rtc_GetTimet());
		buf_Push(b, aBuf, 5);
		switch (xPM.prtl) {
		case ECL_PRTL_DLT645_97:
			for (pR97 = tbl_Di97_Afn0CF25; pR97 < ARR_ENDADR(tbl_Di97_Afn0CF25); pR97++) {
				gw3761_Afn0C_97RealRead(b, &xPM, pR97, 1);
			}
			break;
		default:
			for (pR07 = tbl_Di07_Afn0CF25; pR07 < ARR_ENDADR(tbl_Di07_Afn0CF25); pR07++) {
				gw3761_Afn0C_07RealRead(b, &xPM, pR07, 1);
			}
			break;
		}
		break;
	}
	return 1;
}

static int gw3761_Afn0C_F33(buf b, uint_t nDa, uint_t nMask)
{
	t_afn04_f10 xPM;
	uint_t i;
	uint8_t aBuf[5];
	t_ecl_rtdi97 *pR97;
	t_ecl_rtdi07 *pR07;

	if (icp_Meter4Tn(nDa, &xPM) == 0)
		return 0;
	switch (xPM.port) {
	case ECL_PORT_ACM:
		break;
	default:
		gw3761_ConvertData_15(aBuf, rtc_GetTimet());
		buf_Push(b, aBuf, 5);
		buf_PushData(b, ECL_RATE_QTY, 1);
		switch (xPM.prtl) {
		case ECL_PRTL_DLT645_97:
			for (pR97 = tbl_Di97_Afn0CF33, i = 0; pR97 < ARR_ENDADR(tbl_Di97_Afn0CF33); pR97++, i++) {
				if (nMask & BITMASK(i))
					gw3761_Afn0C_97RealRead(b, &xPM, pR97, 1);
			}
			break;
		default:
			for (pR07 = tbl_Di07_Afn0CF33, i = 0; pR07 < ARR_ENDADR(tbl_Di07_Afn0CF33); pR07++, i++) {
				if (nMask & BITMASK(i))
					gw3761_Afn0C_07RealRead(b, &xPM, pR07, 1);
			}
			break;
		}
		break;
	}
	return 1;
}

static int gw3761_Afn0C_F34(buf b, uint_t nDa, uint_t nMask)
{
	t_afn04_f10 xPM;
	uint_t i;
	uint8_t aBuf[5];
	t_ecl_rtdi97 *pR97;
	t_ecl_rtdi07 *pR07;

	if (icp_Meter4Tn(nDa, &xPM) == 0)
		return 0;
	switch (xPM.port) {
	case ECL_PORT_ACM:
		break;
	default:
		gw3761_ConvertData_15(aBuf, rtc_GetTimet());
		buf_Push(b, aBuf, 5);
		buf_PushData(b, ECL_RATE_QTY, 1);
		switch (xPM.prtl) {
		case ECL_PRTL_DLT645_97:
			for (pR97 = tbl_Di97_Afn0CF34, i = 0; pR97< ARR_ENDADR(tbl_Di97_Afn0CF34); pR97++, i++) {
				if (nMask & BITMASK(i))
					gw3761_Afn0C_97RealRead(b, &xPM, pR97, 1);
			}
			break;
		default:
			for (pR07 = tbl_Di07_Afn0CF34, i = 0; pR07 < ARR_ENDADR(tbl_Di07_Afn0CF34); pR07++, i++) {
				if (nMask & BITMASK(i))
					gw3761_Afn0C_07RealRead(b, &xPM, pR07, 1);
			}
			break;
		}
		break;
	}
	return 1;
}

static int gw3761_Afn0C_F35(buf b, uint_t nDa)
{
	t_afn04_f10 xPM;
	uint_t nIs3P;
	uint8_t aBuf[5];
	t_ecl_rtdi97 *pR97;
	t_ecl_rtdi07 *pR07;

	if (icp_Meter4Tn(nDa, &xPM) == 0)
		return 0;
	switch (xPM.port) {
	case ECL_PORT_ACM:
		break;
	default:
		gw3761_ConvertData_15(aBuf, rtc_GetTimet());
		buf_Push(b, aBuf, 5);
		buf_PushData(b, ECL_RATE_QTY, 1);
		switch (xPM.prtl) {
		case ECL_PRTL_DLT645_97:
			for (pR97 = tbl_Di97_Afn0CF35; pR97 < ARR_ENDADR(tbl_Di97_Afn0CF35); pR97++) {
				gw3761_Afn0C_97RealRead(b, &xPM, pR97, nIs3P);
			}
			break;
		case ECL_PRTL_DLQ_SY:
			break;
		default:
			for (pR07 = tbl_Di07_Afn0CF35; pR07 < ARR_ENDADR(tbl_Di07_Afn0CF35); pR07++) {
				gw3761_Afn0C_07RealRead(b, &xPM, pR07, nIs3P);
			}
			break;
		}
		break;
	}
	return 1;
}

static int gw3761_Afn0C_F36(buf b, uint_t nDa)
{
	t_afn04_f10 xPM;
	uint_t nIs3P;
	uint8_t aBuf[5];
	t_ecl_rtdi97 *pR97;
	t_ecl_rtdi07 *pR07;

	if (icp_Meter4Tn(nDa, &xPM) == 0)
		return 0;
	switch (xPM.port) {
	case ECL_PORT_ACM:
		break;
	default:
		gw3761_ConvertData_15(aBuf, rtc_GetTimet());
		buf_Push(b, aBuf, 5);
		buf_PushData(b, ECL_RATE_QTY, 1);
		switch (xPM.prtl) {
		case ECL_PRTL_DLT645_97:
			for (pR97 = tbl_Di97_Afn0CF36; pR97 < ARR_ENDADR(tbl_Di97_Afn0CF36); pR97++) {
				gw3761_Afn0C_97RealRead(b, &xPM, pR97, nIs3P);
			}
			break;
		default:
			for (pR07 = tbl_Di07_Afn0CF36; pR07 < ARR_ENDADR(tbl_Di07_Afn0CF36); pR07++) {
				gw3761_Afn0C_07RealRead(b, &xPM, pR07, nIs3P);
			}
			break;
		}
		break;
	}
	return 1;
}


static int gw3761_Afn0C_F49(buf b, uint_t nDa)
{
	t_afn04_f10 xPM;
	uint_t i;
	uint8_t aBuf[5];
	t_acm_rtdata *pD;

	if (icp_Meter4Tn(nDa, &xPM) == 0)
		return 0;
	switch (xPM.port) {
	case ECL_PORT_ACM:
		if (acm_IsReady()) {
			pD = &acm_rtd;
			for (i = 0; i < 3; i++) {
				gw3761_ConvertData_05(aBuf, FLOAT2FIX(pD->ua[i]), 1);
				buf_Push(b, aBuf, 2);
			}
			for (i = 0; i < 3; i++) {
				gw3761_ConvertData_05(aBuf, FLOAT2FIX(pD->ia[i]), 1);
				buf_Push(b, aBuf, 2);
			}
		}
		break;
	default:
		break;
	}
	return 1;
}

static int gw3761_Afn0C_F57(buf b, uint_t nDa)
{
	t_afn04_f10 xPM;
	uint_t i, j;
	uint8_t aBuf[5];
	t_acm_xbdata *pD;

	if (icp_Meter4Tn(nDa, &xPM) == 0)
		return 0;
	switch (xPM.port) {
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
	return 1;
}

static int gw3761_Afn0C_F58(buf b, uint_t nDa)
{
	t_afn04_f10 xPM;
	uint_t i, j, k;
	uint8_t aBuf[5];
	t_acm_xbdata *pD;

	if (icp_Meter4Tn(nDa, &xPM) == 0)
		return 0;
	switch (xPM.port) {
	case ECL_PORT_ACM:
		if (acm_IsReady()) {
			buf_PushData(b, 21, 1);
			for (i = 0; i < 6; i++) {
				if (i < 3) {
					pD = &acm_uxb[i];
					k = 0;
				} else {
					pD = &acm_ixb[i - 3];
					k = 1;
				}
				for (j = k; j < 21; j++) {
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
	return 1;
}

static int gw3761_Afn0C_F64(buf b, uint_t nDa)
{
	t_afn04_f10 xPM;
	uint8_t aBuf[5];
	t_ecl_rtdi97 *pR97;
	t_ecl_rtdi07 *pR07;

	if (icp_Meter4Tn(nDa, &xPM) == 0)
		return 0;
	if (xPM.port != ECL_PORT_RS485)
		return 0;
	gw3761_ConvertData_15(aBuf, rtc_GetTimet());
	buf_Push(b, aBuf, 5);
	if (xPM.prtl == ECL_PRTL_DLQ_QL) {
		for (pR97 = tbl_DiQL_Afn0CF64; pR97 < ARR_ENDADR(tbl_DiQL_Afn0CF64); pR97++) {
			gw3761_Afn0C_97RealRead(b, &xPM, pR97, 1);
		}
	} else {
		for (pR07 = tbl_DiSY_Afn0CF64; pR07 < ARR_ENDADR(tbl_DiSY_Afn0CF64); pR07++) {
			gw3761_Afn0C_07RealRead(b, &xPM, pR07, 1);
		}
	}
	return 1;
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
				case 12:
					//终端网络信号品质及IP地址
					buf_PushData(b, modem_GetSignal(), 1);
					net_GetIpPPP(&aBuf[0], &aBuf[4], &aBuf[8]);
					buf_Push(b, aBuf, 4);
					break;
				case 25:
					//当前电压电流功率
					gw3761_Afn0C_F25(b, nDa);
					break;
				case 33:
					//当前正向有无功电能示值
					gw3761_Afn0C_F33(b, nDa, 0x0F);
					break;
				case 34:
					//当前反向有无功电能示值
					gw3761_Afn0C_F34(b, nDa, 0x0F);
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
				case 64:
					//断路器实时数据
					gw3761_Afn0C_F64(b, nDa);
					break;
				case 65:
					//当前遥信状态
					buf_PushData(b, 0, 1);
					buf_PushData(b, evt_YXRead(), 2);
					break;
				case 129:
					//当前正向有功电能示值
					gw3761_Afn0C_F33(b, nDa, 0x01);
					break;
				case 130:
					//当前正向无功电能示值
					gw3761_Afn0C_F33(b, nDa, 0x02);
					break;
				case 131:
					//当前反向有功电能示值
					gw3761_Afn0C_F34(b, nDa, 0x01);
					break;
				case 132:
					//当前反向无功电能示值
					gw3761_Afn0C_F34(b, nDa, 0x02);
					break;
				case 133:
					//当前无功1象限电能示值
					gw3761_Afn0C_F33(b, nDa, 0x04);
					break;
				case 134:
					//当前无功2象限电能示值
					gw3761_Afn0C_F34(b, nDa, 0x04);
					break;
				case 135:
					//当前无功3象限电能示值
					gw3761_Afn0C_F34(b, nDa, 0x08);
					break;
				case 136:
					//当前无功4象限电能示值
					gw3761_Afn0C_F33(b, nDa, 0x08);
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


