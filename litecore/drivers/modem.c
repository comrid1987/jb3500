#if MODEM_ENABLE
#include <drivers/modem.h>

//Private Defines
#define MODEM_PPP_ID			0

#if MODEM_DEBUG_ENABLE
#define modem_DbgOut			dbg_printf
#else
#define modem_DbgOut(...)
#endif


/* Modem Operation */
#if MODEM_DTR_ENABLE
#define modem_Dtr(p, x)			sys_GpioSet(&(p)->dtr, x)
#endif
#if MODEM_DCD_ENABLE
#define modem_Dcd(p)			sys_GpioRead(&(p)->dcd)
#endif

//Private Variables
static t_modem gsmModem[BSP_MODEM_QTY];
#if TCPPS_TYPE == TCPPS_T_KEILTCP
static BOOL tx_active;
/*----------------------------------------------------------------------------
 *      Serial Driver Functions
 *----------------------------------------------------------------------------
 *  Required functions for Serial driver module:
 *   - void init_serial ()
 *   - int  com_getchar ()
 *   - BOOL com_putchar (U8 c)
 *   - BOOL com_tx_active ()
 *   - interrupt function(s)
 *---------------------------------------------------------------------------*/


/*--------------------------- init_serial -----------------------------------*/
void init_serial()
{

	/* Initialize the serial interface */
	tx_active = __FALSE;
}

/*--------------------------- com_getchar -----------------------------------*/
int com_getchar()
{
	int c;

	if (modem_online() != __TRUE)
		return -1;
	/* Read a byte from serial interface */
	c = uart_GetRxChar(gsmModem[MODEM_PPP_ID].uart);
	if (c == -1)
		/* Serial receive buffer is empty. */
		return -1;
	return c;
}

/*--------------------------- com_putchar -----------------------------------*/
BOOL com_putchar(U8 c)
{

	tx_active = __TRUE;
	uart_Send(gsmModem[MODEM_PPP_ID].uart, &c, 1);
	tx_active = __FALSE;
	return __TRUE;
}

/*--------------------------- com_tx_active ---------------------------------*/
BOOL com_tx_active()
{

	/* Return status Transmitter active/not active.         */
	/* When transmit buffer is empty, 'tx_active' is __FALSE. */
	return tx_active;
}

#if __ARMCC_VERSION > 410000 
void modem_init()
#else
void init_modem()
#endif
{

}

void modem_dial(U8 *dialnum)
{

}

void modem_listen()
{

}

void modem_hangup()
{

}

BOOL modem_online()
{
	p_modem p = &gsmModem[MODEM_PPP_ID];

	if (p->dialed)
		return __TRUE;
	return __FALSE;
}

BOOL modem_process(U8 ch)
{

	return modem_online();
}

void modem_run()
{

}
#endif

static void modem_Act(t_modem_def *p, uint_t nHL)
{

	if (p->act_effect == MODEM_EFFECT_HIGH) {
		if (nHL)
			nHL = 0;
		else
			nHL = 1;
	}
	sys_GpioSet(&(p)->act, nHL);
}

#if MODEM_PWR_ENABLE
static void modem_Pwr(t_modem_def *p, uint_t nHL)
{

	if (p->pwr_effect == MODEM_EFFECT_HIGH) {
		if (nHL)
			nHL = 0;
		else
			nHL = 1;
	}
	sys_GpioSet(&(p)->power, nHL);
}
#endif
#if MODEM_RST_ENABLE
static void modem_Rst(t_modem_def *p, uint_t nHL)
{

	if (p->rst_effect == MODEM_EFFECT_HIGH) {
		if (nHL)
			nHL = 0;
		else
			nHL = 1;
	}
	sys_GpioSet(&(p)->rst, nHL);
}
#endif


static char *modem_FindStr(p_modem p, const char *str)
{
	int l1, l2;
	char *s1 = (char *)p->rbuf->p;

	l2 = strlen(str);
	if (!l2)
		return (char *) s1;
	l1 = p->rbuf->len;
	while (l1 >= l2)
	{
		l1--;
		if (!memcmp(s1,str,l2))
			return (char *) s1;
		s1++;
	}
	return NULL;
}




