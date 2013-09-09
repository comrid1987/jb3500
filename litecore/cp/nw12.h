#ifndef __NW12_H__
#define __NW12_H__


#ifdef __cplusplus
extern "C" {
#endif





//Public Defines



//AFN
#define NW12_AFN_CONFIRM			0x00
#define NW12_AFN_LINKCHECK			0x02

//FUN
#define NW12_FUN_RESET				1
#define NW12_FUN_DATA_USER			4
#define NW12_FUN_LINKCHECK			9
#define NW12_FUN_DATA_L1			10
#define NW12_FUN_DATA_L2			11

#define NW12_FUN_CONFIRM			0
#define NW12_FUN_RESPONSE			8
#define NW12_FUN_NODATA				9
#define NW12_FUN_LINKSTATE		11

//无效数据值
#define NW12_DATA_INVALID			0xFFFFFFFF

//错误代码
#define NW12_ERR_NO_ERROR			0x00	//正确,无错误
#define NW12_ERR_RELAY_FALE			0x01	//中继命令没有返回
#define NW12_ERR_INVALID			0x02	//设置内容非法
#define NW12_ERR_NO_PERMIT			0x03	//密码权限不足
#define NW12_ERR_NO_DATA			0x04	//无此项数据
#define NW12_ERR_TIME_INVALID		0x05	//命令时间失效
#define NW12_ERR_NO_ADDR			0x06	//目标地址不存在
#define NW12_ERR_CRC_FAIL			0x07	//校验失败





//Public Typedefs
//帧控制域
typedef __packed struct {
	uint8_t	fun : 4,
			fcv : 1,
			fcb_acd : 1,
			prm : 1,
			dir : 1;
}t_nw12_c;

//帧序列域
typedef __packed struct {
	uint8_t	seq : 4,
			con : 1,
			fin : 1,
			fir : 1,
			tpv : 1;
}t_nw12_seq;

//时间标签域
typedef __packed struct {
	uint8_t	pfc;
	uint8_t	tp[4];
	uint8_t	dly;
}t_nw12_tp;

//NW12规约结构
typedef struct {
	t_dlrcp		parent;
	uint8_t		msa;
 	uint8_t		rtua[3];
 	uint8_t		terid[3];
	t_nw12_c	c;
	uint8_t		afn;
	t_nw12_seq	seq;
	t_nw12_tp	tp;
	uint8_t		pw[16];	
	buf			data;
}t_nw12, *p_nw12;






//External Functions
void nw12_Init(p_nw12 p);
sys_res nw12_Handler(p_nw12 p);
void nw12_Response(p_nw12 p);

sys_res nw12_TmsgSend(p_nw12 p, uint_t nFun, uint_t nAfn, buf b, uint_t nType);
sys_res nw12_TmsgConfirm(p_nw12 p);
sys_res nw12_TmsgReject(p_nw12 p);

uint_t nw12_ConvertDa2DA(uint_t nDa);
uint_t nw12_ConvertDa2Map(uint_t nDA, void *pData);





#ifdef __cplusplus
}
#endif


#endif


