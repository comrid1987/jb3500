#ifndef __I2CBUS_H__
#define __I2CBUS_H__


#ifdef __cplusplus
extern "C" {
#endif








//External Functions
sys_res i2cbus_Init(p_dev_i2c p);
sys_res i2cbus_Write(p_dev_i2c p, uint_t nDev, const void *pData, uint_t nLen);
sys_res i2cbus_Read(p_dev_i2c p, uint_t nDev, void *pData, uint_t nLen);


#ifdef __cplusplus
}
#endif

#endif

