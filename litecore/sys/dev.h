#ifndef __SYS_DEV_H__
#define __SYS_DEV_H__



#ifdef __cplusplus
extern "C" {
#endif


#define DEV_S_IDLE			0
#define DEV_S_READY			1
#define DEV_S_BUSY			2

#define DEV_T_UART			0
#define DEV_T_I2C			1
#define DEV_T_SPI			2
#define DEV_T_ETH			3

#define DEV_PIN_OD			0
#define DEV_PIN_PP			1

typedef struct {
	uint8_t	ste;
	uint8_t	type;
	uint8_t	id;
#if OS_TYPE
	os_tid	tid;
#endif
}dev[1];




//External Functions
sys_res dev_Get(dev p, int nTmo);
sys_res dev_Release(dev p);


#ifdef __cplusplus
}
#endif

#endif

