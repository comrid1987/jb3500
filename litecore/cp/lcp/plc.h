#ifndef __LCP_PLC_H__
#define __LCP_PLC_H__

#ifdef __cplusplus
extern "C" {
#endif



//External Typedefs
typedef const struct {
	uint8_t		uartid;
	uint8_t		rst_effect;
	t_gpio_def	rst;
#if PLC_SET_ENABLE
	uint8_t		set_effect;
	t_gpio_def	set;
#endif
}t_plc_def;






//External Functions
sys_res xcn12_Meter(t_gw3762 *p, buf b, uint_t nCode, const void *pAdr, uint_t nRelay, const void *pRtAdr, const void *pData, uint_t nLen);
sys_res xcn12_Broadcast(t_gw3762 *p, const void *pAdr, const void *pData, uint_t nLen);

sys_res xcn6_MeterRead(t_gw3762 *p, buf b, const void *pAdr, uint_t nRelay, const void *pRtAdr, const void *pData, uint_t nLen);
sys_res xcn6_MeterWrite(t_gw3762 *p, buf b, const void *pAdr, uint_t nRelay, const void *pRtAdr, const void *pData, uint_t nLen);
sys_res xcn6_Broadcast(t_gw3762 *p, const void *pAdr, const void *pData, uint_t nLen);


#ifdef __cplusplus
}
#endif

#endif



