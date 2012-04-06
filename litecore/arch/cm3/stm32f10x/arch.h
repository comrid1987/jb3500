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
#if 0
#include <stm32f10x_cl.h>

//#include <arch/cm3/stm32f10x/usblib/usb_bsp.h>
//#include <arch/cm3/stm32f10x/usblib/usb_hcd_int.h>
//#include <arch/cm3/stm32f10x/usblib/usbh_core.h>

#include <stm32f10x_adc.h>
#include <stm32f10x_bkp.h>
#include <stm32f10x_crc.h>
#include <stm32f10x_dma.h>
#include <stm32f10x_exti.h>
#include <stm32f10x_flash.h>
#include <stm32f10x_fsmc.h>
#include <stm32f10x_gpio.h>
//#include <stm32f10x_it.h>
#include <stm32f10x_iwdg.h>

#include <stm32f10x_nvic.h>

#include <stm32f10x_rcc.h>
#include <stm32f10x_usart.h>
#endif
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

