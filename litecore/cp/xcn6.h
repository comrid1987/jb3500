#ifndef __LCP_XCN6_H__
#define __LCP_XCN6_H__

#ifdef __cplusplus
extern "C" {
#endif










//External Functions
sys_res xcn6_MeterRead(t_gw3762 *p, buf b, const void *pAdr, uint_t nRelay, const void *pRtAdr, const void *pData, uint_t nLen);
sys_res xcn6_MeterWrite(t_gw3762 *p, buf b, const void *pAdr, uint_t nRelay, const void *pRtAdr, const void *pData, uint_t nLen);
sys_res xcn6_Broadcast(t_gw3762 *p, const void *pData, uint_t nLen);


#ifdef __cplusplus
}
#endif

#endif


