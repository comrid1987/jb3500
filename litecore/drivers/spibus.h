#ifndef __SPIBUS_H__
#define __SPIBUS_H__



#ifdef __cplusplus
extern "C" {
#endif








//External Functions
sys_res spibus_Init(p_dev_spi p);
sys_res spibus_Config(p_dev_spi p);
void spibus_Start(p_dev_spi p);
void spibus_SendChar(p_dev_spi p, uint_t nData);
void spibus_End(p_dev_spi p);
sys_res spibus_Send(p_dev_spi p, const void *pData, uint_t nLen);
sys_res spibus_Recv(p_dev_spi p, void *pRec, uint_t nLen);
sys_res spibus_Transce(p_dev_spi p, const void *pCmd, uint_t nCmdLen, void *pRec, uint_t nRecLen);


#ifdef __cplusplus
}
#endif

#endif


