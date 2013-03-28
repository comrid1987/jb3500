#ifndef __GUI_EDITOR_H__
#define __GUI_EDITOR_H__


#ifdef __cplusplus
extern "C" {
#endif





//Public Typedefs
typedef const struct {
	uint_t		val;
	const char *str;
}t_gui_spec_editor;





//External Functions
int gui_Editor_Special(const char *pInfo, t_gui_spec_editor *pTable, uint_t nQty, int nIndex, int nSelect);

void gui_Editor_IPAddressCreate(char *pStr, uint8_t *pIP);
void gui_Editor_IPAddressSet(const char *pInfo, char *pStr, int nIndex, int nSelect);
void gui_Editor_IPAddressDone(char *pStr, uint8_t *pIP);

void gui_Editor_TimeCreate(char *pStr, time_t tTime);
void gui_Editor_TimeSet(const char *pInfo, char *pStr, int nIndex, int nSelect);
time_t gui_Editor_TimeDone(char *pStr);

void gui_Editor_DayCreate(char *pStr, int nDay, time_t tTime);
void gui_Editor_DaySet(const char *pInfo, char *pStr, int nIndex, int nSelect);
void gui_Editor_DayDone(char *pStr, uint8_t *pTime);

void gui_Editor_NumberCreate(char *pStr, sint32_t nNum, uint_t nBits, int nHex);
void gui_Editor_NumberSet(const char *pInfo, char *pStr, int nIndex, int nSelect, int nSign, int nHex);
sint64_t gui_Editor_NumberDone(char *pStr, int nHex);

void gui_Editor_StringSet(const char *pInfo, char *pStr, int nIndex, int nSelect, int nType);


#ifdef __cplusplus
}
#endif

#endif


