#ifndef __LCP_PLC_H__
#define __LCP_PLC_H__

#ifdef __cplusplus
extern "C" {
#endif


//Public Defines
#define PLC_S_INIT				0
#define PLC_S_SYNC				1
#define PLC_S_IDLE				2
#define PLC_S_SLAVE				3
#define PLC_S_WAIT				4

//Modem Type Defines
#define PLC_T_XC_GW				0
#define PLC_T_XC_RT				1
#define PLC_T_XC_GD				2

#define PLC_T_ES_RT				4
#define PLC_T_ES_38				5

#define PLC_T_TOPCOM			7
#define PLC_T_BOST				8
#define PLC_T_RISECOM			9
#define PLC_T_LEAGUERME			10
#define PLC_T_MIARTECH			11


//Public Typedefs
typedef const struct {
	uint8_t		uartid;
	uint8_t		rst_effect;
	t_gpio_def	rst;
#if PLC_SET_ENABLE
	uint8_t		set_effect;
	t_gpio_def	set;
#endif
}t_plc_def;

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
}t_gw3762_ru;


typedef struct {
	uint8_t		ste;
	uint8_t		tmo;
	uint8_t		type;
	uint8_t		mode;
	uint8_t		time;
	uint8_t		adr[6];
	uint8_t		info[8];
	chl			chl;
	buf			rbuf;
	uint8_t		afn;
	t_gw3762_c	c;
	t_gw3762_ru	rup;
	uint16_t	fn;
	uint8_t		madr[6];
	uint8_t		radr[6];
	buf 		data;
}t_plc;





//External Functions
void plc_Init(t_plc *p);

sys_res plc_RealRead(t_plc *p, buf b, const uint8_t *pAdr, uint_t nCode, const void *pData, uint_t nLen, uint_t nRelay, const uint8_t *pRtAdr);
void plc_Broadcast(t_plc *p);
sys_res plc_Handler(t_plc *p, buf b, uint8_t *pAdr);

int plc_GetRetry(t_plc *p);
int plc_GetWait(t_plc *p, uint_t nRelay);
int plc_IsNeedRt(t_plc *p);
int plc_IsNotSync(t_plc *p);
void plc_GetInfo(t_plc *p, char *pInfo);


//User Functions
int plc_MeterRead(uint_t nSn, void *pAdr);
uint32_t plc_Request(const void *pAdr, uint_t *pIs97);
int plc_IsInTime(void);


#ifdef __cplusplus
}
#endif

#endif



