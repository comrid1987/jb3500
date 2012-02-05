#if GUI_ENABLE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>



//-------------------------------------------------------------------------
//数字转换
//-------------------------------------------------------------------------
const char tbl_guiEditorSetNum[] = {'.', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
char gui_EditorCompnet_Number(char ch, int nStart, int nEnd, int nSelect)
{
	int i;

	for (i = 0; i < ARR_SIZE(tbl_guiEditorSetNum); i++)
		if (ch == tbl_guiEditorSetNum[i])
			break;
	i = cycle(i, nStart, nEnd, nSelect);
	return tbl_guiEditorSetNum[i];
}

//-------------------------------------------------------------------------
//特殊设置
//-------------------------------------------------------------------------
int gui_Editor_Special(int y, t_gui_spec_editor *pTable, uint_t nQty, int nIndex, int nSelect)
{

	nIndex = cycle(nIndex, 0, nQty - 1, nSelect);
	gui_DrawString_Mixed_Align(0, y, (char *)pTable[nIndex].str, COLOR_BLACK, GUI_ALIGN_CENTER);
	return nIndex;
}

//-------------------------------------------------------------------------
//IP地址设置
//-------------------------------------------------------------------------
void gui_Editor_IPAddressCreate(char *pStr, uint8_t *pIP)
{

	//转换IP地址为规格字符串
	sprintf(pStr, "%03d.%03d.%03d.%03d", pIP[0], pIP[1], pIP[2], pIP[3]);
}

void gui_Editor_IPAddressSet(int y, char *pStr, int nIndex, int nSelect)
{
	int x;

	//整形
	pStr[3] = pStr[7] = pStr[11] = '.';
	if (nIndex > 8)
		nIndex += 3;
	else if (nIndex > 5)
		nIndex += 2;
	else if (nIndex > 2)
		nIndex += 1;
	//数字滚动
	switch (nIndex) {
	case 0:
	case 4:
	case 8:
	case 12:
		pStr[nIndex] = gui_EditorCompnet_Number(pStr[nIndex], 1, 3, nSelect);
		break;
	default:
		pStr[nIndex] = gui_EditorCompnet_Number(pStr[nIndex], 1, 10, nSelect);
		break;
	}
	//绘制IP地址及光标
	x = gui_DrawString_Mixed_Align(0, y, pStr, COLOR_BLACK, GUI_ALIGN_CENTER) + ASC_WIDTH * nIndex;
	gui_DrawHLine(x, y + HZ_HEIGHT, x + 6, COLOR_BLACK);
}

void gui_Editor_IPAddressDone(char *pStr, uint8_t *pIP)
{
	uint_t i;

	//整形
	pStr[3] = 0;
	pStr[7] = 0;
	pStr[11] = 0;
	//转换规格字符串为IP地址
	for (i = 0; i < 4; ++i)
		pIP[i] = atoi(&pStr[i << 2]);
}


//-------------------------------------------------------------------------
//时间设置
//-------------------------------------------------------------------------
void gui_Editor_TimeCreate(char *pStr, time_t tTime)
{
	struct tm tmTime;

	localtime_r(&tTime, &tmTime);
	//转换time_t为规格字符串
	strftime(pStr, 20, "%Y-%m-%d %H:%M:%S", &tmTime);
}

void gui_Editor_TimeSet(int y, char *pStr, int nIndex, int nSelect)
{
	int x;

	//整形
	pStr[4] = pStr[7] = '-';
	pStr[10] = ' ';
	pStr[13] = pStr[16] = ':';
	if (nIndex > 11)
		nIndex += 5;
	else if (nIndex > 9)
		nIndex += 4;
	else if (nIndex > 7)
		nIndex += 3;
	else if (nIndex > 5)
		nIndex += 2;
	else if (nIndex > 3)
		nIndex += 1;
	//数字滚动
	switch (nIndex) {
	case 5:
		//month(0-1)
		pStr[nIndex] = gui_EditorCompnet_Number(pStr[nIndex], 1, 2, nSelect);
		break;
	case 8:
		//mday(0-3)
		pStr[nIndex] = gui_EditorCompnet_Number(pStr[nIndex], 1, 4, nSelect);
		break;
	case 11:
		//hour(0-2)
		pStr[nIndex] = gui_EditorCompnet_Number(pStr[nIndex], 1, 3, nSelect);
		break;
	case 14:
	case 17:
		//min(0-5)
		pStr[nIndex] = gui_EditorCompnet_Number(pStr[nIndex], 1, 6, nSelect);
		break;
	default:
		pStr[nIndex] = gui_EditorCompnet_Number(pStr[nIndex], 1, 10, nSelect);
		break;
	}
	//绘制时间及光标
	x = gui_DrawString_Mixed_Align(0, y, pStr, COLOR_BLACK, GUI_ALIGN_CENTER) + ASC_WIDTH * nIndex;
	gui_DrawHLine(x, y + HZ_HEIGHT, x + 6, COLOR_BLACK);
}

time_t gui_Editor_TimeDone(char *pStr)
{
	struct tm tmTime;

	//整形
	pStr[4] = pStr[7] = pStr[10] = pStr[13] = pStr[16] = 0;
	//转换规格字符串为time_t
	tmTime.tm_year = atoi(&pStr[0]) - 1900;
	tmTime.tm_mon = atoi(&pStr[5]) - 1;
	tmTime.tm_mday = atoi(&pStr[8]);
	tmTime.tm_hour = atoi(&pStr[11]);
	tmTime.tm_min = atoi(&pStr[14]);
	tmTime.tm_sec = atoi(&pStr[17]);
	return mktime(&tmTime);
}

//-------------------------------------------------------------------------
//日期设置
//-------------------------------------------------------------------------
void gui_Editor_DayCreate(char *pStr, time_t tTime)
{
	uint8_t aTime[3];

	day4timet(tTime, -1, aTime, 0);
	//转换为规格字符串
	sprintf(pStr, "%02d-%02d-%02d", aTime[2], aTime[1], aTime[0]);
}

void gui_Editor_DaySet(int y, char *pStr, int nIndex, int nSelect)
{
	int x;

	if (nIndex > 3)
		nIndex += 2;
	else if (nIndex > 1)
		nIndex += 1;
	pStr[nIndex] = gui_EditorCompnet_Number(pStr[nIndex], 1, 10, nSelect);
	//绘制数字及光标
	x = gui_DrawString_Mixed_Align(0, y, pStr, COLOR_BLACK, GUI_ALIGN_CENTER) + ASC_WIDTH * nIndex;
	gui_DrawHLine(x, y + HZ_HEIGHT, x + 6, COLOR_BLACK);
}

void gui_Editor_DayDone(char *pStr, uint8_t *pTime)
{
	uint_t i;

	//整形
	pStr[2] = 0;
	pStr[5] = 0;
	//转换规格字符串为IP地址
	for (i = 0; i < 3; ++i)
		pTime[i] = bin2bcd8(atoi(&pStr[i * 3]));
	reverse(pTime, 3);
}

//-------------------------------------------------------------------------
//数字设置
//-------------------------------------------------------------------------
void gui_Editor_NumberCreate(char *pStr, sint32_t nNum, uint_t nBits, int nBcd)
{
	char strNum[32];
	uint_t nTemp;

	//转换数字为规格字符串
	rt_memset(pStr, '0', nBits);
	*(pStr+	nBits) = 0;
	if (nBcd)
		sprintf(strNum, "%lX", nNum);
	else
		sprintf(strNum, "%ld", nNum);
	nTemp = strlen(strNum);
	if (nTemp > nBits)
		memcpy(&pStr[0], &strNum[nTemp - nBits], nBits);
	else
		memcpy(&pStr[nBits - nTemp], &strNum[0], nTemp); 
}

void gui_Editor_NumberSet(int y, char *pStr, int nIndex, int nSelect, int nSign, int nBcd)
{
	int x;

	//数字滚动
	if ((nIndex == 0) && nSign)
		//有符号,第一位可选+/-号
		x = 0;
	else
		x = 1;
	if (nBcd)
		nBcd = 15;
	else
		nBcd = 9;
	pStr[nIndex] = gui_EditorCompnet_Number(pStr[nIndex], x, x + nBcd, nSelect);
	//绘制数字及光标
	x = gui_DrawString_Mixed_Align(0, y, pStr, COLOR_BLACK, GUI_ALIGN_CENTER) + ASC_WIDTH * nIndex;
	gui_DrawHLine(x, y + HZ_HEIGHT, x + 6, COLOR_BLACK);
}

sint64_t gui_Editor_NumberDone(char *pStr, int nBcd)
{
	//转换规格字符串为数字
	sint64_t n;
	char **ppStr = NULL;
	
	if (nBcd)
		nBcd = 16;
	else
		nBcd = 10;
	n = strtoul(pStr, ppStr, nBcd);
	return n;
}


//-------------------------------------------------------------------------
//字符串设置
//-------------------------------------------------------------------------
typedef const struct {
	uint_t	 	len;
	const char *tbl;
}t_gui_str_select, *p_gui_str_select;

static const char tbl_guiEditorStrNormal[] = {
	'.', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
};
static const char tbl_guiEditorStrNum[] = {
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
};
static const char tbl_guiEditorStrLower[] = {
	'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
};
static const char tbl_guiEditorStrCapital[] = {
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
};
static const char tbl_guiEditorStrSign[] = {
	'~', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '[', ']', '{', '}', ';', ':', '\'', '\"', ',', ' ', '<', '>', '?', '/', '\\',
};
static const char tbl_guiEditorStrEnter[] = {
	0x7F,		//回车结束符
};
static t_gui_str_select tbl_guiStringSelect[] = {
	ARR_SIZE(tbl_guiEditorStrNormal),	tbl_guiEditorStrNormal,
	ARR_SIZE(tbl_guiEditorStrNum),		tbl_guiEditorStrNum,
	ARR_SIZE(tbl_guiEditorStrLower),	tbl_guiEditorStrLower,
	ARR_SIZE(tbl_guiEditorStrCapital),	tbl_guiEditorStrCapital,
	ARR_SIZE(tbl_guiEditorStrSign),		tbl_guiEditorStrSign,
	ARR_SIZE(tbl_guiEditorStrEnter),	tbl_guiEditorStrEnter,
};

static const char tbl_guiEditorStrDcmFrac[] = {
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '.', '-', '+',
};
static t_gui_str_select tbl_guiStringDcmSelect[] = {
	ARR_SIZE(tbl_guiEditorStrDcmFrac),     tbl_guiEditorStrDcmFrac,
	ARR_SIZE(tbl_guiEditorStrEnter),		tbl_guiEditorStrEnter,
};

static uint_t gui_nStringEditorDcmType;
static char gui_EditorCompnetDcm_Char(char ch, int nSelect, int nType)
{
	p_gui_str_select p;
	uint_t j;
		
	if (nType)
		gui_nStringEditorDcmType = cycle(gui_nStringEditorDcmType, 0, ARR_SIZE(tbl_guiStringDcmSelect) - 1, 1);
	p = &tbl_guiStringDcmSelect[gui_nStringEditorDcmType];
	if (nType)
		ch = p->tbl[0];
	for (j = 0; j < p->len; j++)
		if (ch == p->tbl[j])
			break;
	j = cycle(j, 0, p->len - 1, nSelect);
	ch = p->tbl[j];
	return ch;
}

void gui_Editor_StringDcmCreate(char *pStr, int nIndex)
{
	p_gui_str_select p = tbl_guiStringDcmSelect;
	uint_t i, j;

	for (i = 0; i < ARR_SIZE(tbl_guiStringDcmSelect); ++i, ++p) {
		for (j = 0; j < p->len; j++)
			if (pStr[nIndex] == p->tbl[j])
				break;
		if (j < p->len)
			break;
	}
	if(i>=ARR_SIZE(tbl_guiStringDcmSelect))
		i = 0;
	gui_nStringEditorDcmType = i;
}

void gui_Editor_StringDcmSet(int y, char *pStr, int nIndex, int nSelect, int nType)
{
	int x;

	pStr[nIndex] = gui_EditorCompnetDcm_Char(pStr[nIndex], nSelect, nType);
	//绘制字串及光标
	x = gui_DrawString_Mixed_Align(0, y, pStr, COLOR_BLACK, GUI_ALIGN_CENTER) + ASC_WIDTH * nIndex;
	gui_DrawHLine(x, y + HZ_HEIGHT, x + 6, COLOR_BLACK);
}



static char gui_EditorCompnet_Char(char ch, int nSelect, int nType)
{
	p_gui_str_select p;
	uint_t j;
		
	p = &tbl_guiStringSelect[nType];
	for (j = 0; j < p->len; j++)
		if (ch == p->tbl[j])
			break;
	j = cycle(j, 0, p->len - 1, nSelect);
	ch = p->tbl[j];
	return ch;
}

void gui_Editor_StringSet(int y, char *pStr, int nIndex, int nSelect, int nType)
{
	int x;

	pStr[nIndex] = gui_EditorCompnet_Char(pStr[nIndex], nSelect, nType);
	//绘制字串及光标
	x = gui_DrawString_Mixed_Align(0, y, pStr, COLOR_BLACK, GUI_ALIGN_CENTER) + ASC_WIDTH * nIndex;
	gui_DrawHLine(x, y + HZ_HEIGHT, x + 6, COLOR_BLACK);
}

#endif

