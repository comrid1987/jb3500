#ifndef __MODEM_H__
#define __MODEM_H__


/* Modem State */
enum {
	MODEM_S_RESET = 0,
	MODEM_S_POWEROFF,	
	MODEM_S_POWERON,
	MODEM_S_INIT,
	MODEM_S_WAITDIAL,
	MODEM_S_READY,
	MODEM_S_ONLINE,
};


/* Modem Type */
#define MODEM_TYPE_GPRS		0
#define MODEM_TYPE_CDMA		1

/* Modem Controll Pin Effective */
#define MODEM_EFFECT_LOW	0
#define MODEM_EFFECT_HIGH	1


//Public Typedefs
typedef struct {
	uint8_t		ste;
	uint8_t		type;
	uint8_t		dialed;
	uint8_t		signal;
	uint8_t		retrytime;
	uint8_t		retryed;
	char		apn[19];
	uint16_t	cnt;
	uint16_t	tmo;
	uint16_t	span;
	uint16_t	idle;
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
void modem_Config(char *pApn, uint_t nSpan, uint_t nRetry);
void modem_SetUser(char *pUser);
void modem_SetPwd(char *pPwd);
sys_res modem_IsOnline(void);
int modem_GetSignal(void);
int modem_GetState(void);
sys_res modem_IsReady(void);
void modem_Refresh(void);


#endif

