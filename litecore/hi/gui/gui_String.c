#if GUI_ENABLE


#if GUI_FONT_TYPE == GUI_FONT_STD12
#define gui_DrawChar_ASC		gui_DrawString_ASC6x12
#define gui_DrawChar_HZ			gui_DrawChar_HZ12
#elif GUI_FONT_TYPE == GUI_FONT_STD16
#define gui_DrawChar_ASC		gui_DrawString_ASC6x16
#define gui_DrawChar_HZ			gui_DrawChar_HZ16
#endif




//===============================================================
//混合字符串显示，包含汉字和ASC字符
//===============================================================
int gui_GetStringWidth(const char *pStr)
{
	int nWidth = 0;

	while (*pStr != '\0') {
		if (*pStr < 0x80) {
			nWidth += ASC_WIDTH;
			pStr += 1;
		} else {
			nWidth += HZ_WIDTH;
			pStr += 2;
		}
	}
	return nWidth;
}

//===============================================================
//6*8点阵字符显示
//在字符表里面已经包含了字符之间的间隙位置
//x,y  输入起始坐标,pStr字符串
//===============================================================
void gui_DrawString_ASC6x8(int x, int y, const char *pStr, t_color nColor)
{
	int i, j;
	const uint8_t *pOffset;

	while (*pStr != '\0') {
		pOffset = FONT6x8ASCII + (int)(*pStr++ - ' ') * ASC6x8_SIZE;	//将ASCII转换成实际值"!"的ASCII为33。
		for(i = 0; i < ASC6x8_SIZE; i++, pOffset++)
			for(j = 0; j < 8; j++)
				if (GETBIT(*pOffset, j))
					gui_DrawPoint(x + i, y + j, nColor);
				else
					gui_DrawPoint(x + i, y + j, ~nColor);
		x += ASC_WIDTH;
	}
}


//===============================================================
//显示一个6*12点阵字符
//x,y  输入起始坐标,pStr字符串
//===============================================================
void gui_DrawString_ASC6x12(int x, int y, const char *pStr, t_color nColor)
{
	int i, j;
	const uint8_t *p;

	while (*pStr != '\0') {
		p = FONT6x12ASCII + (int)(*pStr++ - ' ') * ASC6x12_SIZE;	//将ASCII转换成实际值"!"的ASCII为33。
        for(i = 0; i < 12; i++, p++) {
            for(j = 0; j < 6; j++) {
                if (GETBIT(*p, j))
                    lcd_DrawPoint(x + j, y + i, nColor);
                else
                    lcd_DrawPoint(x + j, y + i, ~nColor);
            }
        }
		x += ASC_WIDTH;
	}
}

//===============================================================
//显示一个6*16点阵字符
//x,y  输入起始坐标,pStr字符串
//===============================================================
void gui_DrawString_ASC6x16(int x, int y, const char *pStr, t_color nColor)
{
	int i, j;
	const uint8_t *p1, *p2;

	while (*pStr != '\0') {
		p1 = FONT6x16ASCII + (int)(*pStr++ - ' ') * ASC6x16_SIZE;	//将ASCII转换成实际值"!"的ASCII为33
		p2 = p1 + 6;
		for(i = 0; i < 6; i++, p1++, p2++) {
			for(j = 0; j < 8; j++) {
				if (GETBIT(*p1, j))
				    lcd_DrawPoint(x + i, y + j, nColor);
				else
				    lcd_DrawPoint(x + i, y + j, ~nColor);
				if (GETBIT(*p2, j))
				    lcd_DrawPoint(x + i, y + j + 8, nColor);
				else
				    lcd_DrawPoint(x + i, y + j + 8, ~nColor);
			}
		}               
		x += ASC_WIDTH;
	}
}

