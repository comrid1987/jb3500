

//Private Variables




//Internal Functions
static void plc_Rst(uint_t nHL)
{
	t_plc_def *p = &tbl_bspPlc;

	if (p->rst_effect == GPIO_EFFECT_HIGH)
		nHL ^= 1;
	sys_GpioSet(&p->rst, nHL);
}

#if PLC_SET_ENABLE
static void plc_Pwr(uint_t nHL)
{
	t_plc_def *p = &tbl_bspPlc;

	if (p->set_effect == GPIO_EFFECT_HIGH)
		nHL ^= 1;
	sys_GpioSet(&p->set, nHL);
}
#endif


static void plc_Reset(t_plc *p)
{

	plc_Rst(0);
	os_thd_Sleep(100);
	plc_Rst(1);
	os_thd_Sleep(10000);
	chl_rs232_Config(p->chl, 9600, UART_PARI_EVEN, UART_DATA_8D, UART_STOP_1D);
	if (gw3762_InfoGet(p) != SYS_R_OK) {
		//晓程广东
		p->type = PLC_T_XC_GD;
		memcpy(p->info, "CXDG", 4);
		return;
	}
	memcpy(p->info, p->data->p, 8);
	if (memcmp(p->info, "GB", 2) == 0) {
		//成都博高
		p->type = PLC_T_BOST;
		return;
	}
	if (memcmp(p->info, "SETR", 4) == 0) {
		//东软路由
		p->type = PLC_T_ES_RT;
		return;
	}
	if (memcmp(p->info, "SE", 2) == 0) {
		//东软无路由
		p->type = PLC_T_ES_38;
		return;
	}
	if (memcmp(p->info, "CT", 2) == 0) {
		//鼎信
		p->type = PLC_T_TOPCOM;
		return;
	}
	if (memcmp(p->info, "LM", 2) == 0) {
		//力合微
		p->type = PLC_T_LEAGUERME;
		return;
	}
	if (memcmp(p->info, "10", 2) == 0) {
		//瑞斯康
		p->type = PLC_T_RISECOM;
		return;
	}
	if (memcmp(p->info, "im", 2) == 0) {
		//弥亚微
		p->type = PLC_T_MIARTECH;
		return;
	}
	//默认晓程
	p->type = PLC_T_XC_GW;
}

