#include <net/rtxip/rtxip.h>




//In at_System.c
extern LOCALM localm[];

//Internal Defines
//无效套接字
#define SOCKET_NULL			0
//错误处理
#define RTXIP_ERR_EN		0
//发送缓冲使能
#define RTXIP_TXBUF_ENABLE	0

#define TCP_NUMSOCKS		6
#define UDP_NUMSOCKS		2
#define TICK_INTERVAL		10



//Private Variables
static uint16_t rtxip_aLocPort[TCP_NUMSOCKS + UDP_NUMSOCKS];
static buf rtxip_xBuf[TCP_NUMSOCKS + UDP_NUMSOCKS];
static t_ip4_adr rtxip_xUdpRemIp[UDP_NUMSOCKS];
#if RTXIP_TXBUF_ENABLE
static buf rtxip_xTxBuf[TCP_NUMSOCKS];
#endif



//Public Variables


//Internal Functions
/*=============================================================================
 * 名称:			rtxip_tcp_callback
 * 描述:			TCP事件回调函数
 * 创建:			周恽峰2007年04月24日
 * 最后修订:		周恽峰2007年05月15日
 * 输入参数:		soc:		事件Socket
 *					event:	事件
 *					ptr:		指向事件附带数据的指针
 *					par:		事件附带数据的长度
 * 返回值:		响应, uint16整形
 * 特殊说明:		RVMDK 3.05 + RTL-ARM 3.05
 *=============================================================================*/
static uint16_t rtxip_tcp_callback(uint8_t s, uint8_t event, uint8_t *ptr, uint16_t par)
{

	switch (event) {
	case TCP_EVT_CONREQ:
		/* Remote peer requested connect, accept it */
		return 1;
	case TCP_EVT_CONNECT:
		/* The TCP socket is connected */
		return 1;
	case TCP_EVT_DATA:
		/* Receive TCP package */
		buf_Push(rtxip_xBuf[s - 1], ptr, par);
		break;
	case TCP_EVT_CLOSE:
	case TCP_EVT_ABORT:
	case TCP_EVT_ACK:
	default:
		break;
	}
	return 0;
}

/*=============================================================================
 * 名称:			rtxip_udp_callback
 * 描述:			Udp事件回调函数
 * 创建:			周恽峰2008年03月24日
 * 最后修订:		周恽峰2008年03月27日
 * 输入参数:		soc:		事件Socket
 *					remip:	远程主机地址
 *					remport:	远程主机端口
 *					buf:		指向事件附带数据的指针
 *					len:		事件附带数据的长度
 * 返回值:		响应, uint16整形
 * 特殊说明:		RVMDK 3.05 + RTL-ARM 3.05
 *=============================================================================*/
static uint16_t rtxip_udp_callback (uint8_t s, uint8_t *remip, uint16_t remport, uint8_t *ptr, uint16_t par)
{
	t_ip4_adr *pIp = &rtxip_xUdpRemIp[s - 1];

	memcpy(pIp->ip, remip, sizeof(pIp->ip));
	pIp->port = remport;
	buf_Push(rtxip_xBuf[s + TCP_NUMSOCKS - 1], ptr, par);
	return 1;
}

static void rtxip_ping_cback (U8 event)
{

	switch (event) {
	case ICMP_EVT_SUCCESS:
		break;
	case ICMP_EVT_TIMEOUT:
		/* Timeout, try again. */
		break;
	}
}

static void net_GetIpAddr(int nType, void *pIp, void *pMask, void *pGetway)
{

	memcpy(pIp, localm[nType].IpAdr, IP_ADRLEN);
	memcpy(pMask, localm[nType].NetMask, IP_ADRLEN);
	memcpy(pGetway, localm[nType].DefGW, IP_ADRLEN);
}

static void net_SetIpAddr(int nType, const void *pIp, const void *pMask, const void *pGetway)
{

	memcpy(localm[nType].IpAdr, pIp, IP_ADRLEN);
	memcpy(localm[nType].NetMask, pMask, IP_ADRLEN);
	memcpy(localm[nType].DefGW, pGetway, IP_ADRLEN);
}



