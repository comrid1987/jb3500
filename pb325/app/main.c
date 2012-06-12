#include <litecore.h>
#include <drivers/tdk6515.h>
#include "system.h"
#include "para.h"
#include "data.h"
#include "display.h"
#include "upcomm.h"
#include "meter.h"
#include "alarm.h"



//Public Variables
volatile uint_t g_sys_status = 3;




//OS thread declares
os_thd_declare(Upcom1, 1560);
os_thd_declare(Upcom2, 1560);
os_thd_declare(Daemon, 1024);
os_thd_declare(Display, 1280);
os_thd_declare(Meter, 1560);
os_thd_declare(Idle, 1280);


sys_res sys_IsUsbFormat()
{
	DIR_POSIX *d;

	d = fs_opendir(FS_USBMSC_PATH);
	if (d == NULL)
		return SYS_R_NOK;
	fs_closedir(d);
	return SYS_R_OK;
}

void tsk_Daemon(void *args)
{
	uint_t i, nCnt, nSpan;
	os_que que;

    for (nCnt = 0; ; nCnt++) {
		que = os_que_Wait(QUE_EVT_PULSE, NULL, 200);
		if (que != NULL) {
			nSpan = que->data->val;
			os_que_Release(que);
			for (i = 0; i < 3; i++) {
				if (nSpan & BITMASK(i)){
                    BEEP(1);
                    data_YXWrite(i + 1);
	                os_thd_Sleep(100);
                    BEEP(0);
				}
			}
		}
		if (g_sys_status & BITMASK(0))
			nSpan = 3;
		else
			nSpan = 1;
		if ((nCnt & nSpan) == 0)
			LED_RUN(1);
		if ((nCnt & nSpan) == 1)
			LED_RUN(0);
	}
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


void app_Entry()
{
	uint_t i = 1;

	icp_Init();
	evt_Init();

	if (i)
		os_thd_Create(Daemon, 160);
	if (i)
		os_thd_Create(Display, 140);
	if (i)
		os_thd_Create(Upcom1, 120);
	if (i)
		os_thd_Create(Upcom2, 100);
	if (i)
		os_thd_Create(Meter, 80);
	if (i)
		os_thd_Create(Idle, 20);
}

void hold()
{
	uint_t i;

	for (i = 1; i; );
}

int main(void)
{

	BEEP(0);
//	hold();
	sys_Start();
}

