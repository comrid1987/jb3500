#include <string.h>
#include <litecore.h>
#include "para.h"
#include "data.h"
#include "alarm.h"



//External Functions
#if 0
int gw3761_ResponseData3(p_gw3761 p, buf b, u_word2 *pDu, uint8_t **ppData)
{
	int res = 0;
	uint_t j, nERC, nPm, nPn, nFn;
	t_afn04_f9 xF9;
	uint8_t aBuf[5];

	for (j = 0; j < 8; j++) {
		if ((pDu->word[1] & BITMASK(j)) == 0)
			continue;
		nFn = gw3761_ConvertDt2Fn((pDu->word[1] & 0xFF00) | BITMASK(j));
		buf_PushData(b, evt_GetCount(), 2);
		*ppData += 2;
		icp_ParaRead(4, 9, TERMINAL, &xF9, sizeof(t_afn04_f9));
		for (nERC = 1; nERC < 64; nERC++) {
			if (GETBIT(xF9.valid, nERC - 1))
				break;
 		}
		if (nERC < 64) {
			res += 1;
			buf_PushData(b, 0x0100, 2);
			buf_PushData(b, nERC, 1);
			switch (nERC) {
			case 8:
				buf_PushData(b, 10, 1);
				gw3761_ConvertData_15(aBuf, rtc_GetTimet());
				buf_Push(b, aBuf, 5);
				buf_PushData(b, 2, 2);
				break;
			default:
				buf_Unpush(b, 3);
				res -= 1;
				break;
			}
 		}
	}
	if (res == 0)
		buf_Unpush(b, 6);
	return res;
}
#else
int gw3761_ResponseData3(p_gw3761 p, buf b, u_word2 *pDu, uint8_t **ppData)
{
	int res = 0;
	uint_t j, nPm, nPn, nFn;

	for (j = 0; j < 8; j++) {
		if ((pDu->word[1] & BITMASK(j)) == 0)
			continue;
		nFn = gw3761_ConvertDt2Fn((pDu->word[1] & 0xFF00) | BITMASK(j));
		buf_PushData(b, evt_GetCount(), 2);
		nPm = *(*ppData)++;
		nPn = *(*ppData)++;
		if (nPm == nPn)
			continue;
		switch (nFn) {
		case 1:
			res += evt_Read(b, nPm, nPn, 1);
			break;
		default:
			res += evt_Read(b, nPm, nPn, 0);
			break;
		}
	}
	if (res == 0)
		buf_Unpush(b, 6);
	return res;
}
#endif


