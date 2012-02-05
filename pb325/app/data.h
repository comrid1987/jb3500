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


//External Functions
void data_Clear(void);
void data_MinRead(const uint8_t *pTime, t_data_min *pData);
void data_MinWrite(const uint8_t *pTime, t_data_min *pData);
void data_QuarterRead(const uint8_t *pTime, t_data_quarter *pData);
void data_QuarterWrite(const uint8_t *pTime, t_data_quarter *pData);
void data_DayRead(uint_t nTn, const uint8_t *pAdr, const uint8_t *pTime, t_ecl_energy *pData);
void data_DayWrite(uint_t nTn, const uint8_t *pAdr, const uint8_t *pTime, t_ecl_energy *pData);



#ifdef __cplusplus
}
#endif

#endif


