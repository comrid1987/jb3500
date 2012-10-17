#ifndef __GDVMS_H__
#define __GDVMS_H__


//Public Defines
#define GDVMS_ADR_SIZE					6

//传送方向定义
#define GDVMS_CDIR_RECV					0x00	//主站发出
#define GDVMS_CDIR_SEND					0x01	//终端发出


//错误编码
#define GDVMS_ERR_NO_ERROR				0x00	//正确,无错误
#define GDVMS_ERR_RELAY_FALE			0x01	//中继命令没有返回
#define GDVMS_ERR_INVALID				0x02	//设置内容非法
#define GDVMS_ERR_NO_PERMIT				0x03	//密码权限不足
#define GDVMS_ERR_NO_DATA				0x04	//无此项数据
#define GDVMS_ERR_TIME_INVALID			0x05	//命令时间失效
#define GDVMS_ERR_NO_ADDR				0x11	//目标地址不存在
#define GDVMS_ERR_SEND_FAIL				0x12	//发送失败
#define GDVMS_ERR_SMS_LONG				0x13	//短消息帧太长

#define GDVMS_DATA_INVALID				0xFFFFFFFF



//Public Typedefs
//规约结构
typedef struct {
	t_dlrcp		parent;
	uint8_t		ste;
	uint8_t		adr[GDVMS_ADR_SIZE];
	uint8_t		fseq;			//帧序号
	uint8_t		code : 6,		//控制码
				abn : 1,		//异常标志
				dir : 1;		//传送方向
	buf			data;			//接收数据
}t_gdvms, *p_gdvms;






//External Functions
void gdvms_Init(p_gdvms p);
sys_res gdvms_TmsgSend(p_gdvms p, uint_t nCode, buf b, uint_t nType);
sys_res gdvms_TmsgError(p_gdvms p, uint_t nCode, uint_t nErr);
sys_res gdvms_Transmit(p_gdvms p, p_gdvms pD);
sys_res gdvms_Handler(p_gdvms p);

void gdvms_Response(p_gdvms p);



#endif

