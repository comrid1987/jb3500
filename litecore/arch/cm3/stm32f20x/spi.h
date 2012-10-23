#ifndef __ARCH_SPI_H__
#define __ARCH_SPI_H__


#ifdef __cplusplus
extern "C" {
#endif











//External Functions
void arch_SpiInit(p_dev_spi p);
sys_res arch_SpiConfig(p_dev_spi p);
sys_res arch_SpiSend(p_dev_spi p, uint8_t *pData, uint_t nLen);
sys_res arch_SpiRecv(p_dev_spi p, uint8_t *pRec, uint_t nLen);
sys_res arch_SpiTransce(p_dev_spi p, uint_t nCmd, uint8_t *pRec, uint_t nLen);



#ifdef __cplusplus
}
#endif

#endif

