


//Private Defines
#define GW3762_HEADER_L_SIZE		2


//GW376.2 Code Defines
#define GW3762_CODE_T_PLC_FOCUS		1
#define GW3762_CODE_T_PLC_SCATT		2
#define GW3762_CODE_T_FM			10

#define GW3762_CODE_P_PUSH			1
#define GW3762_CODE_P_ACK			0

#define GW3762_CODE_D_2MODULE		0
#define GW3762_CODE_D_2TERMINAL		1

//GW376.2 Info Zone Defines
#define GW3762_RZONE_M_2MODULE		0
#define GW3762_RZONE_M_2METER		1

#define GW3762_RZONE_R_ROUTE		0
#define GW3762_RZONE_R_TRANS		1





//Private Typedefs
typedef __packed struct {
	uint8_t		sc1;
	uint16_t	len;
	t_gw3762_c	c;
}t_gw3762_header;



//Private Macros


//Internal Functions
#if GW3762_DEBUG_ENABLE
static void gw3762_DbgOut(uint_t nType, const void *pBuf, uint_t nLen)
{
	const uint8_t *pData, *pEnd;
	char str[DBG_BUF_SIZE];

	pData = (const uint8_t *)pBuf;
	pEnd = pData + nLen;

	if (nType)
		nLen = sprintf(str, "<376.2T>");
	else
		nLen = sprintf(str, "<376.2R>");
	while ((pData < pEnd) && (nLen < (sizeof(str) - 3)))
		nLen += sprintf(&str[nLen], " %02X", *pData++);

	dbg_trace(str);
}
#else
#define gw3762_DbgOut(...)
#endif

static sys_res gw3762_Transmit2ES(t_gw3762 *p, uint_t nAfn, uint_t nDT, const void *pData, uint_t nLen)
{
	buf bTx = {0};
	t_gw3762_rdown xR = {0};
	
	buf_PushData(bTx, 0x47000068, 4);
	buf_Push(bTx, &xR, sizeof(xR));

	buf_PushData(bTx, nAfn, 1);
	buf_PushData(bTx, nDT, 2);
	buf_Push(bTx, pData, nLen);
	buf_PushData(bTx, cs8(&bTx->p[1 + GW3762_HEADER_L_SIZE], bTx->len - (1 + GW3762_HEADER_L_SIZE)) | 0x1600, 2);
	memcpy(&bTx->p[1], &bTx->len, GW3762_HEADER_L_SIZE);

	gw3762_DbgOut(1, bTx->p, bTx->len);

	chl_Send(p->chl, bTx->p, bTx->len);
	buf_Release(bTx);

	return SYS_R_OK;
}


static sys_res gw3762_Transmit2Module(t_gw3762 *p, uint_t nAfn, uint_t nDT, const void *pData, uint_t nLen)
{
	buf bTx = {0};
	t_gw3762_rdown xR = {0};
	
	xR.route = GW3762_RZONE_R_TRANS;

	buf_PushData(bTx, 0x41000068, 4);
	buf_Push(bTx, &xR, sizeof(xR));

	buf_PushData(bTx, nAfn, 1);
	buf_PushData(bTx, nDT, 2);
	buf_Push(bTx, pData, nLen);
	buf_PushData(bTx, cs8(&bTx->p[1 + GW3762_HEADER_L_SIZE], bTx->len - (1 + GW3762_HEADER_L_SIZE)) | 0x1600, 2);
	memcpy(&bTx->p[1], &bTx->len, GW3762_HEADER_L_SIZE);

	gw3762_DbgOut(1, bTx->p, bTx->len);

	chl_Send(p->chl, bTx->p, bTx->len);
	buf_Release(bTx);

	return SYS_R_OK;
}


