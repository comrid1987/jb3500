#if MODEM_ENABLE
#include <drivers/gsmmodem.h>








buffer *g_pbufModem = NULL;
SM_PARAM g_stuSmsTxBuf;

void modem_RecData()
{
	UART_RXBUF *p = &g_stuUart1RxBuf;

	if (p->nIn > p->nOut) {
		buffer_Push(g_pbufModem, &p->arrBuf[p->nOut], p->nIn - p->nOut);
	} else if (p->nIn < p->nOut) {
		buffer_Push(g_pbufModem, &p->arrBuf[p->nOut], sizeof(p->arrBuf) - p->nOut);
		buffer_Push(g_pbufModem, &p->arrBuf[0], p->nIn);
	}
	p->nOut = p->nIn;
}

int modem_FindStr(BUFFER *pbuf, int nOffset, char *pStr)
{
	int nTemp = 0;
	U8 *ptr;
	
	//添加字符串结束符以便查找
	buffer_Push(pbuf, (U8 *)&nTemp, 1);
	if ((ptr = (U8 *)rt_strstr((char *)(pbuf->p + nOffset), pStr)) != NULL) {
		nTemp = ptr - pbuf->p;
	} else {
		nTemp = -1;
	}
	pbuf->nLen -= 1;
	return nTemp;
}

int modem_SendCmd(const char *pCmd, const char *pResult, buf b)
{
	U32 i = 0;

	buf_Release(b);
	g_pbufModem = pbuf;
	uart_Send(UART_MODEM, (U8 *)pCmd, rt_strlen(pCmd));
	for (i = 50; i; --i) {
		os_dly_wait(10);
		modem_RecData();
		if (modem_FindStr(pbuf, 0, pResult) != -1) {
			break;
		}
	}
	return i;
}

void modem_BufFree(BUFFER *pbuf)
{
	g_pbufModem = NULL;
	buffer_Free(pbuf);
}

int modem_Initial()
{
	U32 i, nState = 0;
	int nResult = 0;
	BUFFER buf = {0};

	TC35I_IGT_SET();
	TC35I_PWER_DOWN();
	os_dly_wait(50);
	TC35I_PWER_ON();
	os_dly_wait(10);
	TC35I_IGT_CLR();
	os_dly_wait(10);
	TC35I_IGT_SET();
	os_dly_wait(500);
	for (i = 20; i; --i) {
		switch (nState) {
			case 0:
				//回显抑制 
				if (modem_SendCmd("ATE0\r\n", "OK", &buf)) {
					nState++;
				}
				break;
			case 1:
				//设置短信存储在SIM卡
				if (modem_SendCmd("AT+CPMS=SM,SM,SM\r\n", "OK", &buf)) {
					nState++;
				}
				break;
			case 2:
				//如果设置短信存储在SIM卡则必须发该条指令
				if (modem_SendCmd("AT^SSMSS=1\r\n", "OK", &buf)) {
					nState++;
				}
				break;
			case 3:
				//PDU模式
				if (modem_SendCmd("AT+CMGF=0\r\n", "OK", &buf)) {
					nState++;
				}
				break;
			case 4:
				//测试用
				nState++;
				break;
			case 5:
				//请求短消息中心号码
				if (modem_SendCmd("AT+CSCA?\r\n", "OK", &buf)) {
					int k;
					U8 *pNum;
					
					if ((k = modem_FindStr(&buf, 0, "+86")) != -1) {
						pNum = buf.p + k + 1;
						memcpy(g_stuSmsTxBuf.SCA, pNum, 13);
						g_stuSmsTxBuf.SCA[13] = 'F';
					}
					nResult = 1;
				}
				break;
			default:
				break;
		}
		modem_BufFree(&buf);
		if (nResult) {
			break;
		}
		os_dly_wait(10);
	}
	return nResult;
}

