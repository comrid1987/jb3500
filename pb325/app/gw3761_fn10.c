#include <string.h>
#include <litecore.h>
#include "meter.h"





//External Functions
int gw3761_ResponseTransmit(p_gw3761 p, buf b, u_word2 *pDu, uint8_t **ppData)
{
	int nRelay;
	uint_t i, nPort, nBaud, nCtrl, nLen, nFn;
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
			if (pTemp == NULL)
				break;
			buf_Push(bTx, pTemp, nLen - (pTemp - *ppData));
			*ppData += nLen;
			switch (nPort) {
			case ECL_PORT_RS485:
				if (ecl_485_RealRead(bTx, 2400, 2) == SYS_R_OK) {
					buf_PushData(b, nPort, 1);
					buf_PushData(b, bTx->len, 2);
					byteadd(&bTx->p[10], 0x33, bTx->p[9]);
					buf_Push(b, bTx->p, bTx->len);
				}
				break;
			default:
				if (ecl_Plc_RealRead(bTx, -1, NULL, nRelay & 0x7F) == SYS_R_OK) {
					buf_PushData(b, nPort, 1);
					buf_PushData(b, bTx->len, 2);
					byteadd(&bTx->p[10], 0x33, bTx->p[9]);
					buf_Push(b, bTx->p, bTx->len);
				}
				break;
			}
			buf_Release(bTx);
			break;
		case 9:
			nPort = *(*ppData)++;
			nRelay = *(*ppData)++;
			pTemp = *ppData;
			if (nRelay == 0xFF) {
				nRelay = -1;
			} else {
				*ppData += nRelay * 6;
			}
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
			switch (nPort) {
			case ECL_PORT_RS485:
				buf_PushData(b, nPort, 1);
				buf_Push(b, pAdr, 6);
				if (ecl_485_RealRead(bTx, nBaud, 2) == SYS_R_OK) {
					buf_PushData(b, 3, 1);
					buf_Push(b, &bTx->p[9], bTx->p[9] + 1);
				} else {
					buf_PushData(b, 0x000002, 3);
				}
				break;
			case ECL_PORT_PLC:
				buf_PushData(b, nPort, 1);
				buf_Push(b, pAdr, 6);
				if (ecl_Plc_RealRead(bTx, nRelay, pTemp, 60) == SYS_R_OK) {
					buf_PushData(b, 3, 1);
					buf_Push(b, &bTx->p[9], bTx->p[9] + 1);
				} else {
					buf_PushData(b, 0x000002, 3);
				}
				break;
			default:
				break;
			}
			buf_Release(bTx);
			break;
		default:
			break;
		}
	}
	return 0;
}




