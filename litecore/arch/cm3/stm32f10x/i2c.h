#ifndef __ARCH_I2C_H__
#define __ARCH_I2C_H__


#ifdef __cplusplus
extern "C" {
#endif













//External Functions
sys_res arch_I2cInit(p_dev_i2c p);
void arch_I2cIoHandler(p_dev_i2c p);
void arch_I2cErrorHandler(p_dev_i2c p);


#ifdef __cplusplus
}
#endif

#endif

