#ifndef __NRSEC3000_H__
#define __NRSEC3000_H__


#ifdef __cplusplus
extern "C" {
#endif

//Public Typedefs
typedef struct {
	uint_t		ste;
	p_dev_spi	spi;
}t_nrsec3000, *p_nrsec3000;

typedef struct {
	uint8_t cla;
	uint8_t ins;
	uint8_t p1;
	uint8_t p2;
    uint8_t p3;
}nrsec3000_cmd[1], *p_nrsec3000_cmd;

void nrsec3000_Init(void);
sys_res nrsec3000_SendCmd(p_nrsec3000 p,const void  *cmd,uint8_t cmdLen);
sys_res nrsec3000_RcvINS(p_nrsec3000 p,uint8_t *rbuf, uint8_t ins,int cnt);
uint16_t nrsec3000_RcvLEN(p_nrsec3000 p, uint8_t tbufLen,uint8_t *rbuf);
sys_res nrsec3000_RcvData(p_nrsec3000 p, uint8_t *rbuf,uint8_t rbufLen);
sys_res nrsec3000_RcvSW(p_nrsec3000 p, uint8_t *rbuf,int cnt);
sys_res nrsec3000_Sam1ImportKey(p_nrsec3000 p,uint8_t *key);
sys_res nrsec3000_Sam1ImportIV(p_nrsec3000 p,uint8_t *IV);
sys_res nrsec3000_SM1Encrypt(p_nrsec3000 p,uint8_t *buf,uint16_t bufLen,uint8_t *Enbuf);
sys_res nrsec3000_SM1Decrypt(p_nrsec3000 p,uint8_t *buf,uint16_t bufLen,uint8_t *Debuf);
sys_res nrsec3000_SM2NewKey(p_nrsec3000 p,uint8_t keyNo);
sys_res nrsec3000_SM2ExPubKey(p_nrsec3000 p,uint8_t *key,uint8_t keyNo);
sys_res nrsec3000_SM2ExPrvKey(p_nrsec3000 p,uint8_t *key,uint8_t keyNo);
sys_res nrsec3000_SM2ImPubKey(p_nrsec3000 p,uint8_t *key,uint8_t keyNo);
sys_res nrsec3000_SM2ImPrvKey(p_nrsec3000 p,uint8_t *key,uint8_t keyNo);
sys_res nrsec3000_Hash(p_nrsec3000 p,uint8_t *buf,uint16_t bufLen,uint8_t *hash);
sys_res nrsec3000_SM3(p_nrsec3000 p,uint8_t *buf,uint16_t bufLen,uint8_t *hash,uint8_t *pubkey,uint8_t *pucID,uint_t idLen);
sys_res nrsec3000_SM2Sign(p_nrsec3000 p,uint8_t *buf,uint8_t keyNo,uint8_t *sign);
sys_res nrsec3000_SM2CheckSign(p_nrsec3000 p,uint8_t *hash,uint8_t *sign,uint8_t keyNo);
sys_res nrsec3000_SM2Encrypt(p_nrsec3000 p,uint8_t *buf,uint8_t keyNo,uint8_t *Enbuf);
sys_res nrsec3000_SM2Decrypt(p_nrsec3000 p,uint8_t *buf,uint8_t keyNo,uint8_t *Debuf);
sys_res nrsec3000_SM2Credentials(p_nrsec3000 p,uint8_t type,uint8_t keyNo,uint8_t *buf,uint16_t bufLen,uint8_t *Crebuf);
sys_res nrsec3000_GetVer(p_nrsec3000 p,uint8_t *ver);
sys_res nrsec3000_GetRandom(p_nrsec3000 p,uint8_t *random,uint8_t len);
sys_res nrsec3000_SafetyCre(p_nrsec3000 p,uint8_t *buf,uint8_t *safetybuf);

#ifdef __cplusplus
}
#endif

#endif


