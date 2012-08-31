


//Private Defines



//Private Typedef
typedef __packed struct {
	uint8_t	sc1;		//0x68
	uint8_t	adr[6];		//电表地址
	uint8_t	sc2;		//0x68
	uint8_t	code : 5,	//控制码
			con : 1,	//后续帧标志
			abn : 1,	//异常标志
			dir : 1;	//传送方向
	uint8_t	len;		//数据长度
}t_dlt645_header;


//Internal Functions
#if DLT645_DEBUG_ENABLE
static void dlt645_DbgOut(uint_t nType, const void *pBuf, uint_t nLen)
{
	const uint8_t *pData, *pEnd;
	char str[198];

	pData = (const uint8_t *)pBuf;
	pEnd = pData + nLen;

	if (nType)
		nLen = sprintf(str, "<645T>");
	else
		nLen = sprintf(str, "<645R>");
	while ((pData < pEnd) && (nLen < (sizeof(str) - 3)))
		nLen += sprintf(&str[nLen], " %02X", *pData++);

	dbg_trace(str);
}
#else
#define dlt645_DbgOut(...)
#endif




//External Functions
void dlt645_Packet2Buf(buf b, const void *pAdr, uint_t nC, const void *pData, uint_t nLen)
{

	uint_t i;
	const uint8_t *pBuf = pData;

	buf_PushData(b, 0x68, 1);
	buf_Push(b, pAdr, 6);
	buf_PushData(b, 0x68, 1);
	buf_PushData(b, nC, 1);
	buf_PushData(b, nLen, 1);
	//数据0x33处理
	for (i = nLen; i; i--, pBuf++)
		buf_PushData(b, *pBuf + 0x33, 1);
	buf_PushData(b, 0x1600 | cs8(b->p, b->len), 2);
}

uint8_t *dlt645_PacketAnalyze(uint8_t *p, uint_t nLen)
{
	uint8_t *pTemp;
	t_dlt645_header *pH;

	for (; ; p++, nLen--) {
		pH = (t_dlt645_header *)p;
		//不足报文头长度
		if (nLen < sizeof(t_dlt645_header))
			return NULL;
		//校验
		if ((pH->sc1 != 0x68) || (pH->sc2 != 0x68))
			continue;
		//不足长度
		if (nLen < (sizeof(t_dlt645_header) + pH->len + 2))
			continue;
		pTemp = p + sizeof(t_dlt645_header) + pH->len;
		//CS
		if (cs8(p, sizeof(t_dlt645_header) + pH->len) != *pTemp++)
			continue;
		//结束符
		if (*pTemp != 0x16)
			continue;
		break;
	}
	return p;
}

static const uint8_t dlt645_aFE[] = {0xFE, 0xFE};
#if 0
sys_res dlt645_Meter(chl c, buf b, const void *pAdr, const void *pBuf, uint_t nLen, uint_t nTmo)
{
	uint8_t *pH;

#if DLT645_DIR_CTRL
	gpio_Set(2, 0);
	chl_Send(c, dlt645_aFE, 2);
	chl_Send(c, pBuf, nLen);
	chl_Send(c, dlt645_aFE, 2);
	gpio_Set(2, 1);
#else
	chl_Send(c, dlt645_aFE, 2);
	chl_Send(c, pBuf, nLen);
#endif

	dlt645_DbgOut(1, pBuf, nLen);

	for (nTmo /= OS_TICK_MS; nTmo; nTmo--) {
		if (chl_RecData(c, b, OS_TICK_MS) != SYS_R_OK)
			continue;
		pH = dlt645_PacketAnalyze(b->p, b->len);
		if (pH == NULL)
			continue;
		buf_Remove(b, pH - b->p);

		dlt645_DbgOut(0, b->p, b->p[9] + (DLT645_HEADER_SIZE + 2));

		if (memcmp(&b->p[1], pAdr, 6)) {
			buf_Remove(b, DLT645_HEADER_SIZE);
			continue;
		}
		buf_Remove(b, DLT645_HEADER_SIZE - 2);
		byteadd(&b->p[2], -0x33, b->p[1]);
		return SYS_R_OK;
	}
	return SYS_R_TMO;
}
#else
sys_res dlt645_Meter(chl c, buf b, const void *pAdr, const void *pBuf, uint_t nLen, uint_t nTmo)
{
	uint8_t *pH;

#if DLT645_DIR_CTRL
	gpio_Set(2, 0);
	chl_Send(c, dlt645_aFE, 2);
	chl_Send(c, pBuf, nLen);
	chl_Send(c, dlt645_aFE, 2);
	gpio_Set(2, 1);
#else
	chl_Send(c, dlt645_aFE, 2);
	chl_Send(c, pBuf, nLen);
#endif

	dlt645_DbgOut(1, pBuf, nLen);

	for (nTmo /= OS_TICK_MS; nTmo; nTmo--) {
		if (chl_RecData(c, b, OS_TICK_MS) != SYS_R_OK)
			continue;
		pH = dlt645_PacketAnalyze(b->p, b->len);
		if (pH == NULL)
			continue;
		buf_Remove(b, pH - b->p);

		dlt645_DbgOut(0, b->p, b->p[9] + (DLT645_HEADER_SIZE + 2));

		if (memcmp(&b->p[1], pAdr, 6)) {
			buf_Remove(b, DLT645_HEADER_SIZE);
			continue;
		}
		buf_Remove(b, DLT645_HEADER_SIZE - 2);
		byteadd(&b->p[2], -0x33, b->p[1]);
		return SYS_R_OK;
	}
	return SYS_R_TMO;
}
#endif

