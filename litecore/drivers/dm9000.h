#ifndef __DM9000_H__
#define __DM9000_H__



#ifdef __cplusplus
extern "C" {
#endif




sys_res dm9000_PacketReceive(void);
sys_res dm9000_PacketSend(const void *pBuf, uint_t nLen);
sys_res dm9000_Control(uint_t nCmd, void *args);
sys_res dm9000_Reset(const uint8_t *pMacAddr);
void dm9000_Init(void);


#ifdef __cplusplus
}
#endif

#endif

