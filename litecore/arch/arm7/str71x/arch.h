#ifndef __ARCH_ARCH_H__
#define __ARCH_ARCH_H__


//Header Files
#include <71x_lib.h>

#include <arch/str71x/emi.h>
#include <arch/str71x/gpio.h>
#include <arch/str71x/io.h>
#include <arch/str71x/rtc.h>
#include <arch/str71x/uart.h>

#include <arch/str71x/it.h>


//External Functions
void arch_Init(void);
void arch_DbgInit(void);
void arch_IdleEntry(void);
void arch_Reset(void);


#endif

