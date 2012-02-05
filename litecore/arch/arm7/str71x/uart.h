#ifndef __ARCH_UART_H__
#define __ARCH_UART_H__






//External Functions
void arch_UartInit(p_dev_uart p);
sys_res arch_UartOpen(p_dev_uart p, p_uart_para pPara);
void arch_UartTxIEnable(p_dev_uart p);
void arch_UartIoHandler(p_dev_uart p, uint_t nId);
void arch_UartScReset(p_dev_uart p, uint_t nHL);


#endif

