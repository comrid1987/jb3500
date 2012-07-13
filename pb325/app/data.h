#ifndef __APP_DATA_H__
#define __APP_DATA_H__


#ifdef __cplusplus
extern "C" {
#endif





//Public Defines
#define ECL_RATE_QTY				4


//Public Typedefs
typedef struct {
	time_t		time;
	uint8_t		data[60];	
}t_data_min;

typedef struct {
	time_t		time;
	uint8_t		data[156];
}t_data_quarter;

typedef struct {
	time_t		time;
	uint32_t	data;
	uint32_t	rate[ECL_RATE_QTY];
}t_ecl_energy;

//Í³¼Æ
typedef struct {
	uint16_t	run;
	uint16_t	uup[3];
	uint16_t	iup[4];
	uint16_t	uover[3];
	uint16_t	iover[3];
	uint16_t	ulow[3];
	uint16_t	uunder[3];
	uint16_t	uok;
	uint16_t	ubalance;
	uint16_t	ibalance;
	uint16_t	uiup;
	uint16_t	uiover;
	uint16_t	cos[3];
	uint16_t	p0[4];
	float		usum[3];
	float		isum[4];
	float		uibsum[4];
	float		umin[3];
	float		umax[3];
	float		ubmax;
	float		imax[4];
	float		ibmax;
	float		pmax[4];
	time_t		tumin[3];
	time_t		tumax[3];
	time_t		tubmax;
	time_t		timax[4];
	time_t		tpmax[4];
	time_t		tibmax;
}t_stat, *p_stat;

//External Functions
void data_Clear(void);
void data_MinRead(const uint8_t *pTime, t_data_min *pData);
void data_MinWrite(const uint8_t *pTime, t_data_min *pData);
void data_QuarterRead(const uint8_t *pTime, t_data_quarter *pData);
void data_QuarterWrite(const uint8_t *pTime, t_data_quarter *pData);
int data_DayRead(const uint8_t *pTime, void *pData);
void data_DayWrite(const uint8_t *pTime, const void *pData);
void data_YXRead(buf b);
void data_YXWrite(uint_t nId);
void data_Copy2Udisk(void);



#ifdef __cplusplus
}
#endif

#endif


