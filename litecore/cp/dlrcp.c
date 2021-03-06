#if DLRCP_ENABLE
#if DLRCP_ZIP_ENABLE
#include <lib/zip/cceman.h>
#endif

//Private Defines
#if DLRCP_DEBUG_ENABLE
#define dlrcp_DbgOut				dbg_printf
#else
#define dlrcp_DbgOut(...)
#endif

//发送报文类型
#define DLRCP_LINKCHECK_LOGIN		0
#define DLRCP_LINKCHECK_LOGOUT		1
#define DLRCP_LINKCHECK_KEEPALIVE	2

//TCP连接超时
#define DLRCP_TCP_CONNECT_TMO		16


#if DLRCP_SEND_BUFFER
//-------------------------------------------------------------------------
//申请新的发送报文缓冲
//-------------------------------------------------------------------------
static p_dlrcp_tmsg dlrcp_TmsgNew(p_dlrcp p)
{
	t_dlrcp_tmsg xMsg;

	bzero(&xMsg, sizeof(t_dlrcp_tmsg));
	if (buf_Push(p->tmsg, &xMsg, sizeof(t_dlrcp_tmsg)) == SYS_R_OK)
		return (p_dlrcp_tmsg)(p->tmsg->p + p->tmsg->len - sizeof(t_dlrcp_tmsg));
	return NULL;
}

//-------------------------------------------------------------------------
//释放缓冲报文
//-------------------------------------------------------------------------
static void dlrcp_TmsgRelease(p_dlrcp p, p_dlrcp_tmsg pMsg)
{

	buf_Release(pMsg->data);
	buf_Cut(p->tmsg, (uint8_t *)pMsg - p->tmsg->p, sizeof(t_dlrcp_tmsg));
}
#endif







//-------------------------------------------------------------------------
//通道设置
//-------------------------------------------------------------------------
sys_res dlrcp_SetChl(p_dlrcp p, uint_t nType, uint_t nId, uint_t nPar1, uint_t nPar2, uint_t nPar3, uint_t nPar4)
{
	uint_t nChanged = 0;

	if (p->chl->type != nType) {
		p->chl->type = nType;
		nChanged = 1;
	}
	if (p->chlid != nId) {
		p->chlid = nId;
		nChanged = 1;
	}
	switch (nType) {
#if TCPPS_ENABLE
	case CHL_T_SOC_TC_RECON:
	case CHL_T_SOC_TC:
	case CHL_T_SOC_TS:
	case CHL_T_SOC_UC:
	case CHL_T_SOC_US:
		if (p->ip[0] != nPar1) {
			p->ip[0] = nPar1;
			nChanged = 1;
		}
		if (p->ip[1] != nPar2) {
			p->ip[1] = nPar2;
			nChanged = 1;
		}
		if (p->ip[2] != nPar3) {
			p->ip[2] = nPar3;
			nChanged = 1;
		}
		if (p->ip[3] != nPar4) {
			p->ip[3] = nPar4;
			nChanged = 1;
		}
		break;
#endif
#if UART_ENABLE
	case CHL_T_RS232:
	case CHL_T_IRDA:
		if (p->uart.baud != nPar1) {
			p->uart.baud = nPar1;
			nChanged = 1;
		}
		if (p->uart.pari != nPar2) {
			p->uart.pari = nPar2;
			nChanged = 1;
		}
		if (p->uart.data != nPar3) {
			p->uart.data = nPar3;
			nChanged = 1;
		}
		if (p->uart.stop != nPar4) {
			p->uart.stop = nPar4;
			nChanged = 1;
		}
		break;
#endif
#if USBD_CDC_ENABLE
	case CHL_T_USB2UART:
		break;
#endif
	default:
		return SYS_R_ERR;
	}
	if (nChanged)
		chl_Release(p->chl);
	return SYS_R_OK;
}


