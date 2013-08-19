

//Private Variables



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




int plc_IsNeedRt(t_plc *p)
{

	switch (p->type) {
	case GW3762_T_XC_GW:
	case GW3762_T_XC_GD:
	case GW3762_T_ES_38:
		return 1;
	default:
		return 0;
	}
}

int plc_IsNotSync(t_plc *p)
{

	switch (p->type) {
	case GW3762_T_XC_GW:
	case GW3762_T_XC_GD:
	case GW3762_T_ES_38:
	case GW3762_T_RISECOM:
		return 1;
	default:
		return 0;
	}
}

int plc_GetRetry(t_plc *p)
{

	switch (p->type) {
	case GW3762_T_XC_RT:
	case GW3762_T_ES_38:
	case GW3762_T_BOST:
		return 2;
	default:
		return 1;
	}
}

int plc_GetWait(t_plc *p, uint_t nRelay)
{

	switch (p->type) {
 	case GW3762_T_ES_38:
		return 12 + nRelay * 6;
	case GW3762_T_XC_GW:
		return 10 + nRelay * 4;
	case GW3762_T_XC_GD:
		return 15 + nRelay * 10;
	case GW3762_T_XC_RT:
		return 30;
	case GW3762_T_TOPCOM:
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
}






void plc_Reset(t_plc *p)
{

	plc_Rst(0);
	os_thd_Sleep(100);
	plc_Rst(1);
	os_thd_Sleep(10000);
	chl_rs232_Config(p->chl, 9600, UART_PARI_EVEN, UART_DATA_8D, UART_STOP_1D);
	if (gw3762_InfoGet(p, 3000) != SYS_R_OK) {
		//晓程广东
		p->type = GW3762_T_XC_GD;
		return;
	}
	if (memcmp(p->data->p, "GB", 2) == 0) {
		//成都博高
		p->type = GW3762_T_BOST;
		return;
	}
	if (memcmp(p->data->p, "SETR", 4) == 0) {
		//东软路由
		p->type = GW3762_T_ES_RT;
		return;
	}
	if (memcmp(p->data->p, "SE", 2) == 0) {
		//东软无路由
		p->type = GW3762_T_ES_38;
		return;
	}
	if (memcmp(p->data->p, "CT", 2) == 0) {
		//鼎信
		p->type = GW3762_T_TOPCOM;
		return;
	}
	if (memcmp(p->data->p, "LM", 2) == 0) {
		//力合微
		p->type = GW3762_T_LEAGUERME;
		return;
	}
	if (memcmp(p->data->p, "10", 2) == 0) {
		//瑞斯康
		p->type = GW3762_T_RISECOM;
		return;
	}
	if (memcmp(p->data->p, "im", 2) == 0) {
		//弥亚微
		p->type = GW3762_T_MIARTECH;
		return;
	}
	//默认晓程
	p->type = GW3762_T_XC_GW;
}


sys_res plc_Sync(t_plc *p)
{
	uint_t i, j, nSn, nPrtl, nFrom0, nValid = 0;
	uint16_t nQty;
	uint8_t aBuf[6], aMadr[6], aMadr2[6], aMeter[(LCP_SN_MAX + 7) / 8];

	if (p->type != GW3762_T_XC_GD)
		gw3762_ModAdrSet(p, 3000);

	if (plc_IsNotSync(p))
		return SYS_R_OK;

	if (p->type == GW3762_T_ES_RT)
		gw3762_Es_ModeSet(p, p->mode, 3000);

	for (nSn = 1; nSn < LCP_SN_MAX; nSn++) {
		if (plc_MeterRead(nSn, aMadr) == 0)
			continue;
		if (isnotbcd(aMadr, 6))
			continue;
		nValid += 1;
	}
	if (nValid == 0)
		return gw3762_ParaReset(p, 5000);

	if (p->type == GW3762_T_ES_RT)
		nFrom0 = 1;
	else
		nFrom0 = 0;

	for (i = 0; i < 2; i++) {
		if (p->type != GW3762_T_XC_RT) {
			if (gw3762_SubAdrQty(p, &nQty, 3000) != SYS_R_OK)
				return SYS_R_TMO;
		} else {
			nQty = LCP_SN_MAX;
		}
		memset(aMeter, 0, sizeof(aMeter));
		for (nSn = 1; nSn <= nQty; nSn++) {
			if (gw3762_SubAdrRead(p, nSn - nFrom0, NULL, aBuf, 2000) == SYS_R_OK) {
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
					gw3762_SubAdrDelete(p, aBuf, 2000);
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
				if (plc_MeterRead(j, aMadr2) >= 0) {
					if (memcmp(aMadr2, aMadr, 6) == 0)
						break;
				}
			}
			if (j < nSn)
				continue;
			gw3762_SubAdrAdd(p, nSn - nFrom0, aMadr, nPrtl, 2000);
		}
		if (gw3762_SubAdrQty(p, &nQty, 3000) != SYS_R_OK)
			return SYS_R_TMO;
		if (nQty == nValid)
			break;
		if (i == 1) {
			if (gw3762_ParaReset(p, 5000) != SYS_R_OK)
				return SYS_R_TMO;
		}
	}
	if (i < 2)
		return SYS_R_OK;
	return SYS_R_ERR;
}


void plc_Broadcast(t_plc *p)
{
	buf b = {0};
	uint8_t aBuf[6], aTime[6];
	uint_t nTemp;
	
#if XCN6N12_ENABLE
	if (p->type == GW3762_T_XC_GD) {
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

		if (p->type == GW3762_T_ES_RT) {
			if (gw3762_Es_ModeGet(p, &nTemp, 3000) == SYS_R_OK) {
				if (nTemp != 4) {
					gw3762_Es_ModeSet(p, 4, 5000);
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



#if 0

sys_res ecl_Plc_Recv(t_plc *p)
{
	sys_res res = SYS_R_TMO;
	uint8_t *pTemp;
	uint16_t nSn;
	uint32_t nDI, nRecDI = 0;
	uint_t nCode, nLen, nIs97;
	t_afn04_f10 xPM;
	buf bTx = {0};

	if (gw3762_Analyze(p) != SYS_R_OK)
		return res;

	if (p->afn == GW3762_AFN_ROUTE_REQUEST) {
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
				dlt645_Packet2Buf(bTx, &p->data->p[1], nCode, &nDI, nLen);
				gw3762_RequestAnswer(p, p->data->p[0], &p->data->p[1], 1, bTx->p, bTx->len);
				buf_Release(bTx);
			} else {
				nRecDI = 0x00000001;
				gw3762_RequestAnswer(p, p->data->p[0], &p->data->p[1], 0, &nRecDI, 3);
			}
			return SYS_R_BUSY;
		}
		return res;
	}

	if (p->afn == GW3762_AFN_REPORT) {
		//自动上报数据
		if (p->fn == 0x0002) {
			gw3762_Confirm(p, 0xFFFF, 0);
			buf_Remove(p->data, 4);
			//645包解析
			pTemp = dlt645_PacketAnalyze(p->data->p, p->data->len);
			if (pTemp == NULL)
				return SYS_R_ERR;
			//校验表地址
			if (memcmp(p->madr, &pTemp[1], 6))
				return SYS_R_ERR;
			buf_Remove(p->data, pTemp - p->data->p + (DLT645_HEADER_SIZE - 2));
			pTemp = p->data->p;
			byteadd(&pTemp[2], -0x33, pTemp[1]);
			//搜索表号
			if (ecl_Madr2Sn(p->madr, &nSn, 1)) {
				icp_MeterRead(nSn, &xPM, NULL);
				//645规约类型
				if (xPM.prtl == ECL_PRTL_DLT645_97) {
					nCode = DLT645_CODE_READ97;
					nLen = 2;
				} else {
					nCode = DLT645_CODE_READ07;
					nLen = 4;
				}
				if (GETBIT(*pTemp, 6)) {
					//抄通但电表拒绝该表不再抄杂项
					data_FlagWrite(xPM.tn, 0xFFFE);					
					return SYS_R_NO;
				}
				if (*pTemp == (nCode | BITMASK(7))) {
					pTemp += 2;
					memcpy(&nRecDI, pTemp, nLen);
					pTemp += nLen;
					ecl_DataHandler(xPM.tn, xPM.madr, pTask->time, nRecDI, pTemp);
					//抄成功
					return SYS_R_OK;
				}
			} else {
				//发现未知表事件
				evt_ERC35(ECL_PORT_PLC, p->madr);
				SETBIT(g_sys_status, SYS_STATUS_SYNC);
			}
		}
		return res;
	}

	if ((p->afn == GW3762_AFN_TRANSMIT) || (p->afn == GW3762_AFN_ROUTE_TRANSMIT)) {
		if (p->fn == 0x0001) {
			//点抄
			//校验模块主节点地址
			switch (p->type) {
			case GW3762_T_XC_GW:
			case GW3762_T_XC_RT:
			case GW3762_T_XC_GD:
				break;
			default:
				if (memcmp(p->radr, p->adr, 6))
					dbg_printf("<PLC>ModuleAddr %02X%02X%02X%02X%02X%02X", p->radr[5], p->radr[4], p->radr[3], p->radr[2], p->radr[1], p->radr[0]);
				break;
			}
			buf_Remove(p->data, 2);
			//645包解析
			pTemp = dlt645_PacketAnalyze(p->data->p, p->data->len);
			if (pTemp == NULL)
				return SYS_R_ERR;
			//校验表地址
			if (memcmp(p->madr, &pTemp[1], 6))
				return SYS_R_ERR;
			buf_Remove(p->data, pTemp - p->data->p + (DLT645_HEADER_SIZE - 2));
			pTemp = p->data->p;
			byteadd(&pTemp[2], -0x33, pTemp[1]);
			//是否抄表任务的电表回应
			if (memcmp(p->madr, pTask->f10.madr, 6)) {
				dbg_printf("<PLC>MeterDI %02X%02X%02X%02X%02X%02X %08X", p->madr[5], p->madr[4], p->madr[3], p->madr[2], p->madr[1], p->madr[0], nRecDI);
				//搜索表号
				if (ecl_Madr2Sn(p->madr, &nSn, 1)) {
					icp_MeterRead(nSn, &xPM, NULL);
					//645规约类型
					if (xPM.prtl == ECL_PRTL_DLT645_97) {
						nCode = DLT645_CODE_READ97;
						nLen = 2;
					} else {
						nCode = DLT645_CODE_READ07;
						nLen = 4;
					}
					if (GETBIT(*pTemp, 6)) {
						//抄通但电表拒绝该表不再抄杂项
						data_FlagWrite(xPM.tn, 1);
						return SYS_R_NO;
					}
					if (*pTemp == (nCode | BITMASK(7))) {
						pTemp += 2;
						memcpy(&nRecDI, pTemp, nLen);
						pTemp += nLen;
						ecl_DataHandler(xPM.tn, xPM.madr, pTask->time, nRecDI, pTemp);
					}
				} else {
					//发现未知表事件
					evt_ERC35(ECL_PORT_PLC, p->madr);
				}
				return SYS_R_ERR;
			}
			//645规约类型
			if (pTask->f10.prtl == ECL_PRTL_DLT645_97) {
				nCode = DLT645_CODE_READ97;
				nLen = 2;
			} else {
				nCode = DLT645_CODE_READ07;
				nLen = 4;
			}
			if (GETBIT(*pTemp, 6)) {
				//抄通但电表拒绝该表不再抄杂项
				data_FlagWrite(pTask->f10.tn, 1);
				return SYS_R_NO;
			}
			if (*pTemp == (nCode | BITMASK(7))) {
				pTemp += 2;
				memcpy(&nRecDI, pTemp, nLen);
				pTemp += nLen;
				ecl_DataHandler(pTask->f10.tn, pTask->f10.madr, pTask->time, nRecDI, pTemp);
				if (nRecDI != pTask->di)
					return SYS_R_ERR;
			}
			//抄成功
			return SYS_R_OK;
		}
		return res;
	}

	if (p->afn == GW3762_AFN_CONFIRM) {
		switch (p->fn) {
		case 0x0001:	//确认
			break;
		case 0x0002:	//否认
			res = SYS_R_ERR;
			break;
		default:
			break;
		}
		return res;
	}
	return res;
}







#endif



