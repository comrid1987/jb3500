#ifndef __ARCH_BKP_H__
#define __ARCH_BKP_H__


#ifdef __cplusplus
extern "C" {
#endif













//External Functions
void arch_BkpInit(void);
sys_res arch_BkpWrite(uint_t nStartAdr, uint8_t *p, uint_t nLen);
sys_res arch_BkpRead(uint_t nStartAdr, uint8_t *p, uint_t nLen);




#ifdef __cplusplus
}
#endif


#endif

