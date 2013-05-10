

//Private Defines
#define GW3761_DATA_SIZE			4096

//固定帧头长度
#define GW3761_FIXHEADER_SIZE		6




//Private Typedef
typedef __packed struct {
	uint8_t			sc1;
	uint16_t		prtc1 : 2,
					len1 : 14;
	uint16_t		prtc2 : 2,
					len2 : 14;
	uint8_t			sc2;
	t_gw3761_c		c;
	uint16_t		a1;
	uint16_t		a2;
	uint8_t			group : 1,
					msa : 7;
	uint8_t			afn;
	t_gw3761_seq	seq;
}t_gw3761_header, *p_gw3761_header;


static int gw3761_IsPW(uint_t nAfn)
{

	switch (nAfn) {
	case 0x01:
	case 0x04:
	case 0x05:
	case 0x06:
	case 0x10:
		return 1;
	default:
		return 0;
	}
}

static int gw3761_IsEC(uint_t nAfn)
{

#if 1
	switch (nAfn) {
	case GW3761_AFN_CONFIRM:
	case 0x08:
	case 0x09:
	case 0x0A:
	case 0x0B:
	case 0x0C:
	case 0x0D:
	case 0x0E:
	case 0x10:
		return 1;
	default:
		return 0;
	}
#else
	switch (nAfn) {
	case GW3761_AFN_LINKCHECK:
		return 1;
	default:
		return 0;
	}
#endif
}









//Internal Functions
//-------------------------------------------------------------------------
//接收报文分析
//-------------------------------------------------------------------------
static sys_res gw3761_RmsgAnalyze(void *args)
{
	p_gw3761 p = (p_gw3761)args;
	p_gw3761_header pH;
	uint8_t *pTemp;
	int nLen;

	chl_RecData(p->parent.chl, p->parent.rbuf, OS_TICK_MS);
	for (; ; buf_Remove(p->parent.rbuf, 1)) {
		for (; ; buf_Remove(p->parent.rbuf, 1)) {
			//不足报文头长度
			if (p->parent.rbuf->len < sizeof(t_gw3761_header))
				return SYS_R_ERR;
			pH = (p_gw3761_header)p->parent.rbuf->p;
			if ((pH->sc1 == 0x68) && (pH->sc2 == 0x68)) {
#if GW3761_IDCHECK_ENABLE
				if (pH->prtc1 != GW3761_PROTOCOL_ID)
					continue;
				if (pH->prtc2 != GW3761_PROTOCOL_ID)
					continue;
#endif
				if (pH->len1 > GW3761_DATA_SIZE)
					continue;
				if (pH->len1 != pH->len2)
					continue;
				break;
			}
		}
		//不足长度
		if (p->parent.rbuf->len < (GW3761_FIXHEADER_SIZE + 2 + pH->len1))
			return SYS_R_ERR;
		pTemp = p->parent.rbuf->p + GW3761_FIXHEADER_SIZE + pH->len1;
		//CS
		if (cs8(p->parent.rbuf->p + GW3761_FIXHEADER_SIZE, pH->len1) != *pTemp)
			continue;
		//结束符
		if (*(pTemp + 1) != 0x16)
			continue;
		//接收到报文
		p->rmsg.c = pH->c;
		p->rmsg.a1 = pH->a1;
		p->rmsg.a2 = pH->a2;
		p->rmsg.group = pH->group;
		p->rmsg.msa = pH->msa;
		p->rmsg.afn = pH->afn;
		p->rmsg.seq = pH->seq;
		if (pH->seq.tpv) {
			//有时间标志
			pTemp -= sizeof(p->rmsg.tp);
			memcpy(&p->rmsg.tp, pTemp, sizeof(p->rmsg.tp));
		}
		if (gw3761_IsPW(pH->afn)) {
			pTemp -= sizeof(p->rmsg.pw);
			memcpy(&p->rmsg.pw, pTemp, sizeof(p->rmsg.pw));
		}
		buf_Release(p->rmsg.data);
		nLen = pTemp - p->parent.rbuf->p - sizeof(t_gw3761_header);
		if (nLen > 0)
			buf_Push(p->rmsg.data, p->parent.rbuf->p + sizeof(t_gw3761_header), nLen);
		buf_Remove(p->parent.rbuf, pH->len1 + GW3761_FIXHEADER_SIZE + 2);
		return SYS_R_OK;
	}
}


