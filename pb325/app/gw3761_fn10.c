#include <string.h>
#include <litecore.h>
#include "meter.h"





//External Functions
int gw3761_ResponseTransmit(p_gw3761 p, buf b, u_word2 *pDu, uint8_t **ppData)
{
	int nRelay;
	uint_t i, nLen, nFn;
	
	for (i = 0; i < 8; i++) {
		if ((pDu->word[1] & BITMASK(i)) == 0)
			continue;
		nFn = gw3761_ConvertDt2Fn((pDu->word[1] & 0xFF00) | BITMASK(i));
		switch (nFn) {
		case 1:
			*ppData += 4;
			nLen = ((*ppData)[1] << 8) | (*ppData)[0];
			*ppData += 2;
			*ppData += nLen;
			break;
		case 9:
			*ppData += 1;
			nRelay = *(*ppData)++;
			if (nRelay == 0xFF)
				nRelay = -1;
			else
				*ppData += nRelay * 6;
			*ppData += 11;
			break;
		default:
			break;
		}
	}
	return 0;
}




