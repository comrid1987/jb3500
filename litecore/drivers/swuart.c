
#include <drivers/swuart.h>


//Private Defines
#define SWUART_ISO7816_BAUD		4800

#define SWUART_IDLE				0
#define SWUART_START_BIT		1
#define SWUART_DATA_1			2
#define SWUART_DATA_2			3
#define SWUART_DATA_3			4
#define SWUART_DATA_4			5
#define SWUART_DATA_5			6
#define SWUART_DATA_6			7
#define SWUART_DATA_7			8
#define SWUART_DATA_8			9
#define SWUART_DATA_9			10
#define SWUART_STOP_1			11
#define SWUART_STOP_2			12
#define SWUART_WAIT_1			13
#define SWUART_WAIT_2			14
#define SWUART_WAIT_3			15
#define SWUART_WAIT_4			16
#define SWUART_WAIT_5			17
#define SWUART_WAIT_6			18
#define SWUART_WAIT_7			19
#define SWUART_WAIT_8			20
#define SWUART_WAIT_9			21
#define SWUART_WAIT_10			22
#define SWUART_WAIT_11			23
#define SWUART_WAIT_12			24
#define SWUART_WAIT_13			25
#define SWUART_WAIT_14			26
#define SWUART_WAIT_15			27
#define SWUART_WAIT_16			28




//Private Typedef
typedef struct {
	p_dev_uart	parent;
	uint8_t		rxint;
	uint8_t		rxste;
	uint8_t		rxdata;
	uint8_t		txste;
	uint8_t		txdata;
	uint8_t		txpari;
	uint8_t		txv;
	uint16_t	tick;
}t_swuart;



//Private Variables
static t_swuart swuart_aDev[SWUART_QTY];


static void swuart_RxStart(void *args)
{
	t_swuart *p = (t_swuart *)args;
	p_uart_def pDef = p->parent->def;

	p->rxste = SWUART_DATA_1;
	irq_TimerStart(pDef->id, p->tick + (p->tick >> 1) - 200);
	irq_ExtDisable(p->rxint);
}

static void swuart_IrdaTimer(void *args)
{
	t_swuart *p = (t_swuart *)args;
	p_uart_def pDef = p->parent->def;

	p->txv ^= 1;
	arch_GpioSet(pDef->txport, pDef->txpin, p->txv);
}

static void swuart_TxOut(uint_t nFun, uint_t nPort, uint_t nPin, uint_t nValue)
{

#if IRDA_ENABLE
	if (nFun == UART_FUN_IRDA) {
#if IRDA_MODE == IRDA_MODE_TIM
		if (nValue)
			irq_TimerStop(nPort);
		else
			irq_TimerStart(nPort, arch_TimerClockGet() / 76000);
#else
		if (nValue)
			arch_PwmStop(nPort, nPin);
		else
			arch_PwmStart(nPort, nPin);
#endif
	} else
#endif
		arch_GpioSet(nPort, nPin, nValue);
}