static sys_res plc_Sync(t_plc *p)
{
	uint_t i, j, nSn, nPrtl, nFrom0, nValid = 0;
	uint16_t nQty;
	uint8_t aBuf[6], aMadr[6], aMadr2[6], aMeter[(LCP_SN_MAX + 7) / 8];

	if (p->type != PLC_T_XC_GD)
		gw3762_ModAdrSet(p);

	if (plc_IsNotSync(p))
		return SYS_R_OK;

	if (p->type == PLC_T_ES_RT)
		gw3762_Es_ModeSet(p, p->mode);

	gw3762_RtCtrl(p, 0x0002);

	for (nSn = 1; nSn < LCP_SN_MAX; nSn++) {
		if (plc_MeterRead(nSn, aMadr) == 0)
			continue;
		if (isnotbcd(aMadr, 6))
			continue;
		for (j = 1; j < nSn; j++) {
			if (plc_MeterRead(j, aMadr2)) {
				if (memcmp(aMadr2, aMadr, 6) == 0)
					break;
			}
		}
		if (j < nSn)
			continue;
		nValid += 1;
	}
	if (nValid == 0)
		return gw3762_ParaReset(p);

	if (p->type == PLC_T_ES_RT)
		nFrom0 = 1;
	else
		nFrom0 = 0;

	for (i = 0; i < 2; i++) {
		if (p->type != PLC_T_XC_RT) {
			if (gw3762_SubAdrQty(p, &nQty) != SYS_R_OK)
				return SYS_R_TMO;
		} else {
			nQty = LCP_SN_MAX;
		}
		memset(aMeter, 0, sizeof(aMeter));
		for (nSn = 1; nSn <= nQty; nSn++) {
			if (gw3762_SubAdrRead(p, nSn - nFrom0, NULL, aBuf) == SYS_R_OK) {
				for (j = 1; j < LCP_SN_MAX; j++) {
					if (plc_MeterRead(j, aMadr) == 0)
						continue;
					if (isnotbcd(aMadr, 6))
						continue;
					if (memcmp(aBuf, aMadr, 6) == 0) {
						setbit(aMeter, j - 1, 1);
						break;
					}
				}
				if (j >= LCP_SN_MAX)
					gw3762_SubAdrDelete(p, aBuf);
			}
		}
		for (nSn = 1; nSn < LCP_SN_MAX; nSn++) {
			if (getbit(aMeter, nSn - 1))
				continue;
			nPrtl = plc_MeterRead(nSn, aMadr);
			if (nPrtl == 0)
				continue;
			if (isnotbcd(aMadr, 6))
				continue;
			for (j = 1; j < nSn; j++) {
				if (plc_MeterRead(j, aMadr2)) {
					if (memcmp(aMadr2, aMadr, 6) == 0)
						break;
				}
			}
			if (j < nSn)
				continue;
			gw3762_SubAdrAdd(p, nSn - nFrom0, aMadr, nPrtl);
		}
		if (gw3762_SubAdrQty(p, &nQty) != SYS_R_OK)
			return SYS_R_TMO;
		if (nQty == nValid)
			break;
		if (i == 0) {
			if (gw3762_ParaReset(p) != SYS_R_OK)
				return SYS_R_TMO;
		}
	}
	if (i < 2)
		return SYS_R_OK;
	return SYS_R_ERR;
}

static sys_res plc_Recv(t_plc *p, buf b, const uint8_t *pAdr, uint_t nTmo)
{
	uint8_t *pTemp;

	for (nTmo *= (1000 / OS_TICK_MS); nTmo; nTmo--) {
		if (gw3762_Analyze(p) != SYS_R_OK)
			continue;
		switch (p->afn) {
		case GW3762_AFN_CONFIRM:
			//否认
			if (p->fn == 0x0002)
				return SYS_R_ERR;
			break;
		case GW3762_AFN_TRANSMIT:
		case GW3762_AFN_ROUTE_TRANSMIT:
			if (p->fn == 0x0001) {
				buf_Remove(p->data, 2);
				//645包解析
				pTemp = dlt645_PacketAnalyze(p->data->p, p->data->len);
				if (pTemp != NULL) {
					//校验表地址
					if (p->rup.module) {
						if (memcmp(p->madr, &pTemp[1], 6))
							break;;
					}
					if (memcmp(&pTemp[1], pAdr, 6) == 0) {
						buf_Push(b, &pTemp[DLT645_HEADER_SIZE - 2], pTemp[DLT645_HEADER_SIZE - 1] + 2);
						byteadd(&b->p[2], -0x33, b->p[1]);
						return SYS_R_OK;
					}
				}
			}
			break;
		default:
			break;
		}
	}
	return SYS_R_TMO;
}