//可打印字符串转换为字节数据
//如："C8329BFD0E01" --> {0xC8, 0x32, 0x9B, 0xFD, 0x0E, 0x01}
//输入: pSrc - 源字符串指针
//     nSrcLength - 源字符串长度
//输出: pDst - 目标数据指针
//返回: 目标数据长度
int gsmString2Bytes(const char* pSrc, U8* pDst, int nSrcLength)
{
	int i;

	for (i = 0; i < nSrcLength; i += 2)
	{
		//输出高4位
		if ((*pSrc >= '0') && (*pSrc <= '9'))
		{
			*pDst = (*pSrc - '0') << 4;
		}
		else
		{
			*pDst = (*pSrc - 'A' + 10) << 4;
		}

		pSrc++;

		//输出低4位
		if ((*pSrc>='0') && (*pSrc<='9'))
		{
			*pDst |= *pSrc - '0';
		}
		else
		{
			*pDst |= *pSrc - 'A' + 10;
		}

		pSrc++;
		pDst++;
	}

	//返回目标数据长度
	return (nSrcLength / 2);
}

//字节数据转换为可打印字符串
//如：{0xC8, 0x32, 0x9B, 0xFD, 0x0E, 0x01} --> "C8329BFD0E01" 
//输入: pSrc - 源数据指针
//     nSrcLength - 源数据长度
//输出: pDst - 目标字符串指针
//返回: 目标字符串长度
const char tab[]="0123456789ABCDEF";	//0x0-0xf的字符查找表
int gsmBytes2String(const U8* pSrc, char* pDst, int nSrcLength)
{
	int i;

	for (i = 0; i < nSrcLength; i++)
	{
		*pDst++ = tab[*pSrc >> 4];		//输出高4位
		*pDst++ = tab[*pSrc & 0x0f];	//输出低4位
		pSrc++;
	}

	//输出字符串加个结束符
	*pDst = '\0';

	//返回目标字符串长度
	return (nSrcLength * 2);
}

//7bit编码
//输入: pSrc - 源字符串指针
//     nSrcLength - 源字符串长度
//输出: pDst - 目标编码串指针
//返回: 目标编码串长度
int gsmEncode7bit(const char* pSrc, U8* pDst, int nSrcLength)
{
	int nSrc;		//源字符串的计数值
	int nDst;		//目标编码串的计数值
	int nChar;		//当前正在处理的组内字符字节的序号，范围是0-7
	U8 nLeft = 0; 	//上一字节残余的数据

	//计数值初始化
	nSrc = 0;
	nDst = 0;

	//将源串每8个字节分为一组，压缩成7个字节
	//循环该处理过程，直至源串被处理完
	//如果分组不到8字节，也能正确处理
	while (nSrc < nSrcLength)
	{
		//取源字符串的计数值的最低3位
		nChar = nSrc & 7;

		//处理源串的每个字节
		if(nChar == 0)
		{
			//组内第一个字节，只是保存起来，待处理下一个字节时使用
			nLeft = *pSrc;
		}
		else
		{
			//组内其它字节，将其右边部分与残余数据相加，得到一个目标编码字节
			*pDst = (*pSrc << (8-nChar)) | nLeft;

			//将该字节剩下的左边部分，作为残余数据保存起来
			nLeft = *pSrc >> nChar;

			//修改目标串的指针和计数值
			pDst++;
			nDst++;
		}

		//修改源串的指针和计数值
		pSrc++;
		nSrc++;
	}

	//返回目标串长度
	return nDst;
}

