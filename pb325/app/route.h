#ifndef __APP_ROUTE_H__
#define __APP_ROUTE_H__

#ifdef __cplusplus
extern "C" {
#endif




//Public Defines
#define ECL_RT_LEVEL_QTY		7
#define ECL_RT_PATH_QTY			2
#define ECL_RT_TIME_SPAN		(3600 * 24 / 16)

#define ECL_RT_S_INVALID		0
#define ECL_RT_S_VALID			1



//Public Typedefs
typedef struct {
	uint8_t		ste : 1,				//标志
				rn : 3,					//路径号
				deep : 4;				//中继级别
	uint8_t		fail;					//失败次数
	uint16_t	asn[ECL_RT_LEVEL_QTY];	//中继序号
	uint8_t		rate[16];				//时段成功率
}t_ecl_rtnode;


typedef struct {
	t_ecl_rtnode node[4];
}t_ecl_relay;







//External Functions
void ecl_Rt_Maintain(void);
void ecl_Rt_NodeInit(t_ecl_rtnode *p);
uint_t ecl_Rt_ValidQty(t_ecl_relay *p);
void ecl_Rt_Read(uint_t nSn, const void *pAdr, t_ecl_relay *p);
void ecl_Rt_Write(uint_t nSn, t_ecl_relay *p);
void ecl_Rt_Save(uint_t nSn, t_ecl_relay *p, uint_t nRelay, const uint8_t *pRtAdr, sys_res res);
void ecl_Rt_Flush(uint_t nSn, t_ecl_relay *p, uint_t nRn, sys_res res);
void ecl_Rt_Clear(void);



#ifdef __cplusplus
}
#endif

#endif


