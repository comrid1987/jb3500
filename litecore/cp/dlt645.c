


//Private Defines
#define DLT645_HEADER_SIZE			10



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

sys_res dlt645_Transmit2Meter(chl c, buf bRx, const void *pAdr, const void *pBuf, uint_t nLen, uint_t nTmo)
{
	sys_res res;
	uint8_t *pHeader;

	res = chl_Send(c, pBuf, nLen);
	if (res != SYS_R_OK)
		return res;
	buf_Release(bRx);
	for (nTmo /= OS_TICK_MS; nTmo; nTmo--) {
		if (chl_RecData(c, bRx, OS_TICK_MS) != SYS_R_OK)
			continue;
		pHeader = dlt645_PacketAnalyze(bRx->p, bRx->len);
		if (pHeader == NULL)
			continue;
		buf_Remove(bRx, pHeader - bRx->p);
		if (memcmp(&bRx->p[1], pAdr, 6)) {
			buf_Remove(bRx, DLT645_HEADER_SIZE);
			continue;
		}
		byteadd(&bRx->p[10], -0x33, bRx->p[9]);
		return SYS_R_OK;
	}
	return SYS_R_TMO;
}


