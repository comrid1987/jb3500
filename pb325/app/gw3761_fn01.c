#include <litecore.h>
#include "para.h"
#include "data.h"
#include "alarm.h"


//Internal Functions




//External Functions
int gw3761_ResponseReset(p_gw3761 p)
{
	int res = 0;
	uint_t nFn;

	nFn = p->rmsg.data->p[2];
	switch (nFn) {
	case 1:		//F1
		res += 1;
		break;
	case 2:		//F2
		data_Clear();
		evt_Clear();
		res += 1;
		break;
	case 4:		//F3
	case 8:		//F4
		icp_Clear();
		data_Clear();
		evt_Clear();
		res += 1;
		break;
	default:
		break;
	}
	if (res)
		gw3761_TmsgConfirm(p);
	else
		gw3761_TmsgReject(p);
	if (nFn == 1) {
		os_thd_Sleep(10000);
		sys_Reset();
	}
	return res;
}