static sys_res modem_SendCmd(p_modem p, const char *pCmd, const char *pRes, uint_t nRetry)
{
	uint_t i;

	for (; nRetry; nRetry--) {
		buf_Release(p->rbuf);
		uart_Send(p->uart, pCmd, strlen(pCmd));
		os_thd_Sleep(100);
		for (i = 900 / OS_TICK_MS; i; i--) {
			if (uart_RecData(p->uart, p->rbuf, OS_TICK_MS) != SYS_R_OK)
				continue;
			if (modem_FindStr(p, pRes) != NULL) {
				modem_DbgOut("<Modem> %s OK", pCmd);
				return SYS_R_OK;
			}
		}
	}
	modem_DbgOut("<Modem> %s ERR", pCmd);
	return SYS_R_TMO;
}

static sys_res modem_InitCmd(p_modem p)
{
	uint_t i, nTemp;
	char *pTemp, str[64];

	uart_Config(p->uart, 115200, UART_PARI_NO, UART_DATA_8D, UART_STOP_1D);
#if MODEM_AUTOBAUD_ENABLE
	if (modem_SendCmd(p, "ATZ0\r", "OK\r", 30) != SYS_R_OK) {
		uart_Config(p->uart, 57600, UART_PARI_NO, UART_DATA_8D, UART_STOP_1D);
		if (modem_SendCmd(p, "ATZ0\r", "OK\r", 10) != SYS_R_OK)
			return SYS_R_TMO;
		if (modem_SendCmd(p, "AT+IPR=115200\r", "OK\r", 5) != SYS_R_OK)
			return SYS_R_TMO;
		uart_Config(p->uart, 115200, UART_PARI_NO, UART_DATA_8D, UART_STOP_1D);
	}
#else
	if (modem_SendCmd(p, "ATZ0\r", "OK\r", 30) != SYS_R_OK)
		return SYS_R_TMO;
#endif
	if (modem_SendCmd(p, "ATE0\r", "OK\r", 4) != SYS_R_OK)
		return SYS_R_TMO;
	//SIM卡串号
	p->ccid[0] = 0;
	for (i = 0; i < 3; i++) {
		if (modem_SendCmd(p, "AT+CCID\r", "OK\r", 1) != SYS_R_OK) {
	 		if (modem_SendCmd(p, "AT+ZGETICCID\r", "OK\r", 1) != SYS_R_OK)
				continue;
		}
		if ((pTemp = modem_FindStr(p, "CCID: ")) == NULL)
			continue;
		memcpy(p->ccid, pTemp + 6, 20);
		break;
	}
	if (modem_SendCmd(p, "AT+CPIN?\r", "OK\r", 30) != SYS_R_OK)
		return SYS_R_TMO;
	//注册网络
	for (i = 0; i < 20; i++) {
 		if (modem_SendCmd(p, "AT+CREG?\r", "OK\r", 1) != SYS_R_OK)
			continue;
		if ((pTemp = modem_FindStr(p, ",")) == NULL)
			continue;
		nTemp = atoi(pTemp + 1);
		if ((nTemp == 1) || (nTemp == 5))
			break;
		os_thd_Sleep(1000);
	}
	if (i >= 20)
		return SYS_R_TMO;
	//获得信号强度
	for (i = 0; i < 20; i++) {
		p->signal = 0;
 		if (modem_SendCmd(p, "AT+CSQ\r", "OK\r", 1) != SYS_R_OK)
			continue;
		if ((pTemp = modem_FindStr(p, ",")) == NULL)
			continue;
		p->signal = atoi(pTemp - 2);
		if ((p->signal > 0) && (p->signal != 99))
			break;
		os_thd_Sleep(1000);
	}
	if (p->signal == 0)
		return SYS_R_ERR;
	if (modem_SendCmd(p, "AT+CIMI\r", "OK\r", 10) != SYS_R_OK)
		return SYS_R_TMO;
	//获得网络
	if ((pTemp = modem_FindStr(p, "460")) == NULL)
		return SYS_R_ERR;
	if (pTemp[4] == '3')
		p->type = MODEM_TYPE_CDMA;
	else
		p->type = MODEM_TYPE_GPRS;

	switch (p->type) {
	case MODEM_TYPE_GPRS:
		sprintf(str, "AT+CGDCONT=1,\"IP\",\"%s\"\r", p->apn);
		break;
	case MODEM_TYPE_CDMA:
		sprintf(str, "AT&D2\r");
		break;
	default:
		return SYS_R_ERR;
	}
	if (modem_SendCmd(p, str, "OK\r", 20) != SYS_R_OK)
		return SYS_R_TMO;

	if (p->type == MODEM_TYPE_GPRS) {
		modem_SendCmd(p, "AT+CGATT=1\r", "OK\r", 10);
		modem_SendCmd(p, "AT+CGATT?\r", "+CGATT: 1", 30);
	}
		
	return SYS_R_OK;
}

