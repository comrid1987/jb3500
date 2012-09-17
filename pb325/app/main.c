#include <litecore.h>
#include <drivers/tdk6515.h>
#include "system.h"
#include "para.h"
#include "data.h"
#include "display.h"
#include "upcomm.h"
#include "meter.h"
#include "alarm.h"
#include "acm.h"

//Private Defines


//Public Variables
volatile uint_t g_sys_status;




//OS thread declares
os_thd_declare(Upcom1, 1800);
os_thd_declare(Upcom2, 1800);
os_thd_declare(Display, 1280);
os_thd_declare(Meter, 1560);
os_thd_declare(Acm, 1560);
os_thd_declare(Idle, 1280);


sys_res sys_IsUsbFormat()
{
	DIR_POSIX *d;

	d = fs_opendir(FS_USBMSC_PATH);
	if (d == NULL)
		return SYS_R_ERR;
	fs_closedir(d);
	return SYS_R_OK;
}



void tsk_Idle(void *args)
{
	os_que que;

	for (; ; ) {
		que = os_que_Wait(QUE_EVT_USER_EVT, NULL, 1000);
		if (que != NULL) {
			os_que_Release(que);
			data_Copy2Udisk();
		}
	}
}

void app_Daemon(uint_t nCnt)
{
	uint_t nTemp;
	static uint_t nRstCnt = 0;

	//运行指示灯
	if (g_sys_status & BITMASK(SYS_STATUS_UART))
		nTemp = 1;
	else
		nTemp = 7;
	if ((nCnt & nTemp) == 0)
		LED_RUN(1);
	if ((nCnt & nTemp) == 1)
		LED_RUN(0);
	//2小时无通讯复位终端
	if ((nCnt & 0xFF) == 0) {
		if (g_sys_status & BITMASK(SYS_STATUS_LOGIN)) {
			CLRBIT(g_sys_status, SYS_STATUS_LOGIN);
			nRstCnt = 0;
		} else {
			nRstCnt += 1;
			if (nRstCnt > 300)
				sys_Reset();
		}
	}
}


void app_Entry()
{

	icp_Init();
	evt_Init();

	os_thd_Create(Display, 180);
	os_thd_Create(Acm, 160);
	os_thd_Create(Upcom1, 140);
	os_thd_Create(Upcom2, 120);
	os_thd_Create(Meter, 100);
	os_thd_Create(Idle, 20);
}

int main(void)
{

	BEEP(0);
	sys_Start();
}

