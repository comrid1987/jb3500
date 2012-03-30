#ifndef __HEADER_H__
#define __HEADER_H__



#ifdef __cplusplus
extern "C" {
#endif



//Include HeaderFiles
#if (ARCH_TYPE == ARCH_T_STM32F10X_HD) || (ARCH_TYPE == ARCH_T_STM32F10X_CL) || \
	(ARCH_TYPE == ARCH_T_LM3S9X) || (ARCH_TYPE == ARCH_T_LM3S5X) || \
	(ARCH_TYPE == ARCH_T_LPC176X)

#include <arch/cm3/typedef.h>
#endif

#if ARCH_TYPE == ARCH_T_M051X

#include <arch/cm0/typedef.h>
#endif

#if (ARCH_TYPE == ARCH_T_STR71X) || \
	(ARCH_TYPE == ARCH_T_LPC22XX)

#include <arch/arm7/typedef.h>
#endif


//Waiting timeout defines
#define OS_TMO_FOREVER			(-1)
#define OS_TMO_NO				0

#if OS_TYPE
#include <os/os.h>
#endif

#include <sys/system.h>


#if (ARCH_TYPE == ARCH_T_STM32F10X_HD) || (ARCH_TYPE == ARCH_T_STM32F10X_CL)
#include <arch/cm3/stm32f10x/arch.h>

#elif (ARCH_TYPE == ARCH_T_LM3S9X) || (ARCH_TYPE == ARCH_T_LM3S5X)
#include <arch/cm3/lm3s/arch.h>

#elif ARCH_TYPE == ARCH_T_LPC176X
#include <arch/cm3/lpc176x/arch.h>

#elif ARCH_TYPE == ARCH_T_M051X
#include <arch/cm0/m051/arch.h>

#elif ARCH_TYPE == ARCH_T_STR71X
#include <arch/arm7/str71x/arch.h>

#elif ARCH_TYPE == ARCH_T_LPC22XX
#include <arch/arm7/lpc22xx/arch.h>

#endif


#ifdef __cplusplus
}
#endif


#endif


