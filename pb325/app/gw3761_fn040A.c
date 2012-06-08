#include <string.h>
#include <litecore.h>
#include "para.h"
#include "alarm.h"
#include "acm.h"


//Private Variables
struct byte2 {
	uint8_t	fn;
	uint8_t	len;
};
static const struct byte2 tbl_afn04_len[] = {
	1,	sizeof(t_afn04_f1),
	3,	sizeof(t_afn04_f3),
	4,	16,
	5,	3,
	6,	16,
	8,	8,
	9,	16,
	12,	2,
	16,	64,
	17,	2,
	18,	12,
	19,	1,
	20,	1,
	21,	49,
	23,	3,
	25,	sizeof(t_afn04_f25),
	26,	57,
	27,	24,
	28,	4,
	29,	12,
	30,	1,
	36,	4,
	42,	6,
	43,	1,
	44,	8,
	45,	1,
	46,	4,
	47,	17,
	48,	1,
	49,	1,
	57,	3,
	58,	1,
	59,	4,
	60,	80,
	61,	1,
	67,	1,
	68,	1,
	73,	48,
	74,	10,
	75,	16,
	76,	1,
	81,	4,
	82,	4,
	83,	1,
	85,	sizeof(t_afn04_f85),	//终端地址
	203,1,						//端口2抄表设置
};





//Internal Functions
static int gw3761_Afn04_GetFnLen(uint_t nFn)
{
	const struct byte2 *p;

	for (p = &tbl_afn04_len[0]; p < ARR_ENDADR(tbl_afn04_len); p++) {
		if (nFn == p->fn)
			return p->len;
	}
	return 0;
}

//电表参数
static void gw3761_Afn04_F10(uint8_t **ppData)
{
	uint_t i, nQty, nPn;

	nQty = ((*ppData)[1] << 8) | (*ppData)[0];
	*ppData += 2;
	for (i = 0; i < nQty; i++) {
		nPn = ((*ppData)[1] << 8) | (*ppData)[0];
		*ppData += 2;
		icp_MeterWrite(nPn, (t_afn04_f10 *)*ppData);
		*ppData += sizeof(t_afn04_f10);
	}
}

static int gw3761_Afn0A_F10(buf b, uint8_t **ppData)
{
	int res = 0;
	uint_t i, nQty, nPn, nOffset = 0;
	t_afn04_f10 xF10;

	nQty = ((*ppData)[1] << 8) | (*ppData)[0];
	*ppData += 2;
	if (nQty) {
		buf_PushData(b, nQty, 2);
		nOffset = b->len - 2;
	}
	for (i = 0; i < nQty; i++) {
		nPn = ((*ppData)[1] << 8) | (*ppData)[0];
		*ppData += 2;
		icp_MeterRead(nPn, &xF10);
		if ((xF10.tn != 0) && (xF10.tn < ECL_SN_MAX)) {
			buf_PushData(b, nPn, 2);
			buf_Push(b, &xF10, sizeof(t_afn04_f10));
			res += 1;
		}
	}
	if (res)
		memcpy(&b->p[nOffset], &res, 2);
	else
		buf_Unpush(b, 2);
	return res;
}

//脉冲配置参数
static void gw3761_Afn04_F11(uint8_t **ppData)
{
	uint_t i, nQty, nPn;

	nQty = *(*ppData)++;
	for (i = 0; i < nQty; i++) {
		nPn = *(*ppData)++;
		icp_ParaWrite(4, 11, nPn, *ppData, 4);
		*ppData += 4;
	}
}

static int gw3761_Afn0A_F11(buf b, uint8_t **ppData)
{
	int res = 0;
	uint_t i, nQty, nPn, nOffset = 0;
	uint8_t aBuf[4];

	nQty = *(*ppData)++;
	if (nQty) {
		buf_PushData(b, nQty, 1);
		nOffset = b->len - 1;
	}
	for (i = 0; i < nQty; i++) {
		nPn = *(*ppData)++;
		if (icp_ParaRead(4, 11, nPn, aBuf, sizeof(aBuf)) > 0) {
			buf_PushData(b, nPn, 1);
			buf_Push(b, aBuf, sizeof(aBuf));
			res += 1;
		}
	}
	if (res)
		memcpy(&b->p[nOffset], &res, 1);
	else
		buf_Unpush(b, 1);
	return res;
}