static sys_res plc_Slave(t_plc *p, buf b, uint8_t *pAdr)
{
	uint8_t *pTemp;
	uint_t nIs97, nCode, nLen;
	uint32_t nDI;

	if (gw3762_Analyze(p) != SYS_R_OK)
		return SYS_R_TMO;
	switch (p->afn) {
	case GW3762_AFN_ROUTE_REQUEST:
		//路由请求
		if (p->fn == 0x0001) {
			nDI = plc_Request(&p->data->p[1], &nIs97);
			if (nDI) {
				if (nIs97) {
					nCode = DLT645_CODE_READ97;
					nLen = 2;
				} else {
					nCode = DLT645_CODE_READ07;
					nLen = 4;
				}
				dlt645_Packet2Buf(b, &p->data->p[1], nCode, &nDI, nLen);
				gw3762_RequestAnswer(p, p->data->p[0], &p->data->p[1], 1, b->p, b->len);
				buf_Release(b);
			} else {
				nDI = 0x00000001;
				gw3762_RequestAnswer(p, p->data->p[0], &p->data->p[1], 0, &nDI, 3);
			}
			return SYS_R_BUSY;
		}
		break;
 	case GW3762_AFN_REPORT:
		//自动上报数据
		if (p->fn == 0x0002) {
			gw3762_Confirm(p, 0xFFFF, 0);
			buf_Remove(p->data, 4);
			//645包解析
			pTemp = dlt645_PacketAnalyze(p->data->p, p->data->len);
			if (pTemp != NULL) {
				//校验表地址
				if (p->rup.module) {
					if (memcmp(p->madr, &pTemp[1], 6))
						break;
				}
				memcpy(pAdr, &pTemp[1], 6);
				buf_Push(b, &pTemp[DLT645_HEADER_SIZE - 2], pTemp[DLT645_HEADER_SIZE - 1] + 2);
				byteadd(&b->p[2], -0x33, b->p[1]);
				return SYS_R_OK;
			}
		}
		break;
	default:
		break;
	}
	return SYS_R_TMO;
}

static sys_res plc_MeterReport(t_plc *p, uint8_t *pAdr)
{

	if (gw3762_Analyze(p) != SYS_R_OK)
		return SYS_R_TMO;
	//自动上报表号
	if (p->afn == GW3762_AFN_REPORT) {
		if (p->fn == 0x0001) {
			gw3762_Confirm(p, 0xFFFF, 0);
			memcpy(pAdr, &p->data->p[1], 6);
			return SYS_R_OK;
		}
	}
	return SYS_R_TMO;
}








//External Functions
int plc_IsNeedRt(t_plc *p)
{

	switch (p->type) {
	case PLC_T_XC_GW:
	case PLC_T_XC_GD:
	case PLC_T_ES_38:
		return 1;
	default:
		return 0;
	}
}

int plc_IsNotSync(t_plc *p)
{

	switch (p->type) {
	case PLC_T_XC_GW:
	case PLC_T_XC_GD:
	case PLC_T_ES_38:
	case PLC_T_RISECOM:
		return 1;
	default:
		return 0;
	}
}

int plc_GetRetry(t_plc *p)
{

	switch (p->type) {
	case PLC_T_XC_RT:
	case PLC_T_ES_38:
	case PLC_T_BOST:
		return 2;
	default:
		return 1;
	}
}

int plc_GetWait(t_plc *p, uint_t nRelay)
{

	switch (p->type) {
 	case PLC_T_ES_38:
		return 12 + nRelay * 6;
	case PLC_T_XC_GW:
		return 10 + nRelay * 4;
	case PLC_T_XC_GD:
		return 15 + nRelay * 10;
	case PLC_T_XC_RT:
		return 30;
	case PLC_T_TOPCOM:
		return 90;
	default:
		return 20;
	}
}



void plc_Init(t_plc *p)
{
	t_plc_def *pDef = &tbl_bspPlc;

	sys_GpioConf(&pDef->rst);
#if PLC_SET_ENABLE
	sys_GpioConf(&pDef->set);
#endif
	memset(p, 0, sizeof(t_plc));
	chl_Bind(p->chl, CHL_T_RS232, pDef->uartid, OS_TICK_MS);
	plc_Reset(p);
	p->ste = PLC_S_SYNC;
}


