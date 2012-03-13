#include <litecore.h>
#include <drivers/tdk6515.h>
#include "system.h"
#include "display.h"
#include "upcomm.h"
#include "meter.h"
#include "para.h"



//Public Variables
volatile uint_t g_sys_status = 3;




//OS thread declares
os_thd_declare(Upcom1, 1280);
os_thd_declare(Upcom2, 1024);
os_thd_declare(Daemon, 1024);
os_thd_declare(Display, 1024);
os_thd_declare(Meter, 1280);


sys_res sys_IsUsbReady()
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
			for (i = 0; i < 3; i++) {
				if (nSpan & BITMASK(i)){
                    BEEP(1);
                    data_YXWrite(i + 1);
	                os_thd_Sleep(100);
                    BEEP(0);
				}
			}
			os_que_Release(que);
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

void app_Entry()
{
	uint_t i = 1;

	BEEP(0);
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
}

int main(void)
{

	sys_Start();
}

