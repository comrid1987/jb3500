

//Private Defines
#define NW12_DATA_SIZE				4096

//固定帧头长度
#define NW12_FIXHEADER_SIZE			6

//传送方向定义
#define NW12_DIR_RECV				0	//主站发出
#define NW12_DIR_SEND				1	//终端发出



//Private Typedef
typedef __packed struct {
	uint8_t		sc1;
	uint16_t	len1;
	uint16_t	len2;
	uint8_t		sc2;
	t_nw12_c	c;
	uint8_t		a1[3];
	uint8_t		a2[3];
	uint8_t		msa;
	uint8_t		afn;
	t_nw12_seq	seq;
}t_nw12_header, *p_nw12_header;

static int nw12_IsPW(uint_t nAfn)
{

	switch (nAfn) {
	case 0x04:
	case 0x05:
		return 1;
	default:
		return 0;
	}
}


//Private Macros
#if NW12_DEBUG_ENABLE
static void nw12_DbgTx(const void *pHeader, const void *pBuf, uint_t nTxLen)
{
	const uint8_t *pData, *pEnd;
	char str[DBG_BUF_SIZE];
	uint_t nLen;

	nLen = sprintf(str, "<NWT>");

	pData = (const uint8_t *)pHeader;
	pEnd = pData + sizeof(t_nw12_header);

	while (pData < pEnd) {
		nLen += sprintf(&str[nLen], " %02X", *pData++);
	}

	pData = (const uint8_t *)pBuf;
	pEnd = pData + nTxLen;

	while ((pData < pEnd) && (nLen < (sizeof(str) - 3))) {
		nLen += sprintf(&str[nLen], " %02X", *pData++);
	}

	dbg_trace(str);
}
static void nw12_DbgRx(const void *pBuf, uint_t nLen)
{
	const uint8_t *pData, *pEnd;
	char str[DBG_BUF_SIZE];

	pData = (const uint8_t *)pBuf;
	pEnd = pData + nLen;

	nLen = sprintf(str, "<NWR>");
	while ((pData < pEnd) && (nLen < (sizeof(str) - 3))) {
		nLen += sprintf(&str[nLen], " %02X", *pData++);
	}

	dbg_trace(str);
}
#else
#define nw12_DbgTx(...)
#define nw12_DbgRx(...)
#endif




//Internal Functions
//-------------------------------------------------------------------------
//接收报文分析
//-------------------------------------------------------------------------
static sys_res nw12_RmsgAnalyze(void *args)
{
	p_nw12 p = (p_nw12)args;
	p_nw12_header pH;
	uint8_t *pTemp;
	int nLen;

	chl_RecData(p->parent.chl, p->parent.rbuf, OS_TICK_MS);
	for (; ; buf_Remove(p->parent.rbuf, 1)) {
		for (; ; buf_Remove(p->parent.rbuf, 1)) {
			//不足报文头长度
			if (p->parent.rbuf->len < sizeof(t_nw12_header))
				return SYS_R_ERR;
			pH = (p_nw12_header)p->parent.rbuf->p;
			if ((pH->sc1 == 0x68) && (pH->sc2 == 0x68)) {
				if (pH->len1 > NW12_DATA_SIZE)
					continue;
				if (pH->len1 != pH->len2)
					continue;
				break;
			}
		}
		//不足长度
		if (p->parent.rbuf->len < (NW12_FIXHEADER_SIZE + 2 + pH->len1))
			return SYS_R_ERR;
		pTemp = p->parent.rbuf->p + NW12_FIXHEADER_SIZE + pH->len1;
		//CS
		if (cs8(p->parent.rbuf->p + NW12_FIXHEADER_SIZE, pH->len1) != *pTemp)
			continue;
		//结束符
		if (*(pTemp + 1) != 0x16)
			continue;
#if NW12_DEBUG_ENABLE
		nw12_DbgRx(p->parent.rbuf->p, NW12_FIXHEADER_SIZE + 2 + pH->len1);
#endif
		//接收到报文
		p->msa = pH->msa;		
		p->c = pH->c;
		p->afn = pH->afn;
		p->seq = pH->seq;
		memcpy(p->rcv_rtua,pH->a1,3);
		memcpy(p->rcv_terid,pH->a2,3);
		if (pH->seq.tpv) {
			//有时间标志
			pTemp -= sizeof(p->tp);
			memcpy(&p->tp, pTemp, sizeof(p->tp));
		}
		if (nw12_IsPW(pH->afn)) {
			pTemp -= sizeof(p->pw);
			memcpy(&p->pw, pTemp, sizeof(p->pw));
		}
		buf_Release(p->data);
		nLen = pTemp - p->parent.rbuf->p - sizeof(t_nw12_header);
		if (nLen > 0)
			buf_Push(p->data, p->parent.rbuf->p + sizeof(t_nw12_header), nLen);
		buf_Remove(p->parent.rbuf, pH->len1 + NW12_FIXHEADER_SIZE + 2);
		return SYS_R_OK;
	}
}


//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
static void nw12_TmsgHeaderInit(p_nw12 p, p_nw12_header pH)
{

	bzero(pH, sizeof(t_nw12_header));
	pH->sc1 = 0x68;
	pH->sc2 = 0x68;
	memcpy(pH->a1, p->rtua, 3);
	memcpy(pH->a2, p->terid, 3);
}


