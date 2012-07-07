#include <drivers/rtcst.h>





//Private Defines
#define RTCST_DEBUG_METHOD		0

#define RTCST_SLAVEADR			0xD0

#define RTCST_REG_SEC			0x00
#define RTCST_REG_MIN			0x01
#define RTCST_REG_HOUR			0x02
#define RTCST_REG_WDAY			0x03
#define RTCST_REG_MDAY			0x04
#define RTCST_REG_MONTH			0x05
#define RTCST_REG_YEAR			0x06

#define RTCST_REG_CR1			0x0C



//-------------------------------------------------------------------
//External Functions
//-------------------------------------------------------------------
sys_res rtc_i2c_Init(p_dev_i2c p)
{

	//24小时制使能
	return i2c_WriteByte(p, RTCST_SLAVEADR, RTCST_REG_CR1, 0x00);
}

sys_res rtc_i2c_GetTime(p_dev_i2c p, time_t *pTime)
{
	uint8_t aTime[8];

	if (i2c_WriteAdr(p, RTCST_SLAVEADR, RTCST_REG_SEC) != SYS_R_OK)
		return SYS_R_TMO;
	if (i2c_Read(p, RTCST_SLAVEADR, aTime, 8) != SYS_R_OK)
		return SYS_R_TMO;
	memmove(&aTime[4], &aTime[5], 3);
#if R202X_DEBUG_METHOD & 1
	printf("<<<R202x TimeReg>>> %02X-%02X-%02X %02X:%02X:%02X\n", aTime[1], aTime[2], aTime[3], aTime[4], aTime[5], aTime[6]);
#endif
	*pTime = array2timet(&aTime[1], 1);
	return SYS_R_OK;
}

sys_res rtc_i2c_SetTime(p_dev_i2c p, time_t tTime)
{
	uint8_t aTime[9];

	aTime[0] = RTCST_REG_SEC;
	aTime[1] = 0;
	timet2array(tTime, &aTime[2], 1);
	memmove(&aTime[6], &aTime[5], 3);
	if (i2c_Write(p, RTCST_SLAVEADR, aTime, sizeof(aTime)) != SYS_R_OK)
		return SYS_R_TMO;
	return SYS_R_OK;
}