//7bit解码
//输入: pSrc - 源编码串指针
//     nSrcLength - 源编码串长度
//输出: pDst - 目标字符串指针
//返回: 目标字符串长度
int gsmDecode7bit(const U8* pSrc, char* pDst, int nSrcLength)
{
	int nSrc;		//源字符串的计数值
	int nDst;		//目标解码串的计数值
	int nByte;		//当前正在处理的组内字节的序号，范围是0-6
	U8 nLeft;	//上一字节残余的数据

	//计数值初始化
	nSrc = 0;
	nDst = 0;
	
	//组内字节序号和残余数据初始化
	nByte = 0;
	nLeft = 0;

	//将源数据每7个字节分为一组，解压缩成8个字节
	//循环该处理过程，直至源数据被处理完
	//如果分组不到7字节，也能正确处理
	while(nSrc<nSrcLength)
	{
		//将源字节右边部分与残余数据相加，去掉最高位，得到一个目标解码字节
		*pDst = ((*pSrc << nByte) | nLeft) & 0x7f;

		//将该字节剩下的左边部分，作为残余数据保存起来
		nLeft = *pSrc >> (7-nByte);

		//修改目标串的指针和计数值
		pDst++;
		nDst++;

		//修改字节计数值
		nByte++;

		//到了一组的最后一个字节
		if(nByte == 7)
		{
			//额外得到一个目标解码字节
			*pDst = nLeft;

			//修改目标串的指针和计数值
			pDst++;
			nDst++;

			//组内字节序号和残余数据初始化
			nByte = 0;
			nLeft = 0;
		}

		//修改源串的指针和计数值
		pSrc++;
		nSrc++;
	}

	//输出字符串加个结束符
	*pDst = '\0';

	//返回目标串长度
	return nDst;
}

//8bit编码
//输入: pSrc - 源字符串指针
//     nSrcLength - 源字符串长度
//输出: pDst - 目标编码串指针
//返回: 目标编码串长度
int gsmEncode8bit(const char* pSrc, U8* pDst, int nSrcLength)
{
	//简单复制
	memcpy(pDst, pSrc, nSrcLength);

	return nSrcLength;
}

//8bit解码
//输入: pSrc - 源编码串指针
//     nSrcLength -  源编码串长度
//输出: pDst -  目标字符串指针
//返回: 目标字符串长度
int gsmDecode8bit(const U8* pSrc, char* pDst, int nSrcLength)
{
	//简单复制
	memcpy(pDst, pSrc, nSrcLength);

	//输出字符串加个结束符
	*(pDst + nSrcLength) = '\0';

	return nSrcLength;
}


#if GSM_UNICODE
//UCS2编码
//输入: pSrc - 源字符串指针
//     nSrcLength - 源字符串长度
//输出: pDst - 目标编码串指针
//返回: 目标编码串长度
int gsmEncodeUcs2(const char* pSrc, U8* pDst, int nSrcLength)
{
	int i, nDstLength;		//UNICODE宽字符数目
	WCHAR wchar[128];	//UNICODE串缓冲区

	//字符串-->UNICODE串
	nDstLength = MultiByteToWideChar(CP_ACP, 0, pSrc, nSrcLength, wchar, 128);

	//高低字节对调，输出
	for(i=0; i<nDstLength; i++)
	{
		*pDst++ = wchar[i] >> 8;		//先输出高位字节
		*pDst++ = wchar[i] & 0xff;		//后输出低位字节
	}

	//返回目标编码串长度
	return nDstLength * 2;
}

//UCS2解码
//输入: pSrc - 源编码串指针
//     nSrcLength -  源编码串长度
//输出: pDst -  目标字符串指针
//返回: 目标字符串长度
int gsmDecodeUcs2(const U8* pSrc, char* pDst, int nSrcLength)
{
	int nDstLength;		//UNICODE宽字符数目
	WCHAR wchar[128];	//UNICODE串缓冲区

	//高低字节对调，拼成UNICODE
	for(int i=0; i<nSrcLength/2; i++)
	{
		wchar[i] = *pSrc++ << 8;	//先高位字节
		wchar[i] |= *pSrc++;		//后低位字节
	}

	//UNICODE串-->字符串
	nDstLength = WideCharToMultiByte(CP_ACP, 0, wchar, nSrcLength/2, pDst, 160, NULL, NULL);

	//输出字符串加个结束符
	pDst[nDstLength] = '\0';

	//返回目标字符串长度
	return nDstLength;
}
#endif


