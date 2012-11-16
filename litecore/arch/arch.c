


#if (ARCH_TYPE == ARCH_T_STM32F10X_HD) || (ARCH_TYPE == ARCH_T_STM32F10X_CL) || \
	(ARCH_TYPE == ARCH_T_LM3S5X) || (ARCH_TYPE == ARCH_T_LM3S9X) || \
	(ARCH_TYPE == ARCH_T_LPC176X) || (ARCH_TYPE == ARCH_T_STM32F20X)

#if OS_TYPE
#include <arch/cm3/cpuport.c>
#endif

#elif ARCH_TYPE == ARCH_T_M051X
#include <arch/cm0/fault.c>
#include <arch/cm0/stack.c>

#endif





#if (ARCH_TYPE == ARCH_T_STM32F10X_HD) || (ARCH_TYPE == ARCH_T_STM32F10X_CL)
#ifdef RT_USING_CONSOLE
#include <arch/cm3/stm32f10x/serial.c>
#endif
#if ARCH_TYPE == ARCH_T_STM32F10X_CL
#include <arch/cm3/stm32f10x/system_stm32f10x.c>
#include <arch/cm3/stm32f10x/stm32f10x_rcc.c>
#endif
#include <arch/cm3/stm32f10x/arch.c>
#include <arch/cm3/stm32f10x/arch_adc.c>
#include <arch/cm3/stm32f10x/arch_bkp.c>
#include <arch/cm3/stm32f10x/arch_emac.c>
#include <arch/cm3/stm32f10x/arch_flash.c>
#include <arch/cm3/stm32f10x/arch_fsmc.c>
#include <arch/cm3/stm32f10x/arch_gpio.c>
#include <arch/cm3/stm32f10x/arch_i2c.c>
#include <arch/cm3/stm32f10x/arch_it.c>
#include <arch/cm3/stm32f10x/arch_iwdg.c>
#include <arch/cm3/stm32f10x/arch_nand.c>
#include <arch/cm3/stm32f10x/arch_rtc.c>
#include <arch/cm3/stm32f10x/arch_serial.c>
#include <arch/cm3/stm32f10x/arch_spi.c>
#include <arch/cm3/stm32f10x/arch_timer.c>
#include <arch/cm3/stm32f10x/arch_uart.c>
#include <arch/cm3/stm32f10x/arch_retarget.c>

#elif ARCH_TYPE == ARCH_T_STM32F20X
#include <arch/cm3/stm32f20x/misc.c>
#include <arch/cm3/stm32f20x/stm32f2xx_exti.c>
#include <arch/cm3/stm32f20x/stm32f2xx_fsmc.c>
#include <arch/cm3/stm32f20x/stm32f2xx_flash.c>
#include <arch/cm3/stm32f20x/stm32f2xx_gpio.c>
#include <arch/cm3/stm32f20x/stm32f2xx_iwdg.c>
#include <arch/cm3/stm32f20x/stm32f2xx_rcc.c>
#include <arch/cm3/stm32f20x/stm32f2xx_syscfg.c>
#include <arch/cm3/stm32f20x/stm32f2xx_tim.c>
#include <arch/cm3/stm32f20x/stm32f2xx_usart.c>


#ifdef RT_USING_CONSOLE
#include <arch/cm3/stm32f20x/serial.c>
#endif
#include <arch/cm3/stm32f20x/system_stm32f2xx.c>
#include <arch/cm3/stm32f20x/arch.c>
#include <arch/cm3/stm32f20x/arch_adc.c>
//#include <arch/cm3/stm32f20x/arch_bkp.c>
#include <arch/cm3/stm32f20x/arch_emac.c>
#include <arch/cm3/stm32f20x/arch_flash.c>
#include <arch/cm3/stm32f20x/arch_fsmc.c>
#include <arch/cm3/stm32f20x/arch_gpio.c>
#include <arch/cm3/stm32f20x/arch_i2c.c>
#include <arch/cm3/stm32f20x/arch_it.c>
#include <arch/cm3/stm32f20x/arch_iwdg.c>
#include <arch/cm3/stm32f20x/arch_nand.c>
//#include <arch/cm3/stm32f20x/arch_rtc.c>
#include <arch/cm3/stm32f20x/arch_serial.c>
#include <arch/cm3/stm32f20x/arch_spi.c>
#include <arch/cm3/stm32f20x/arch_timer.c>
#include <arch/cm3/stm32f20x/arch_uart.c>
#include <arch/cm3/stm32f20x/arch_retarget.c>