//模拟量配置参数
static void gw3761_Afn04_F13(uint8_t **ppData)
{
	uint_t i, nQty, nPn;

	nQty = *(*ppData)++;
	for (i = 0; i < nQty; i++) {
		nPn = *(*ppData)++;
		icp_ParaWrite(4, 13, nPn, *ppData, 2);
		*ppData += 2;
	}
}

static int gw3761_Afn0A_F13(buf b, uint8_t **ppData)
{
	int res = 0;
	uint_t i, nQty, nPn, nOffset = 0;
	uint8_t aBuf[2];

	nQty = *(*ppData)++;
	if (nQty) {
		buf_PushData(b, nQty, 1);
		nOffset = b->len - 1;
	}
	for (i = 0; i < nQty; i++) {
		nPn = *(*ppData)++;
		if (icp_ParaRead(4, 13, nPn, aBuf, sizeof(aBuf)) > 0) {
			buf_PushData(b, nPn, 1);
			buf_Push(b, aBuf, sizeof(aBuf));
			res += 1;
		}
	}
	if (res)
		memcpy(&b->p[nOffset], &res, 1);
	else
		buf_Unpush(b, 1);
	return res;
}

//总加组配置参数
static void gw3761_Afn04_F14(uint8_t **ppData)
{
	uint_t i, nQty, nPn, nLen;
	t_afn04_f14 *pF14;

	nQty = *(*ppData)++;
	for (i = 0; i < nQty; i++) {
		nPn = *(*ppData)++;
		pF14 = (t_afn04_f14 *)(*ppData);
		nLen = pF14->qty + FPOS(t_afn04_f14, tn);
		icp_ParaWrite(4, 14, nPn, pF14, nLen);
		*ppData += nLen;
	}
}

static int gw3761_Afn0A_F14(buf b, uint8_t **ppData)
{
	int res = 0, nLen;
	uint_t i, nQty, nSn, nOffset = 0;
	t_afn04_f14 xF14;

	nQty = *(*ppData)++;
	if (nQty) {
		buf_PushData(b, nQty, 1);
		nOffset = b->len - 1;
	}
	for (i = 0; i < nQty; i++) {
		nSn = *(*ppData)++;
		nLen = icp_ParaRead(4, 14, nSn, &xF14, sizeof(t_afn04_f14));
		if (nLen > 0) {
			buf_PushData(b, nSn, 1);
			buf_Push(b, &xF14, nLen);
			res += 1;
		}
	}
	if (res)
		b->p[nOffset] = res;
	else
		buf_Unpush(b, 1);
	return res;
}


//费率参数
static void gw3761_Afn04_F22(uint8_t **ppData)
{
	uint_t nQty, nLen;

	nQty = **ppData;
	nLen = nQty * 4 + FPOS(t_afn04_f22, rate);
	icp_ParaWrite(4, 22, TERMINAL, *ppData, nLen);
	*ppData += nLen;
}

static int gw3761_Afn0A_F22(buf b, uint8_t **ppData)
{
	int res = 0, nLen;
	t_afn04_f22 xF22;

	nLen = icp_ParaRead(4, 22, TERMINAL, &xF22, sizeof(t_afn04_f22));
	if (nLen > 0) {
		nLen = xF22.qty * 4 + FPOS(t_afn04_f22, rate);
		buf_Push(b, &xF22, nLen);
		res += 1;
	}
	return res;
}

//终端抄表运行参数
static void gw3761_Afn04_F33(uint8_t **ppData)
{
	uint_t i, nQty, nPn, nLen;
	t_afn04_f33 *pF33;

	nQty = *(*ppData)++;
	for (i = 0; i < nQty; i++) {
		nPn = *(*ppData)++;
		pF33 = (t_afn04_f33 *)(*ppData);
		nLen = pF33->qty * 4 + FPOS(t_afn04_f33, timeinfo);
		icp_ParaWrite(4, 33, nPn, pF33, nLen);
		*ppData += nLen;
	}
}

static int gw3761_Afn0A_F33(buf b, uint8_t **ppData)
{
	int res = 0, nLen;
	uint_t i, nQty, nSn, nOffset = 0;
	t_afn04_f33 xF33;

	nQty = *(*ppData)++;
	if (nQty) {
		buf_PushData(b, nQty, 1);
		nOffset = b->len - 1;
	}
	for (i = 0; i < nQty; i++) {
		nSn = *(*ppData)++;
		nLen = icp_ParaRead(4, 33, nSn, &xF33, sizeof(t_afn04_f33));
		if (nLen > 0) {
			buf_PushData(b, nSn, 1);
			buf_Push(b, &xF33, nLen);
			res += 1;
		}
	}
	if (res)
		b->p[nOffset] = res;
	else
		buf_Unpush(b, 1);
	return res;
}


