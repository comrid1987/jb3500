#if TCPPS_ENABLE



static int chl_soc_GetNoblock(int domain, int type, int protocol)
{
	int soc;
#if TCPPS_TYPE == TCPPS_T_LWIP
	int mode = 1;
#endif

	if ((soc = socket(domain, type, protocol)) != -1)
#if TCPPS_TYPE == TCPPS_T_LWIP
		if (ioctlsocket(soc, FIONBIO, &mode) == 0)
#endif
			return soc;
	return -1;
}


sys_res chl_soc_Bind(chl p, uint_t nType, uint_t nId)
{
	int soc;
	struct sockaddr_in addr_in = {0};

	switch (nType) {
	case CHL_T_SOC_TC:
		if ((soc = chl_soc_GetNoblock(AF_INET, SOCK_STREAM, 0)) != -1) {
			p->pIf = (void *)soc;
			return SYS_R_OK;
		}
		break;
	case CHL_T_SOC_TS:
		if ((soc = chl_soc_GetNoblock(AF_INET, SOCK_STREAM, 0)) != -1) {
			addr_in.sin_family = AF_INET;
			addr_in.sin_addr.s_addr = INADDR_ANY;
			addr_in.sin_port = htons(nId);
			if (bind(soc, (struct sockaddr *)&addr_in, sizeof(struct sockaddr_in)) == 0) {
				p->pIf = (void *)soc;
				return SYS_R_OK;
			} else
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
			} else
				closesocket(soc);
		}
		break;
	}
	return SYS_R_ERR;
}

sys_res chl_soc_Connect(chl p, uint8_t *pIp, uint_t nPort)
{
	struct sockaddr_in adr;

	if (p->ste == CHL_S_IDLE)
		return SYS_R_ERR;
	switch (p->type) {
	case CHL_T_SOC_TC:
	case CHL_T_SOC_UC:
		adr.sin_family = AF_INET;
		rt_memcpy(&adr.sin_addr.s_addr, pIp, 4);
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

	if (p->ste == CHL_S_IDLE)
		return SYS_R_ERR;
 	if (listen((int)p->pIf, 0) != 0)
		return SYS_R_ERR;
	p->ste = CHL_S_CONNECT;
	return SYS_R_OK;
}

sys_res chl_soc_IsConnect(chl p)
{
#if TCPPS_TYPE == TCPPS_T_LWIP
	socklen_t len;
	struct sockaddr_in adr;
#endif

	if (p->ste == CHL_S_IDLE)
		return SYS_R_ERR;
#if TCPPS_TYPE == TCPPS_T_LWIP
	len = sizeof(adr);
	if (getpeername((int)p->pIf, (struct sockaddr *)&adr, &len) != 0)
		return SYS_R_ERR;
	if (adr.sin_port == 0)
		return SYS_R_ERR;
#endif
#if TCPPS_TYPE == TCPPS_T_KEILTCP
	if (recv((int)p->pIf, NULL, 0, MSG_DONTWAIT) != 0)
		return SYS_R_ERR;
#endif
	p->ste = CHL_S_READY;
	return SYS_R_OK;
}



#endif


