#include <string.h>
#include <litecore.h>
#include "para.h"
#include "data.h"
#include "acm.h"



//External Functions
int gw3761_ResponseData2(p_gw3761 p, buf b, u_word2 *pDu, uint8_t **ppData)
{
#if GW3761_TYPE == GW3761_T_GWJC2009
	uint16_t aDa[8];
#else
	uint8_t aDa[64];
#endif
	int res = 0;
	time_t tTime;
	uint_t i, j, k, nDa, nDaQty, nFn, nTemp, nData, nOffset, nLen, nFnOff;
	t_data_min xMin;
	t_data_quarter xQuar;
	uint8_t aTime[6];
	
	nDaQty = gw3761_ConvertDa2Map(pDu->word[0], aDa);
	for (i = 0; i < nDaQty; i++) {
		nDa = aDa[i];
		for (j = 0; j < 8; j++) {
			if ((pDu->word[1] & BITMASK(j)) == 0)
				continue;
			nFn = gw3761_ConvertDt2Fn((pDu->word[1] & 0xFF00) | BITMASK(j));
			switch (nFn) {
			case 49:
				buf_Push(b, *ppData, 3);
				buf_PushData(b, 1440, 2);
				buf_PushData(b, 0, 2);
				res += 1;
				*ppData += 3;
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
			case 25:
			case 26:
			case 27:
			case 28:
			case 29:
			case 30:
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
				*ppData += 3;
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
				*ppData += 2;
				break;
			default:
				memcpy(&aTime[1], *ppData, 5);
				aTime[0] = 0;
				tTime = array2timet(aTime, 1);
				nTemp = (*ppData)[5];
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
				nData = (*ppData)[6];
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
						buf_Push(b, *ppData, 7);
						for (k = 0; k < nData; k++, tTime += nTemp) {
							timet2array(tTime, aTime, 1);
							data_QuarterRead(&aTime[1], &xQuar);
							if (xQuar.time != GW3761_DATA_INVALID)
								buf_Push(b, xQuar.data, 146);
							else
								buf_Fill(b, GW3761_DATA_INVALID, 146);
						}
						res += 1;
						nLen = 0;
						break;
                    case 232:
                        //配电扩展组合数据读取
						buf_Push(b, *ppData, 7);
						for (k = 0; k < nData; k++, tTime += nTemp) {
							timet2array(tTime, aTime, 1);
							data_MinRead(&aTime[1], &xMin);
							if (xMin.time != GW3761_DATA_INVALID)
								buf_Push(b, xMin.data, 42);
							else
								buf_Fill(b, GW3761_DATA_INVALID, 42);
						}
						res += 1;
						nLen = 0;
						break;
					default:
						nLen = 0;
						break;
					}
					if (nLen) {
						buf_Push(b, *ppData, 7);
						for (k = 0; k < nData; k++, tTime += nTemp) {
							timet2array(tTime, aTime, 1);
							data_MinRead(&aTime[1], &xMin);
							if (xMin.time != GW3761_DATA_INVALID)
								buf_Push(b, &xMin.data[nOffset + nFnOff * nLen], nLen);
							else
								buf_Fill(b, GW3761_DATA_INVALID, nLen);
						}
						res += 1;
					}
				}
				*ppData += 7;
				break;
			}
		}
	}
	if (res == 0)
		buf_Unpush(b, 4);
	return res;
}


