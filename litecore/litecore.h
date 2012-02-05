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



//Warning
#if WDG_ENABLE == 0
#warning "WatchDog is disabled........................."
#endif




#ifdef __cplusplus
}
#endif


#endif

