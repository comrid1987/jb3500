#ifndef __ARCH_WDG_H__
#define __ARCH_WDG_H__


#ifdef __cplusplus
extern "C" {
#endif











//External Functions
#define arch_WdgReload()			IWDG_ReloadCounter()

void arch_WdgInit(void);


#ifdef __cplusplus
}
#endif

#endif

