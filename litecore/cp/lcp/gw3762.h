#ifndef __GW3762_H__
#define __GW3762_H__


#ifdef __cplusplus
extern "C" {
#endif

//Public Defines

//GW376.2 AFN Defines
#define GW3762_AFN_CONFIRM			0x00
#define GW3762_AFN_RESET			0x01
#define GW3762_AFN_TRANSMIT			0x02
#define GW3762_AFN_DATA_FETCH		0x03
#define GW3762_AFN_DATA_SET			0x05
#define GW3762_AFN_REPORT			0x06
#define GW3762_AFN_ROUTE_FETCH		0x10
#define GW3762_AFN_ROUTE_SET		0x11
#define GW3762_AFN_ROUTE_CTRL		0x12
#define GW3762_AFN_ROUTE_TRANSMIT	0x13
#define GW3762_AFN_ROUTE_REQUEST	0x14
#define GW3762_AFN_AUTOREPORT		0xF0


//Modem Type Defines
#define GW3762_T_XC_GW				0
#define GW3762_T_XC_RT				1
#define GW3762_T_XC_GD				2

#define GW3762_T_ES_RT				4
#define GW3762_T_ES_38				5

#define GW3762_T_TOPCOM				7
#define GW3762_T_BOST				8
#define GW3762_T_RISECOM			9
#define GW3762_T_LEAGUERME			10
#define GW3762_T_MIARTECH			11



//Public Typedefs
typedef __packed struct {
	uint8_t		fun : 6,
				prm : 1,
				dir : 1;
}t_gw3762_c;

typedef __packed struct {
	uint8_t		route : 1,
				addit : 1,
				module : 1,
				clash : 1,
				relay : 4;
	uint8_t		chlid : 4,
				eccid : 4;
	uint8_t		acklen;
	uint16_t	rate : 15,
				unit : 1;
	uint8_t		reserve;
}t_gw3762_rdown;


typedef __packed struct {
	uint8_t		route : 1,
				addit : 1,
				module : 1,
				clash : 1,
				relay : 4;
	uint8_t		chlid : 4,
				eccid : 4;
	uint8_t		pulse : 4,
				chltype : 4;
	uint8_t		cmdq: 4,
				ackq: 4;
	uint16_t	reserve;
}t_gw3762_rup;


//GDW3761Ω” ’÷°
typedef struct {
	t_gw3762_c		c;
	t_gw3762_rup	rup;
	uint8_t			afn;
	uint16_t		fn;
	uint8_t			madr[6];
	uint8_t			adr[6];
	buf 			data;
}t_gw3762_rmsg;

typedef struct {
	uint8_t			type;
	uint8_t			adr[6];
	chl 			chl;
	buf				rbuf;
	t_gw3762_rmsg	rmsg;
}t_gw3762;




//External Functions
void gw3762_Init(t_gw3762 *p);

int gw3762_IsNeedRt(t_gw3762 *p);
int gw3762_IsNotSync(t_gw3762 *p);
int gw3762_GetRetry(t_gw3762 *p);
int gw3762_GetWait(t_gw3762 *p, uint_t nRelay);

sys_res gw3762_Analyze(t_gw3762 *p);

sys_res gw3762_Broadcast(t_gw3762 *p, const void *pAdr, const void *pData, uint_t nLen);
sys_res gw3762_MeterRead(t_gw3762 *p, const void *pAdr, uint_t nRelay, const void *pRtAdr, const void *pData, uint_t nLen);
sys_res gw3762_MeterRT(t_gw3762 *p, const void *pAdr, const void *pData, uint_t nLen);

sys_res gw3762_Confirm(t_gw3762 *p, uint_t nFlag, uint_t nTmo);
sys_res gw3762_HwReset(t_gw3762 *p, uint_t nTmo);
sys_res gw3762_ParaReset(t_gw3762 *p, uint_t nTmo);
sys_res gw3762_InfoGet(t_gw3762 *p, uint_t nTmo);
sys_res gw3762_ModAdrSet(t_gw3762 *p, const void *pAdr, uint_t nTmo);
sys_res gw3762_SubAdrQty(t_gw3762 *p, uint16_t *pQty, uint_t nTmo);
sys_res gw3762_SubAdrRead(t_gw3762 *p, uint_t nSn, uint16_t *pQty, uint8_t *pAdr, uint_t nTmo);
sys_res gw3762_StateGet(t_gw3762 *p, uint_t nTmo);
sys_res gw3762_SubAdrAdd(t_gw3762 *p, uint_t nSn, const void *pAdr, uint_t nPrtl, uint_t nTmo);
sys_res gw3762_SubAdrDelete(t_gw3762 *p, const void *pAdr, uint_t nTmo);
sys_res gw3762_ModeSet(t_gw3762 *p, uint_t nMode, uint_t nTmo);
sys_res gw3762_RtCtrl(t_gw3762 *p, uint_t nDT, uint_t nTmo);
sys_res gw3762_RequestAnswer(t_gw3762 *p, uint_t nPhase, const void *pAdr, uint_t nIsRead, const void *pData, uint_t nLen);

sys_res gw3762_Transmit(t_gw3762 *p, buf b, const void *pData, uint_t nLen);

sys_res gw3762_Es_ModeGet(t_gw3762 *p, uint_t *pMode, uint_t nTmo);
sys_res gw3762_Es_ModeSet(t_gw3762 *p, uint_t nMode, uint_t nTmo);


#ifdef __cplusplus
}
#endif


#endif

