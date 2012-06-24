#ifndef __LITECORE_H__
#define __LITECORE_H__


#ifdef __cplusplus
extern "C" {
#endif



//Header Includes
#include <def.h>

#include "bsp_cfg.h"

#include <lib/lib.h>

#if OS_TYPE
#include <fs/fs.h>
#include <chl/chl.h>

#include <reg/reg.h>

#include <cp/cp.h>
#endif

#include <dbg/dbg.h>

#if SPIFLASH_ENABLE
#include <drivers/spiflash.h>
#endif

#if MODEM_ENABLE
#include <drivers/modem.h>
#endif



#if TCPPS_ENABLE
#include <net/net.h>
#endif



//Warning
#if WDG_ENABLE == 0
#warning "WatchDog is disabled........................."
#endif




#ifdef __cplusplus
}
#endif


#endif

