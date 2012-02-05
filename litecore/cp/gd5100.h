#ifndef __GD5100_H__
#define __GD5100_H__


//Public Defines

//通讯权限
#define GD5100_PWD_LOW					0x01
#define GD5100_PWD_HIGH					0x02
#define GD5100_PWD_ALL					(GD5100_PWD_LOW | GD5100_PWD_HIGH)


//错误编码
#define GD5100_ERR_NO_ERROR				0x00	//正确,无错误
#define GD5100_ERR_RELAY_FALE			0x01	//中继命令没有返回
#define GD5100_ERR_INVALUE				0x02	//设置内容非法
#define GD5100_ERR_NO_PERMIT			0x03	//密码权限不足
#define GD5100_ERR_NO_DATA				0x04	//无此项数据
#define GD5100_ERR_TIME_INVALUE			0x05	//命令时间失效
#define GD5100_ERR_NO_ADDR				0x11	//目标地址不存在
#define GD5100_ERR_SEND_FALE			0x12	//发送失败
#define GD5100_ERR_SMS_LONG				0x13	//短消息帧太长

//Public Typedefs

//接收报文结构
typedef struct {
 	uint16_t	rtua;			//地市区县码
 	uint16_t	terid;			//终端地址
 	uint16_t	ste : 6,		//状态
 				fseq : 7,		//帧序号
 				iseq : 3;		//帧内序号
 	uint8_t		code : 6,		//控制码
 				abn : 1,		//异常标志
 				dir : 1;		//传送方向
	uint8_t		tmo;			//超时处理
 	buf			data;			//接收数据
}gd5100_rmsg[1];


//发送报文结构
typedef struct {
	uint8_t	ste : 4,
			retry : 4;
	uint8_t	tmo;
	buf		data;
}t_gd5100_tmsg, *gd5100_tmsg;


//规约结构
typedef struct {
	t_dlrcp		parent;
	uint16_t	group;
 	uint16_t	rtua;
 	uint16_t	terid;
	gd5100_rmsg	rmsg;
}t_gd5100, gd5100[1];






//External Functions
void gd5100_Init(gd5100 p);
sys_res gd5100_TmsgSend(gd5100 p, uint_t nCode, buf b, uint_t nType);
sys_res gd5100_TmsgError(gd5100 p, uint_t nCode, uint_t nErr);
sys_res gd5100_Handler(gd5100 p);



#endif