#elif ARCH_TYPE == ARCH_T_LPC176X
#ifdef RT_USING_CONSOLE
#include <arch/cm3/lpc176x/serial.c>
#endif
#include <arch/cm3/lpc176x/system_LPC17xx.c>
#include <arch/cm3/lpc176x/arch.c>
#include <arch/cm3/lpc176x/arch_emac.c>
#include <arch/cm3/lpc176x/arch_flash.c>
#include <arch/cm3/lpc176x/arch_gpio.c>
#include <arch/cm3/lpc176x/arch_iwdg.c>
#include <arch/cm3/lpc176x/arch_it.c>
#include <arch/cm3/lpc176x/arch_pwm.c>
#include <arch/cm3/lpc176x/arch_serial.c>
#include <arch/cm3/lpc176x/arch_timer.c>
#include <arch/cm3/lpc176x/arch_uart.c>



#elif (ARCH_TYPE == ARCH_T_LM3S9X) || (ARCH_TYPE == ARCH_T_LM3S5X)
#ifdef RT_USING_CONSOLE
#include <arch/cm3/lm3s/serial.c>
#endif
#if 0
#include <arch/cm3/lm3s/driverlib/adc.c>
#include <arch/cm3/lm3s/driverlib/cpu.c>
#include <arch/cm3/lm3s/driverlib/epi.c>
#include <arch/cm3/lm3s/driverlib/ethernet.c>
#include <arch/cm3/lm3s/driverlib/flash.c>
#include <arch/cm3/lm3s/driverlib/gpio.c>
#include <arch/cm3/lm3s/driverlib/interrupt.c>
#include <arch/cm3/lm3s/driverlib/sysctl.c>
#include <arch/cm3/lm3s/driverlib/systick.c>
#include <arch/cm3/lm3s/driverlib/timer.c>
#include <arch/cm3/lm3s/driverlib/uart.c>
#include <arch/cm3/lm3s/driverlib/usb.c>
#include <arch/cm3/lm3s/driverlib/watchdog.c>
#endif
#include <arch/cm3/lm3s/arch.c>
#include <arch/cm3/lm3s/arch_emac.c>
#include <arch/cm3/lm3s/arch_flash.c>
#include <arch/cm3/lm3s/arch_gpio.c>
#include <arch/cm3/lm3s/arch_iwdg.c>
#include <arch/cm3/lm3s/arch_it.c>
#include <arch/cm3/lm3s/arch_pwm.c>
#include <arch/cm3/lm3s/arch_serial.c>
#include <arch/cm3/lm3s/arch_timer.c>
#include <arch/cm3/lm3s/arch_uart.c>



#elif ARCH_TYPE == ARCH_T_M051X
#include <arch/cm0/m051/arch.c>
#include <arch/cm0/m051/arch_gpio.c>
#include <arch/cm0/m051/arch_it.c>
#include <arch/cm0/m051/arch_serial.c>
#include <arch/cm0/m051/arch_timer.c>
#include <arch/cm0/m051/arch_uart.c>







#elif ARCH_TYPE == ARCH_T_STR71X
#include <arch/arm7/str71x/arch.c>
#include <arch/arm7/str71x/arch_emi.c>
#include <arch/arm7/str71x/arch_gpio.c>
#include <arch/arm7/str71x/arch_it.c>
#include <arch/arm7/str71x/arch_rtc.c>
#include <arch/arm7/str71x/arch_serial.c>
#include <arch/arm7/str71x/arch_uart.c>
#include <arch/arm7/str71x/arch_retarget.c>

#elif ARCH_TYPE == ARCH_T_LPC22XX
#if OS_TYPE
#include <arch/arm7/lpc22xx/cpuport.c>
#endif
#include <arch/arm7/lpc22xx/arch.c>
#include <arch/arm7/lpc22xx/arch_flash.c>
#include <arch/arm7/lpc22xx/arch_gpio.c>
#include <arch/arm7/lpc22xx/arch_iwdg.c>
#include <arch/arm7/lpc22xx/arch_it.c>
#include <arch/arm7/lpc22xx/arch_timer.c>
#include <arch/arm7/lpc22xx/arch_uart.c>


#endif



