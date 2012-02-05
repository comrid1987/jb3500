#ifndef __GW3761_DATA_H__
#define __GW3761_DATA_H__

#ifdef __cplusplus
extern "C" {
#endif




//Public Typedefs
typedef struct {
	uint16_t	sn;
	uint8_t		port : 4,
				baud : 4;
	uint8_t		prtl;				//通信规约类型
	uint8_t		madr[6];			//表地址
	uint8_t		pw[6];				//表密码
	uint8_t		rate;				//费率数
	uint8_t		bits;				//有功电能示值整数位及小数位个数
	uint8_t		tadr[6];			//采集终端地址  
	uint8_t		stype : 4,			//用户小类号
				btype : 4;			//用户大类号
}t_gw3761_meterparam;


//External Functions



#ifdef __cplusplus
}
#endif

#endif








