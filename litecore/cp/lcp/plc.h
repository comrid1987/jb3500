#ifndef __LCP_PLC_H__
#define __LCP_PLC_H__

#ifdef __cplusplus
extern "C" {
#endif



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


//GDW3761Ω” ’÷°
typedef struct {
	uint8_t		type;
	uint8_t		mode;
	uint8_t		adr[6];
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
void plc_Reset(t_plc *p);
sys_res plc_Sync(t_plc *p);

int plc_GetRetry(t_plc *p);
int plc_GetWait(t_plc *p, uint_t nRelay);
int plc_IsNeedRt(t_plc *p);
int plc_IsNotSync(t_plc *p);

//User Functions
int plc_MeterRead(uint_t nSn, void *pAdr);


#ifdef __cplusplus
}
#endif

#endif



