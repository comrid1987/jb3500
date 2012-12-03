#include <string.h>

#include <litecore.h>
#include "gw3761.h"



//External Functions
void gw3761_Response(p_gw3761 p)
{
	uint8_t *pData, *pEnd;
	int res = 0;
	buf b = {0};
	u_word2 uDu;

	switch (p->rmsg.afn) {
	case GW3761_AFN_CONFIRM:
	case GW3761_AFN_LINKCHECK:
		//不需回应
		break;
	case GW3761_AFN_RESET:
		gw3761_ResponseReset(p);
		break;
	case GW3761_AFN_PARA_SET:
		gw3761_ResponseSetParam(p);
		break;
	case GW3761_AFN_DATA_L1:
		gw3761_ResponseData1(p);
		break;
	case GW3761_AFN_DATA_L2:
		gw3761_ResponseData2(p);
		break;
	default:
		//统一回应
		pData = p->rmsg.data->p;
		pEnd = pData + p->rmsg.data->len;
		for (; (pData + 4) <= pEnd; ) {
			memcpy(&uDu, pData, 4);
			buf_Push(b, pData, 4);
			pData += 4;
			switch (p->rmsg.afn) {
			case GW3761_AFN_CMD_RELAY:
				break;
			case GW3761_AFN_CMD_CTRL:
				res += gw3761_ResponseCtrlCmd(p, &uDu, &pData);
				break;
			case GW3761_AFN_CONFIG_GET:
				res += gw3761_ResponseGetConfig(p, b, &uDu);
				break;
			case GW3761_AFN_PARA_GET:
				res += gw3761_ResponseGetParam(p, b, &uDu, &pData);
				break;
			case GW3761_AFN_DATA_L3:
				res += gw3761_ResponseData3(p, b, &uDu, &pData);
				break;
			case GW3761_AFN_FILE_TRANS:
				res += gw3761_ResponseFileTrans(p, b, &uDu, &pData);
				break;
			case GW3761_AFN_DATA_TRANS:
				res += gw3761_ResponseTransmit(p, b, &uDu, &pData);
				break;
			default:
				buf_Unpush(b, 4);
				break;
			}
		}
		if (b->len > 4) {
			gw3761_TmsgSend(p, GW3761_FUN_RESPONSE, p->rmsg.afn, b, DLRCP_TMSG_RESPOND);
		} else {
			if (res)
				gw3761_TmsgConfirm(p);
			else
				gw3761_TmsgReject(p);
		}
		buf_Release(b);
		break;
	}
}


