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
#define APP_DAEMON_TIMER			1


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



#if APP_DAEMON_TIMER
static struct rt_timer timer_app_daemon;
void timer_Daemon(void *args)
{
	uint_t nTemp;
	static uint_t nCnt, nRstCnt = 0;

	//看门狗
#if WDG_ENABLE
	wdg_Reload(1);
#endif
	//运行指示灯
	if (g_sys_status & BITMASK(SYS_STATUS_UART))
		nTemp = 1;
	else
		nTemp = 3;
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
#else
os_thd_declare(Daemon, 512);
void tsk_Daemon(void *args)
{
	uint_t nTemp, nCnt, nRstCnt = 0;

    for (nCnt = 0; ; nCnt++) {
		os_thd_Sleep(200);
		//看门狗
#if WDG_ENABLE
		wdg_Reload(1);
#endif
		//运行指示灯
		if (g_sys_status & BITMASK(SYS_STATUS_UART))
			nTemp = 1;
		else
			nTemp = 3;
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
}
#endif

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

#if APP_DAEMON_TIMER
	rt_timer_init(&timer_app_daemon, "aDaemon", timer_Daemon, NULL, 200 / OS_TICK_MS, RT_TIMER_FLAG_PERIODIC);
	rt_timer_start(&timer_app_daemon);
#else
	os_thd_Create(Daemon, 220);
#endif

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

