#include <math.h>
#include <string.h>
#include <litecore.h>
#include "para.h"
#include "data.h"
#include "acm.h"



//External Functions
static void gw3761_Data2_Other(buf b, time_t tTime)
{
	uint_t i;
	uint32_t nP, nQ;
	float fP, fQ;
	uint8_t aBuf[6];
	t_data_min xMin;

	timet2array(tTime, aBuf, 1);
	data_MinRead(&aBuf[1], &xMin);
	if (xMin.time == GW3761_DATA_INVALID) {
		buf_Fill(b, GW3761_DATA_INVALID, 62);
	} else {
		buf_Push(b, &xMin.data[ACM_MSAVE_PP], 12);
		buf_Push(b, &xMin.data[ACM_MSAVE_PQ], 12);
		buf_Push(b, &xMin.data[ACM_MSAVE_COS], 8);
		buf_Push(b, &xMin.data[ACM_MSAVE_VOL], 6);
		buf_Push(b, &xMin.data[ACM_MSAVE_CUR], 12);
		for (i = 0; i < 4; i++) {
			memcpy(&nP, &xMin.data[ACM_MSAVE_PP + i * 3], 3);
			fP = (float)bcd2bin32(nP & 0x007FFFFF) / 10000.0f;
			memcpy(&nQ, &xMin.data[ACM_MSAVE_PQ + i * 3], 3);
			fQ = (float)bcd2bin32(nQ & 0x007FFFFF) / 10000.0f;
			fP = sqrtf(fP * fP + fQ * fQ);
			gw3761_ConvertData_09(aBuf, FLOAT2FIX(fP), 0);
			buf_Push(b, aBuf, 3);
		}
	}
}

