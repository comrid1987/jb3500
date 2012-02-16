#include <string.h>
#include <litecore.h>
#include "para.h"
#include "data.h"
#include "alarm.h"



//External Functions
int gw3761_ResponseData3(p_gw3761 p, buf b, u_word2 *pDu, uint8_t **ppData)
{
	uint_t i;

	for (i = 0; i < 8; i++) {
		if ((pDu->word[1] & BITMASK(i)) == 0)
			continue;
		*ppData += 2;
	}
	return 0;
}


