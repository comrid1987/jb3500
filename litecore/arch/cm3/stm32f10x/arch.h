#ifndef __ARCH_ARCH_H__
#define __ARCH_ARCH_H__

#ifdef __cplusplus
extern "C" {
#endif


//Header Files
#if ARCH_TYPE == ARCH_T_STM32F10X_HD
#define STM32F10X_HD
#endif
#if ARCH_TYPE == ARCH_T_STM32F10X_CL
#define STM32F10X_CL
#define USE_USB_OTG_FS
#endif

#define USE_STDPERIPH_DRIVER
#include "stm32f10x.h"


#include <arch/cm3/stm32f10x/adc.h>
#include <arch/cm3/stm32f10x/bkp.h>
#include <arch/cm3/stm32f10x/fsmc.h>
#include <arch/cm3/stm32f10x/flash.h>
#include <arch/cm3/stm32f10x/gpio.h>
#include <arch/cm3/stm32f10x/i2c.h>
#include <arch/cm3/stm32f10x/iwdg.h>
#include <arch/cm3/stm32f10x/nand.h>
#include <arch/cm3/stm32f10x/rtc.h>
#include <arch/cm3/stm32f10x/spi.h>

#if OS_TYPE
#include <arch/cm3/stm32f10x/uart.h>
#endif

#include <arch/cm3/stm32f10x/it.h>



#if MCU_FREQUENCY == MCU_SPEED_LOW
#define MCU_CLOCK			8000000
#elif MCU_FREQUENCY == MCU_SPEED_HALF
#define MCU_CLOCK			36000000
#else
#define MCU_CLOCK			72000000
#endif







//External Functions
void arch_Init(void);
void arch_IdleEntry(void);
void arch_Reset(void);



#ifdef __cplusplus
}
#endif


#endif