int gw3761_ResponseData2(p_gw3761 p)
{
#if GW3761_TYPE == GW3761_T_GWJC2009
	uint16_t aDa[8];
#else
	uint8_t aDa[64];
#endif
	int res = 0;
	time_t tTime;
	uint_t i, j, k, nDa, nDaQty, nFn, nTemp, nData, nOffset, nLen, nFnOff, nSucc;
	t_data_min xMin;
	t_data_quarter xQuar;
	uint8_t *pData, *pEnd, aBuf[6];
	t_stat xStat;
	p_stat ps = &xStat;
	u_word2 uDu;
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
				nSucc = 0;
				nFn = gw3761_ConvertDt2Fn((uDu.word[1] & 0xFF00) | BITMASK(j));
				switch (nFn) {
				case 25:
					if (data_DayRead(pData, ps)) {
						buf_Push(b, pData, 3);
						gw3761_ConvertData_23(aBuf, FLOAT2FIX(ps->pmax[3]));
						buf_Push(b, aBuf, 3);
						gw3761_ConvertData_18(aBuf, ps->tpmax[3]);
						buf_Push(b, aBuf, 3);
						gw3761_Data2_Other(b, ps->tpmax[3]);
						for (i = 0; i < 3; i++) {
							gw3761_ConvertData_23(aBuf, FLOAT2FIX(ps->pmax[i]));
							buf_Push(b, aBuf, 3);
							gw3761_ConvertData_18(aBuf, ps->tpmax[i]);
							buf_Push(b, aBuf, 3);
							gw3761_Data2_Other(b, ps->tpmax[i]);
						}
						buf_PushData(b, ps->p0[3], 2);
						for (i = 0; i < 3; i++)
							buf_PushData(b, ps->p0[i], 2);
						nSucc = 1;
					}
					pData += 3;
					break;
				case 27:
					if (data_DayRead(pData, ps)) {
						buf_Push(b, pData, 3);
						for (i = 0; i < 3; i++) {
							buf_PushData(b, ps->uover[i], 2);
							buf_PushData(b, ps->uunder[i], 2);
							buf_PushData(b, ps->uup[i], 2);
							buf_PushData(b, ps->ulow[i], 2);
							buf_PushData(b, ps->run - ps->uup[i] - ps->ulow[i], 2);
						}
						for (i = 0; i < 3; i++) {
							gw3761_ConvertData_07(aBuf, FLOAT2FIX(ps->umax[i]));
							buf_Push(b, aBuf, 2);
							gw3761_ConvertData_18(aBuf, ps->tumax[i]);
							buf_Push(b, aBuf, 3);
							gw3761_Data2_Other(b, ps->tumax[i]);
							gw3761_ConvertData_07(aBuf, FLOAT2FIX(ps->umin[i]));
							buf_Push(b, aBuf, 2);
							gw3761_ConvertData_18(aBuf, ps->tumin[i]);
							buf_Push(b, aBuf, 3);
							gw3761_Data2_Other(b, ps->tumin[i]);
						}
						for (i = 0; i < 3; i++) {
							gw3761_ConvertData_07(aBuf, FLOAT2FIX(ps->usum[i] / (float)ps->run));
							buf_Push(b, aBuf, 2);
						}
						nSucc = 1;
					}
					pData += 3;
					break;
				case 28:
					if (data_DayRead(pData, ps)) {
						buf_Push(b, pData, 3);
						buf_PushData(b, ps->ibalance, 2);
						buf_PushData(b, ps->ubalance, 2);
						gw3761_ConvertData_05_Percent(aBuf, FLOAT2FIX(ps->ibmax), 0);
						buf_Push(b, aBuf, 2);
						gw3761_ConvertData_18(aBuf, ps->tibmax);
						buf_Push(b, aBuf, 3);
						gw3761_Data2_Other(b, ps->tibmax);
						gw3761_ConvertData_05_Percent(aBuf, FLOAT2FIX(ps->ubmax), 0);
						buf_Push(b, aBuf, 2);
						gw3761_ConvertData_18(aBuf, ps->tubmax);
						buf_Push(b, aBuf, 3);
						gw3761_Data2_Other(b, ps->tubmax);
						nSucc = 1;
					}
					pData += 3;
					break;
				case 29:
					if (data_DayRead(pData, ps)) {
						buf_Push(b, pData, 3);
						for (i = 0; i < 3; i++) {
							buf_PushData(b, ps->iover[i], 2);
							buf_PushData(b, ps->iup[i], 2);
						}
						buf_PushData(b, ps->iup[3], 2);
						for (i = 0; i < 4; i++) {
							gw3761_ConvertData_25(aBuf, FLOAT2FIX(ps->imax[i]), 1);
							buf_Push(b, aBuf, 3);
							gw3761_ConvertData_18(aBuf, ps->timax[i]);
							buf_Push(b, aBuf, 3);
							gw3761_Data2_Other(b, ps->timax[i]);
						}
						nSucc = 1;
					}
					pData += 3;
					break;
				case 30:
					if (data_DayRead(pData, ps)) {
						buf_Push(b, pData, 3);
						buf_PushData(b, ps->uiover, 2);
						buf_PushData(b, ps->uiup, 2);
						nSucc = 1;
					}
					pData += 3;
					break;
				case 49:
					buf_Push(b, pData, 3);
					buf_PushData(b, 1440, 2);
					buf_PushData(b, 0, 2);
					nSucc = 1;
					pData += 3;
					break;
				case 2:
				case 3:
				case 4:
				case 5:
				case 6:
				case 7:
				case 8:
				case 9:
				case 10:
				case 11:
				case 12:
				case 26:
				case 31:
				case 32:
				case 41:
				case 42:
				case 43:
				case 45:
				case 50:
				case 53:
				case 57:
				case 58:
				case 59:
				case 113:
				case 114:
				case 115:
				case 116:
				case 117:
				case 118:
				case 121:
				case 122:
				case 123:
				case 129:
				case 153:
				case 154:
				case 155:
				case 156:
				case 162:
				case 163:
				case 164:
				case 165:
				case 166:
				case 167:
				case 168:
				case 169:
				case 170:
				case 171:
				case 172:
				case 173:
				case 174:
				case 175:
				case 176:
				case 185:
				case 186:
				case 187:
				case 188:
				case 189:
				case 190:
				case 191:
				case 192:
				case 209:
					pData += 3;
					break;
				case 17:
				case 18:
				case 19:
				case 20:
				case 21:
				case 22:
				case 23:
				case 24:
				case 33:
				case 34:
				case 35:
				case 36:
				case 37:
				case 38:
				case 39:
				case 44:
				case 46:
				case 51:
				case 52:
				case 54:
				case 60:
				case 61:
				case 62:
				case 65:
				case 66:
				case 130:
				case 157:
				case 158:
				case 159:
				case 160:
				case 177:
				case 178:
				case 179:
				case 180:
				case 181:
				case 182:
				case 183:
				case 184:
				case 193:
				case 194:
				case 195:
				case 196:
				case 201:
				case 202:
				case 203:
				case 204:
				case 205:
				case 206:
				case 207:
				case 208:
				case 213:
				case 214:
				case 215:
				case 216:
					pData += 2;
					break;
				default:
					memcpy(&aBuf[1], pData, 5);
					aBuf[0] = 0;
					tTime = array2timet(aBuf, 1);
					nTemp = pData[5];
					switch (nTemp) {
					case 1:
						nTemp = 15;
						break;
					case 2:
						nTemp = 30;
						break;
					case 3:
						nTemp = 60;
						break;
					case 254:
						nTemp = 5;
						break;
					case 255:
						nTemp = 1;
						break;
					default:
						nTemp = 0;
						break;
					}
					nTemp *= 60;
					nData = pData[6];
					if (nTemp) {
						switch (nFn) {
						case 81:
						case 82:
						case 83:
						case 84:
							nFnOff = nFn - 81;
							nOffset = ACM_MSAVE_PP;
							nLen = 3;
							break;
						case 85:
						case 86:
						case 87:
						case 88:
							nFnOff = nFn - 85;
							nOffset = ACM_MSAVE_PQ;
							nLen = 3;
							break;
						case 89:
						case 90:
						case 91:
							nFnOff = nFn - 89;
							nOffset = ACM_MSAVE_VOL;
							nLen = 2;
							break;
						case 92:
						case 93:
						case 94:
						case 95:
							nFnOff = nFn - 92;
							nOffset = ACM_MSAVE_CUR;
							nLen = 3;
							break;
						case 105:
						case 106:
						case 107:
						case 108:
							nFnOff = nFn - 105;
							nOffset = ACM_MSAVE_COS;
							nLen = 2;
							break;
						case 219:
							//配电扩展15分钟冻结
							buf_Push(b, pData, 7);
							for (k = 0; k < nData; k++, tTime += nTemp) {
								timet2array(tTime, aBuf, 1);
								data_QuarterRead(&aBuf[1], &xQuar);
								if (xQuar.time != GW3761_DATA_INVALID)
									buf_Push(b, xQuar.data, 146);
								else
									buf_Fill(b, GW3761_DATA_INVALID, 146);
							}
							nSucc = 1;
							nLen = 0;
							break;
	                    case 232:
							//配电扩展组合数据读取
							buf_Push(b, pData, 7);
							for (k = 0; k < nData; k++, tTime += nTemp) {
								timet2array(tTime, aBuf, 1);
								data_MinRead(&aBuf[1], &xMin);
								if (xMin.time != GW3761_DATA_INVALID)
									buf_Push(b, xMin.data, 42);
								else
									buf_Fill(b, GW3761_DATA_INVALID, 42);
							}
							nSucc = 1;
							nLen = 0;
							break;
						default:
							nLen = 0;
							break;
						}
						if (nLen) {
							buf_Push(b, pData, 7);
							for (k = 0; k < nData; k++, tTime += nTemp) {
								timet2array(tTime, aBuf, 1);
								data_MinRead(&aBuf[1], &xMin);
								if (xMin.time != GW3761_DATA_INVALID)
									buf_Push(b, &xMin.data[nOffset + nFnOff * nLen], nLen);
								else
									buf_Fill(b, GW3761_DATA_INVALID, nLen);
							}
							nSucc = 1;
						}
					}
					pData += 7;
					break;
				}
				if (nSucc)
					res += 1;
			}
		}
	}
	if (res)
		gw3761_TmsgSend(p, GW3761_FUN_RESPONSE, GW3761_AFN_DATA_L2, b, DLRCP_TMSG_RESPOND);
	else
		gw3761_TmsgReject(p);
	return res;
}