//正常顺序的字符串转换为两两颠倒的字符串，若长度为奇数，补'F'凑成偶数
//如："8613851872468" --> "683158812764F8"
//输入: pSrc - 源字符串指针
//     nSrcLength - 源字符串长度
//输出: pDst - 目标字符串指针
//返回: 目标字符串长度
int gsmInvertNumbers(const char* pSrc, char* pDst, int nSrcLength)
{
	int i, nDstLength;		//目标字符串长度
	char ch;			//用于保存一个字符

	//复制串长度
	nDstLength = nSrcLength;

	//两两颠倒
	for(i=0; i<nSrcLength;i+=2)
	{
		ch = *pSrc++;		//保存先出现的字符
		*pDst++ = *pSrc++;	//复制后出现的字符
		*pDst++ = ch;		//复制先出现的字符
	}

	//源串长度是奇数吗？
	if(nSrcLength & 1)
	{
		*(pDst-2) = 'F';	//补'F'
		nDstLength++;		//目标串长度加1
	}

	//输出字符串加个结束符
	*pDst = '\0';

	//返回目标字符串长度
	return nDstLength;
}

//两两颠倒的字符串转换为正常顺序的字符串
//如："683158812764F8" --> "8613851872468"
//输入: pSrc - 源字符串指针
//     nSrcLength - 源字符串长度
//输出: pDst - 目标字符串指针
//返回: 目标字符串长度
int gsmSerializeNumbers(const char* pSrc, char* pDst, int nSrcLength)
{
	int i, nDstLength;		//目标字符串长度
	char ch;			//用于保存一个字符

	//复制串长度
	nDstLength = nSrcLength;

	//两两颠倒
	for(i=0; i<nSrcLength;i+=2)
	{
		ch = *pSrc++;		//保存先出现的字符
		*pDst++ = *pSrc++;	//复制后出现的字符
		*pDst++ = ch;		//复制先出现的字符
	}

	//最后的字符是'F'吗？
	if(*(pDst-1) == 'F')
	{
		pDst--;
		nDstLength--;		//目标字符串长度减1
	}

	//输出字符串加个结束符
	*pDst = '\0';

	//返回目标字符串长度
	return nDstLength;
}

