#include <litecore.h>
#include <drivers/tdk6515.h>
#include "display.h"
#include "upcomm.h"
#include "meter.h"
#include "para.h"


//OS thread declares
os_thd_declare(Upcom1, 1280);
os_thd_declare(Upcom2, 1024);
os_thd_declare(Daemon, 1024);
os_thd_declare(Display, 1024);
os_thd_declare(Meter, 1280);


void tsk_Daemon(void *args)
{
	uint_t nTick;

    for (nTick = 0; ; nTick++) {
		os_thd_Sleep(200);
		if (nTick & 2)
			LED_RUN(1);
		else
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

void hold()
{
	uint_t i;

	for (i = 1; i; );
}

int main(void)
{

//	hold();
	sys_Start();
}