static sys_res gw3762_Transmit2Meter(t_gw3762 *p, uint_t nRoute, uint_t nAfn, uint_t nDT, const void *pAdr, uint_t nRelay, const void *pRtAdr, const void *pData, uint_t nLen)
{
	buf bTx = {0};
	t_gw3762_rdown xR = {0};

	if (nRoute == 0)
		xR.route = GW3762_RZONE_R_TRANS;
	xR.module = GW3762_RZONE_M_2METER;
	xR.relay = nRelay;

	buf_PushData(bTx, 0x41000068, 4);
	buf_Push(bTx, &xR, sizeof(xR));
	buf_Push(bTx, p->adr, sizeof(p->adr));
	if (nRelay)
		buf_Push(bTx, pRtAdr, nRelay * 6);
	buf_Push(bTx, pAdr, 6);

	buf_PushData(bTx, nAfn, 1);
	buf_PushData(bTx, nDT, 2);
	if (nAfn == GW3762_AFN_ROUTE_TRANSMIT)
		buf_PushData(bTx, 0x0002, 2);
	else
		buf_PushData(bTx, 0x02, 1);
	buf_PushData(bTx, nLen, 1);
	buf_Push(bTx, pData, nLen);
	buf_PushData(bTx, cs8(&bTx->p[1 + GW3762_HEADER_L_SIZE], bTx->len - (1 + GW3762_HEADER_L_SIZE)) | 0x1600, 2);
	memcpy(&bTx->p[1], &bTx->len, GW3762_HEADER_L_SIZE);

	gw3762_DbgOut(1, bTx->p, bTx->len);

	chl_Send(p->chl, bTx->p, bTx->len);
	buf_Release(bTx);
	
	return SYS_R_OK;
}






//External Functions
void gw3762_Init(t_gw3762 *p)
{

	memset(p, 0, sizeof(t_gw3762));
}

int gw3762_IsNeedRt(t_gw3762 *p)
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




int gw3762_IsNotSync(t_gw3762 *p)
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

int gw3762_GetRetry(t_gw3762 *p)
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

int gw3762_GetWait(t_gw3762 *p, uint_t nRelay)
{

	switch (p->type) {
 	case GW3762_T_ES_38:
		return 12 + nRelay * 6;
	case GW3762_T_XC_GW:
		return 10 + nRelay * 4;
	case GW3762_T_XC_GD:
		return 10 + nRelay * 6;
	case GW3762_T_XC_RT:
		return 30;
	case GW3762_T_TOPCOM:
		return 90;
	default:
		return 20;
	}
}



//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
sys_res gw3762_Analyze(t_gw3762 *p)
{
	uint8_t *pData;
	int nLen;
	uint_t nOffset;
	t_gw3762_header *pH;

	chl_RecData(p->chl, p->rbuf, OS_TICK_MS);
	for (; ; buf_Remove(p->rbuf, 1)) {
		for (; ; buf_Remove(p->rbuf, 1)) {
			//不足报文头长度
			if (p->rbuf->len < sizeof(t_gw3762_header))
				return SYS_R_ERR;
			pH = (t_gw3762_header *)p->rbuf->p;
			//报文头检查
			if (pH->sc1 == 0x68) {
				if (pH->c.dir == GW3762_CODE_D_2TERMINAL)
					break;
			}
		}
		//不足长度
		if (p->rbuf->len < pH->len)
			return SYS_R_ERR;
		pData = p->rbuf->p + pH->len - 2;
		//CS
		if (cs8(&p->rbuf->p[sizeof(t_gw3762_header) - 1], pH->len - (sizeof(t_gw3762_header) + 1)) != *pData++)
			continue;
		//结束符
		if (*pData != 0x16)
			continue;

		gw3762_DbgOut(0, pH, pH->len);

		memcpy(&p->rmsg.rup, &p->rbuf->p[sizeof(t_gw3762_header)], 6);
		if (p->rmsg.rup.module) {
			memcpy(p->rmsg.madr, &p->rbuf->p[sizeof(t_gw3762_header) + 6], 6);
			memcpy(p->rmsg.adr, &p->rbuf->p[sizeof(t_gw3762_header) + 12], 6);
			nOffset = 18;
		} else
			nOffset = 6;
		p->rmsg.afn = p->rbuf->p[sizeof(t_gw3762_header) + nOffset];
		memcpy(&p->rmsg.fn, &p->rbuf->p[sizeof(t_gw3762_header) + nOffset + 1], 2);
		buf_Release(p->rmsg.data);
		nLen = pH->len - (sizeof(t_gw3762_header) + nOffset + 5);
		if (nLen > 0)
			buf_Push(p->rmsg.data, &p->rbuf->p[sizeof(t_gw3762_header) + nOffset + 3], nLen);
		buf_Remove(p->rbuf, pH->len);
		return SYS_R_OK;
	}
}


