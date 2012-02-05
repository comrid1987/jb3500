#ifndef __GUI_COMBO_H__
#define __GUI_COMBO_H__


#ifdef __cplusplus
extern "C" {
#endif









#define COMBO_TYPE_MENU		0x01	//²Ëµ¥
#define COMBO_TYPE_EDITOR	0x02	

typedef struct {
	uint8_t	nType;
	uint8_t	nSubQty;
	uint16_t	nParam;
	char *	str;
	void *	pSub;
}COMBO;

#ifdef __cplusplus
}
#endif

#endif