//-------------------------------------------------------------------------
//发送报文
//-------------------------------------------------------------------------
#if DLRCP_SEND_BUFFER
sys_res dlrcp_TmsgSend(p_dlrcp p, void *pHeader, uint_t nHeaderLen, void *pData, uint_t nDataLen, uint_t nType)
{
	sys_res res = SYS_R_ERR;
	p_dlrcp_tmsg pMsg;
	uint_t nTmo;

	pMsg = dlrcp_TmsgNew(p);
	if (pMsg != NULL) {
		buf_Push(pMsg->data, pHeader, nHeaderLen);
		buf_Push(pMsg->data, pData, nDataLen);
#if DLRCP_ZIP_ENABLE
		if (p->zip) {
			int nLen;
			nLen = EnData(pMsg->data->p, pMsg->data->len, EXE_COMPRESS_NEW);
			buf_Release(pMsg->data);
			if (nLen > 0)
				buf_Push(pMsg->data, SendBuf, nLen);
		}
#endif
		pMsg->tmo = 0;
		//Unfinished
		//if (DLRCP_TMSG_NEED_ACK) {
		//	pMsg->retry = p->retry;
		//	return SYS_R_OK;
		//} else {
#if TCPPS_ETH_ENABLE
			if ((nType == DLRCP_TMSG_REPORT) && (p->chl->type == CHL_T_SOC_TC_RECON)) {
				chl_Release(p->chl);
				chl_Bind(p->chl, p->chl->type, p->chlid, OS_TICK_MS);
				chl_soc_Connect(p->chl, p->ip, p->chlid);
				for (nTmo = 5000 / OS_TICK_MS ; nTmo; nTmo--) {
					if (chl_soc_IsConnect(p->chl))
						break;
					os_thd_Slp1Tick();
				}
			}
#endif
			res = chl_Send(p->chl, pMsg->data->p, pMsg->data->len);
		//}
		dlrcp_TmsgRelease(p, pMsg);
	}
	return res;
}
#else
sys_res dlrcp_TmsgSend(p_dlrcp p, void *pHeader, uint_t nHeaderLen, void *pData, uint_t nDataLen, uint_t nType)
{
	sys_res res = SYS_R_ERR;
	uint_t nTmo;
	buf b = {0};

	buf_Push(b, pHeader, nHeaderLen);
	buf_Push(b, pData, nDataLen);
#if DLRCP_ZIP_ENABLE
	if (p->zip) {
		int nLen;
		nLen = EnData(b->p, b->len, EXE_COMPRESS_NEW);
		buf_Release(b);
		if (nLen > 0)
			buf_Push(b, SendBuf, nLen);
	}
#endif
#if TCPPS_ETH_ENABLE
	if ((nType == DLRCP_TMSG_REPORT) && (p->chl->type == CHL_T_SOC_TC_RECON)) {
		chl_Release(p->chl);
		chl_Bind(p->chl, p->chl->type, p->chlid, OS_TICK_MS);
		chl_soc_Connect(p->chl, p->ip, p->chlid);
		for (nTmo = 5000 / OS_TICK_MS ; nTmo; nTmo--) {
			if (chl_soc_IsConnect(p->chl))
				break;
			os_thd_Slp1Tick();
		}
	}
#endif
	res = chl_Send(p->chl, b->p, b->len);
	buf_Release(b);
	return res;
}
#endif