sys_res gw3762_Broadcast(t_gw3762 *p, const void *pAdr, const void *pData, uint_t nLen)
{
	uint_t nAfn, nDT, nRoute, nTmo;

	nRoute = 1;
	switch (p->type) {
	case GW3762_T_XC_GW:
		nRoute = 0;
	case GW3762_T_XC_RT:
		nAfn = GW3762_AFN_ROUTE_TRANSMIT;
		nDT = 0x0001;
		break;
	case GW3762_T_ES_38:
		nRoute = 0;
	default:
		nAfn = GW3762_AFN_DATA_SET;
		nDT = 0x0004;
		break;
	}
	gw3762_Transmit2Meter(p, nRoute, nAfn, nDT, pAdr, 0, NULL, pData, nLen);;
	for (nTmo = 30000 / OS_TICK_MS; nTmo; nTmo--) {
		if (gw3762_Analyze(p) == SYS_R_OK)
			break;
	}
	if (nTmo == 0)
		return SYS_R_TMO;
	if (p->rmsg.afn != GW3762_AFN_CONFIRM)
		return SYS_R_ERR;
	if (p->rmsg.fn != 0x0001)
		return SYS_R_ERR;
	return SYS_R_OK;
}

sys_res gw3762_MeterRead(t_gw3762 *p, const void *pAdr, uint_t nRelay, const void *pRtAdr, const void *pData, uint_t nLen)
{
	uint_t nAfn;

	switch (p->type) {
	case GW3762_T_ES_38:
	case GW3762_T_XC_GD:
		nAfn = GW3762_AFN_TRANSMIT;
		break;
	default:
		nAfn = GW3762_AFN_ROUTE_TRANSMIT;
		break;
	}
	return gw3762_Transmit2Meter(p, 0, nAfn, 0x0001, pAdr, nRelay, pRtAdr, pData, nLen);
}

sys_res gw3762_MeterRT(t_gw3762 *p, const void *pAdr, const void *pData, uint_t nLen)
{
	uint_t nAfn;

	if (p->type == GW3762_T_RISECOM)
		nAfn = GW3762_AFN_TRANSMIT;
	else
		nAfn = GW3762_AFN_ROUTE_TRANSMIT;
	return gw3762_Transmit2Meter(p, 1, nAfn, 0x0001, pAdr, 0, NULL, pData, nLen);
}


//-------------------------------------------------------------------------------------
// 确认
//-------------------------------------------------------------------------------------
sys_res gw3762_Confirm(t_gw3762 *p, uint_t nFlag, uint_t nTmo)
{
	buf bTx = {0};
	t_gw3762_rdown xR = {0};
	
	xR.route = GW3762_RZONE_R_TRANS;

	buf_PushData(bTx, 0x01000068, 4);
	buf_Push(bTx, &xR, sizeof(xR));

	buf_PushData(bTx, GW3762_AFN_CONFIRM, 1);
	buf_PushData(bTx, 0x0001, 2);
	buf_PushData(bTx, nFlag, 2);
	buf_PushData(bTx, nTmo, 2);
	buf_PushData(bTx, cs8(&bTx->p[1 + GW3762_HEADER_L_SIZE], bTx->len - (1 + GW3762_HEADER_L_SIZE)) | 0x1600, 2);
	memcpy(&bTx->p[1], &bTx->len, GW3762_HEADER_L_SIZE);

	gw3762_DbgOut(1, bTx->p, bTx->len);

	chl_Send(p->chl, bTx->p, bTx->len);
	buf_Release(bTx);

	return SYS_R_OK;
}


//-------------------------------------------------------------------------------------
// 硬件初始化
//-------------------------------------------------------------------------------------
sys_res gw3762_HwReset(t_gw3762 *p, uint_t nTmo)
{

	gw3762_Transmit2Module(p, GW3762_AFN_RESET, 0x0001, NULL, 0);
	for (nTmo /= OS_TICK_MS; nTmo; nTmo--) {
		if (gw3762_Analyze(p) == SYS_R_OK)
			break;
	}
	if (nTmo == 0)
		return SYS_R_TMO;
	if (p->rmsg.afn != GW3762_AFN_CONFIRM)
		return SYS_R_ERR;
	if (p->rmsg.fn != 0x0001)
		return SYS_R_ERR;
	return SYS_R_OK;
}

