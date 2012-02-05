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
int gui_Editor_Special(int y, t_gui_spec_editor *pTable, uint_t nQty, int nIndex, int nSelect);

void gui_Editor_IPAddressCreate(char *pStr, uint8_t *pIP);
void gui_Editor_IPAddressSet(int y, char *pStr, int nIndex, int nSelect);
void gui_Editor_IPAddressDone(char *pStr, uint8_t *pIP);

void gui_Editor_TimeCreate(char *pStr, time_t tTime);
void gui_Editor_TimeSet(int y, char *pStr, int nIndex, int nSelect);
time_t gui_Editor_TimeDone(char *pStr);

void gui_Editor_DayCreate(char *pStr, time_t tTime);
void gui_Editor_DaySet(int y, char *pStr, int nIndex, int nSelect);
void gui_Editor_DayDone(char *pStr, uint8_t *pTime);

void gui_Editor_NumberCreate(char *pStr, sint32_t nNum, uint_t nBits, int nBcd);
void gui_Editor_NumberSet(int y, char *pStr, int nIndex, int nSelect, int nSign, int nBcd);
sint64_t gui_Editor_NumberDone(char *pStr, int nBcd);
void gui_Editor_StringDcmCreate(char *pStr, int nIndex);
void gui_Editor_StringDcmSet(int y,char * pStr,int nIndex,int nSelect,int nType);

void gui_Editor_StringSet(int y, char *pStr, int nIndex, int nSelect, int nType);


#ifdef __cplusplus
}
#endif

#endif