//PDU编码，用于编制、发送短消息
//输入: pSrc - 源PDU参数指针
//输出: pDst - 目标PDU串指针
//返回: 目标PDU串长度
int gsmEncodePdu(const SM_PARAM* pSrc, char* pDst)
{
	int nLength;			//内部用的串长度
	int nDstLength;			//目标PDU串长度
	U8 pBuf[256];	//内部用的缓冲区

	//SMSC地址信息段
	nLength = rt_strlen(pSrc->SCA);	//SMSC地址字符串的长度	
	pBuf[0] = (char)((nLength & 1) == 0 ? nLength : nLength + 1) / 2 + 1;	//SMSC地址信息长度
	pBuf[1] = 0x91;		//固定: 用国际格式号码
	nDstLength = gsmBytes2String(pBuf, pDst, 2);		//转换2个字节到目标PDU串
	nDstLength += gsmInvertNumbers(pSrc->SCA, &pDst[nDstLength], nLength);	//转换SMSC号码到目标PDU串

	//TPDU段基本参数、目标地址等
	nLength = rt_strlen(pSrc->TPA);	//TP-DA地址字符串的长度
	pBuf[0] = 0x11;					//是发送短信(TP-MTI=01)，TP-VP用相对格式(TP-VPF=10)
	pBuf[1] = 0;						//TP-MR=0
	pBuf[2] = (char)nLength;			//目标地址数字个数(TP-DA地址字符串真实长度)
	pBuf[3] = 0x91;					//固定: 用国际格式号码
	nDstLength += gsmBytes2String(pBuf, &pDst[nDstLength], 4);		//转换4个字节到目标PDU串
	nDstLength += gsmInvertNumbers(pSrc->TPA, &pDst[nDstLength], nLength);	//转换TP-DA到目标PDU串

	//TPDU段协议标识、编码方式、用户信息等
	if (pSrc->TP_DCS == GSM_UCS2) {
		nLength = rt_strlen(pSrc->TP_UD);
	} else {
		nLength = pSrc->nUD;
	}
	pBuf[0] = pSrc->TP_PID;			//协议标识(TP-PID)
	pBuf[1] = pSrc->TP_DCS;			//用户信息编码方式(TP-DCS)
	pBuf[2] = 0;						//有效期(TP-VP)为5分钟
	if(pSrc->TP_DCS == GSM_7BIT)	
	{
		//7-bit编码方式
		pBuf[3] = nLength;			//编码前长度
		nLength = gsmEncode7bit(pSrc->TP_UD, &pBuf[4], nLength+1) + 4;	//转换TP-DA到目标PDU串
	}
#if GSM_UNICODE
	else if(pSrc->TP_DCS == GSM_UCS2)
	{
		//UCS2编码方式
		pBuf[3] = gsmEncodeUcs2(pSrc->TP_UD, &pBuf[4], nLength);	//转换TP-DA到目标PDU串
		nLength = pBuf[3] + 4;		//nLength等于该段数据长度
	}
#endif
	else
	{
		//8-bit编码方式
		pBuf[3] = gsmEncode8bit(pSrc->TP_UD, &pBuf[4], nLength);	//转换TP-DA到目标PDU串
		nLength = pBuf[3] + 4;		//nLength等于该段数据长度
	}
	nDstLength += gsmBytes2String(pBuf, &pDst[nDstLength], nLength);		//转换该段数据到目标PDU串

	//返回目标字符串长度
	return nDstLength;
}

//PDU解码，用于接收、阅读短消息
//输入: pSrc - 源PDU串指针
//输出: pDst - 目标PDU参数指针
//返回: 用户信息串长度
int gsmDecodePdu(const char* pSrc, SM_PARAM* pDst)
{
	int nDstLength;			//目标PDU串长度
	U8 tmp;		//内部用的临时字节变量
	U8 pBuf[256];	//内部用的缓冲区

	//SMSC地址信息段
	gsmString2Bytes(pSrc, &tmp, 2);	//取长度
	tmp = (tmp - 1) * 2;	//SMSC号码串长度
	pSrc += 4;			//指针后移，忽略了SMSC地址格式
	gsmSerializeNumbers(pSrc, pDst->SCA, tmp);	//转换SMSC号码到目标PDU串
	pSrc += tmp;		//指针后移

	//TPDU段基本参数
	gsmString2Bytes(pSrc, &tmp, 2);	//取基本参数
	pSrc += 2;		//指针后移

	//取回复号码
	gsmString2Bytes(pSrc, &tmp, 2);	//取长度
	if(tmp & 1) tmp += 1;	//调整奇偶性
	pSrc += 4;			//指针后移，忽略了回复地址(TP-RA)格式
	gsmSerializeNumbers(pSrc, pDst->TPA, tmp);	//取TP-RA号码
	pSrc += tmp;		//指针后移

	//TPDU段协议标识、编码方式、用户信息等
	gsmString2Bytes(pSrc, (U8*)&pDst->TP_PID, 2);	//取协议标识(TP-PID)
	pSrc += 2;		//指针后移
	gsmString2Bytes(pSrc, (U8*)&pDst->TP_DCS, 2);	//取编码方式(TP-DCS)
	pSrc += 2;		//指针后移
	gsmSerializeNumbers(pSrc, pDst->TP_SCTS, 14);		//服务时间戳字符串(TP_SCTS) 
	pSrc += 14;		//指针后移
	gsmString2Bytes(pSrc, &tmp, 2);	//用户信息长度(TP-UDL)
	pSrc += 2;		//指针后移
	if(pDst->TP_DCS == GSM_7BIT)
	{
		//7-bit解码
		nDstLength = gsmString2Bytes(pSrc, pBuf, tmp & 7 ? (int)tmp * 7 / 4 + 2 : (int)tmp * 7 / 4);	//格式转换
		gsmDecode7bit(pBuf, pDst->TP_UD, nDstLength);	//转换到TP-DU
		nDstLength = tmp;
	}
#if GSM_UNICODE
	else if(pDst->TP_DCS == GSM_UCS2)
	{
		//UCS2解码
		nDstLength = gsmString2Bytes(pSrc, pBuf, tmp * 2);			//格式转换
		nDstLength = gsmDecodeUcs2(pBuf, pDst->TP_UD, nDstLength);	//转换到TP-DU
	}
#endif
	else
	{
		//8-bit解码
		nDstLength = gsmString2Bytes(pSrc, pBuf, tmp * 2);			//格式转换
		nDstLength = gsmDecode8bit(pBuf, pDst->TP_UD, nDstLength);	//转换到TP-DU
	}

	//返回目标字符串长度
	return nDstLength;
}