#if TCPPS_TYPE == TCPPS_T_LWIP
static void modem_linkStatusCB(void *ctx, int errCode, void *arg)
{
	p_modem p = &gsmModem[(int)ctx];
	os_tid pThread;

	switch (errCode) {
	case PPPERR_NONE:
		p->ste = MODEM_S_READY;
		sifdefaultroute((int)ctx, 0, 0);
		break;
	case PPPERR_CONNECT:
		p->ste = MODEM_S_CLOSE;
		break;
	default:
		p->ste = MODEM_S_RESET;
//Unfinished		netif_set_default(dm9000_device.parent.netif);
		if (p->dialed) {
			pThread = os_thd_IdSelf();
			if (pThread->user_data != NULL)
				rt_free((void *)pThread->user_data);
			p->dialed = 0;
		}
		break;
	}
}
#endif

#if MODEM_DEBUG_ENABLE
static void modem_DbgIpInfo()
{
	uint8_t aIp[4], aMask[4], aGateway[4];

	net_GetIpPPP(aIp, aMask, aGateway);
	dbg_printf("<Modem> Online IP:%d.%d.%d.%d", aIp[0], aIp[1], aIp[2], aIp[3]);
}
#endif

static int modem_IsPowerOnEnable()
{
	p_modem p = &gsmModem[MODEM_PPP_ID];
	int res = 1;

	if (p->retrytime) {
		if (p->retryed >= p->retrytime) {
			res = 0;
			if (p->hour != rtc_pTm()->tm_hour)
				p->retryed = 0;
		} else
			p->hour = rtc_pTm()->tm_hour;
	}
	if (res) {
		if (p->cnt > p->span) {
			p->cnt = 0;
			p->retryed += 1;
		} else
			res = 0;
	}
	return res;
}



void modem_Init()
{
	p_modem p = &gsmModem[MODEM_PPP_ID];
	t_modem_def *pDef = tbl_bspModem[MODEM_PPP_ID];
	uint_t i;

	memset(p, 0, sizeof(t_modem));
	for (i = 0; i < BSP_MODEM_QTY; i++, p++) {
		sys_GpioConf(&pDef->act);
#if MODEM_PWR_ENABLE
		sys_GpioConf(&pDef->power);
#endif
#if MODEM_RST_ENABLE
		sys_GpioConf(&pDef->rst);
#endif
#if MODEM_CTS_ENABLE
		sys_GpioConf(&pDef->cts);
#endif
#if MODEM_RTS_ENABLE
		sys_GpioConf(&pDef->rts);
#endif
#if MODEM_DTR_ENABLE
		sys_GpioConf(&pDef->dtr);
#endif
#if MODEM_DCD_ENABLE
		sys_GpioConf(&pDef->dcd);
#endif
		p->uart = uart_Get(pDef->uartid, OS_TICK_MS);
	}
}



