#include <drivers/r802xsa.h>





//Private Defines
#define R202X_DEBUG_METHOD		0

#define R202X_SLAVEADR			0x64

#define R202X_REG_SEC			0x00
#define R202X_REG_MIN			0x10
#define R202X_REG_HOUR			0x20
#define R202X_REG_WDAY			0x30
#define R202X_REG_MDAY			0x40
#define R202X_REG_MONTH			0x50
#define R202X_REG_YEAR			0x60

#define R202X_REG_CR1			0xE0
#define R202X_REG_CR2			0xF0



//-------------------------------------------------------------------
//External Functions
//-------------------------------------------------------------------
sys_res rtc_i2c_Init(p_dev_i2c p)
{

	//24小时制使能
	return i2c_WriteByte(p, R202X_SLAVEADR, R202X_REG_CR1, 0x23);
}

sys_res rtc_i2c_GetTime(p_dev_i2c p, time_t *pTime)
{
	uint8_t aTime[8];

	if (i2c_WriteAdr(p, R202X_SLAVEADR, R202X_REG_SEC) != SYS_R_OK)
		return SYS_R_TMO;
	if (i2c_Read(p, R202X_SLAVEADR, aTime, 8) != SYS_R_OK)
		return SYS_R_TMO;
	memmove(&aTime[3], &aTime[4], 3);
#if R202X_DEBUG_METHOD & 1
	printf("<<<R202x TimeReg>>> %02X-%02X-%02X %02X:%02X:%02X\n", aTime[1], aTime[2], aTime[3], aTime[4], aTime[5], aTime[6]);
#endif
	*pTime = array2timet(&aTime[0], 1);
	return SYS_R_OK;
}

sys_res rtc_i2c_SetTime(p_dev_i2c p, time_t tTime)
{
	uint8_t aTime[6];

	timet2array(tTime, aTime, 1);
	if (i2c_WriteByte(p, R202X_SLAVEADR, R202X_REG_SEC, aTime[0]) != SYS_R_OK)
		return SYS_R_TMO;
	if (i2c_WriteByte(p, R202X_SLAVEADR, R202X_REG_MIN, aTime[1]) != SYS_R_OK)
		return SYS_R_TMO;
	if (i2c_WriteByte(p, R202X_SLAVEADR, R202X_REG_HOUR, aTime[2]) != SYS_R_OK)
		return SYS_R_TMO;
	if (i2c_WriteByte(p, R202X_SLAVEADR, R202X_REG_MDAY, aTime[3]) != SYS_R_OK)
		return SYS_R_TMO;
	if (i2c_WriteByte(p, R202X_SLAVEADR, R202X_REG_MONTH, aTime[4]) != SYS_R_OK)
		return SYS_R_TMO;
	if (i2c_WriteByte(p, R202X_SLAVEADR, R202X_REG_YEAR, aTime[5]) != SYS_R_OK)
		return SYS_R_TMO;
	return SYS_R_OK;
}