//===============================================================
//显示一个汉字
//12*12点阵
//x,y  输入起始坐标,pStr字符串
//===============================================================
#define GUI_GETFONT_STATIC_BUF			(1 && NANDFLASH_ENABLE)
#if GUI_GETFONT_STATIC_BUF
static uint8_t gui_aGetFontBuf[NAND_PAGE_DATA + NAND_PAGE_SPARE];
#endif
#if GUI_FONT_CARR_TYPE == GUI_FONT_CARR_T_DIRACCESS
static void gui_GetFont(uint_t nOffset, void *pBuf, uint_t nLen)
{

	memcpy(pBuf, (const void *)(GUI_FONT_BASE + nOffset), nLen);
}
#endif

#if GUI_FONT_CARR_TYPE == GUI_FONT_CARR_T_FILE
static void gui_GetFont(uint_t nOffset, void *pBuf, uint_t nLen)
{
	int fd;

	os_thd_Lock();
	do {
		fd = open(GUI_FONT_FILENAME, O_RDONLY, 0);
		if (fd < 0)
			break;
		lseek(fd, nOffset, DFS_SEEK_SET);
		read(fd, pBuf, nLen);
		close(fd);
	} while (0);
	os_thd_Unlock();
}
#endif

#if GUI_FONT_CARR_TYPE == GUI_FONT_CARR_T_NAND
static void gui_GetFont(uint_t nOffset, void *pBuf, uint_t nLen)
{
	uint_t nTemp;
	uint32_t nEcc;
	u_byte4 uPage;
	uint8_t *pMem;

#if GUI_GETFONT_STATIC_BUF
	pMem = gui_aGetFontBuf;
#else
	pMem = mem_Malloc(NAND_PAGE_DATA + NAND_PAGE_SPARE);
	if (pMem == NULL)
		return;
#endif
	uPage.n = (GUI_FONT_BASE * NAND_BLK_PAGE) + (nOffset / NAND_PAGE_DATA);
	nOffset %= NAND_PAGE_DATA;
	nTemp = NAND_PAGE_DATA - nOffset;
	os_thd_Lock();
	nand_ReadPage(&uPage, pMem, &nEcc);
	if (nTemp < nLen) {
		memcpy(pBuf, pMem + nOffset, nTemp);
		uPage.n += 1;
		nand_ReadPage(&uPage, pMem, &nEcc);
		memcpy((uint8_t *)pBuf + nTemp, pMem, nLen - nTemp);
	} else {
		memcpy(pBuf, pMem + nOffset, nLen);
	}
	os_thd_Unlock();
#if GUI_GETFONT_STATIC_BUF == 0
	mem_Free(pMem);
#endif
}
#endif

#if GUI_FONT_CARR_TYPE == GUI_FONT_CARR_T_SPIF
static void gui_GetFont(uint_t nOffset, void *pBuf, uint_t nLen)
{

	spif_Read(GUI_FONT_BASE + nOffset, pBuf, nLen);
}
#endif


#define     ASC12_OFFSET            0x00000
#define     ASC16_OFFSET            0x00474
#define     FNT12_OFFSET_A1         0x00A64
#define     FNT12_OFFSET_A2         0x045E0
#define     FNT16_OFFSET_A1         0x19058
#define     FNT16_OFFSET_A2         0x1FA18

void gui_DrawChar_HZ12(int x, int y, char *pStr, t_color nColor)
{
	int x1, y1;
	uint_t i = 0, nOffset;
	uint8_t aData[HZ_SIZE];

	if (pStr[0] >= 0xA0 + 16) {
		nOffset = FNT12_OFFSET_A2 + ((pStr[0] - 0xA0 - 16) * 94 + pStr[1] - 0xA1) * HZ_SIZE;
	} else if (pStr[0] <= 0xA0 + 9) {
	    //字库中没有 A1 区
		if (pStr[0] > 0xA1)
			pStr[0]--;
		nOffset = FNT12_OFFSET_A1 + ((pStr[0] - 0xA0) * 94 + pStr[1] - 0xA1) * HZ_SIZE;
    } else {
		pStr[0]  = 0xa1;
		pStr[1]  = 0xf5;
		nOffset = FNT12_OFFSET_A1 + ((pStr[0] - 0xA0) * 94 + pStr[1] - 0xA1) * HZ_SIZE;
    }    
	gui_GetFont(nOffset, aData, sizeof(aData));
	for (y1 = y; y1 < (y + HZ_WIDTH); y1++) {
		for (x1 = x; x1 < (x + HZ_WIDTH); x1++) {
			if (aData[i >> 3] & BITMASK(7 - (i & 7)))
				lcd_DrawPoint(x1, y1, nColor);
			else
				lcd_DrawPoint(x1, y1, ~nColor);
			i++;
		}
	}
}