static void swuart_RxTx(void *args)
{
	t_swuart *p = (t_swuart *)args;
	p_dev_uart pUart = p->parent;
	p_uart_def pDef = pUart->def;
	int nData;
	uint_t nPort, nPin;

	if (p->rxste != SWUART_IDLE) {
		nPort = pDef->rxport;
		nPin = pDef->rxpin;
		switch (p->rxste) {
		case SWUART_DATA_1:
			irq_TimerStart(pDef->id, p->tick);
			if (arch_GpioRead(nPort, nPin))
				p->rxdata = 1;
			else
				p->rxdata = 0;
			p->rxste += 1;
			break;
		case SWUART_DATA_7:
			if (arch_GpioRead(nPort, nPin))
				p->rxdata |= BITMASK(p->rxste - SWUART_DATA_1);
			if (pUart->para.data == UART_DATA_7D) {
				if (pUart->para.pari == UART_PARI_NO) {
					p->rxste = SWUART_STOP_1;
					break;
				}
			}
			p->rxste += 1;
			break;
		case SWUART_DATA_8:
			if (pUart->para.data == UART_DATA_7D) {
				p->rxste = SWUART_STOP_1;
				break;
			}
			if (arch_GpioRead(nPort, nPin))
				p->rxdata |= BITMASK(p->rxste - SWUART_DATA_1);
			if (pUart->para.pari == UART_PARI_NO) {
				p->rxste = SWUART_STOP_1;
				break;
			}
			p->rxste += 1;
			break;
		case SWUART_DATA_9:
			p->rxste = SWUART_STOP_1;
			break;
		case SWUART_STOP_1:
#if IO_BUF_TYPE == BUF_T_BUFFER
			buf_PushData(pUart->bufrx, pUart->rxdata, 1);
#elif IO_BUF_TYPE == BUF_T_DQUEUE
			dque_Push(dqueue, pUart->parent->id | UART_DQUE_RX_CHL, &p->rxdata, 1);
#endif
			irq_ExtEnable(p->rxint);
			if (pUart->para.stop == UART_STOP_1D)
				p->rxste = SWUART_WAIT_1;
			else
				p->rxste = SWUART_STOP_2;
			break;
		case SWUART_STOP_2:
			p->rxste = SWUART_WAIT_1;
			break;
		case SWUART_WAIT_1:
		case SWUART_WAIT_2:
		case SWUART_WAIT_3:
			p->rxste += 1;
			break;
		case SWUART_WAIT_4:
			if (p->txste == SWUART_IDLE)
				irq_TimerStop(pDef->id);
			p->rxste = SWUART_IDLE;
			break;
		default:
			if (arch_GpioRead(nPort, nPin))
				p->rxdata |= BITMASK(p->rxste - SWUART_DATA_1);
			p->rxste += 1;
			break;
		}
		return;
	}
	if (p->txste != SWUART_IDLE) {
#if SMARTCARD_ENABLE
		if (pDef->fun == UART_FUN_SC) {
			nPort = pDef->rxport;
			nPin = pDef->rxpin;
		} else {
#endif
			nPort = pDef->txport;
			nPin = pDef->txpin;
#if SMARTCARD_ENABLE
		}
#endif
		switch (p->txste) {
		case SWUART_START_BIT:
#if IO_BUF_TYPE == BUF_T_BUFFER
			if (p->buftx->len == 0) {
				irq_TimerStop(pDef->id);
				irq_ExtEnable(p->rxint);
				p->txste = SWUART_IDLE;
				break;
			}
			p->txdata = pUart->buftx->p[0];
			buf_Remove(pUart->buftx, 1);
#elif IO_BUF_TYPE == BUF_T_DQUEUE
			nData = dque_PopChar(dqueue, pUart->parent->id | UART_DQUE_TX_CHL);
			if (nData < 0) {
				irq_TimerStop(pDef->id);
				irq_ExtEnable(p->rxint);
				p->txste = SWUART_IDLE;
				break;
			}
			p->txdata = nData;
#endif
			p->txpari = 0;
			swuart_TxOut(pDef->fun, nPort, nPin, 0);
			p->txste = SWUART_DATA_1;
			break;
		case SWUART_DATA_7:
			if (p->txdata & BITMASK(6)) {
				swuart_TxOut(pDef->fun, nPort, nPin, 1);
				p->txpari ^= 1;
			} else
				swuart_TxOut(pDef->fun, nPort, nPin, 0);
			if (pUart->para.data == UART_DATA_7D) {
				if (pUart->para.pari == UART_PARI_NO) {
					p->txste = SWUART_STOP_1;
					break;
				}
			}
			p->txste += 1;
			break;
		case SWUART_DATA_8:
			if (pUart->para.data == UART_DATA_7D) {
				if (pUart->para.pari == UART_PARI_EVEN) {
					swuart_TxOut(pDef->fun, nPort, nPin, p->txpari);
				} else {
					if (p->txpari)
						swuart_TxOut(pDef->fun, nPort, nPin, 0);
					else
						swuart_TxOut(pDef->fun, nPort, nPin, 1);
				}
				p->txste = SWUART_STOP_1;
				break;
			}
			if (p->txdata & BITMASK(7)) {
				swuart_TxOut(pDef->fun, nPort, nPin, 1);
				p->txpari ^= 1;
			} else
				swuart_TxOut(pDef->fun, nPort, nPin, 0);
			if (pUart->para.pari == UART_PARI_NO) {
				p->txste = SWUART_STOP_1;
				break;
			}
			p->txste += 1;
			break;
		case SWUART_DATA_9:
			if (pUart->para.pari == UART_PARI_EVEN) {
				swuart_TxOut(pDef->fun, nPort, nPin, p->txpari);
			} else {
				if (p->txpari)
					swuart_TxOut(pDef->fun, nPort, nPin, 0);
				else
					swuart_TxOut(pDef->fun, nPort, nPin, 1);
			}
			p->txste = SWUART_STOP_1;
			break;
		case SWUART_STOP_1:
			swuart_TxOut(pDef->fun, nPort, nPin, 1);
			if (pUart->para.stop == UART_STOP_1D)
				p->txste = SWUART_START_BIT;
			else
				p->txste = SWUART_STOP_2;
			break;
		case SWUART_STOP_2:
			p->txste = SWUART_START_BIT;
			break;
		default:
			if (p->txdata & BITMASK(p->txste - SWUART_DATA_1)) {
				swuart_TxOut(pDef->fun, nPort, nPin, 1);
				p->txpari ^= 1;
			} else
				swuart_TxOut(pDef->fun, nPort, nPin, 0);
			p->txste += 1;
			break;
		}
		return;
	}
	irq_TimerStop(pDef->id);
}