sys_res plc_RealRead(t_plc *p, buf b, const uint8_t *pAdr, uint_t nCode, const void *pData, uint_t nLen, uint_t nRelay, const uint8_t *pRtAdr)
{
	sys_res res = SYS_R_ERR;

#if XCN6N12_ENABLE
	if (p->type == PLC_T_XC_GD) {
		chl_rs232_Config(p->chl, 2400, UART_PARI_NO, UART_DATA_8D, UART_STOP_1D);
		switch (nCode) {
		case DLT645_CODE_READ97:
			res = xcn6_MeterRead(p, b, pAdr, nRelay, pRtAdr, pData, nLen);
			break;
		case DLT645_CODE_WRITE97:
			res = xcn6_MeterWrite(p, b, pAdr, nRelay, pRtAdr, pData, nLen);
			break;
		case DLT645_CODE_READ07:
		case DLT645_CODE_CTRL07:
			res = xcn12_Meter(p, b, nCode, pAdr, nRelay, pRtAdr, pData, nLen);
			break;
		default:
			res = SYS_R_ERR;
			break;
		}
	} else {
#endif
		if (p->ste == PLC_S_SLAVE) {
			if (gw3762_RtCtrl(p, 0x0002) != SYS_R_OK)
				return res;
			os_thd_Sleep(1000);
			p->ste = PLC_S_WAIT;
			p->tmo = 20;
		}
		chl_rs232_Config(p->chl, 9600, UART_PARI_EVEN, UART_DATA_8D, UART_STOP_1D);
		dlt645_Packet2Buf(b, pAdr, nCode, pData, nLen);
		if (plc_IsNeedRt(p))
			gw3762_MeterRead(p, pAdr, nRelay, pRtAdr, b->p, b->len);
		else
			gw3762_MeterRT(p, pAdr, b->p, b->len);
		buf_Release(b);
		res = plc_Recv(p, b, pAdr, plc_GetWait(p, nRelay));
#if XCN6N12_ENABLE
	}
#endif
	return res;
}

void plc_Broadcast(t_plc *p)
{
	buf b = {0};
	uint8_t aBuf[6], aTime[6];
	uint_t nTemp;
	
#if XCN6N12_ENABLE
	if (p->type == PLC_T_XC_GD) {
		chl_rs232_Config(p->chl, 2400, UART_PARI_NO, UART_DATA_8D, UART_STOP_1D);
		timet2array(rtc_GetTimet(), aTime, 1);
		xcn6_Broadcast(p, aBuf, aTime, 6);
		os_thd_Sleep(3000);
		timet2array(rtc_GetTimet(), aTime, 1);
		xcn12_Broadcast(p, aBuf, aTime, 6);
		os_thd_Sleep(3000);
	} else {
#endif
		chl_rs232_Config(p->chl, 9600, UART_PARI_EVEN, UART_DATA_8D, UART_STOP_1D);
		timet2array(rtc_GetTimet(), aTime, 1);
		dlt645_Packet2Buf(b, aBuf, DLT645_CODE_BROADCAST, aTime, 6);
		gw3762_Broadcast(p, aBuf, b->p, b->len);
		buf_Release(b);
		os_thd_Sleep(5000);
		aBuf[5] = 0x98;
		timet2array(rtc_GetTimet(), aTime, 1);
		dlt645_Packet2Buf(b, aBuf, DLT645_CODE_BROADCAST, aTime, 6);
		if (plc_IsNeedRt(p))
			gw3762_MeterRead(p, aBuf, 0, NULL, b->p, b->len);
		else
			gw3762_MeterRT(p, aBuf, b->p, b->len);
		buf_Release(b);
		os_thd_Sleep(5000);

		if (p->type == PLC_T_ES_RT) {
			if (gw3762_Es_ModeGet(p, &nTemp) == SYS_R_OK) {
				if (nTemp != 4) {
					gw3762_Es_ModeSet(p, 4);
					os_thd_Sleep(6000);

					aBuf[5] = 0x99;
					timet2array(rtc_GetTimet(), aTime, 1);
					dlt645_Packet2Buf(b, aBuf, DLT645_CODE_BROADCAST, aTime, 6);
					gw3762_Broadcast(p, aBuf, b->p, b->len);
					buf_Release(b);
					os_thd_Sleep(5000);
					aBuf[5] = 0x98;
					timet2array(rtc_GetTimet(), aTime, 1);
					dlt645_Packet2Buf(b, aBuf, DLT645_CODE_BROADCAST, aTime, 6);
					if (plc_IsNeedRt(p))
						gw3762_MeterRead(p, aBuf, 0, NULL, b->p, b->len);
					else
						gw3762_MeterRT(p, aBuf, b->p, b->len);
					buf_Release(b);

					os_thd_Sleep(5000);
					plc_Sync(p);
				}
			}
		}
#if XCN6N12_ENABLE
	}
#endif
}


