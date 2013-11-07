#ifndef __LITECORE_H__
#define __LITECORE_H__


#ifdef __cplusplus
extern "C" {
#endif



//Header Includes
#include <def.h>

#include "bsp_cfg.h"

#include <lib/lib.h>

#include <mtd/flash.h>

#include <fs/bkp/bkp.h>
#if OS_TYPE
#include <fs/fs.h>

#include <chl/chl.h>

#include <reg/reg.h>

#include <cp/cp.h>

#include <hi/gui.h>
#endif

#include <dbg/dbg.h>

#if USB_ENABLE
#include <usb/usb.h>
#endif

#if MODEM_ENABLE
#include <drivers/modem.h>
#endif

#if TCPPS_ENABLE
#include <net/net.h>
#endif


//Warning
#if DEBUG_ENABLE
#warning "DebugMode is enabled........................."
#endif
#if WDG_ENABLE == 0
#warning "WatchDog is disabled........................."
#endif

//External Macros
#if WDG_ENABLE == 0
#define wdg_Reload(...)
#endif


#ifdef __cplusplus
}
#endif


#endif

