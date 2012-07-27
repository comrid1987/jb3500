#include <string.h>
#include <stdio.h>
#include <litecore.h>
#include "iap.h"

//Private Typedefs
typedef __packed struct {
	uint8_t 	flag;		// 文件标识
	uint8_t 	attrib;		// 文件属性
	uint8_t 	cmd;		// 文件指令
	uint16_t	sector;		// 总段数n 
	uint32_t	offset;		// 第i 段标识或偏移（i=0～n）
	uint16_t	size;		// 第i 段数据长度
	uint8_t		data[];		// 文件数据
}t_afn0F_f1;




//Internal Functions
static void iap_SetFlag(uint32_t nSize)
{
	struct iap_info iap;
	uint8_t aBuf[256];
	uint_t i, nLen;

	//检查传输过程是否有漏包(超过256个字节0xFF)
	for (nLen = 0; nLen < nSize; i++, nLen += sizeof(aBuf)) {
		spif_Read((UPDATE_SECTOR_START + 1) * SPIF_SEC_SIZE + nLen, aBuf, sizeof(aBuf));
		for (i = 0; i < sizeof(aBuf); i++)
			if (aBuf[i] != 0xFF)
				break;
		if (i >= sizeof(aBuf))
			break;
	}
	if (nLen >= nSize) {
		iap.magic = IAP_MAGIC_WORD;
		iap.size = nSize;
		spif_Write(UPDATE_SECTOR_START * SPIF_SEC_SIZE, &iap, sizeof(struct iap_info));
		flash_Flush(0);
	}
}


//External Functions
int gw3761_ResponseFileTrans(p_gw3761 p, buf b, u_word2 *pDu, uint8_t **ppData)
{
	static uint8_t nUpdateVer = 0;				//升级版本
	static uint32_t nUpdateOffset = 0;			//升级偏移
	t_afn0F_f1 f01, *pft;
	char str[6];

	if ((pDu->word[0] != TERMINAL) || (pDu->word[1] != 1))
		return 0;

	pft = (t_afn0F_f1 *)*ppData;
	*ppData += 11;
	switch(pft->cmd) {
	case 'R':			//读文件
	case 'D':			//删除文件
		break;
	default:			//升级
		if (pft->offset > pft->sector)
			break;
		switch (pft->offset) {
		case 0:
			// 询问
			memset(&f01, 0, sizeof(f01));
			snprintf(str, sizeof(str), "%04X", VER_SOFT);
			memcpy((void *)&f01.sector, &str[0], 2);
			memcpy((void *)&f01.size, &str[2], 2);
			if (nUpdateVer != pft->flag) {
				nUpdateVer = pft->flag;
				nUpdateOffset = 0;
			}
			f01.offset = nUpdateOffset;
			buf_Push(b, &f01, sizeof(t_afn0F_f1));
			break;
		case 1:
			nUpdateOffset = 0;
		default:
			if (pft->sector != pft->offset)
				nUpdateOffset = (pft->offset - 1) * pft->size;
			spif_Write((UPDATE_SECTOR_START + 1) * SPIF_SEC_SIZE + nUpdateOffset, pft->data, pft->size);
			nUpdateOffset += pft->size;
			*ppData += pft->size;
			if (pft->sector == pft->offset) {
				gw3761_TmsgConfirm(p);
				// do.. 置标志,退出应用程序,重新IAP
				iap_SetFlag(nUpdateOffset);
				os_thd_Sleep(10000);
				sys_Reset();
			}
			break;
		}
		break;
	}	
	return 1;
}



