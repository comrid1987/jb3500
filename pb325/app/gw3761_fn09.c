#include <stdio.h>
#include <litecore.h>


//Internal Functions
//终端版本信息
static int gw3761_Afn09_F01(buf b)
{
	char str[8];

	buf_Push(b, "GZJB", 4);
	buf_Push(b, "GWJC0001", 8);
	sprintf(str, "%04X", VER_SOFT);
	buf_Push(b, str, 4);
	buf_PushData(b, 0x111112, 3);
	buf_Push(b, "       32MB", 11);
	buf_Push(b, "GWSX", 4);
	sprintf(str, "%04X", VER_HARD);
	buf_Push(b, str, 4);
	buf_PushData(b, 0x110310, 3);
	return 1;
}




//External Functions
int gw3761_ResponseGetConfig(p_gw3761 p, buf b, u_word2 *pDu)
{
	uint_t j, nFn;

	for (j = 0; j < 8; j++) {
		if ((pDu->word[1] & BITMASK(j)) == 0)
			continue;
		nFn = gw3761_ConvertDt2Fn((pDu->word[1] & 0xFF00) | BITMASK(j));
		switch (nFn) {
		case 1:
			//终端版本信息
			gw3761_Afn09_F01(b);
			break;
		default:
			break;
		}
	}
	return 0;
}



