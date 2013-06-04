#include <string.h>
#include <litecore.h>
#include "meter.h"





//External Functions

int gw3761_ResponseTransmit(p_gw3761 p, buf b, u_word2 *pDu, uint8_t **ppData)
{
	sys_res res;
	int nRelay;
	uint_t i, nPort, nBaud, nCtrl, nLen, nFn, nTemp;
	uint8_t *pAdr, *pTemp;
	uint32_t nDI;
	buf bTx = {0};
	
	for (i = 0; i < 8; i++) {
		if ((pDu->word[1] & BITMASK(i)) == 0)
			continue;
		nFn = gw3761_ConvertDt2Fn((pDu->word[1] & 0xFF00) | BITMASK(i));
		switch (nFn) {
		case 1:
			nPort = *(*ppData)++;
			nCtrl = *(*ppData)++;
			nRelay = **ppData;
			*ppData += 2;
			nLen = ((*ppData)[1] << 8) | (*ppData)[0];
			*ppData += 2;
	        pTemp = dlt645_PacketAnalyze(*ppData, nLen);
			buf_Push(bTx, pTemp, nLen - (pTemp - *ppData));
			*ppData += nLen;
            nTemp = nCtrl >> 5;
            if (2 == nTemp) {
                res = ecl_485_RealRead(bTx, 1200, 2000);

            }
            else{
                res = ecl_485_RealRead(bTx, 2400, 2000);
            }

			if (res == SYS_R_OK) {
				buf_PushData(b, nPort, 1);
				buf_PushData(b, bTx->p[1] + (DLT645_HEADER_SIZE + 2), 2);

				buf_Push(b, pTemp, DLT645_HEADER_SIZE - 2);
				byteadd(&bTx->p[2], 0x33, bTx->p[1]);
				buf_Push(b, bTx->p, bTx->p[1] + 4);
			}
			buf_Release(bTx);
			break;
		case 9:
			nPort = *(*ppData)++;
	nRelay = *(*ppData)++;
			pTemp = *ppData;
			if (nRelay == 0xFF)
				nRelay = -1;
			else
				*ppData += nRelay * 6;
			pAdr = *ppData;
			*ppData += 6;
			nCtrl = *(*ppData)++;
			memcpy(&nDI, *ppData, 4);
			*ppData += 4;
			if (nCtrl) {
				//dlt645-07
				dlt645_Packet2Buf(bTx, pAdr, DLT645_CODE_READ07, &nDI, 4);
				nBaud = 2400;
			} else {
				//dlt645-97
				dlt645_Packet2Buf(bTx, pAdr, DLT645_CODE_READ97, &nDI, 2);
				nBaud = 1200;
			}
			buf_PushData(b, nPort, 1);
			buf_Push(b, pAdr, 6);
			if (ecl_485_RealRead(bTx, nBaud, 2000) == SYS_R_OK) {
				buf_PushData(b, 3, 1);
				buf_Push(b, &bTx->p[1], bTx->p[1] + 1);
			} else {
				buf_PushData(b, 0x000002, 3);
			}
			buf_Release(bTx);
			break;
		default:
			break;
		}
	}
	return 0;
}