//-------------------------------------------------------------------------
//通讯处理
//-------------------------------------------------------------------------
sys_res dlrcp_Handler(p_dlrcp p)
{
	sys_res res = SYS_R_ERR;
#if DLRCP_SEND_BUFFER
	p_dlrcp_tmsg pMsg;
#endif
	
	switch (p->chl->ste) {
	default:
		p->ste = DLRCP_S_IDLE;
		if (chl_Bind(p->chl, p->chl->type, p->chlid, OS_TICK_MS) != SYS_R_OK)
			os_thd_Slp1Tick();
		break;
	case CHL_S_STANDBY:
		switch (p->chl->type) {
#if TCPPS_ENABLE
		case CHL_T_SOC_TC_RECON:
		case CHL_T_SOC_TC:
		case CHL_T_SOC_UC:
			if (p->cnt == 0) {
				if (chl_soc_Connect(p->chl, p->ip, p->chlid) != SYS_R_OK)
					p->cnt = 1;
				dlrcp_DbgOut("[RCP] connect to %d.%d.%d.%d:%d", p->ip[0], p->ip[1], p->ip[2], p->ip[3], p->chlid);
			} else {
				if (p->time == (uint8_t)rtc_GetTimet()) {
					os_thd_Slp1Tick();
					break;
				}
				p->time = rtc_GetTimet();
				p->cnt += 1;
				if (p->cnt > (p->refresh >> 3))
					p->cnt = 0;
			}
			break;
		case CHL_T_SOC_TS:
		case CHL_T_SOC_US:
  			chl_soc_Listen(p->chl);
			break;
#endif
#if UART_ENABLE
		case CHL_T_RS232:
		case CHL_T_IRDA:
			chl_rs232_Config(p->chl, p->uart.baud, p->uart.pari, p->uart.data, p->uart.stop);
			break;
#endif
#if USBD_CDC_ENABLE
		case CHL_T_USB2UART:
			p->chl->ste = CHL_S_READY;
			break;
#endif
		default:
			break;
		}
		break;
	case CHL_S_CONNECT:
		if (p->time == (uint8_t)rtc_GetTimet()) {
			os_thd_Slp1Tick();
			break;
		}
		p->time = rtc_GetTimet();
		p->cnt += 1;
		switch (p->chl->type) {
#if TCPPS_ENABLE
		case CHL_T_SOC_TC_RECON:
		case CHL_T_SOC_TC:
		case CHL_T_SOC_UC:
			if (chl_soc_IsConnect(p->chl)) {
				p->cnt = 0;
				p->ste = DLRCP_S_CHECK;
#if TCPPS_TYPE == TCPPS_T_LWIP
				os_thd_Sleep(2000);
#endif
				(p->linkcheck)(p, DLRCP_LINKCHECK_LOGIN);
				dlrcp_DbgOut("[RCP] login %d.%d.%d.%d:%d", p->ip[0], p->ip[1], p->ip[2], p->ip[3], p->chlid);
			} else {
				if (p->cnt > DLRCP_TCP_CONNECT_TMO)
					chl_Release(p->chl);
			}
			break;
		case CHL_T_SOC_TS:
		case CHL_T_SOC_US:
			if (p->cnt > 900) {
				p->cnt = 0;
				chl_Release(p->chl);
			}
			if (chl_soc_IsConnect(p->chl))
				p->cnt = 0;
			break;
#endif
		default:
			break;
		}
		break;
	case CHL_S_READY:
		//接收处理
		if (p->time != (uint8_t)rtc_GetTimet()) {
			p->time = rtc_GetTimet();
			p->cnt += 1;
			//链路检测
			switch (p->chl->type) {
#if TCPPS_ENABLE
			case CHL_T_SOC_TC_RECON:
			case CHL_T_SOC_TC:
			case CHL_T_SOC_UC:
				if (chl_soc_IsConnect(p->chl)) {
					if (p->cnt > p->refresh) {
						switch (p->ste) {
						case DLRCP_S_CHECK:
							chl_Release(p->chl);
							break;
						case DLRCP_S_READY:
							p->ste = DLRCP_S_CHECK;
							(p->linkcheck)(p, DLRCP_LINKCHECK_KEEPALIVE);
							dlrcp_DbgOut("[RCP] keep-alive %d.%d.%d.%d:%d", p->ip[0], p->ip[1], p->ip[2], p->ip[3], p->chlid);
							break;
						default:
							break;
						}
						p->cnt = 0;
					}
				} else {
					dlrcp_DbgOut("[RCP] unknow ste %d", p->chl->err);
					chl_Release(p->chl);
				}
				break;
			case CHL_T_SOC_TS:
			case CHL_T_SOC_US:
				if ((p->cnt > p->refresh) || (chl_soc_IsConnect(p->chl) == 0))
					chl_Release(p->chl);
				break;
#endif
#if UART_ENABLE
			case CHL_T_RS232:
			case CHL_T_IRDA:
				if (p->cnt > 900) {
					p->cnt = 0;
					chl_Release(p->chl);
				}
				break;
#endif
			default:
				break;
			}
#if DLRCP_SEND_BUFFER
			//发送处理
			pMsg = (p_dlrcp_tmsg)p->tmsg->p;
			for (; (uint8_t *)pMsg < (p->tmsg->p + p->tmsg->len); pMsg++) {
				pMsg->tmo -= 1;
				if (pMsg->tmo == 0) {
					if (pMsg->retry) {
						chl_Send(p->chl, pMsg->data->p, pMsg->data->len);
						pMsg->retry -= 1;
						pMsg->tmo = p->tmo;
					}
					//超时,释放
					if (pMsg->retry == 0) {
						dlrcp_TmsgRelease(p, pMsg);
						pMsg -= 1;
					}
				}
			}
#endif
		}
		res = (p->analyze)(p);
		if (res == SYS_R_OK) {
			dlrcp_DbgOut("[RCP] recv from %d.%d.%d.%d:%d", p->ip[0], p->ip[1], p->ip[2], p->ip[3], p->chlid);
			p->cnt = 0;
			switch (p->chl->type) {
#if TCPPS_ENABLE
			case CHL_T_SOC_TC_RECON:
			case CHL_T_SOC_TC:
			case CHL_T_SOC_UC:
				p->ste = DLRCP_S_READY;
#if MODEM_ENABLE
				modem_Refresh();
#endif
				break;
#endif
			default:
				break;
			}
		}
		break;
	}
	return res;
}

#endif