//终端地址
static void gw3761_Afn04_F201(uint8_t **ppData)
{
	t_afn04_f85 xF85;

	memcpy(&xF85, *ppData - 2, sizeof(t_afn04_f85));
	xF85.mfcode = 0;
	*ppData += 4;
	icp_ParaWrite(4, 85, TERMINAL, &xF85, sizeof(t_afn04_f85));
}





//External Functions
int gw3761_ResponseSetParam(p_gw3761 p)
{
#if GW3761_TYPE == GW3761_T_GWJC2009
	uint16_t aDa[8];
#else
	uint8_t aDa[64];
#endif
	int res = 0;
	u_word2 uDu;
	uint_t i, j, nDa, nDaQty, nFn, nLen;
	uint8_t *pData, *pEnd;
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
				case 10:
					//电表参数
					gw3761_Afn04_F10(&pData);
					res += 1;
					break;
				case 11:
					//脉冲配置参数
					gw3761_Afn04_F11(&pData);
					res += 1;
					break;
				case 13:
					//模拟量配置参数
					gw3761_Afn04_F13(&pData);
					res += 1;
					break;					
				case 14:
					//总加组配置参数
					gw3761_Afn04_F14(&pData);
					res += 1;
					break;
				case 22:
					//费率参数
					gw3761_Afn04_F22(&pData);
					res += 1;
					break;
				case 33:
					//终端抄表运行参数
					gw3761_Afn04_F33(&pData);
					res += 1;
					break;
				case 201:
					//终端地址
					gw3761_Afn04_F201(&pData);
					res += 1;
					break;
				default:
					nLen = gw3761_Afn04_GetFnLen(nFn);
					if (nLen) {
						icp_ParaWrite(4, nFn, nDa, pData, nLen);
						pData += nLen;
						res += 1;
					}
					break;
				}
			}
		}
	}
	if (res) {
		evt_ERC3(p->parent.msta, &uDu);
		gw3761_TmsgConfirm(p);
	} else
		gw3761_TmsgReject(p);
	buf_Release(b);
	return res;
}


int gw3761_ResponseGetParam(p_gw3761 p, buf b, u_word2 *pDu, uint8_t **ppData)
{
#if GW3761_TYPE == GW3761_T_GWJC2009
	uint16_t aDa[8];
#else
	uint8_t aDa[64];
#endif
	uint_t i, j, nDa, nDaQty, nFn;
	int nLen;
	uint8_t *pTemp;

	nDaQty = gw3761_ConvertDa2Map(pDu->word[0], aDa);
	for (i = 0; i < nDaQty; i++) {
		nDa = aDa[i];
		for (j = 0; j < 8; j++) {
			if ((pDu->word[1] & BITMASK(j)) == 0)
				continue;
			nFn = gw3761_ConvertDt2Fn((pDu->word[1] & 0xFF00) | BITMASK(j));
			switch (nFn) {
			case 10:
				//电表参数
				gw3761_Afn0A_F10(b, ppData);
				break;
			case 11:
				//脉冲配置参数
				gw3761_Afn0A_F11(b, ppData);
				break;
			case 13:
				//模拟量配置参数
				gw3761_Afn0A_F13(b, ppData);
				break;
			case 14:
				//总加组配置参数
				gw3761_Afn0A_F14(b, ppData);
				break;
			case 22:
				//费率参数
				gw3761_Afn0A_F22(b, ppData);
				break;
			case 33:
				//终端抄表运行参数
				gw3761_Afn0A_F33(b, ppData);
				break;
			default:
				nLen = gw3761_Afn04_GetFnLen(nFn);
				if (nLen == 0)
					break;
				pTemp = mem_Malloc(nLen);
				if (pTemp != NULL) {
					nLen = icp_ParaRead(4, nFn, nDa, pTemp, nLen);
					if (nLen > 0)
						buf_Push(b, pTemp, nLen);
					mem_Free(pTemp);
				}
				break;
			}
		}
	}
	return 0;
}



