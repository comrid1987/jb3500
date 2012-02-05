#ifndef __METER_H__
#define __METER_H__





typedef struct {
	uint8_t	ver;
	uint8_t	type;
	uint8_t	adr[8];
	uint16_t	ct;
	uint16_t	pt;
	union {
		t_md645_para md645;
		t_mcarr_para mcarr;
	}para;
}meter_para[1];


typedef struct {










}meter_data[1];



typedef struct {
	t_uart_para	x485;
	uint8_t		port;
	uint8_t 		aUser[6];
	uint8_t		aPwd[6];
}t_md645_para;

typedef struct {
/*	//固定中继序号
	uint16 arrFixRelay[4];
	//自动中继约束条件
	uint8 arrMIU[6];		//采集终端地址
	uint16 nLine;			//线路编号
	uint16 nMeterBox;		//表箱编号
	uint16 nMultiModel;	//多表模块编号
	uint8 nPhase;			//约束条件相序号
	//载波初抄相位
	uint8 nFirstPhase;
*/
}t_mcarr_para;





#endif

