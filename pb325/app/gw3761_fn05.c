#include <litecore.h>
//#include "para.h"
#include "meter.h"


//Internal Functions


//External Functions
const uint8_t ecl_DlqQlCmd[2][3] = {
	{0x36, 0xC0, 0x50},
	{0x36, 0xC0, 0x5F},
};
int gw3761_ResponseCtrlCmd(p_gw3761 p, u_word2 *pDu, uint8_t **ppData)
{
#if GW3761_TYPE == GW3761_T_GWJC2009
	uint16_t aDa[8];
#else
	uint8_t aDa[64];
#endif
	int res = 0;
	uint_t i, j, nFn, nDa, nDaQty;
	t_afn04_f10 xF10;
	buf bTx = {0};
	
	nDaQty = gw3761_ConvertDa2Map(pDu->word[0], aDa);
	for (i = 0; i < nDaQty; i++) {
		nDa = aDa[i];
		for (j = 0; j < 8; j++) {
			if ((pDu->word[1] & BITMASK(j)) == 0)
				continue;
			nFn = gw3761_ConvertDt2Fn((pDu->word[1] & 0xFF00) | BITMASK(j));
			switch (nFn) {
			case 1:	//Ò£¿ØÌøÕ¢
				*ppData += 1;
			case 2:	//Ò£¿ØºÏÕ¢
				if (icp_MeterRead(nDa, &xF10) < 0)
					break;
				if (xF10.prtl != ECL_PRTL_DLQ_QL)
					break;
				dlt645_Packet2Buf(bTx, xF10.madr, 0x04, ecl_DlqQlCmd[nFn - 1], 3);
				if (ecl_485_RealRead(bTx, 1200, 2) == SYS_R_OK)
					res += 1;
				break;
			case 31:
				//¶ÔÊ±ÃüÁî
				rtc_SetTimet(bin2timet((*ppData)[0], (*ppData)[1], (*ppData)[2], (*ppData)[3], (*ppData)[4] & 0x1F, (*ppData)[5], 1));
				*ppData += 6;
				res += 1;
				break;
			default:
				break;
			}
		}
	}
	return res;
}

