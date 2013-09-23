#ifndef __ARCH_UART_H__
#define __ARCH_UART_H__


#ifdef __cplusplus
extern "C" {
#endif









//External Functions
void arch_UartInit(p_dev_uart p);
sys_res arch_UartOpen(uint_t nId, p_uart_para pPara);
void arch_UartTxIEnable(uint_t nID);
void arch_UartSendChar(uint_t nId, const int nData);
int arch_UartGetChar(uint_t nId);

void arch_UartISR(uint_t nId);



#ifdef __cplusplus
}
#endif

#endif

