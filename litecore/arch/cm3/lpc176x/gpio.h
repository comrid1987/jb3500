#ifndef __ARCH_GPIO_H__
#define __ARCH_GPIO_H__


#ifdef __cplusplus
extern "C" {
#endif






//External Functions
void arch_GpioClockEnable(uint_t nPort);
void arch_GpioSel(uint_t nPort, uint_t nPin, uint_t nSel);
void arch_GpioConf(uint_t nPort, uint_t nPin, uint_t nMode, uint_t nInit);
void arch_GpioSet(uint_t nPort, uint_t nPin, uint_t nHL);
int arch_GpioRead(uint_t nPort, uint_t nPin);





#ifdef __cplusplus
}
#endif

#endif