#if RTXIP_TXBUF_ENABLE
void rtxip_TxBuf()
{
	int s;
	uint_t nLen;
	uint8_t *pTemp;

	for (s = 1; s <= TCP_NUMSOCKS; s++) {
		nLen = rtxip_xTxBuf[s - 1]->len;
		if (nLen) {
			if (tcp_check_send(s) == __TRUE) {
				nLen = MIN(tcp_max_dsize(s), nLen);
				pTemp = tcp_get_buf(nLen);
				if (pTemp != NULL) {
					rt_memcpy(pTemp, rtxip_xTxBuf[s - 1]->p, nLen);
					if (tcp_send(s, pTemp, nLen) == __TRUE)
						buf_Remove(rtxip_xTxBuf[s - 1], nLen);
				}
			}
		}
	}
}
#endif

void rtxip_Handler(void *args)
{
	static uint_t nCnt = 0;

#if RTXIP_TXBUF_ENABLE
	rtxip_TxBuf();
#endif
	//TCP协议栈处理
#if ETH_INT_ENABLE == 0
	poll_ethernet();
#endif
	main_TcpNet();
	//TCP协议栈时间Tick
	if ((nCnt % (TICK_INTERVAL / OS_TICK_MS)) == 0)
		timer_tick();
#if TCPPS_ETH_ENABLE
	if ((nCnt % (2000 / OS_TICK_MS)) == 0)
		icmp_ping(localm[NETIF_ETH].IpAdr, rtxip_ping_cback);
#endif
	nCnt += 1;
}



//System Functions
void sys_error(ERROR_CODE code)
{
#if RTXIP_ERR_EN
	/* This function is called when a fatal error is encountered. The normal */
	/* program execution is not possible anymore. Add your crytical error   .*/
	/* handler code here.                                                    */
	
	switch (code) {
	/* Out of memory. */
	case ERR_MEM_ALLOC:
		break;
	/* Trying to release non existing memory block. */
	case ERR_MEM_FREE:
		break;
	/* Memory Link pointer is Corrupted. */
	/* More data written than the size of allocated mem block. */
	case ERR_MEM_CORRUPT:
		break;
	/* Out of UDP Sockets. */
	case ERR_UDP_ALLOC:
		break;
	/* Out of TCP Sockets. */
	case ERR_TCP_ALLOC:
		break;
	/* TCP State machine in undefined state. */
	case ERR_TCP_STATE:
		break;
	}
#endif
}


//External Functions
void net_Init()
{

	bzero(rtxip_aLocPort, sizeof(rtxip_aLocPort));
	bzero(rtxip_xBuf, sizeof(rtxip_xBuf));
	bzero(rtxip_xUdpRemIp, sizeof(rtxip_xUdpRemIp));
#if RTXIP_TXBUF_ENABLE
	bzero(rtxip_xTxBuf, sizeof(rtxip_xTxBuf));
#endif
	init_TcpNet();
}


int net_Socket(int domain, int type, int protocol)
{
	int s;

	//分配Socket,tout = 0xFFFF无超时控制
	switch (type) {
	case SOCK_DGRAM:
		if ((s = udp_get_socket(0, UDP_OPT_SEND_CS | UDP_OPT_CHK_CS, rtxip_udp_callback)) != SOCKET_NULL)
			s += TCP_NUMSOCKS;
		break;
	default:
		s = tcp_get_socket(TCP_TYPE_CLIENT_SERVER, 0, 0xFFFF, rtxip_tcp_callback);
		break;
	}
	return s;
}

int net_Close(int s)
{

	if (s > TCP_NUMSOCKS) {
		s -= TCP_NUMSOCKS;
		udp_close(s);
		udp_release_socket(s);
	} else {
		tcp_abort(s);
		tcp_release_socket(s);
	}
	return 0;
}

int net_Bind(int s, const struct sockaddr *name, int namelen)
{
	struct sockaddr_in *pTo = (struct sockaddr_in *)name;

	rtxip_aLocPort[s - 1] = pTo->sin_port;
	return 0;
}

int net_Listen(int s, int backlog)
{

	if (s > TCP_NUMSOCKS) {
		if (udp_open(s - TCP_NUMSOCKS, rtxip_aLocPort[s - 1]) == __TRUE)
			return 0;
	} else if (tcp_listen(s, rtxip_aLocPort[s - 1]) == __TRUE)
		return 0;
	return -1;
}

