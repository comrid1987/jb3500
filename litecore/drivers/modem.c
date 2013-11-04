#if MODEM_ENABLE
#include <drivers/modem.h>

//Private Defines



//Private Macros
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
static t_modem gsmModem;
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
	c = uart_GetRxChar(gsmModem.uart);
	if (c == -1)
		/* Serial receive buffer is empty. */
		return -1;
#if MODEM_FLOWCTL_ENABLE
	gsmModem.flow += 1;
#endif
	return c;
}

/*--------------------------- com_putchar -----------------------------------*/
BOOL com_putchar(U8 c)
{

	tx_active = __TRUE;
	arch_UartSendChar(gsmModem.uart->def->id, c);
	tx_active = __FALSE;
#if MODEM_FLOWCTL_ENABLE
	gsmModem.flow += 1;
#endif
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
	p_modem p = &gsmModem;

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

	if (p->act_effect == GPIO_EFFECT_HIGH)
		nHL ^= 1;
	sys_GpioSet(&p->act, nHL);
}

#if MODEM_PWR_ENABLE
static void modem_Pwr(t_modem_def *p, uint_t nHL)
{

	if (p->pwr_effect == GPIO_EFFECT_HIGH)
		nHL ^= 1;
	sys_GpioSet(&p->power, nHL);
}
#endif
#if MODEM_RST_ENABLE
static void modem_Rst(t_modem_def *p, uint_t nHL)
{

	if (p->rst_effect == GPIO_EFFECT_HIGH)
		nHL ^= 1;
	sys_GpioSet(&p->rst, nHL);
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

	uart_Config(p->uart, MODEM_UART_BAUD, UART_PARI_NO, UART_DATA_8D, UART_STOP_1D);
	
#if MODEM_BAUD_ADJUST
	if (modem_SendCmd(p, "ATZ0\r", "OK\r", 30) != SYS_R_OK) {
		uart_Config(p->uart, MODEM_BAUD_ADJUST, UART_PARI_NO, UART_DATA_8D, UART_STOP_1D);
		if (modem_SendCmd(p, "ATZ0\r", "OK\r", 10) != SYS_R_OK)
			return SYS_R_TMO;
		sprintf(str, "AT+IPR=%d\r", MODEM_UART_BAUD);
		if (modem_SendCmd(p, str, "OK\r", 5) != SYS_R_OK)
			return SYS_R_TMO;
		uart_Config(p->uart, MODEM_UART_BAUD, UART_PARI_NO, UART_DATA_8D, UART_STOP_1D);
	}
#else
	if (modem_SendCmd(p, "ATZ0\r", "OK\r", 30) != SYS_R_OK)
		return SYS_R_TMO;
#endif
	if (modem_SendCmd(p, "ATE0\r", "OK\r", 4) != SYS_R_OK)
		return SYS_R_TMO;
	//模块信息
	p->info[0] = '\0';
	if (modem_SendCmd(p, "AT+CGMM\r", "OK\r", 1) == SYS_R_OK) {
		buf_Remove(p->rbuf, 2);
		if ((pTemp = modem_FindStr(p, ":")) != NULL)
			buf_Remove(p->rbuf, (uint8_t *)pTemp - p->rbuf->p + 1);
		if ((pTemp = modem_FindStr(p, "\r\n")) != NULL)
			pTemp[0] = '\0';
		memcpy(p->info, p->rbuf->p, 20);
	}
	p->ver[0] = '\0';
	if (modem_SendCmd(p, "AT+CGMR\r", "OK\r", 1) == SYS_R_OK) {
		buf_Remove(p->rbuf, 2);
		if ((pTemp = modem_FindStr(p, ":")) != NULL)
			buf_Remove(p->rbuf, (uint8_t *)pTemp - p->rbuf->p + 1);
		if ((pTemp = modem_FindStr(p, "\r\n")) != NULL)
			pTemp[0] = '\0';
		memcpy(p->ver, p->rbuf->p, 20);
	}
	if (modem_SendCmd(p, "AT+CPIN?\r", "OK\r", 30) != SYS_R_OK)
		return SYS_R_TMO;
	//注册网络
	for (i = 0; i < 30; i++) {
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

	//SIM卡串号
	p->ccid[0] = 0;
	if (p->type == MODEM_TYPE_GPRS) {
		for (i = 0; i < 3; i++) {
			if (modem_SendCmd(p, "AT+CCID\r", "OK\r", 1) != SYS_R_OK) {
		 		if (modem_SendCmd(p, "AT+ZGETICCID\r", "OK\r", 1) != SYS_R_OK)
					continue;
			}
			if ((pTemp = modem_FindStr(p, "CCID: ")) != NULL) {
				memcpy(p->ccid, pTemp + 6, 20);
				break;
			}
			if ((pTemp = modem_FindStr(p, "CCID:")) != NULL) {
				memcpy(p->ccid, pTemp + 5, 20);
				break;
			}
		}
	}

#if MODEM_ME3000_TCP
	if (modem_SendCmd(p, "AT+ZVERS\r", "OK\r", 3) == SYS_R_OK) {
		if ((pTemp = modem_FindStr(p, "ME3000_E")) != NULL) {
			sprintf(str, "AT+ZPNUM=\"%s\"\r", p->apn);
			if (modem_SendCmd(p, str, "OK\r", 10) == SYS_R_OK) {
				p->me3000 = 1;
				return SYS_R_OK;
			}
		}
	}
#endif

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
	
	if (p->type == MODEM_TYPE_CDMA) {
		os_thd_Sleep(5000);
	} else {
		modem_SendCmd(p, "AT+CGATT=1\r", "OK\r", 10);
		modem_SendCmd(p, "AT+CGATT?\r", "+CGATT: 1", 30);
	}
		
	return SYS_R_OK;
}

#if TCPPS_TYPE == TCPPS_T_LWIP
static void modem_linkStatusCB(void *ctx, int errCode, void *arg)
{
	p_modem p = &gsmModem;
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
	p_modem p = &gsmModem;
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
	p_modem p = &gsmModem;
	t_modem_def *pDef = tbl_bspModem[0];
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
	p_modem p = &gsmModem;
	t_modem_def *pDef = tbl_bspModem[0];
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
		if (p->cnt == 12)
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
#if MODEM_ME3000_TCP
			if (p->me3000) {
				if (modem_SendCmd(p, "AT+ZPPPOPEN\r", "OK\r", 10) == SYS_R_OK)
					p->ste = MODEM_S_WAITDIAL;
				else
					p->ste = MODEM_S_RESET;
			} else
#endif
			{
				if (p->type == MODEM_TYPE_CDMA)
					uart_Send(p->uart, "ATDT#777\r", 9);
				else
					uart_Send(p->uart, "ATD*99***1#\r", 12);
				p->dialed = 1;
#if TCPPS_TYPE == TCPPS_T_LWIP
				pppSetAuth(PPPAUTHTYPE_ANY, NULL, NULL);
				pppOpen(p->uart, modem_linkStatusCB, 0);
#endif
#if TCPPS_TYPE == TCPPS_T_KEILTCP
				if (p->type == MODEM_TYPE_CDMA)
					ppp_connect("ATDT#777\r", p->user, p->pwd);
				else
					ppp_connect("ATD*99***1#\r", "", "");
#endif
				p->ste = MODEM_S_WAITDIAL;
			}
			p->cnt = 0;
		}
		break;
	case MODEM_S_WAITDIAL:
		//Waitting for dial
		if (p->cnt > 120) {
			p->ste = MODEM_S_RESET;
			break;
		}
#if MODEM_ME3000_TCP
		if (p->me3000) {
			if (modem_SendCmd(p, "AT+ZPPPSTATUS\r", "ESTABLISHED\r", 1) == SYS_R_OK) {
				p->ste = MODEM_S_ONLINE;
				p->cnt = 0;
				p->tmo = 0;
			}
			break;
		}
#endif
		if (modem_IsOnline()) {
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
			pppClose(0);
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


void modem_Config(const char *pApn, const char *pUser, const char *pPwd, uint_t nSpan, uint_t nRetry)
{
	p_modem p = &gsmModem;
	int nLen;

	nLen = MIN(sizeof(p->apn) - 1, strlen(pApn));
	memcpy(p->apn, pApn, nLen);
	nLen = MIN(sizeof(p->user) - 1, strlen(pUser));
	memcpy(p->user, pUser, nLen);
	nLen = MIN(sizeof(p->pwd) - 1, strlen(pPwd));
	memcpy(p->pwd, pPwd, nLen);
	p->apn[nLen] = '\0';
	p->idle = nSpan * 3;
	p->retrytime = nRetry;
}

int modem_IsOnline()
{
	p_modem p = &gsmModem;

#if MODEM_DCD_ENABLE
	uint_t i;
	for (i = 5; i; i--) {
		if (modem_Dcd() == 0)
			break;
	}
	if (i = 0)
		return 0;
#endif
 
#if MODEM_ME3000_TCP
	if (p->me3000) {
		if (p->ste == MODEM_S_ONLINE)
			return 1;
	}
#endif
#if TCPPS_TYPE == TCPPS_T_LWIP
	if ((p->ste == MODEM_S_READY) || (p->ste == MODEM_S_ONLINE))
		return 1;
#endif
#if TCPPS_TYPE == TCPPS_T_KEILTCP
	if (ppp_is_up() == __TRUE)
		return 1;
#endif
	return 0;
}

int modem_GetSignal()
{

	return gsmModem.signal;
}

int modem_GetState()
{

	return gsmModem.ste;
}

int modem_GetType()
{

	return gsmModem.type;
}

int modem_GetInfo(char *pInfo)
{
	p_modem p = &gsmModem;

	if (p->info[0] == '\0')
		return 0;
	memcpy(pInfo, p->info, 20);
	return 1;
}

int modem_GetVersion(char *pVersion)
{
	p_modem p = &gsmModem;

	if (p->ver[0] == '\0')
		return 0;
	memcpy(pVersion, p->ver, 20);
	return 1;
}


int modem_GetCCID(char *pCCID)
{
	p_modem p = &gsmModem;

	if (p->ccid[0] == '\0')
		return 0;
	memcpy(pCCID, p->ccid, 20);
	return 1;
}


#if MODEM_FLOWCTL_ENABLE
int modem_GetFlow()
{
	int nFlow;

	nFlow = gsmModem.flow;
	gsmModem.flow = 0;
	return nFlow;
}
#endif

void modem_Refresh()
{

	if (modem_IsOnline())
		gsmModem.cnt = 0;
}


#if MODEM_ME3000_TCP

int modem_IsMe3000()
{
	
	return gsmModem.me3000;
}

int me3000_IsTcpCon()
{
	
	return gsmModem.tcpcon;
}

sys_res me3000_TcpRecv(buf b)
{
	p_modem p = &gsmModem;
	uint_t i, j, nLen;
	char *pTemp;

	if (modem_IsOnline() == 0)
		return SYS_R_ERR;
	for (i = 200 / OS_TICK_MS; i; i--) {
		if (uart_RecData(p->uart, p->rbuf, OS_TICK_MS) != SYS_R_OK)
			continue;
		pTemp = modem_FindStr(p, "+ZIPRECV:1,");
		if (pTemp != NULL)
			break;
	}
	if (i == 0)
		return SYS_R_ERR;
	buf_Remove(p->rbuf, (uint8_t *)pTemp - p->rbuf->p + 11);
	nLen = atoi((char *)p->rbuf->p);
	if (nLen > 1460)
		nLen = 1460;
	for (i = 200 / OS_TICK_MS; i; i--) {
		pTemp = modem_FindStr(p, ",");
		if (pTemp != NULL)
			break;
	}
	if (i == 0)
		return SYS_R_ERR;
	buf_Remove(p->rbuf, (uint8_t *)pTemp - p->rbuf->p + 1);
	for (j = 2000 / OS_TICK_MS; j; j--) {
		uart_RecData(p->uart, p->rbuf, OS_TICK_MS);
		if (p->rbuf->len >= nLen)
			break;
	}
	if (j) {
		buf_Push(b, p->rbuf->p, nLen);
		buf_Remove(p->rbuf, nLen);
#if MODEM_FLOWCTL_ENABLE
		gsmModem.flow += nLen;
#endif
		return SYS_R_OK;
	}
	buf_Release(p->rbuf);
	return SYS_R_ERR;
}

sys_res me3000_TcpConnect(const uint8_t *pIp, uint_t nPort)
{
	p_modem p = &gsmModem;
	char str[64];
	uint_t i;

	if (modem_IsOnline() == 0)
		return SYS_R_ERR;
	sprintf(str, "AT+ZIPSETUP=1,%d.%d.%d.%d,%d\r", pIp[0], pIp[1], pIp[2], pIp[3], nPort);
	buf_Release(p->rbuf);
	uart_Send(p->uart, str, strlen(str));
	os_thd_Sleep(3000);
	for (i = 5000 / OS_TICK_MS; i; i--) {
		if (uart_RecData(p->uart, p->rbuf, OS_TICK_MS) != SYS_R_OK)
			continue;
		if (modem_FindStr(p, "CONNECTED\r") != NULL)
			break;
		if (modem_FindStr(p, "ESTABLISHED\r") != NULL)
			break;
	}
	if (i) {
		p->tcpcon = 1;
		buf_Release(p->rbuf);
		return SYS_R_OK;
	}
	return SYS_R_TMO;
}

sys_res me3000_TcpSend(const void *pData, uint_t nLen)
{
	p_modem p = &gsmModem;
	char str[20];
	uint_t i;

	if (modem_IsOnline() == 0)
		return SYS_R_ERR;
	if (p->tcpcon == 0)
		return SYS_R_ERR;
	sprintf(str, "AT+ZIPSEND=1,%d\r", nLen);
	uart_Send(p->uart, str, strlen(str));
	os_thd_Sleep(100);
	for (i = 5000 / OS_TICK_MS; i; i--) {
		if (uart_RecData(p->uart, p->rbuf, OS_TICK_MS) != SYS_R_OK)
			continue;
		if (modem_FindStr(p, ">") == NULL)
			continue;
		uart_Send(p->uart, pData, nLen);
		uart_Send(p->uart, "\r", 1);
#if MODEM_FLOWCTL_ENABLE
		gsmModem.flow += nLen;
#endif
		return SYS_R_OK;
	}
	return SYS_R_TMO;
}

sys_res me3000_TcpClose()
{
	p_modem p = &gsmModem;
	uint_t i;

	p->tcpcon = 0;
	uart_Send(p->uart, "AT+ZIPCLOSE=1\r", 14);
	os_thd_Sleep(100);
	for (i = 1000 / OS_TICK_MS; i; i--) {
		if (uart_RecData(p->uart, p->rbuf, OS_TICK_MS) != SYS_R_OK)
			continue;
		if (modem_FindStr(p, "OK\r") != NULL)
			break;
		if (modem_FindStr(p, "ERROR\r") != NULL)
			break;
	}
	return SYS_R_OK;
}


#endif


#endif

