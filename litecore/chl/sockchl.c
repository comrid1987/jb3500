#if TCPPS_ENABLE



static int chl_soc_GetNoblock(int domain, int type, int protocol)
{
	int soc;
#if TCPPS_TYPE == TCPPS_T_LWIP
	int mode = 1;
#endif

	if ((soc = socket(domain, type, protocol)) != -1) {
#if TCPPS_TYPE == TCPPS_T_LWIP
		ioctlsocket(soc, FIONBIO, &mode);
		setsockopt(soc, SOL_SOCKET, SO_REUSEADDR, &mode, sizeof(int));
#endif
		return soc;
	}
	return -1;
}


sys_res chl_soc_Bind(chl p, uint_t nType, uint_t nId)
{
	int soc;
	struct sockaddr_in addr_in = {0};

	switch (nType) {
	case CHL_T_SOC_TC_RECON:
	case CHL_T_SOC_TC:
		if ((soc = chl_soc_GetNoblock(AF_INET, SOCK_STREAM, 0)) != -1) {
			p->pIf = (void *)soc;
			return SYS_R_OK;
		}
		break;
	case CHL_T_SOC_TS:
#if MODEM_ZTE_TCP
		zte_ListenPort(nId);
#endif
		if ((soc = chl_soc_GetNoblock(AF_INET, SOCK_STREAM, 0)) != -1) {
			addr_in.sin_family = AF_INET;
			addr_in.sin_addr.s_addr = INADDR_ANY;
			addr_in.sin_port = htons(nId);
			if (bind(soc, (struct sockaddr *)&addr_in, sizeof(struct sockaddr_in)) == 0) {
				p->pIf = (void *)soc;
				return SYS_R_OK;
			}
			closesocket(soc);
		}
		break;
	case CHL_T_SOC_UC:
		if ((soc = chl_soc_GetNoblock(AF_INET, SOCK_DGRAM, 0)) != -1) {
			p->pIf = (void *)soc;
			return SYS_R_OK;
		}
		break;
	case CHL_T_SOC_US:
		if ((soc = chl_soc_GetNoblock(AF_INET, SOCK_DGRAM, 0)) != -1) {
			addr_in.sin_family = AF_INET;
			addr_in.sin_addr.s_addr = INADDR_ANY;
			addr_in.sin_port = htons(nId);
			if (bind(soc, (struct sockaddr *)&addr_in, sizeof(struct sockaddr_in)) == 0) {
				p->pIf = (void *)soc;
				return SYS_R_OK;
			}
			closesocket(soc);
		}
		break;
	}
	return SYS_R_ERR;
}

sys_res chl_soc_Connect(chl p, const void *pIp, uint_t nPort)
{
	struct sockaddr_in adr;

	switch (p->type) {
	case CHL_T_SOC_TC_RECON:
	case CHL_T_SOC_TC:
#if MODEM_ZTE_TCP
		if (modem_IsZteTcp()) {
			if(modem_IsMe3000()){
				if (me3000_TcpConnect(pIp, nPort) != SYS_R_OK)
					return SYS_R_TMO;
			}
			else{
				if (zte_TcpConnect(pIp, nPort) != SYS_R_OK)
					return SYS_R_TMO;
			}
			p->ste = CHL_S_CONNECT;
			break;
		}
#endif
	case CHL_T_SOC_UC:
		adr.sin_family = AF_INET;
		memcpy(&adr.sin_addr.s_addr, pIp, 4);
		adr.sin_port = htons(nPort);
		connect((int)p->pIf, (struct sockaddr *)&adr, sizeof(struct sockaddr_in));
		p->ste = CHL_S_CONNECT;
		break;
	default:
		return SYS_R_ERR;
	}
	return SYS_R_OK;
}

sys_res chl_soc_Listen(chl p)
{

#if MODEM_ZTE_TCP
	if (modem_IsZteTcp()) {
		if (modem_IsOnline() == 0)
			return SYS_R_ERR;
		if(modem_IsMe3000())
			return SYS_R_ERR;
		if (zte_TcpListen() != SYS_R_OK)
			return SYS_R_ERR;
		p->ste = CHL_S_CONNECT;
		return SYS_R_OK;
	}
#endif
	if (listen((int)p->pIf, 0) != 0)
		return SYS_R_ERR;
	p->ste = CHL_S_CONNECT;
	return SYS_R_OK;
}

int chl_soc_IsConnect(chl p)
{
#if TCPPS_TYPE == TCPPS_T_LWIP
	int newsoc;
	socklen_t len;
	struct sockaddr_in adr;
#endif

#if MODEM_ZTE_TCP
	if (modem_IsZteTcp()) {
		if (modem_IsOnline() == 0)
			return 0;
		if (p->type == CHL_T_SOC_TC) {
			if (zte_IsTcpCon() == 0)
				return 0;
		}
		if (p->ste == CHL_S_CONNECT)
			p->ste = CHL_S_READY;
		if (p->ste == CHL_S_READY)
			return 1;
		return 0;
	}
#endif
#if TCPPS_TYPE == TCPPS_T_LWIP
	switch (p->type) {
	case CHL_T_SOC_TS:
		if (p->ste == CHL_S_READY) {
			len = sizeof(adr);
			p->err = getpeername((int)p->pIf, (struct sockaddr *)&adr, &len);
			if (p->err != 0)
				return 0;
			if (adr.sin_port == 0)
				return 0;
		} else {
			newsoc = accept((int)p->pIf, NULL, NULL);
			if (newsoc == -1)
				return 0;
			closesocket((int)p->pIf);
			p->pIf = (void *)newsoc;
		}
		break;
	default:
		len = sizeof(adr);
		p->err = getpeername((int)p->pIf, (struct sockaddr *)&adr, &len);
		if (p->err != 0)
			return 0;
		if (adr.sin_port == 0)
			return 0;
		break;
	}
#endif
#if TCPPS_TYPE == TCPPS_T_KEILTCP
	p->err = recv((int)p->pIf, NULL, 0, MSG_DONTWAIT);
	if (p->err != 0)
		return 0;
#endif
	p->ste = CHL_S_READY;
	return 1;
}



#endif