void gui_DrawChar_HZ16(int x, int y, const char *pStr, t_color nColor)
{
	int x1, y1;
	uint_t i, nOffset, nLow, nHigh, nByte = 0;
	uint8_t aData[HZ_SIZE];
	const uint8_t *pOffset = &aData[0];

	if (pStr[0] >= 0xA0 + 16) {
		nOffset = FNT16_OFFSET_A2 + ((pStr[0] - 0xA0 - 16) * 94 + pStr[1] - 0xA1) * HZ_SIZE;
	} else if (pStr[0] <= 0xA0 + 9) {
	    //字库中没有 A1 区
		if (pStr[0] > 0xA1)
			nLow = pStr[0] - 1;
		else
			nLow = pStr[0];
		nOffset = FNT16_OFFSET_A1 + ((nLow - 0xA0) * 94 + pStr[1] - 0xA1) * HZ_SIZE;
    } else {
		nLow = 0xA1;
		nHigh = 0xF5;
		nOffset = FNT16_OFFSET_A1 + ((nLow - 0xA0) * 94 + nHigh - 0xA1) * HZ_SIZE;
    }    
	gui_GetFont(nOffset, aData, sizeof(aData));
	x1 = x + HZ_WIDTH;
	y1 = y + HZ_HEIGHT;
	for (; y < y1; y++, x -= HZ_WIDTH) {
		for (i = 7; x < x1; i--, x++) {
			if (i == 7)
				nByte = *pOffset++;
			if (GETBIT(nByte, i))
				gui_DrawPoint(x, y, nColor);
			else
				gui_DrawPoint(x, y, ~nColor);
			if (i == 0)
				i = 8;
		}
 	}
}


//===============================================================
//混合字符串显示，包含汉字和ASC字符
//===============================================================
void gui_DrawChar_ASC4HZK(int x, int y, char *pStr, t_color nColor)
{
	int i, j;
	uint8_t aData[HZ_SIZE];
	const uint8_t *pOffset = &aData[0];

	gui_GetFont(((10 - 1) * 94 + *pStr - 33) * HZ_SIZE, aData, sizeof(aData));
	for (i = 0; i < HZ_HEIGHT; i++, y++, x -= ASC_WIDTH) {
		//取8bits
		for (j = ASC_WIDTH - 1; j >= 0; j--, x++)
			if (GETBIT(*pOffset, j))
				gui_DrawPoint(x, y, nColor);
			else
				gui_DrawPoint(x, y, ~nColor);
		pOffset += 2;
	}
}

int gui_DrawString_Mixed(int x, int y, const char *pStr, t_color nColor)
{

	while (*pStr != '\0') {
		if (*pStr < 0x80) {
			gui_DrawChar_ASC(x, y, pStr, nColor);		//ASC码
			pStr += 1;
			x += ASC_WIDTH;
		} else {
			gui_DrawChar_HZ(x, y, pStr, nColor);		//汉字
			pStr += 2;
			x += HZ_WIDTH;
		}
	}
	return x;
}

//===============================================================
//混合字符串显示，包含汉字和ASC字符
//===============================================================
int gui_DrawString_Mixed_Align(int x, int y, const char *pStr, t_color nColor, int nAlignType)
{
	int nX, nWidth;
	
	nWidth = gui_GetStringWidth(pStr);
	switch (nAlignType) {
	case GUI_ALIGN_CENTER:
		nX = (LCD_X_MAX - x - nWidth) >> 1;
		break;
	case GUI_ALIGN_RIGHT:
		nX = LCD_X_MAX - x - nWidth;
		break;
	default:
		nX = x;
		break;
	}
	gui_DrawString_Mixed(nX, y, pStr, nColor);
	return nX;
}


#endif

