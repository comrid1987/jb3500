#ifndef __GWVMS_H__
#define __GWVMS_H__


//Public Defines
#define GWVMS_ADR_SIZE					17

//传送方向定义
#define GWVMS_CDIR_RECV					0x00	//主站发出
#define GWVMS_CDIR_SEND					0x01	//终端发出


//错误编码
#define GWVMS_ERR_NO_ERROR				0x00	//正确,无错误
#define GWVMS_ERR_RELAY_FALE			0x01	//中继命令没有返回
#define GWVMS_ERR_INVALID				0x02	//设置内容非法
#define GWVMS_ERR_NO_PERMIT				0x03	//密码权限不足
#define GWVMS_ERR_NO_DATA				0x04	//无此项数据
#define GWVMS_ERR_TIME_INVALID			0x05	//命令时间失效
#define GWVMS_ERR_NO_ADDR				0x11	//目标地址不存在
#define GWVMS_ERR_SEND_FAIL				0x12	//发送失败
#define GWVMS_ERR_SMS_LONG				0x13	//短消息帧太长

#define GWVMS_DATA_INVALID				0xFFFFFFFF



//Public Typedefs
//规约结构
typedef struct {
	t_dlrcp		parent;
	uint8_t		ste;
	uint8_t		adr[GWVMS_ADR_SIZE];
	uint8_t		fseq;			//帧序号
	uint8_t		ftype;			//帧类型
	uint8_t		ptype;			//报文类型
	buf			data;			//接收数据
}t_gwvms, *p_gwvms;


//External Functions
void gwvms_Init(p_gwvms p);
sys_res gwvms_TmsgSend(p_gwvms p, uint_t nFType,uint_t nPType, buf b, uint_t nType);
sys_res gwvms_Handler(p_gwvms p);

void gwvms_Response(p_gwvms p);



#endif

