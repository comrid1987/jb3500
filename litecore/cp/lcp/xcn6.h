#ifndef __LCP_XCN6_H__
#define __LCP_XCN6_H__

#ifdef __cplusplus
extern "C" {
#endif










//External Functions
sys_res xcn12_Meter(t_plc *p, buf b, uint_t nCode, const void *pAdr, uint_t nRelay, const void *pRtAdr, const void *pData, uint_t nLen);
sys_res xcn12_Broadcast(t_plc *p, const void *pAdr, const void *pData, uint_t nLen);

sys_res xcn6_MeterRead(t_plc *p, buf b, const void *pAdr, uint_t nRelay, const void *pRtAdr, const void *pData, uint_t nLen);
sys_res xcn6_MeterWrite(t_plc *p, buf b, const void *pAdr, uint_t nRelay, const void *pRtAdr, const void *pData, uint_t nLen);
sys_res xcn6_Broadcast(t_plc *p, const void *pAdr, const void *pData, uint_t nLen);


#ifdef __cplusplus
}
#endif

#endif


