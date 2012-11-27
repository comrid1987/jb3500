#ifndef __SYS_WDG_H__
#define __SYS_WDG_H__


#ifdef __cplusplus
extern "C" {
#endif


//External Macros
#if WDG_ENABLE == 0
#define wdg_Reload(...)
#endif


//External Functions
void wdg_Init(void);
void wdg_Reload(uint_t nIntEnable);


#ifdef __cplusplus
}
#endif

#endif

