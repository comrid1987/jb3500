#ifndef __VK321X_H__
#define __VK321X_H__








//-------------------------------------------------------------------
//External Functions
//-------------------------------------------------------------------
sys_res vk321x_Init(void);
sys_res vk321x_UartConfig(uint_t nChl, t_uart_para *pPara);
sys_res vk321x_UartSend(uint_t nChl, const void *pData, uint_t nLen);




#endif