//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
static void gw3761_TmsgHeaderInit(p_gw3761 p, p_gw3761_header pH)
{

	bzero(pH, sizeof(t_gw3761_header));
	pH->sc1 = 0x68;
	pH->sc2 = 0x68;
	pH->prtc1 = GW3761_PROTOCOL_ID;
	pH->prtc2 = GW3761_PROTOCOL_ID;
	pH->a1 = p->rtua;
	pH->a2 = p->terid;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
static void gw3761_TmsgAfn00(p_gw3761 p, uint32_t nDu, uint_t nFun)
{
	buf b = {0};

	buf_PushData(b, nDu, 4);
	gw3761_TmsgSend(p, nFun, GW3761_AFN_CONFIRM, b, DLRCP_TMSG_RESPOND);
	buf_Release(b);
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
static sys_res gw3761_TmsgLinkcheck(void *p, uint_t nCmd)
{
	buf b = {0};

	switch (nCmd) {
	case DLRCP_LINKCHECK_LOGIN:
		nCmd = 1;
		break;
	case DLRCP_LINKCHECK_LOGOUT:
		nCmd = 2;
		break;
	default:
		nCmd = 3;
		break;
	}
	buf_PushData(b, gw3761_ConvertDa2DA(0), 2);
	buf_PushData(b, gw3761_ConvertFn2DT(nCmd), 2);
	gw3761_TmsgSend(p, GW3761_FUN_LINKCHECK, GW3761_AFN_LINKCHECK, b, DLRCP_TMSG_PULSEON);
	buf_Release(b);
#if GW3761_ECREPORT_ENABLE
	buf_PushData(b, gw3761_ConvertDa2DA(0), 2);
	buf_PushData(b, gw3761_ConvertFn2DT(7), 2);
	buf_PushData(b, evt_GetCount(), 2);
	gw3761_TmsgSend(p, GW3761_FUN_RESPONSE, 0x0C, b, DLRCP_TMSG_REPORT);
	buf_Release(b);
#endif
	return SYS_R_OK;
}




//External Functions
//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
void gw3761_Init(p_gw3761 p)
{

	memset(p, 0, sizeof(t_gw3761));
	p->parent.linkcheck = gw3761_TmsgLinkcheck;
	p->parent.analyze = gw3761_RmsgAnalyze;
}

//-------------------------------------------------------------------------
//发送报文
//-------------------------------------------------------------------------
sys_res gw3761_TmsgSend(p_gw3761 p, uint_t nFun, uint_t nAfn, buf b, uint_t nType)
{
	t_gw3761_header xH;
	uint_t nCS;

	gw3761_TmsgHeaderInit(p, &xH);
	switch (nType) {
	case DLRCP_TMSG_REPORT:
		xH.c.prm = 1;
		xH.seq.seq = p->parent.pfc++;
		xH.seq.con = 0;
		break;
	case DLRCP_TMSG_PULSEON:
		xH.c.prm = 1;
		xH.seq.seq = p->parent.pfc++;
		xH.seq.con = 1;
		break;
	default:
		xH.msa = p->rmsg.msa;
		xH.seq.seq = p->rmsg.seq.seq;
		break;
	}
	xH.c.dir = GW3761_DIR_SEND;
	xH.c.fun = nFun;
	xH.seq.fin = 1;
	xH.seq.fir = 1;
	xH.afn = nAfn;
	if (gw3761_IsEC(nAfn)) {
		xH.c.fcb_acd = 1;
		buf_PushData(b, evt_GetCount(), 2);
	}
	if (nType == DLRCP_TMSG_RESPOND) {
		if (p->rmsg.seq.tpv) {
			xH.seq.tpv = 1;
			buf_Push(b, &p->rmsg.tp, sizeof(p->rmsg.tp));
		}
	}
	xH.len1 = xH.len2 = b->len + (sizeof(t_gw3761_header) - GW3761_FIXHEADER_SIZE);
	nCS = cs8((uint8_t *)&xH + GW3761_FIXHEADER_SIZE, (sizeof(t_gw3761_header) - GW3761_FIXHEADER_SIZE));
	nCS += cs8(b->p, b->len);
	buf_PushData(b, 0x1600 | (nCS & 0xFF), 2);
	return dlrcp_TmsgSend(&p->parent, &xH, sizeof(t_gw3761_header), b->p, b->len);
}


//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
sys_res gw3761_TmsgConfirm(p_gw3761 p)
{

	gw3761_TmsgAfn00(p, 0x00010000, GW3761_FUN_CONFIRM);
	return SYS_R_OK;
}


//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
sys_res gw3761_TmsgReject(p_gw3761 p)
{

	gw3761_TmsgAfn00(p, 0x00020000, GW3761_FUN_NODATA);
	return SYS_R_OK;
}


//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
sys_res gw3761_Transmit(p_gw3761 p, p_gw3761 pD)
{
	sys_res res;
	t_gw3761_header xH;
	uint_t nCS;
	buf b = {0};

	buf_Push(b, p->rmsg.data->p, p->rmsg.data->len);
	xH.sc1 = 0x68;
	xH.sc2 = 0x68;
	xH.prtc1 = GW3761_PROTOCOL_ID;
	xH.prtc2 = GW3761_PROTOCOL_ID;
	xH.c = p->rmsg.c;
	xH.a1 = p->rmsg.a1;
	xH.a2 = p->rmsg.a2;
	xH.group = p->rmsg.group;
	xH.msa = p->rmsg.msa;
	xH.afn = p->rmsg.afn;
	xH.seq = p->rmsg.seq;
	//有时间标志
	if (xH.seq.tpv)
		buf_Push(b, &p->rmsg.tp, sizeof(p->rmsg.tp));
	//有密码
	if (gw3761_IsPW(xH.afn))
		buf_Push(b, &p->rmsg.pw, sizeof(p->rmsg.pw));
	xH.len1 = xH.len2 = b->len + (sizeof(t_gw3761_header) - GW3761_FIXHEADER_SIZE);
	nCS = cs8((uint8_t *)&xH + GW3761_FIXHEADER_SIZE, (sizeof(t_gw3761_header) - GW3761_FIXHEADER_SIZE));
	nCS += cs8(b->p, b->len);
	buf_PushData(b, 0x1600 | (nCS & 0xFF), 2);
	res = dlrcp_TmsgSend(&pD->parent, &xH, sizeof(t_gw3761_header), b->p, b->len);
	buf_Release(b);
	return res;
}


//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
int gw3761_RecvCheck(p_gw3761 p)
{

	if ((p->rmsg.a1 != 0xFFFF) || (p->rmsg.a2 != 0xFFFF)) {
		if ((p->rtua != p->rmsg.a1) || (p->terid != p->rmsg.a2))
			return 0;
	}
	return 1;
}


//-------------------------------------------------------------------------
//规约处理
//-------------------------------------------------------------------------
sys_res gw3761_Handler(p_gw3761 p)
{

	return dlrcp_Handler(&p->parent);
}








