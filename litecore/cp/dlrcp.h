#ifndef __DLRCP_H__
#define __DLRCP_H__

#ifdef __cplusplus
extern "C" {
#endif


//-------------------------------------------------------------------------
//Public Defines
//-------------------------------------------------------------------------
//通讯状态定义
#define DLRCP_S_IDLE					0
#define DLRCP_S_CHECK					1
#define DLRCP_S_READY					2

//发送报文处理策略
#define DLRCP_TMSG_PULSEON				0
#define DLRCP_TMSG_RESPOND				1
#define DLRCP_TMSG_REPORT				2
#define DLRCP_TMSG_CASCADE				3



//-------------------------------------------------------------------------
//Public Typedefs
//-------------------------------------------------------------------------
#ifdef __CC_ARM
#pragma anon_unions
#endif

typedef struct {
	uint8_t		ste;
	uint8_t		time;
	uint8_t		retry;
	uint8_t		tmo;
	uint8_t		pfc;
	uint16_t	refresh;
	uint16_t	cnt;
	uint16_t	chlid;
	chl 		chl;
	buf			rbuf;
	buf			tmsg;
	union {
		t_uart_para uart;
		uint8_t		ip[4];
	};
	sys_res (*analyze)(void *);
	sys_res	(*linkcheck)(void *, uint_t);
}t_dlrcp, *p_dlrcp;

#ifdef __CC_ARM
#pragma no_anon_unions
#endif

//发送报文结构
typedef struct {
	uint8_t	ste;
	uint8_t	retry;
	uint8_t	tmo;
	buf		data;
}t_dlrcp_tmsg, *p_dlrcp_tmsg;








//External Functions
sys_res dlrcp_SetChl(p_dlrcp p, uint_t nType, uint_t nId, uint_t nPar1, uint_t nPar2, uint_t nPar3, uint_t nPar4);
sys_res dlrcp_TmsgSend(p_dlrcp p, void *pHeader, uint_t nHeaderLen, void *pData, uint_t nDataLen);
sys_res dlrcp_Handler(p_dlrcp p);





#ifdef __cplusplus
}
#endif

#endif

