#include <string.h>


//Private Defines
#define OS_QUE_S_IDLE			0
#define OS_QUE_S_ALLOC			1
#define OS_QUE_S_BUSY			2


//Private Variables
static t_os_que queue[OS_QUEUE_QTY];

//------------------------------------------------------------------
//Function Name  :os_que_Alloc
//Description    :   找出 空闲的消息队列空间，并返回  其地址
//Input            : None
//Output         :  None
//Return          : -que:空闲的消息队列空间的地址
//                   -NULL: 没找到
//------------------------------------------------------------------
static os_que os_que_Alloc()
{
	os_que p;

	for (p = queue; p < ARR_ENDADR(queue); p++)
		if (p->ste == OS_QUE_S_IDLE) {
			p->ste = OS_QUE_S_ALLOC;
			return p;
		}
	return NULL;
}


//----------------------------------------------------------------------------
//Function Name  :os_que_Release
//Description    :   释放p的内存空间
//Input            : p:从函数os_que_Find()返回的空闲 的队列空间的地址
//Output         :  None
//Return          : None
//----------------------------------------------------------------------------
void os_que_Release(os_que p)
{

	if (p->ste != OS_QUE_S_IDLE) {
		if (p->evt & QUE_EVT_BUF_MASK)
			buf_Release(p->data->b);
		bzero(p, sizeof(t_os_que));
	}
}

//----------------------------------------------------------------------------
//Function Name  :os_que_Init
//Description    :   初始化消息队列空间
//Input            : None
//Output         :  None
//Return          : None
//----------------------------------------------------------------------------
void os_que_Init()
{

	bzero(queue, sizeof(queue));
}


//----------------------------------------------------------------------------
//Function Name  :os_que_Send
//Description    :    向特定的队列空间发送数据
//Input            : -nEvt:特定的事件
//                   -*pDev:特定的设备
//                   -*pData:待发送的数据
//                   -nLen:数据长度
//                   - nTmo:超时时间
//Output         :  
//Return          : -SYS_R_FULL:消息队列空间已满
//                   -SYS_R_TMO:超时
//----------------------------------------------------------------------------
sys_res os_que_Send(uint_t nEvt, void *pDev, void *pData, uint_t nLen, int nTmo)
{
	os_que p;

	for (nTmo /= OS_TICK_MS; ; nTmo--) {
		if ((p = os_que_Alloc()) != NULL) {
			p->evt = nEvt;
			p->dev = pDev;
			p->tmo = nTmo;
			if (p->evt & QUE_EVT_BUF_MASK)
				buf_Push(p->data->b, pData, nLen);
			else
				p->data->val = *(uint_t *)pData;
			return SYS_R_OK;
		}
		if (nTmo == 0)
			break;
		os_thd_Slp1Tick();
	}
	return SYS_R_TMO;
}

//----------------------------------------------------------------------------
//Function Name  :os_que_IsrSend
//Description    :    利用中断向特定的队列空间发送数据
//Input            : -nEvt:特定的事件
//                   -*pDev:特定的设备
//                   -*pData:待发送的数据
//                   -nLen:数据长度      
//Output         :  
//Return          : -SYS_R_FULL:消息队列空间已满
//                   -SYS_R_TMO:超时
//                   -OS_R_OK:成功
//----------------------------------------------------------------------------
sys_res os_que_IsrSend(uint_t nEvt, void *pDev, void *pData, uint_t nLen, int nTmo)
{
	os_que p;

	if ((p = os_que_Alloc()) != NULL) {
		p->evt = nEvt;
		p->dev = pDev;
		p->tmo = nTmo / OS_TICK_MS;
		if (p->evt & QUE_EVT_BUF_MASK)
			buf_Push(p->data->b, pData, nLen);
		else
			p->data->val = *(uint_t *)pData;
		return SYS_R_OK;
	}
	return SYS_R_TMO;
}


//----------------------------------------------------------------------------
//Function Name  :os_que_Wait
//Description    :    等待特定事件和特定设备
//Input            : -nEvt:特定的事件
//                   -*pDev:特定的设备
//                   - nTmo:超时时间
//Output         :  None
//Return          : -p:特定的事件和特定的设备的指针地址
//                   -NULL:超时时间到，等待失败
//----------------------------------------------------------------------------
os_que os_que_Wait(uint_t nEvt, void *pDev, int nTmo)
{
	os_que p;

	for (nTmo /= OS_TICK_MS; ; nTmo--) {
		for (p = queue; p < ARR_ENDADR(queue); p++) {
			if (p->ste == OS_QUE_S_ALLOC) {
				if ((p->evt == nEvt) && (p->dev == pDev)) {
					p->ste = OS_QUE_S_BUSY;
					return p;
				}
			}
		}
		if (nTmo == 0)
			break;
		os_thd_Slp1Tick();
	}
	return NULL;
}

//----------------------------------------------------------------------------
//Function Name  :os_que_Idle
//Description    :   使消息队列空间变为空闲状态
//Input            : None
//Output         :  None
//Return          : None
//----------------------------------------------------------------------------
void os_que_Idle()
{
	os_que p;

	for (p = queue; p < ARR_ENDADR(queue); p++) {
		if (p->ste == OS_QUE_S_ALLOC) {
			if (--p->tmo == 0)
				os_que_Release(p);
		}
	}
}

