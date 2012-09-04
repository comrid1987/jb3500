#ifndef __APP_METER_H__
#define __APP_METER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "para.h"


//Public Defines
#define ECL_PORT_ACM				1
#define ECL_PORT_RS485				2
#define ECL_PORT_CASCADE			30
#define ECL_PORT_PLC				31

#define ECL_PRTL_DLT645_97			1
#define ECL_PRTL_ACM				2
#define ECL_PRTL_DLT645_07			30
#define ECL_PRTL_DLQ_SY				201
#define ECL_PRTL_DLQ_QL				202



//Public Typedefs
typedef const struct {
	uint8_t		flag;
	uint8_t		type;
	uint32_t	di07;
}t_ecl_rtdi07;

typedef const struct {
	uint8_t		flag;
	uint8_t		type;
	uint16_t	di97;
}t_ecl_rtdi97;

typedef struct {
	uint8_t	ste;
	uint8_t	sn;
	void *	chl;
}t_ecl_task;



//External Functions
void tsk_Meter(void *args);

sys_res ecl_485_RealRead(buf b, uint_t nBaud, uint_t nTmo);




#ifdef __cplusplus
}
#endif

#endif

