#ifndef __RC522_H__
#define __RC522_H__




#ifdef __cplusplus
extern "C" {
#endif






#define MF_S_IDEL			0
#define MF_S_ACTIVE			1
#define MF_S_BUSY			2
typedef struct {
	p_dev_spi	p;
	uint8_t		ste : 7,
				block : 1;
	uint16_t	ctype;
	uint32_t	cid;
}mifare[1];




//External Functions
void mf_InitGpio(void);
void mf_Init(mifare mf);
void mf_Release(mifare mf);
sys_res mf_Get(mifare mf, uint_t nId, uint_t nTmo);
sys_res mf_FindCard(mifare mf, uint_t nOp);
sys_res mf_SelectCard(mifare mf);
sys_res mf_PcdReset(mifare mf);
sys_res mf_SelectApp(mifare mf, uint_t nParam);
sys_res mf_Send(mifare mf, uint_t nIns, uint_t nP1, uint_t nP2, void *pBuf, uint_t nIn, uint_t nOut);




#ifdef __cplusplus
}
#endif

#endif