//-------------------------------------------------------------------------------------
//参数初始化
//-------------------------------------------------------------------------------------
sys_res gw3762_ParaReset(t_gw3762 *p, uint_t nTmo)
{

	gw3762_Transmit2Module(p, GW3762_AFN_RESET, 0x0002, NULL, 0);
	for (nTmo /= OS_TICK_MS; nTmo; nTmo--) {
		if (gw3762_Analyze(p) == SYS_R_OK)
			break;
	}
	if (nTmo == 0)
		return SYS_R_TMO;
	if (p->rmsg.afn != GW3762_AFN_CONFIRM)
		return SYS_R_ERR;
	if (p->rmsg.fn != 0x0001)
		return SYS_R_ERR;
	return SYS_R_OK;
}

//-------------------------------------------------------------------------------------
// 获取厂商代码和版本信息
//-------------------------------------------------------------------------------------
sys_res gw3762_InfoGet(t_gw3762 *p, uint_t nTmo)
{

	//查厂商代码及版本信息
	gw3762_Transmit2Module(p, GW3762_AFN_DATA_FETCH, 0x0001, NULL, 0);
	for (nTmo /= OS_TICK_MS; nTmo; nTmo--) {
		if (gw3762_Analyze(p) == SYS_R_OK)
			break;
	}
	if (nTmo == 0)
		return SYS_R_TMO;
	if (p->rmsg.afn != GW3762_AFN_DATA_FETCH)
		return SYS_R_ERR;
	if (p->rmsg.fn != 0x0001)
		return SYS_R_ERR;
	return SYS_R_OK;
}

//-------------------------------------------------------------------------------------
// 设置模块主节点地址
//-------------------------------------------------------------------------------------
sys_res gw3762_ModAdrSet(t_gw3762 *p, const void *pAdr, uint_t nTmo)
{

	gw3762_Transmit2Module(p, GW3762_AFN_DATA_SET, 0x0001, pAdr, 6);
	for (nTmo /= OS_TICK_MS; nTmo; nTmo--) {
		if (gw3762_Analyze(p) == SYS_R_OK)
			break;
	}
	if (nTmo == 0)
		return SYS_R_TMO;
	if (p->rmsg.afn != GW3762_AFN_CONFIRM)
		return SYS_R_ERR;
	if (p->rmsg.fn != 0x0001)
		return SYS_R_ERR;
	memcpy(p->adr, pAdr, 6);
	return SYS_R_OK;
}

//-------------------------------------------------------------------------------------
// 读取从节点数量
//-------------------------------------------------------------------------------------
sys_res gw3762_SubAdrQty(t_gw3762 *p, uint16_t *pQty, uint_t nTmo)
{

	gw3762_Transmit2Module(p, GW3762_AFN_ROUTE_FETCH, 0x0001, NULL, 0);
	for (nTmo /= OS_TICK_MS; nTmo; nTmo--) {
		if (gw3762_Analyze(p) == SYS_R_OK)
			break;
	}
	if (nTmo == 0)
		return SYS_R_TMO;
	if (p->rmsg.afn != GW3762_AFN_ROUTE_FETCH)
		return SYS_R_ERR;
	if (p->rmsg.fn != 0x0001)
		return SYS_R_ERR;
	memcpy(pQty, &p->rmsg.data->p[0], 2);
	return SYS_R_OK;
}

//-------------------------------------------------------------------------------------
// 读取从节点信息
//-------------------------------------------------------------------------------------
sys_res gw3762_SubAdrRead(t_gw3762 *p, uint_t nSn, uint16_t *pQty, uint8_t *pAdr, uint_t nTmo)
{
	uint_t nTemp;

	nTemp = 0x00010000 | nSn;
	gw3762_Transmit2Module(p, GW3762_AFN_ROUTE_FETCH, 0x0002, &nTemp, 3);
	for (nTmo /= OS_TICK_MS; nTmo; nTmo--) {
		if (gw3762_Analyze(p) == SYS_R_OK)
			break;
	}
	if (nTmo == 0)
		return SYS_R_TMO;
	if (p->rmsg.afn != GW3762_AFN_ROUTE_FETCH)
		return SYS_R_ERR;
	if (p->rmsg.fn != 0x0002)
		return SYS_R_ERR;
	if (pQty != NULL)
		memcpy(pQty, &p->rmsg.data->p[0], 2);
	if (p->rmsg.data->p[2] == 0)
		return SYS_R_ERR;
	memcpy(pAdr, &p->rmsg.data->p[3], 6);
	return SYS_R_OK;
}

