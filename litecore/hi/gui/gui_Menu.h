#ifndef __GUI_MENU_H__
#define __GUI_MENU_H__


#ifdef __cplusplus
extern "C" {
#endif









#define GUI_MENU_T_SUBMENU	0
#define GUI_MENU_T_FUN		1

typedef __packed const struct {
	uint8_t type;
	uint8_t qty;
	void *p;
}


#ifdef __cplusplus
}
#endif

#endif

