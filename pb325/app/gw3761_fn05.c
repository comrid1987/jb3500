#include <litecore.h>
#include "meter.h"


//Internal Functions


//External Functions
int gw3761_ResponseCtrlCmd(p_gw3761 p, u_word2 *pDu, uint8_t **ppData)
{
	int res = 0;
	uint_t i, nFn;
	
	for (i = 0; i < 8; i++) {
		if ((pDu->word[1] & BITMASK(i)) == 0)
			continue;
		nFn = gw3761_ConvertDt2Fn((pDu->word[1] & 0xFF00) | BITMASK(i));
		switch (nFn) {
		case 1:
			//遥控跳闸
			res += 1;
			break;
		case 31:
			//对时命令
			rtc_SetTimet(bin2timet((*ppData)[0], (*ppData)[1], (*ppData)[2], (*ppData)[3], (*ppData)[4] & 0x1F, (*ppData)[5], 1));
			*ppData += 6;
			res += 1;
			break;
		case 33:
			//保电解除
			res += 1;
			break;
		case 37:
			//声音告警
			res += 1;
			break;
		default:
			break;
		}
	}
	return res;
}