//-------------------------------------------------------------------------------------
// 读取路由运行状态
//-------------------------------------------------------------------------------------
sys_res gw3762_StateGet(t_gw3762 *p, uint_t nTmo)
{

	gw3762_Transmit2Module(p, GW3762_AFN_ROUTE_FETCH, 0x0008, NULL, 0);
	for (nTmo /= OS_TICK_MS; nTmo; nTmo--) {
		if (gw3762_Analyze(p) == SYS_R_OK)
			break;
	}
	if (nTmo == 0)
		return SYS_R_TMO;
	if (p->rmsg.afn != GW3762_AFN_ROUTE_FETCH)
		return SYS_R_ERR;
	if (p->rmsg.fn != 0x0008)
		return SYS_R_ERR;
	return SYS_R_OK;
}


//-------------------------------------------------------------------------------------
// 添加从节点
//-------------------------------------------------------------------------------------
sys_res gw3762_SubAdrAdd(t_gw3762 *p, uint_t nSn, const void *pAdr, uint_t nPrtl, uint_t nTmo)
{
	uint8_t aBuf[10];

	aBuf[0] = 1;
	memcpy(&aBuf[1], pAdr, 6);
	memcpy(&aBuf[7], &nSn, 2);
	aBuf[9] = nPrtl;
	gw3762_Transmit2Module(p, GW3762_AFN_ROUTE_SET, 0x0001, aBuf, 10);
	for (nTmo /= OS_TICK_MS; nTmo; nTmo--) {
		if (gw3762_Analyze(p) == SYS_R_OK)
			break;
	}
	if (nTmo == 0)
		return SYS_R_TMO;
	if (p->rmsg.afn != GW3762_AFN_CONFIRM)
		return SYS_R_ERR;
	if (p->rmsg.fn != 0x0001)
		return SYS_R_ERR;
	return SYS_R_OK;
}


//-------------------------------------------------------------------------------------
// 删除从节点
//-------------------------------------------------------------------------------------
sys_res gw3762_SubAdrDelete(t_gw3762 *p, const void *pAdr, uint_t nTmo)
{
	uint8_t aBuf[7];

	aBuf[0] = 1;
	memcpy(&aBuf[1], pAdr, 6);
	gw3762_Transmit2Module(p, GW3762_AFN_ROUTE_SET, 0x0002, aBuf, 7);
	for (nTmo /= OS_TICK_MS; nTmo; nTmo--) {
		if (gw3762_Analyze(p) == SYS_R_OK)
			break;
	}
	if (nTmo == 0)
		return SYS_R_TMO;
	if (p->rmsg.afn != GW3762_AFN_CONFIRM)
		return SYS_R_ERR;
	if (p->rmsg.fn != 0x0001)
		return SYS_R_ERR;
	return SYS_R_OK;
}

//-------------------------------------------------------------------------------------
// 模式设置
//-------------------------------------------------------------------------------------
sys_res gw3762_ModeSet(t_gw3762 *p, uint_t nMode, uint_t nTmo)
{
	uint8_t aBuf[3];

	aBuf[0] = nMode;
	aBuf[1] = 0xF4;
	aBuf[2] = 0x01;
	gw3762_Transmit2Module(p, GW3762_AFN_ROUTE_SET, 0x0008, aBuf, 3);
	for (nTmo /= OS_TICK_MS; nTmo; nTmo--) {
		if (gw3762_Analyze(p) == SYS_R_OK)
			break;
	}
	if (nTmo == 0)
		return SYS_R_TMO;
	if (p->rmsg.afn != GW3762_AFN_CONFIRM)
		return SYS_R_ERR;
	if (p->rmsg.fn != 0x0001)
		return SYS_R_ERR;
	return SYS_R_OK;
}

//-------------------------------------------------------------------------------------
// 路由控制
//-------------------------------------------------------------------------------------
sys_res gw3762_RtCtrl(t_gw3762 *p, uint_t nDT, uint_t nTmo)
{

	gw3762_Transmit2Module(p, GW3762_AFN_ROUTE_CTRL, nDT, NULL, 0);
	for (nTmo /= OS_TICK_MS; nTmo; nTmo--) {
		if (gw3762_Analyze(p) == SYS_R_OK)
			break;
	}
	if (nTmo == 0)
		return SYS_R_TMO;
	if (p->rmsg.afn != GW3762_AFN_CONFIRM)
		return SYS_R_ERR;
	if (p->rmsg.fn != 0x0001)
		return SYS_R_ERR;
	return SYS_R_OK;
}

