#include <string.h>
#include <litecore.h>
#include "para.h"
#include "data.h"
#include "alarm.h"



//External Functions
int gw3761_ResponseData3(p_gw3761 p, buf b, u_word2 *pDu, uint8_t **ppData)
{
	int res = 0;
	uint_t nPm, nPn;

	buf_PushData(b, evt_GetCount(), 2);
	nPm = *(*ppData)++;
	nPn = *(*ppData)++;
	if (nPm != nPn) {
		switch (pDu->word[1]) {
		case 0x0001:
			res += evt_Read(b, nPm, nPn, 0);
			break;
		default:
			res += evt_Read(b, nPm, nPn, 1);
			break;
		}
	}
	if (res == 0)
		buf_Unpush(b, 6);
	return res;
}

