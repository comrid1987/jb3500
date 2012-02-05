#ifndef __DM9000_H__
#define __DM9000_H__







sys_res dm9000_PacketReceive(void);
sys_res dm9000_PacketSend(const void *pData, uint_t nLen);
sys_res dm9000_Control(uint_t nCmd, void *args);
void dm9000_Init(void);


#endif