//-------------------------------------------------------------------------------------
// 路由请求应答
//-------------------------------------------------------------------------------------
sys_res gw3762_RequestAnswer(t_gw3762 *p, uint_t nPhase, const void *pAdr, uint_t nIsRead, const void *pData, uint_t nLen)
{
	buf bTx = {0};
	t_gw3762_rdown xR = {0};

	xR.module = GW3762_RZONE_M_2METER;
	xR.chlid = nPhase;
	xR.acklen = 40;

	buf_PushData(bTx, 0x01000068, 4);
	buf_Push(bTx, &xR, sizeof(xR));
	buf_Push(bTx, p->adr, sizeof(p->adr));
	buf_Push(bTx, pAdr, 6);

	buf_PushData(bTx, GW3762_AFN_ROUTE_REQUEST, 1);
	buf_PushData(bTx, 0x0001, 2);
	if (nIsRead) {
		buf_PushData(bTx, 0x02, 1);
		buf_PushData(bTx, nLen, 1);
	}
	buf_Push(bTx, pData, nLen);
	buf_PushData(bTx, 0, 1);
	buf_PushData(bTx, cs8(&bTx->p[1 + GW3762_HEADER_L_SIZE], bTx->len - (1 + GW3762_HEADER_L_SIZE)) | 0x1600, 2);
	memcpy(&bTx->p[1], &bTx->len, GW3762_HEADER_L_SIZE);

	gw3762_DbgOut(1, bTx->p, bTx->len);

	chl_Send(p->chl, bTx->p, bTx->len);
	buf_Release(bTx);
	
	return SYS_R_OK;
}

//-------------------------------------------------------------------------------------
// 透明转发
//-------------------------------------------------------------------------------------
sys_res gw3762_Transmit(t_gw3762 *p, buf b, const void *pData, uint_t nLen)
{
	uint_t nTmo;

	gw3762_DbgOut(1, pData, nLen);

	chl_Send(p->chl, pData, nLen);

	for (nTmo = 200 / OS_TICK_MS; nTmo; nTmo--) {
		chl_RecData(p->chl, b, OS_TICK_MS);
	}
	if (b->len == 0)
		return SYS_R_ERR;		

	gw3762_DbgOut(0, b->p, b->len);
	return SYS_R_OK;
}

//-------------------------------------------------------------------------------------
// 东软扩展-读取路由运行模式
//-------------------------------------------------------------------------------------
sys_res gw3762_Es_ModeGet(t_gw3762 *p, uint_t *pMode, uint_t nTmo)
{

	gw3762_Transmit2ES(p, GW3762_AFN_TRANSMIT, 0x0010, NULL, 0);
	for (nTmo /= OS_TICK_MS; nTmo; nTmo--) {
		if (gw3762_Analyze(p) == SYS_R_OK)
			break;
	}
	if (nTmo == 0)
		return SYS_R_TMO;
	if (p->rmsg.afn != GW3762_AFN_TRANSMIT)
		return SYS_R_ERR;
	if (p->rmsg.fn != 0x0010)
		return SYS_R_ERR;
	*pMode = p->rmsg.data->p[0];
	return SYS_R_OK;
}

//-------------------------------------------------------------------------------------
// 东软扩展-设置路由运行模式
//-------------------------------------------------------------------------------------
sys_res gw3762_Es_ModeSet(t_gw3762 *p, uint_t nMode, uint_t nTmo)
{

	gw3762_Transmit2ES(p, GW3762_AFN_RESET, 0x0040, &nMode, 1);
	for (nTmo /= OS_TICK_MS; nTmo; nTmo--) {
		if (gw3762_Analyze(p) == SYS_R_OK)
			break;
	}
	if (nTmo == 0)
		return SYS_R_TMO;
	if (p->rmsg.afn != GW3762_AFN_CONFIRM)
		return SYS_R_ERR;
	if (p->rmsg.fn != 0x0001)
		return SYS_R_ERR;
	return SYS_R_OK;
}



