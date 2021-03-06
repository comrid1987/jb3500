#ifndef __CHANNEL_H__
#define __CHANNEL_H__


//通道状态
#define CHL_S_IDLE				0
#define CHL_S_STANDBY			1
#define CHL_S_CONNECT			2
#define CHL_S_READY				3

//通道类型
#define CHL_T_RS232				0
#define CHL_T_IRDA				1
#define CHL_T_USB2UART			2

#define CHL_T_SOC_TC			(BITMASK(4) | 0)
#define CHL_T_SOC_TS			(BITMASK(4) | 1)
#define CHL_T_SOC_UC			(BITMASK(4) | 2)
#define CHL_T_SOC_US			(BITMASK(4) | 3)
#define CHL_T_SOC_TC_RECON		(BITMASK(4) | 4)


typedef struct {
	uint8_t	ste;
	uint8_t	type;
	sint8_t	err;
	void *	pIf;
}chl[1], t_chl;


#include <chl/rs232.h>
#include <chl/sockchl.h>





//External Functions
void chl_Init(chl p);
sys_res chl_Bind(chl p, uint_t nType, uint_t nId, int nTmo);
sys_res chl_Release(chl p);
sys_res chl_Send(chl p, const void *pData, uint_t nLen);
sys_res chl_RecData(chl p, buf b, uint_t nTmo);


#endif