//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
void swuart_Init(p_dev_uart p)
{
	uint_t nMode;
	t_uart_para xUart;
	p_uart_def pDef = p->def;

	swuart_aDev[pDef->id].parent = p;

	//Rx Pin Init
#if SMARTCARD_ENABLE
	if (pDef->fun == UART_FUN_SC)
		arch_GpioConf(pDef->rxport, pDef->rxpin, GPIO_M_OUT_OD, GPIO_INIT_HIGH);
	else
#endif
		arch_GpioConf(pDef->rxport, pDef->rxpin, GPIO_M_IN_FLOAT, GPIO_INIT_NULL);

	//Tx Pin Init
	if (pDef->outmode == DEV_PIN_PP)
		nMode = GPIO_M_OUT_PP;
	else
		nMode = GPIO_M_OUT_OD;
	switch (pDef->fun) {
#if SMARTCARD_ENABLE
	case UART_FUN_SC:
		arch_PwmConf(pDef->txport, pDef->txpin, nMode, SWUART_ISO7816_BAUD * 372);
		arch_PwmStart(pDef->txport, pDef->txpin);
		//Reset Pin Init
		arch_GpioConf(pDef->fport, pDef->fpin, nMode, GPIO_INIT_HIGH);
		xUart.baud = SWUART_ISO7816_BAUD;
		break;
#endif
#if IRDA_ENABLE
	case UART_FUN_IRDA:
#if IRDA_MODE == IRDA_MODE_TIM
		arch_GpioConf(pDef->txport, pDef->txpin, nMode, GPIO_INIT_HIGH);
		irq_TimerRegister(pDef->txport, swuart_IrdaTimer, &swuart_aDev[pDef->id]);
#else
		arch_PwmConf(pDef->txport, pDef->txpin, nMode, 38000);
#endif
		xUart.baud = 1200;
		break;
#endif
	default:
		arch_GpioConf(pDef->txport, pDef->txpin, nMode, GPIO_INIT_HIGH);
		xUart.baud = 115200;
		break;
	}
	xUart.pari = UART_PARI_EVEN;
	xUart.data = UART_DATA_8D;
	xUart.stop = UART_STOP_2D;
	swuart_Open(pDef->id, &xUart);
}


//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
sys_res swuart_Open(uint_t nId, p_uart_para pPara)
{
	t_swuart *pSW = &swuart_aDev[nId];
	p_dev_uart p = pSW->parent;
	int nIntId;

	if (memcmp(&p->para, pPara, sizeof(p->para))) {
		memcpy(&p->para, pPara, sizeof(p->para));
		pSW->tick = arch_TimerClockGet() / pPara->baud;
		irq_TimerRegister(nId, swuart_RxTx, pSW);
		nIntId = irq_ExtRegister(p->def->rxport, p->def->rxpin, IRQ_TRIGGER_FALLING, swuart_RxStart, pSW, IRQ_MODE_NORMAL);
		if (nIntId == -1)
			return SYS_R_ERR;
		pSW->rxint = nIntId;
		irq_ExtEnable(nIntId);
	}
	return SYS_R_OK;
}


//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
void swuart_TxStart(uint_t nId)
{
	t_swuart *p = &swuart_aDev[nId];
	p_uart_def pDef = p->parent->def;

	if (p->txste == SWUART_IDLE) {
		p->txste = SWUART_START_BIT;
		if (p->rxste == SWUART_IDLE) {
			irq_ExtDisable(p->rxint);
			irq_TimerStart(pDef->id, p->tick);
			swuart_RxTx(p);
		}
	}
}





