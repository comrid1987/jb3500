#ifndef __ARCH_TIMER_H__
#define __ARCH_TIMER_H__


#ifdef __cplusplus
extern "C" {
#endif













//External Functions
void arch_TimerInit(uint_t nId);
void arch_TimerIntClear(uint_t nId);
void arch_TimerStart(uint_t nId, uint_t nValue);
void arch_TimerStop(uint_t nId);
uint_t arch_TimerClockGet(void);


#ifdef __cplusplus
}
#endif

#endif

