#ifndef __GW3762_H__
#define __GW3762_H__


#ifdef __cplusplus
extern "C" {
#endif

//Public Defines

//GW376.2 AFN Defines
#define GW3762_AFN_CONFIRM			0x00
#define GW3762_AFN_RESET			0x01
#define GW3762_AFN_TRANSMIT			0x02
#define GW3762_AFN_DATA_FETCH		0x03
#define GW3762_AFN_DATA_SET			0x05
#define GW3762_AFN_REPORT			0x06
#define GW3762_AFN_ROUTE_FETCH		0x10
#define GW3762_AFN_ROUTE_SET		0x11
#define GW3762_AFN_ROUTE_CTRL		0x12
#define GW3762_AFN_ROUTE_TRANSMIT	0x13
#define GW3762_AFN_ROUTE_REQUEST	0x14
#define GW3762_AFN_AUTOREPORT		0xF0


//Modem Type Defines
#define GW3762_T_XC_GW				0
#define GW3762_T_XC_RT				1
#define GW3762_T_XC_GD				2

#define GW3762_T_ES_RT				4
#define GW3762_T_ES_38				5

#define GW3762_T_TOPCOM				7
#define GW3762_T_BOST				8
#define GW3762_T_RISECOM			9
#define GW3762_T_LEAGUERME			10
#define GW3762_T_MIARTECH			11



//Public Typedefs





//External Functions
sys_res gw3762_Analyze(t_plc *p);

sys_res gw3762_Broadcast(t_plc *p, const void *pAdr, const void *pData, uint_t nLen);
sys_res gw3762_MeterRead(t_plc *p, const void *pAdr, uint_t nRelay, const void *pRtAdr, const void *pData, uint_t nLen);
sys_res gw3762_MeterRT(t_plc *p, const void *pAdr, const void *pData, uint_t nLen);

sys_res gw3762_Confirm(t_plc *p, uint_t nFlag, uint_t nTmo);
sys_res gw3762_HwReset(t_plc *p, uint_t nTmo);
sys_res gw3762_ParaReset(t_plc *p, uint_t nTmo);
sys_res gw3762_InfoGet(t_plc *p, uint_t nTmo);
sys_res gw3762_ModAdrSet(t_plc *p, uint_t nTmo);
sys_res gw3762_SubAdrQty(t_plc *p, uint16_t *pQty, uint_t nTmo);
sys_res gw3762_SubAdrRead(t_plc *p, uint_t nSn, uint16_t *pQty, uint8_t *pAdr, uint_t nTmo);
sys_res gw3762_StateGet(t_plc *p, uint_t nTmo);
sys_res gw3762_SubAdrAdd(t_plc *p, uint_t nSn, const void *pAdr, uint_t nPrtl, uint_t nTmo);
sys_res gw3762_SubAdrDelete(t_plc *p, const void *pAdr, uint_t nTmo);
sys_res gw3762_ModeSet(t_plc *p, uint_t nMode, uint_t nTmo);
sys_res gw3762_RtCtrl(t_plc *p, uint_t nDT, uint_t nTmo);
sys_res gw3762_RequestAnswer(t_plc *p, uint_t nPhase, const void *pAdr, uint_t nIsRead, const void *pData, uint_t nLen);

sys_res gw3762_Transmit(t_plc *p, buf b, const void *pData, uint_t nLen);

sys_res gw3762_Es_ModeGet(t_plc *p, uint_t *pMode, uint_t nTmo);
sys_res gw3762_Es_ModeSet(t_plc *p, uint_t nMode, uint_t nTmo);


#ifdef __cplusplus
}
#endif


#endif