void modem_Run()
{
	p_modem p = &gsmModem[MODEM_PPP_ID];
	t_modem_def *pDef = tbl_bspModem[MODEM_PPP_ID];
	sys_res res;

	p->cnt += 1;
	switch (p->ste) {
	case MODEM_S_RESET:
#if MODEM_PWR_ENABLE
		modem_Pwr(pDef, 0);
#endif
		modem_Act(pDef, 0);
#if MODEM_RST_ENABLE
		modem_Rst(pDef, 0);
#endif
#if MODEM_DTR_ENABLE
		modem_Dtr(pDef, 0);
#endif
		p->ste = MODEM_S_POWEROFF;
		p->signal = 0;
		p->dialed = 0;
		p->tmo = (1 + p->tmo) * 4;
		if (p->tmo > 3600)
			p->tmo = 3600;
		p->cnt = 0;
		modem_DbgOut("<Modem> Power OFF %dS", p->tmo);
		break;
	case MODEM_S_POWEROFF:
//		if (modem_IsPowerOnEnable()) {
		if (p->cnt > p->tmo) {
			modem_Act(pDef, 1);
			p->ste = MODEM_S_POWERON;
			p->cnt = 0;
			modem_DbgOut("<Modem> Power ON");
		}
		break;
	case MODEM_S_POWERON:
#if MODEM_RST_ENABLE
		if (p->cnt == 3)
			modem_Rst(pDef, 1);
#endif
#if MODEM_PWR_ENABLE
		if (p->cnt == 5)
			modem_Pwr(pDef, 1);
#endif
#if MODEM_RST_ENABLE
		if (p->cnt == 7)
			modem_Rst(pDef, 0);
		if (p->cnt == 8)
			modem_Rst(pDef, 1);
#endif
		if (p->cnt == 9)
			modem_Act(pDef, 0);
		if (p->cnt == 11)
			modem_Act(pDef, 1);
		if (p->cnt > 16) {
			p->ste = MODEM_S_INIT;
			p->cnt = 0;
		}
		break;
	case MODEM_S_INIT:
		//Initiate modem and get type of modem
		res = modem_InitCmd(p);
		buf_Release(p->rbuf);
		if (res != SYS_R_OK) {
			modem_DbgOut("<Modem> Init fail");
			p->ste = MODEM_S_RESET;
		} else {
			modem_DbgOut("<Modem> Dial");
			if (p->type == MODEM_TYPE_CDMA)
				uart_Send(p->uart, "ATDT#777\r", 9);
			else
				uart_Send(p->uart, "ATD*99***1#\r", 12);
			p->dialed = 1;
#if TCPPS_TYPE == TCPPS_T_LWIP
			pppSetAuth(PPPAUTHTYPE_ANY, NULL, NULL);
			pppOpen(p->uart, modem_linkStatusCB, MODEM_PPP_ID);
#endif
#if TCPPS_TYPE == TCPPS_T_KEILTCP
			if (p->type == MODEM_TYPE_CDMA)
				ppp_connect("ATDT#777\r", "CARD", "CARD");
			else
				ppp_connect("ATD*99***1#\r", "", "");
#endif
			p->ste = MODEM_S_WAITDIAL;
			p->cnt = 0;
		}
		break;
	case MODEM_S_WAITDIAL:
		//Waitting for dial
		if (p->cnt > 120) {
			p->ste = MODEM_S_RESET;
			break;
		}
		if (modem_IsOnline() == SYS_R_OK) {
#if MODEM_DEBUG_ENABLE
			modem_DbgIpInfo();
#endif
			p->ste = MODEM_S_ONLINE;
			p->cnt = 0;
			p->tmo = 0;
		}
		break;
	case MODEM_S_ONLINE:
		if (p->cnt > p->idle) {
#if TCPPS_TYPE == TCPPS_T_LWIP
			pppClose(MODEM_PPP_ID);
			p->ste = MODEM_S_RESET;
#endif
#if TCPPS_TYPE == TCPPS_T_KEILTCP
			p->ste = MODEM_S_RESET;
#endif
			modem_DbgOut("<Modem> Idle %d tmo %d", p->idle, p->cnt);
		}
		break;
	default:
		modem_DbgOut("<Modem> Unknow ste ", p->ste);
		p->ste = MODEM_S_RESET;
		break;
	}
}


void modem_Config(char *pApn, uint_t nSpan, uint_t nRetry)
{
	p_modem p = &gsmModem[MODEM_PPP_ID];

	sprintf(p->apn, pApn);
	p->idle = nSpan * 3;
	p->retrytime = nRetry;
}

sys_res modem_IsOnline()
{
#if TCPPS_TYPE == TCPPS_T_LWIP
	p_modem p = &gsmModem[MODEM_PPP_ID];

	if ((p->ste == MODEM_S_READY) || (p->ste == MODEM_S_ONLINE)) {
#endif
#if TCPPS_TYPE == TCPPS_T_KEILTCP
	if (ppp_is_up() == __TRUE) {
#endif
#if MODEM_DCD_ENABLE
		uint_t i;
		for (i = 5; i; i--)
			if (modem_Dcd() == 0)
				break;
		if (i)
#endif
			return SYS_R_OK;
	}
	return SYS_R_ERR;
}

int modem_GetSignal()
{

	return gsmModem[MODEM_PPP_ID].signal;
}

int modem_GetState()
{

	return gsmModem[MODEM_PPP_ID].ste;
}

int modem_GetCCID(char *pCCID)
{
	p_modem p = &gsmModem[MODEM_PPP_ID];

	if (p->ccid[0] == 0)
		return 0;
	memcpy(pCCID, p->ccid, 20);
	return 1;
}




void modem_Refresh()
{

	if (modem_IsOnline() == SYS_R_OK)
		gsmModem[MODEM_PPP_ID].cnt = 0;
}



#endif