//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
static sys_res nw12_TmsgLinkcheck(void *p, uint_t nCmd)
{
	buf b = {0};

	buf_PushData(b, nw12_ConvertDa2DA(0), 2);
	switch (nCmd) {
	case DLRCP_LINKCHECK_LOGIN:
		buf_PushData(b, 0xE0001000, 4);
		buf_PushData(b, 0x0100, 2);
		break;
	case DLRCP_LINKCHECK_LOGOUT:
		buf_PushData(b, 0xE0001002, 4);
		break;
	default:
		buf_PushData(b, 0xE0001001, 4);
		break;
	}
	nw12_TmsgSend(p, NW12_FUN_LINKCHECK, NW12_AFN_LINKCHECK, b, DLRCP_TMSG_PULSEON);
	buf_Release(b);
	return SYS_R_OK;
}




//External Functions
//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
void nw12_Init(p_nw12 p)
{

	memset(p, 0, sizeof(t_nw12));
	chl_Init(p->parent.chl);
	p->parent.linkcheck = nw12_TmsgLinkcheck;
	p->parent.analyze = nw12_RmsgAnalyze;
}


//-------------------------------------------------------------------------
//发送报文
//-------------------------------------------------------------------------
sys_res nw12_TmsgSend(p_nw12 p, uint_t nFun, uint_t nAfn, buf b, uint_t nType)
{
	t_nw12_header xH;
	uint_t nCS;

	nw12_TmsgHeaderInit(p, &xH);
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
		xH.msa = p->msa;
		xH.seq.seq = p->seq.seq;
		break;
	}
	xH.c.dir = NW12_DIR_SEND;
	xH.c.fun = nFun;
	xH.seq.fin = 1;
	xH.seq.fir = 1;
	xH.afn = nAfn;
	if (nType == DLRCP_TMSG_RESPOND) {
		if (p->seq.tpv) {
			xH.seq.tpv = 1;
			buf_Push(b, &p->tp, sizeof(p->tp));
		}
	}
	xH.len1 = xH.len2 = b->len + (sizeof(t_nw12_header) - NW12_FIXHEADER_SIZE);
	nCS = cs8((uint8_t *)&xH + NW12_FIXHEADER_SIZE, (sizeof(t_nw12_header) - NW12_FIXHEADER_SIZE));
	nCS = (nCS + cs8(b->p, b->len)) & 0xFF;
	buf_PushData(b, 0x1600 | nCS, 2);
#if NW12_DEBUG_ENABLE
	nw12_DbgTx(&xH, b->p, b->len);
#endif
	return dlrcp_TmsgSend(&p->parent, &xH, sizeof(t_nw12_header), b->p, b->len, nType);
}




//-------------------------------------------------------------------------
//数据转换
//-------------------------------------------------------------------------
uint_t nw12_ConvertDa2DA(uint_t nDa)
{

	if (nDa) {
		nDa -= 1;
		return (((nDa >> 3) + 1) << 8) | BITMASK(nDa & 7);
	}
	return 0;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
uint_t nw12_ConvertDa2Map(uint_t nDA, void *pData)
{
	uint16_t *p = (uint16_t *)pData;
	uint_t j, nQty = 0, nDa;

	nDa = nDA >> 8;
	if (nDa == 0) {
		*p = 0;
		return 1;
	}
	nDa -= 1;
	for (j = 0; j < 8; j++) {
		if (nDA & BITMASK(j))
			p[nQty++] = nDa * 8 + j + 1;
	}
	return nQty;
}


//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
sys_res nw12_Transmit(p_nw12 p, p_nw12 pD)
{
	sys_res res;
	t_nw12_header xH;
	uint_t nCS;
	buf b={0};

	xH.sc1 = 0x68;
	xH.len1 = xH.len2 = p->data->len + (sizeof(t_nw12_header) - NW12_FIXHEADER_SIZE);
	xH.sc2 = 0x68;
	xH.c = p->c;
	memcpy(xH.a1,p->rcv_rtua,3);
	memcpy(xH.a2,p->rcv_terid,3);
	xH.msa = p->msa;
	xH.afn = p->afn;
	xH.seq = p->seq;
	buf_Push( b,p->data->p, p->data->len);
	nCS = cs8((uint8_t *)&xH + NW12_FIXHEADER_SIZE, (sizeof(t_nw12_header) - NW12_FIXHEADER_SIZE));
	nCS = (nCS + cs8(b->p, b->len)) & 0xFF;
	buf_PushData(b, 0x1600 | nCS, 2);
	res = dlrcp_TmsgSend(&pD->parent, &xH, sizeof(t_nw12_header), b->p, b->len, DLRCP_TMSG_RESPOND);
	buf_Release(b);
	return res;
}
//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
int nw12_RecvCheck(p_nw12 p)
{
	if ((memtest(p->rcv_rtua, 0xFF, 3) != 0) || (memtest(p->rcv_terid, 0xFF, 3) != 0)) {
		if ((memcmp(p->rtua, p->rcv_rtua, 3) != 0) || (memcmp(p->terid, p->rcv_terid, 3) != 0))
			return 0;
	}
	return 1;
}


//-------------------------------------------------------------------------
//规约处理
//-------------------------------------------------------------------------
sys_res nw12_Handler(p_nw12 p)
{

	return dlrcp_Handler(&p->parent);
}



