#ifndef __SYS_UART_H__
#define __SYS_UART_H__


#ifdef __cplusplus
extern "C" {
#endif



//Public Defines



//Public Typedefs
typedef struct {
	uint32_t	pari : 2,
				data : 4,
				stop : 2,
				baud : 24;
}t_uart_para, *p_uart_para;

typedef const struct {
	uint8_t		type;
	uint8_t		id;
	uint32_t	txport : 4,
				txpin : 5,
				txmode : 1,
				rxport : 4,
				rxpin : 5,
				rxmode : 1,
				fport : 4,
				fpin : 5,
				fun : 2,
				pinmode : 1;
}t_uart_def, *p_uart_def;

typedef struct {
	dev			parent;
	t_uart_para	para;
	p_uart_def	def;
#if IO_BUF_TYPE == BUF_T_BUFFER
	buf			buftx;
	buf			bufrx;
#endif
}t_dev_uart, *p_dev_uart;

#if IO_BUF_TYPE == BUF_T_DQUEUE
#define UART_DQUE_RX_CHL		0
#define UART_DQUE_TX_CHL		BITMASK(4)
#endif

#define UART_T_INT				0
#define UART_T_TIMER			1
#define UART_T_SC16IS7X			2
#define UART_T_VK321X			3

#define UART_FUN_NORMAL			0
#define UART_FUN_SC				1
#define UART_FUN_IRDA			2

#define UART_MODE_POLL			0
#define UART_MODE_IRQ			1

#define UART_PARI_NO			0
#define UART_PARI_EVEN			1
#define UART_PARI_ODD			2

#define UART_DATA_7D			0
#define UART_DATA_8D			1

#define UART_STOP_0_5D			0
#define UART_STOP_1D			1
#define UART_STOP_1_5D			2
#define UART_STOP_2D			3

#define IRDA_MODE_PWM			0
#define IRDA_MODE_TIM			1

#define SWUART_RX_M_EINT		0
#define SWUART_RX_M_CAP			1







//External Functions
void uart_Init(p_dev_uart p);
p_dev_uart uart_Get(uint_t nId, int nTmo);
sys_res uart_Release(p_dev_uart p);
sys_res uart_Config(p_dev_uart p, uint_t nBaud, uint_t nPari, uint_t nData, uint_t nStop);
sys_res uart_Send(p_dev_uart p, const void *pData, uint_t nLen);
sys_res uart_RecData(p_dev_uart p, buf b, int nTmo);
sys_res uart_RecLength(p_dev_uart p, buf b, uint_t nLen, int nTmo);
int uart_GetRxData(p_dev_uart p, buf b);
int uart_GetRxChar(p_dev_uart p);
sys_res uart_ScReset(p_dev_uart p, uint_t nHL);
void uart_Maintain(void);
void uart_Reopen(void);


#ifdef __cplusplus
}
#endif

#endif

