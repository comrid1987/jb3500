#ifndef __CHL_SOCKET_H__
#define __CHL_SOCKET_H__











//External Functions
sys_res chl_soc_Bind(chl p, uint_t nType, uint_t nId);
sys_res chl_soc_Connect(chl p, uint8_t *pIp, uint_t nPort);
sys_res chl_soc_Listen(chl p);
sys_res chl_soc_IsConnect(chl p);




#endif