int modem_GetSms(SM_PARAM *pMsg)
{
	U32 i, j;
	int k, nResult = 0;
	BUFFER buf = {0};

	buffer_Free(&buf);
	g_pbufModem = &buf;
	uart_Send(UART_MODEM, "AT+CMGL=4\r\n", 11);
	for (i = 60; i; --i) {
		os_dly_wait(5);
		modem_RecData();
		if ((k = modem_FindStr(&buf, 0, "+CMGL:")) != -1) {
			for (j = 100; j; --j) {
				os_dly_wait(5);
				modem_RecData();
				if ((modem_FindStr(&buf, k, "+CMGL:") != -1) || (modem_FindStr(&buf, k, "OK") != -1)) {
					break;
				}
			}
			//跳过"+CMGL:",读取序号
			k += 6;
			sscanf((char *)buf.p + k, "%d", (int *)&pMsg->nIndex);
			//找下一行
			if ((k = modem_FindStr(&buf, k, "\r\n")) != -1) {
				nResult = gsmDecodePdu((char *)buf.p + k + 2, pMsg);	//PDU串解码
			}
			break;
		} else if (modem_FindStr(&buf, 0, "OK") != -1) {
			break;
		}
	}
	modem_BufFree(&buf);
	return nResult;
}

int modem_SendSms(SM_PARAM *pSrc)
{
	int nPduLength;				//PDU串长度
	unsigned char nSmscLength;	//SMSC串长度
	char cmd[16], *pPdu;		//命令串
	BUFFER buf = {0};

	pPdu = (char *)mem_Malloc(512);
	nPduLength = gsmEncodePdu(pSrc, pPdu);	//根据PDU参数，编码PDU串
	strcat(pPdu, "\x01a");						//以Ctrl-Z结束
	gsmString2Bytes(pPdu, &nSmscLength, 2);		//取PDU串中的SMSC信息长度
	//命令中的长度，不包括SMSC信息长度，以数据字节计
	nSmscLength++;								//加上长度字节本身
	sprintf(cmd, "AT+CMGS=%d\r", nPduLength / 2 - nSmscLength);	//生成命令
	if (modem_SendCmd(cmd, "\r\n> ", &buf)) {
		//得到肯定回答，继续输出PDU串
		nPduLength = modem_SendCmd(pPdu, "OK", &buf);
	}
	mem_Free((U8 **)&pPdu);
	modem_BufFree(&buf);
	return nPduLength;
}

int modem_DeleteSms(int nIndex)
{
	int nResult = 0;
	char arrCmd[48];
	BUFFER buf = {0};

	sprintf(arrCmd, "AT+CMGD=%d\r\n", nIndex);	//生成命令
	nResult = modem_SendCmd(arrCmd, "OK", &buf);
	modem_BufFree(&buf);
	return nResult;
}
#endif


#endif


