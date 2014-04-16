#ifndef __MODEM_H__
#define __MODEM_H__


/* Modem State */
enum {
	MODEM_S_RESET = 0,
	MODEM_S_POWEROFF,	
	MODEM_S_POWERON,
	MODEM_S_INIT,
	MODEM_S_WAITDIAL,
	MODEM_S_ONLINE,
};


/* Modem Type */
#define MODEM_TYPE_GPRS		1
#define MODEM_TYPE_CDMA		0



//Public Typedefs
typedef struct {
	uint8_t		ste;
	uint8_t		type;
#if MODEM_ZTE_TCP
	uint8_t		ztetcp;
	uint8_t		ztecon;
#endif
	uint8_t		dialed;
	uint8_t		signal;
	uint8_t		retrytime;
	uint8_t		retryed;
	uint8_t		hour;
	uint8_t		flag;	//强制使用内部协议栈标识
	char		apn[20];
	char		user[32];
	char		pwd[32];
	char		info[20];
	char		ver[20];
	char		ccid[20];
	uint16_t	cnt;
	uint16_t	tmo;
	uint16_t	span;
	uint16_t	idle;
#if MODEM_FLOWCTL_ENABLE
	uint_t		flow_r;
	uint_t		flow_t;
#endif
	p_dev_uart	uart;
	buf			rbuf;
}t_modem, *p_modem;


typedef const struct {
	uint8_t		uartid;
	uint8_t		act_effect;
	t_gpio_def	act;
#if MODEM_PWR_ENABLE
	uint8_t		pwr_effect;
	t_gpio_def	power;
#endif
#if MODEM_RST_ENABLE
	uint8_t		rst_effect;
	t_gpio_def	rst;
#endif
#if MODEM_CTS_ENABLE
	t_gpio_def	cts;
#endif
#if MODEM_RTS_ENABLE
	t_gpio_def	rts;
#endif
#if MODEM_DTR_ENABLE
	t_gpio_def	dtr;
#endif
#if MODEM_DCD_ENABLE
	t_gpio_def	dcd;
#endif
}t_modem_def;


//External Functions
void modem_Init(void);
void modem_Run(void);
void modem_Reset(void);
void modem_PowerOff(void);
void modem_Config(const char *pApn, const char *pUser, const char *pPwd, uint_t nSpan, uint_t nRetry, uint_t nFlag);
int modem_IsOnline(void);
int modem_GetSignal(void);
int modem_GetState(void);
int modem_GetType(void);
int modem_GetInfo(char *pInfo);
int modem_GetVersion(char *pVersion);
int modem_GetCCID(char *pCCID);
#if MODEM_FLOWCTL_ENABLE
int modem_GetFlow(void);
int modem_GetRcvFlow(void);
int modem_GetSendFlow(void);
#endif
void modem_Refresh(void);
#if MODEM_ZTE_TCP
int modem_IsZteTcp(void);
sys_res me3000_TcpRecv(buf b);
sys_res me3000_TcpConnect(const uint8_t *pIp, uint_t nPort);
sys_res me3000_TcpSend(const void *pData, uint_t nLen);
sys_res me3000_TcpClose(void);
#endif

#endif

