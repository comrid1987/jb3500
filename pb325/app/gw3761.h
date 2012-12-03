#ifndef __APP_GW3761_H__
#define __APP_GW3761_H__

#ifdef __cplusplus
extern "C" {
#endif












//External Functions
int gw3761_ResponseReset(p_gw3761 p);
int gw3761_ResponseSetParam(p_gw3761 p);
int gw3761_ResponseCtrlCmd(p_gw3761 p, u_word2 *pDu, uint8_t **ppData);
int gw3761_ResponseGetConfig(p_gw3761 p, buf b, u_word2 *pDu);
int gw3761_ResponseGetParam(p_gw3761 p, buf b, u_word2 *pDu, uint8_t **ppData);
int gw3761_ResponseData1(p_gw3761 p);
int gw3761_ResponseData2(p_gw3761 p);
int gw3761_ResponseData3(p_gw3761 p, buf b, u_word2 *pDu, uint8_t **ppData);
int gw3761_ResponseFileTrans(p_gw3761 p, buf b, u_word2 *pDu, uint8_t **ppData);
int gw3761_ResponseTransmit(p_gw3761 p, buf b, u_word2 *pDu, uint8_t **ppData);

int gw3761_ResponseAuthority(p_gw3761 p, buf b, u_word2 *pDu, uint8_t **ppData);



#ifdef __cplusplus
}
#endif

#endif


