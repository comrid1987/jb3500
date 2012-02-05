#ifndef __PCA955X_H__
#define __PCA955X_H__




#ifdef __cplusplus
extern "C" {
#endif










//External Functions
sys_res pca955x_Init(p_dev_i2c p);
int pca955x_GpioRead(p_dev_i2c p, uint_t n);
void pca955x_GpioSet(p_dev_i2c p, uint_t nPin, uint_t nHL);



#ifdef __cplusplus
}
#endif

#endif