int net_Connect(int s, struct sockaddr *name, int namelen)
{
	struct sockaddr_in *pTo = (struct sockaddr_in *)name;
	t_ip4_adr *pIp;

	if (s > TCP_NUMSOCKS) {
		pIp = &rtxip_xUdpRemIp[s - TCP_NUMSOCKS - 1];
		memcpy(&pIp->ip[0], &pTo->sin_addr.s_addr, sizeof(pIp->ip));
		memcpy(&pIp->port, &pTo->sin_port, sizeof(pIp->port));
		if (udp_open(s - TCP_NUMSOCKS, 0) != __TRUE)
			return -1;
	} else if (tcp_connect(s, (uint8_t *)&pTo->sin_addr.s_addr, pTo->sin_port, 0) != __TRUE)
		return -1;
	return 0;
}

int net_IsConnect(int s)
{
	int nSte;

	if (s > TCP_NUMSOCKS)
		//UDP
		return 0;
	//TCP
	nSte = tcp_get_state(s);
	if (nSte == TCP_STATE_CONNECT)
		return 0;
	return (nSte + 1);
}

int net_Send(int s, uint8_t *pBuf, uint_t nLen)
{
	uint_t nMaxLen = 0, nTmo;
	uint8_t *pTemp, *pEnd = pBuf + nLen;
	t_ip4_adr *pIp;

	if (s > TCP_NUMSOCKS) {
		//Udp Send
		s -= TCP_NUMSOCKS;
		pIp = &rtxip_xUdpRemIp[s - 1];
		for (; pBuf < pEnd; pBuf += nMaxLen) {
			nMaxLen = 512;
			if (nMaxLen > (pEnd - pBuf))
				nMaxLen = pEnd - pBuf;
			pTemp = udp_get_buf(nMaxLen);
			memcpy(pTemp, pBuf, nMaxLen);
			//发送失败
			if (udp_send(s, pIp->ip, pIp->port, pTemp, nMaxLen) != __TRUE)
				break;
		}
	} else {
		//Tcp Send
		for (; pBuf < pEnd; pBuf += nMaxLen) {
			//检查是否可以发送数据
#if RTXIP_TXBUF_ENABLE
			if (tcp_check_send(s) != __TRUE) {
				buf_Push(rtxip_xTxBuf[s - 1], pBuf, pEnd - pBuf);
				pBuf = pEnd;
				break;
			}
#else
			nTmo = 30000 / OS_TICK_MS;
			for (; (tcp_check_send(s) != __TRUE) && nTmo; nTmo--)
				os_thd_Slp1Tick();
			if (nTmo == 0)
				break;
#endif
			nMaxLen = MIN(tcp_max_dsize(s), pEnd - pBuf);
			pTemp = tcp_get_buf(nMaxLen);
			if (pTemp == NULL)
				break;
			memcpy(pTemp, pBuf, nMaxLen);
 			if (tcp_send(s, pTemp, nMaxLen) != __TRUE)
				break;
		}
	}
	if (pBuf < pEnd)
		return nLen - (pEnd - pBuf);
	return nLen;
}

int net_Recv(int s, char *pBuf, uint_t nMaxLen)
{

	s -= 1;
	nMaxLen = MIN(nMaxLen, rtxip_xBuf[s]->len);
	if (nMaxLen) {
		memcpy(pBuf, rtxip_xBuf[s]->p, nMaxLen);
		buf_Cut(rtxip_xBuf[s], 0, nMaxLen);
	}
	return nMaxLen;
}

void net_GetIpPPP(void *pIp, void *pMask, void *pGetway)
{

	net_GetIpAddr(NETIF_PPP, pIp, pMask, pGetway);
}

void net_GetIpETH(void *pIp, void *pMask, void *pGetway)
{

	net_GetIpAddr(NETIF_ETH, pIp, pMask, pGetway);
}


void net_SetIpETH(const void *pIp, const void *pMask, const void *pGetway)
{

	net_SetIpAddr(NETIF_ETH, pIp, pMask, pGetway);
}



