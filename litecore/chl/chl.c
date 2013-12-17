
//Private Defines
#define CHL_PRECODE_ENABLE				1


//Private const variables
#if CHL_PRECODE_ENABLE
static uint8_t const tbl_chlHeaderCodes[] = {0xFE, 0xFE};
#endif


//Internal Functions



//External Functions
void chl_Init(chl p)
{

	memset(p, 0, sizeof(chl));
	(int)p->pIf = (-1);
}

sys_res chl_Bind(chl p, uint_t nType, uint_t nId, int nTmo)
{
	sys_res res = SYS_R_ERR;

	if (p->ste != CHL_S_IDLE)
		return SYS_R_ERR;
	switch (nType) {
#if UART_ENABLE
	case CHL_T_RS232:
	case CHL_T_IRDA:
		p->pIf = uart_Get(nId, nTmo);
		if (p->pIf != NULL)
			res = SYS_R_OK;
		break;
#endif
#if TCPPS_ENABLE
	case CHL_T_SOC_TC_RECON:
	case CHL_T_SOC_TC:
	case CHL_T_SOC_TS:
	case CHL_T_SOC_UC:
	case CHL_T_SOC_US:
		res = chl_soc_Bind(p, nType, nId);
		break;
#endif
	default:
		return SYS_R_ERR;
	}
	if (res == SYS_R_OK) {
		p->type = nType;
		p->ste = CHL_S_STANDBY;
	}
	return res;
}

sys_res chl_Release(chl p)
{
	sys_res res = SYS_R_ERR;

	switch (p->type) {
#if UART_ENABLE
	case CHL_T_RS232:
	case CHL_T_IRDA:
		if (((int)p->pIf != -1) && (p->pIf != NULL))
			res = uart_Release(p->pIf);
		break;
#endif
#if TCPPS_ENABLE
	case CHL_T_SOC_TC_RECON:
	case CHL_T_SOC_TC:
#if MODEM_ZTE_TCP
		if (modem_IsZteTcp()) {
			res = zte_TcpClose();
			break;
		}
#endif
	case CHL_T_SOC_TS:
#if MODEM_ZTE_TCP
		if (modem_IsZteTcp()) {
			res = zte_TcpSerClose();
			break;
		}
#endif
	case CHL_T_SOC_UC:
	case CHL_T_SOC_US:
		if ((int)p->pIf != -1) {
			if (closesocket((int)p->pIf) == 0)
				res = SYS_R_OK;
		}
		break;
#endif
	default:
		break;
	}
	if (res != SYS_R_OK)
		return res;
	p->ste = CHL_S_IDLE;
	return res;
}

sys_res chl_Send(chl p, const void *pData, uint_t nLen)
{
	sys_res res = SYS_R_ERR;

	if (p->ste != CHL_S_READY)
		return SYS_R_ERR;
	switch (p->type) {
#if UART_ENABLE
	case CHL_T_IRDA:
#if CHL_PRECODE_ENABLE
		uart_Send(p->pIf, (void *)tbl_chlHeaderCodes, sizeof(tbl_chlHeaderCodes));
#endif
	case CHL_T_RS232:
		res = uart_Send(p->pIf, pData, nLen);
		break;
#endif
#if TCPPS_ENABLE
	case CHL_T_SOC_TC_RECON:
	case CHL_T_SOC_TC:
	case CHL_T_SOC_TS:
#if MODEM_ZTE_TCP
		if (modem_IsZteTcp()) {
			res = zte_TcpSend(p->type, pData, nLen);
			break;
		}
#endif
	case CHL_T_SOC_UC:
	case CHL_T_SOC_US:
		if (send((int)p->pIf, pData, nLen, MSG_DONTWAIT) == nLen)
			res = SYS_R_OK;
		break;
#endif
	default:
		break;
	}
	return res;
}

sys_res chl_RecData(chl p, buf b, uint_t nTmo)
{
	sys_res res = SYS_R_ERR;
#if TCPPS_ENABLE
	char aBuf[64];
	int nLen;
#endif

	if (p->ste != CHL_S_READY)
		return SYS_R_ERR;
	switch (p->type) {
#if UART_ENABLE
	case CHL_T_RS232:
	case CHL_T_IRDA:
		res = uart_RecData(p->pIf, b, nTmo);
		break;
#endif
#if TCPPS_ENABLE
	case CHL_T_SOC_TC_RECON:
	case CHL_T_SOC_TC:
#if MODEM_ZTE_TCP
		if (modem_IsZteTcp())
			zte_TcpRecv();
#endif
	case CHL_T_SOC_TS:
#if MODEM_ZTE_TCP
		if (modem_IsZteTcp()) {
			res = zte_TcpRead(p->type, b);
			break;
		}
#endif
	case CHL_T_SOC_UC:
	case CHL_T_SOC_US:
		for (nTmo /= OS_TICK_MS; nTmo; nTmo--) {
			while ((nLen = recv((int)(p->pIf), aBuf, sizeof(aBuf), MSG_DONTWAIT)) > 0) {
				buf_Push(b, aBuf, nLen);
				res = SYS_R_OK;
			}
			if (res == SYS_R_OK)
				break;
			os_thd_Slp1Tick();
		}
		break;
#endif
	default:
		break;
	}
	return res;
}


