#if VOICE_ENABLE
#include <drivers/wtv.h>

#define WTV_AUDIO_MAX			20000

#define wtv_Reset(x)			sys_GpioSet(&tbl_bspVoice.tbl[0], x)
#define wtv_Nss(x)				sys_GpioSet(&tbl_bspVoice.tbl[1], x)
#define wtv_Busy()				sys_GpioRead(&tbl_bspVoice.tbl[2])





//-------------------------------------------------------------------------
//External Functions
//-------------------------------------------------------------------------
void wtv_Init()
{
	uint_t i;

	for (i = 0; i < tbl_bspVoice.qty; i++)
		sys_GpioConf(&tbl_bspVoice.tbl[i]);
}

sys_res wtv_Cmd(p_dev_spi p, uint_t nCmd, uint_t nIsBusy)
{
	uint_t i;
	sys_res res;

	wtv_Reset(0);
	os_thd_Slp1Tick();
	wtv_Reset(1);
	os_thd_Slp1Tick();
	wtv_Nss(0);
	os_thd_Slp1Tick();
	nCmd = invert_bits(nCmd, 8);
	res = spi_Send(p, &nCmd, 1);
	wtv_Nss(1);
	if (res != SYS_R_OK)
		return res;
	if (nIsBusy == 0)
		return SYS_R_OK;
	for (i = 100 / OS_TICK_MS; i; i--) {
		if (wtv_Busy() == 0)
			break;
		os_thd_Slp1Tick();
	}
	if (i)
		for (i = WTV_AUDIO_MAX / OS_TICK_MS; i; i--) {
			if (wtv_Busy())
				return SYS_R_OK;
			os_thd_Slp1Tick();
		}
	return SYS_R_TMO;
}


#endif

