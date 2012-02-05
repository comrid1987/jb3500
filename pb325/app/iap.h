#ifndef __APP_IAP_H__
#define __APP_IAP_H__


#ifdef __cplusplus
extern "C" {
#endif

#include <drivers/spiflash.h>


//Public Defines
#define IAP_MAGIC_WORD			0x77982161

#define UPDATE_SECTOR_START		0
#define UPDATE_SECTOR_BOOT		64



//Public Typedefs
struct iap_info {
	uint32_t	magic;
	uint32_t	size;
	uint8_t		reserve[120];
};








#ifdef __cplusplus
}
#endif


#endif