sys_res plc_Handler(t_plc *p, buf b, uint8_t *pAdr)
{
	sys_res res;

	if (p->time != (uint8_t)rtc_GetTimet()) {
		p->time = rtc_GetTimet();
		if (p->tmo)
			p->tmo -= 1;
	}
	switch (p->ste) {
	case PLC_S_INIT:
		plc_Reset(p);
		p->ste = PLC_S_SYNC;
		break;
	case PLC_S_SYNC:
		if (plc_Sync(p) != SYS_R_OK)
			dbg_trace("<PLC> Sync failed...");
		p->ste = PLC_S_IDLE;
		p->tmo = 3;
#if PLC_PROBE_ENABLE
		if (p->type != PLC_T_XC_GD) {
			//允许主动注册
			if (gw3762_ModeSet(p, 0x02) == SYS_R_OK) {
				if (gw3762_MeterProbe(p, 4) == SYS_R_OK) {
					p->ste = PLC_S_PROBE;
					p->tmo = 4 * 60;
				}
			}
		}					
#endif
		break;
#if PLC_PROBE_ENABLE
	case PLC_S_PROBE:
		if (plc_MeterReport(p, pAdr) == SYS_R_OK)
			plc_NewMeter(pAdr);
		if (p->tmo == 0) {
			p->ste = PLC_S_IDLE;
			p->tmo = 3;
		}
		break;
#endif
	case PLC_S_IDLE:
		if (plc_IsInTime() == 0)
			break;
		switch (p->type) {
		case PLC_T_TOPCOM:
		case PLC_T_ES_RT:
			if (p->tmo == 0) {
				p->tmo = 0xFF;
				if (gw3762_RtCtrl(p, 0x0001) == SYS_R_OK)
					p->ste = PLC_S_SLAVE;
			}
			break;
		default:
			break;
		}
		break;
	case PLC_S_WAIT:
		if (p->tmo == 0) {
			p->tmo = 0xFF;
			if (gw3762_RtCtrl(p, 0x0004) == SYS_R_OK)
				p->ste = PLC_S_SLAVE;
		}
		break;
	case PLC_S_SLAVE:
		//被动抄表状态
		res = plc_Slave(p, b, pAdr);
		if (res != SYS_R_TMO)
			p->tmo = 0xFF;
		if (res == SYS_R_OK)
			return SYS_R_OK;
		//长时间无交互
		if (p->tmo == 0) {
			p->tmo = 0xFF;
			if (gw3762_StateGet(p) == SYS_R_OK) {
				if (memtest(&p->data->p[13], 0x08, 3) == 0)
					p->ste = PLC_S_INIT;
			} else {
				if (gw3762_StateGet(p) != SYS_R_OK)
					p->ste = PLC_S_INIT;
			}
		}
		//抄表时段完
		if (plc_IsInTime() == 0) {
			gw3762_RtCtrl(p, 0x0002);
			p->ste = PLC_S_SYNC;
		}
		break;
	default:
		p->ste = PLC_S_INIT;
		break;
	}
	return SYS_R_ERR;
}



void plc_GetInfo(t_plc *p, char *pInfo)
{
	uint_t i;

	pInfo[0] = p->info[1];
	pInfo[1] = p->info[0];
	pInfo[3] = p->info[3];
	pInfo[4] = p->info[2];
	for (i = 0; i < 4; i++) {
		sprintf(&pInfo[6 + 3 * i], "%X", p->info[4 + i]);
	}
	for (i = 0; i < 6; i++) {
		pInfo[2 + 3 * i] = ' ';
	}
}




