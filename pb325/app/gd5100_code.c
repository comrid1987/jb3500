#include <string.h>
#include <litecore.h>
#include "para.h"
#include "data.h"
#include "meter.h"
#include "iap.h"


//Private Defines
#define GD5100_FRAME_SIZE			4000

//Private Typedefs
typedef __packed struct {
	uint8_t		runver[8];
	uint32_t	runlen;
	uint16_t	runcrc;
	uint8_t		upver[8];
	uint32_t	uplen;
	uint16_t	frame;
	uint16_t	upcrc;
	uint8_t		reserve[2];
}t_iap_update;

typedef __packed struct {
	uint8_t		ver[8];
	uint16_t	num;
	uint16_t	qty;
	uint16_t	crc;
}t_iap_frame;




//Internal Functions




//External Functions
//读参数
int gd5100_Response01(p_gd5100 p, buf b)
{
	uint_t nDi, nTemp;
	uint8_t *pData, *pEnd, aBuf[128];
	uint32_t nAdr;

	pData = p->rmsg->data->p;
	pEnd = pData + p->rmsg->data->len;
	buf_Push(b, pData, 8);
	pData += 8;
	for (; (pData + 2) <= pEnd; ) {
		nDi = (pData[1] << 8) | pData[0];
		buf_Push(b, pData, 2);
		pData += 2;
		switch (nDi) {
		case 0x8016:	//地市区县码
			aBuf[1] = p->rtua;
			aBuf[0] = p->rtua >> 8;
			buf_Push(b, aBuf, 2);
			break;
		case 0x8017:	//终端地址
			buf_PushData(b, bin2bcd16(p->terid), 2);
			break;
		case 0x8030:	//终端时间
			timet2array(rtc_GetTimet(), aBuf, 1);
			buf_Push(b, aBuf, 6);
			break;
		case 0x8809:	//软件版本,8B,CCXX-AIT-FFF-NNNNNN
			//厂商代号
			aBuf[7] = 0x88;
			//文件类型
			aBuf[6] = 0x01;
			//版本号
			aBuf[5] = (uint8_t)VER_SOFT;
			aBuf[4] = VER_SOFT >> 8;
			//
			aBuf[3] = 0x00;
			//信号强度
			aBuf[2] = modem_GetSignal();
			if (aBuf[2] > 31)
				aBuf[2] = 31;
			aBuf[2] /= 2;
			//登录
			if (rcp_IsLogin())
				SETBIT(aBuf[2], 4);
			//
			aBuf[1] = 0x00;
			aBuf[0] = 0x00;
			buf_Push(b, aBuf, 8);
			break;
		case 0xFFC0:	//调试,读指定地址(4字节)长度(2字节)
			memcpy(&nAdr, pData, 4);
			pData += 4;
			nTemp = (pData[1] << 8) | pData[0];
			pData += 2;
			if (nTemp > GD5100_FRAME_SIZE)
				nTemp = GD5100_FRAME_SIZE;
			buf_Push(b, (const void *)nAdr, nTemp);
			break;
		default:
			buf_Unpush(b, 2);
			break;
		}
	}
	return 1;
}


//自定义
int gd5100_Response0F(p_gd5100 p, buf b)
{
	static uint_t nSaved = 0;
	uint8_t *pData;
	t_iap_frame *pFrame;
	struct iap_info iap;
	uint_t i, nQty;
	uint16_t nCRC;

	pData = p->rmsg->data->p;
	buf_Push(b, pData, 1);
	pData += 5;
	buf_Push(b, pData, 1);
	switch (*pData++) {
	case 0x01:
		//主程序升级请求
		nSaved = 0;
		buf_Push(b, pData, sizeof(t_iap_update));
		break;
	case 0x02:
		//主程序升级数据帧
		pFrame = (t_iap_frame *)pData;
		pData += sizeof(t_iap_frame);
		nCRC = crc16(pData, 2048);
		if (nCRC == pFrame->crc) {		//CRC效验正确
			if (pFrame->num == nSaved) {
				spif_Write((UPDATE_SECTOR_START + 1) * SPIF_SEC_SIZE + nSaved * 2048, pData, 2048);
				nSaved += 1;
				spif_Write(UPDATE_SECTOR_BOOT * SPIF_SEC_SIZE - 2 * nSaved, (uint8_t *)&nCRC, 2);
			}
		}
		pFrame->qty = nSaved;
		pFrame->crc = nCRC;
		buf_Push(b, pFrame, sizeof(t_iap_frame));
		break;
	case 0x03:
		//主程序数据校验请求
		break;
	case 0x04:
		//主程序升级中止
		nSaved = 0;
		buf_Push(b, pData, 10);
		break;
	case 0x05:
		//主程序升级确认
		buf_Push(b, pData, 10);
		pData += 8;
		nQty = (pData[1] << 8) | pData[0];
		pData += 4;
		for (i = 0; i < nQty; ++i) {
			spif_Read(UPDATE_SECTOR_BOOT * SPIF_SEC_SIZE - (i + 1) * 2, &nCRC, 2);
			if (memcmp(&nCRC, pData + i * 2, 2))
				break;
		}
		if (i < nQty) {
			buf_PushData(b, 0, 1);
		} else {
			buf_PushData(b, 1, 1);
			gd5100_TmsgSend(p, 0x0F, b, DLRCP_TMSG_RESPOND);
			iap.magic = IAP_MAGIC_WORD;
			iap.size = nQty * 2048;
			spif_Write(UPDATE_SECTOR_START * SPIF_SEC_SIZE, &iap, sizeof(struct iap_info));
			flash_Flush(0);
			os_thd_Sleep(10000);
			sys_Reset();
		}
		break;
	default:
		break;
	}
	return 1;
}



void gd5100_Response(p_gd5100 p)
{
	uint_t nCode;
	buf b = {0};

	nCode = p->rmsg->code;
	switch (nCode) {
	case 0x01:	//读参数
		gd5100_Response01(p, b);
		break;
	case 0x0F:	//自定义
		gd5100_Response0F(p, b);
		break;
	default:
		break;
	}
	if (b->len)
		gd5100_TmsgSend(p, nCode, b, DLRCP_TMSG_RESPOND);
	else
		gd5100_TmsgError(p, nCode, GD5100_ERR_RELAY_FALE);
	buf_Release(b);
}


