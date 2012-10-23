#ifndef __ARCH_EPI_H__
#define __ARCH_EPI_H__






//External Functions
void stm32_FsmcInit(void);

__inline uint32_t arch_NandGetEcc(void);
__inline void arch_NandEccStart(void);


#endif

