#include <stdio.h>
#include <string.h>
#include <litecore.h>
#include "alarm.h"
#include "para.h"



//Private Defines
#define EVT_LOCK_ENABLE			1
#define EVT_MAGIC_WORD			0x67527920

#define EVT_SIZE				128
#define EVT_QTY					2048
#define EVT_DATA_BASE			(196 * 0x1000)

#define EVT_CNT_ADDR			0xFFFF0001	// 4
#define EVT_FLAG_ADDR			0xFFFF0010	// 8

#define EVT_RUNVER_ADDR			0xFFFF0101	// 4
#define EVT_RUNTIME_ADDR		0xFFFF0102	// 4


//Private Variables
#if EVT_LOCK_ENABLE
static os_sem evt_sem;
#endif


//Private Macros
#if EVT_LOCK_ENABLE
#define evt_Lock()				rt_sem_take(&evt_sem, RT_WAITING_FOREVER)
#define evt_Unlock()			rt_sem_release(&evt_sem)
#else
#define evt_Lock()
#define evt_Unlock()
#endif


//Private Consts
t_flash_blk _tbl_evt_SfsDev[] = {
	INTFLASH_BASE_ADR + 0x30000 + INTFLASH_BLK_SIZE * 0,	INTFLASH_BLK_SIZE,
	INTFLASH_BASE_ADR + 0x30000 + INTFLASH_BLK_SIZE * 1,	INTFLASH_BLK_SIZE,
	INTFLASH_BASE_ADR + 0x30000 + INTFLASH_BLK_SIZE * 2,	INTFLASH_BLK_SIZE,
	INTFLASH_BASE_ADR + 0x30000 + INTFLASH_BLK_SIZE * 3,	INTFLASH_BLK_SIZE,
	INTFLASH_BASE_ADR + 0x31000 + INTFLASH_BLK_SIZE * 0,	INTFLASH_BLK_SIZE,
	INTFLASH_BASE_ADR + 0x31000 + INTFLASH_BLK_SIZE * 1,	INTFLASH_BLK_SIZE,
	INTFLASH_BASE_ADR + 0x31000 + INTFLASH_BLK_SIZE * 2,	INTFLASH_BLK_SIZE,
	INTFLASH_BASE_ADR + 0x31000 + INTFLASH_BLK_SIZE * 3,	INTFLASH_BLK_SIZE,
	INTFLASH_BASE_ADR + 0x32000 + INTFLASH_BLK_SIZE * 0,	INTFLASH_BLK_SIZE,
	INTFLASH_BASE_ADR + 0x32000 + INTFLASH_BLK_SIZE * 1,	INTFLASH_BLK_SIZE,
	INTFLASH_BASE_ADR + 0x32000 + INTFLASH_BLK_SIZE * 2,	INTFLASH_BLK_SIZE,
	INTFLASH_BASE_ADR + 0x32000 + INTFLASH_BLK_SIZE * 3,	INTFLASH_BLK_SIZE,
	INTFLASH_BASE_ADR + 0x33000 + INTFLASH_BLK_SIZE * 0,	INTFLASH_BLK_SIZE,
	INTFLASH_BASE_ADR + 0x33000 + INTFLASH_BLK_SIZE * 1,	INTFLASH_BLK_SIZE,
	INTFLASH_BASE_ADR + 0x33000 + INTFLASH_BLK_SIZE * 2,	INTFLASH_BLK_SIZE,
	INTFLASH_BASE_ADR + 0x33000 + INTFLASH_BLK_SIZE * 3,	INTFLASH_BLK_SIZE,
	INTFLASH_BASE_ADR + 0x34000 + INTFLASH_BLK_SIZE * 0,	INTFLASH_BLK_SIZE,
	INTFLASH_BASE_ADR + 0x34000 + INTFLASH_BLK_SIZE * 1,	INTFLASH_BLK_SIZE,
	INTFLASH_BASE_ADR + 0x34000 + INTFLASH_BLK_SIZE * 2,	INTFLASH_BLK_SIZE,
	INTFLASH_BASE_ADR + 0x34000 + INTFLASH_BLK_SIZE * 3,	INTFLASH_BLK_SIZE,
	INTFLASH_BASE_ADR + 0x35000 + INTFLASH_BLK_SIZE * 0,	INTFLASH_BLK_SIZE,
	INTFLASH_BASE_ADR + 0x35000 + INTFLASH_BLK_SIZE * 1,	INTFLASH_BLK_SIZE,
	INTFLASH_BASE_ADR + 0x35000 + INTFLASH_BLK_SIZE * 2,	INTFLASH_BLK_SIZE,
	INTFLASH_BASE_ADR + 0x35000 + INTFLASH_BLK_SIZE * 3,	INTFLASH_BLK_SIZE,
	INTFLASH_BASE_ADR + 0x36000 + INTFLASH_BLK_SIZE * 0,	INTFLASH_BLK_SIZE,
	INTFLASH_BASE_ADR + 0x36000 + INTFLASH_BLK_SIZE * 1,	INTFLASH_BLK_SIZE,
	INTFLASH_BASE_ADR + 0x36000 + INTFLASH_BLK_SIZE * 2,	INTFLASH_BLK_SIZE,
	INTFLASH_BASE_ADR + 0x36000 + INTFLASH_BLK_SIZE * 3,	INTFLASH_BLK_SIZE,
	INTFLASH_BASE_ADR + 0x37000 + INTFLASH_BLK_SIZE * 0,	INTFLASH_BLK_SIZE,
	INTFLASH_BASE_ADR + 0x37000 + INTFLASH_BLK_SIZE * 1,	INTFLASH_BLK_SIZE,
	INTFLASH_BASE_ADR + 0x37000 + INTFLASH_BLK_SIZE * 2,	INTFLASH_BLK_SIZE,
	INTFLASH_BASE_ADR + 0x37000 + INTFLASH_BLK_SIZE * 3,	INTFLASH_BLK_SIZE,
	INTFLASH_BASE_ADR + 0x38000 + INTFLASH_BLK_SIZE * 0,	INTFLASH_BLK_SIZE,
	INTFLASH_BASE_ADR + 0x38000 + INTFLASH_BLK_SIZE * 1,	INTFLASH_BLK_SIZE,
	INTFLASH_BASE_ADR + 0x38000 + INTFLASH_BLK_SIZE * 2,	INTFLASH_BLK_SIZE,
	INTFLASH_BASE_ADR + 0x38000 + INTFLASH_BLK_SIZE * 3,	INTFLASH_BLK_SIZE,
	INTFLASH_BASE_ADR + 0x39000 + INTFLASH_BLK_SIZE * 0,	INTFLASH_BLK_SIZE,
	INTFLASH_BASE_ADR + 0x39000 + INTFLASH_BLK_SIZE * 1,	INTFLASH_BLK_SIZE,
	INTFLASH_BASE_ADR + 0x39000 + INTFLASH_BLK_SIZE * 2,	INTFLASH_BLK_SIZE,
	INTFLASH_BASE_ADR + 0x39000 + INTFLASH_BLK_SIZE * 3,	INTFLASH_BLK_SIZE,
	INTFLASH_BASE_ADR + 0x3A000 + INTFLASH_BLK_SIZE * 0,	INTFLASH_BLK_SIZE,
	INTFLASH_BASE_ADR + 0x3A000 + INTFLASH_BLK_SIZE * 1,	INTFLASH_BLK_SIZE,
	INTFLASH_BASE_ADR + 0x3A000 + INTFLASH_BLK_SIZE * 2,	INTFLASH_BLK_SIZE,
	INTFLASH_BASE_ADR + 0x3A000 + INTFLASH_BLK_SIZE * 3,	INTFLASH_BLK_SIZE,
	INTFLASH_BASE_ADR + 0x3B000 + INTFLASH_BLK_SIZE * 0,	INTFLASH_BLK_SIZE,
	INTFLASH_BASE_ADR + 0x3B000 + INTFLASH_BLK_SIZE * 1,	INTFLASH_BLK_SIZE,
	INTFLASH_BASE_ADR + 0x3B000 + INTFLASH_BLK_SIZE * 2,	INTFLASH_BLK_SIZE,
	INTFLASH_BASE_ADR + 0x3B000 + INTFLASH_BLK_SIZE * 3,	INTFLASH_BLK_SIZE,
};
t_flash_dev evt_SfsDev = {
	FLASH_DEV_INT, ARR_SIZE(_tbl_evt_SfsDev), _tbl_evt_SfsDev,
};


static void evt_Format()
{

	sfs_Init(&evt_SfsDev);
	sfs_Write(&evt_SfsDev, EVT_MAGIC_WORD, NULL, 0);
	evt_RunTimeWrite(rtc_GetTimet());
}

static int evt_Attrib(uint_t nERC)
{
	t_afn04_f9 xF9;

	icp_ParaRead(4, 9, TERMINAL, &xF9, sizeof(t_afn04_f9));
	if (GETBIT(xF9.valid, nERC - 1) == 0)
		return 0;
	if (GETBIT(xF9.imp, nERC - 1) == 0)
		return 2;
	return 1;
}



static void evt_Save(uint_t nERC, const void *pBuf, uint_t nLen, uint_t nAtt)
{
	uint_t nCycle = 0, nCnt = 0;
	uint8_t aBuf[EVT_SIZE];
	uint64_t nFlag = 0;

	nAtt -= 1;
	evt_Lock();
	sfs_Read(&evt_SfsDev, EVT_CNT_ADDR + nAtt, &nCnt);
	if (nCnt >= EVT_QTY) {
		nCnt &= (EVT_QTY - 1);
		nCycle = 1;
	}
	aBuf[0] = nERC;
	aBuf[1] = nLen;
	memcpy(&aBuf[2], pBuf, nLen);
	spif_Write(EVT_DATA_BASE + nAtt * (EVT_QTY * EVT_SIZE) + nCnt * EVT_SIZE, aBuf, nLen + 2);
	flash_Flush(0);
	if (nCycle)
		nCnt += EVT_QTY;
	nCnt += 1;
	sfs_Write(&evt_SfsDev, EVT_CNT_ADDR + nAtt, &nCnt, 2);
	sfs_Read(&evt_SfsDev, EVT_FLAG_ADDR, &nFlag);
	SETBIT(nFlag, nERC - 1);
	sfs_Write(&evt_SfsDev, EVT_FLAG_ADDR, &nFlag, sizeof(uint64_t));
	evt_Unlock();
}



//数据初始化和版本变更事件
static void evt_ERC1(uint_t nFlag)
{
	uint8_t aBuf[14];
	uint_t nAtt;
	char str1[5], str2[5];

	nAtt = evt_Attrib(1);
	if (nAtt) {
		sfs_Read(&evt_SfsDev, EVT_RUNVER_ADDR, str1);
		sprintf(str2, "%04X", VER_SOFT);
		if (nFlag & BITMASK(1)) {
			if (memcmp(str1, str2, 4))
				sfs_Write(&evt_SfsDev, EVT_RUNVER_ADDR, str2, 4);
			else
				nAtt = 0;
		}
		if (nAtt) {
			gw3761_ConvertData_15(aBuf, rtc_GetTimet());
			aBuf[5] = nFlag;
			memcpy(&aBuf[6], str1, 4);
			memcpy(&aBuf[10], str2, 4);
			evt_Save(1, aBuf, sizeof(aBuf), nAtt);
		}
	}
}


//终端停上电事件
static void evt_ERC14()
{
	uint8_t aBuf[10];
	uint_t nAtt;
	time_t tStop;

	nAtt = evt_Attrib(14);
	if (nAtt) {
		sfs_Read(&evt_SfsDev, EVT_RUNTIME_ADDR, &tStop);
		gw3761_ConvertData_15(aBuf, tStop);
		gw3761_ConvertData_15(&aBuf[5], rtc_GetTimet());
		evt_Save(14, aBuf, sizeof(aBuf), nAtt);
	}
}




//External Functions
//参数变更事件
void evt_ERC3(uint_t nMSA, u_word2 *pDu)
{
	uint8_t aBuf[10];
	uint_t nAtt;

	nAtt = evt_Attrib(3);
	if (nAtt) {
		gw3761_ConvertData_15(aBuf, rtc_GetTimet());
		aBuf[5] = nMSA;
		memcpy(&aBuf[6], pDu, 4);
		evt_Save(3, aBuf, sizeof(aBuf), nAtt);
	}
}

//电能表参数变更事件
void evt_ERC8(uint_t nTn, uint_t nFlag)
{
	uint8_t aBuf[8];
	uint_t nAtt;

	nAtt = evt_Attrib(8);
	if (nAtt) {
		gw3761_ConvertData_15(aBuf, rtc_GetTimet());
		memcpy(&aBuf[5], &nTn, 2);
		aBuf[7] = nFlag;
		evt_Save(8, aBuf, sizeof(aBuf), nAtt);
	}
}

//发现未知电表事件
void evt_ERC35(uint_t nPort, const uint8_t *pAdr)
{
	uint8_t aBuf[15];
	uint_t nAtt;

	nAtt = evt_Attrib(35);
	if (nAtt) {
		gw3761_ConvertData_15(aBuf, rtc_GetTimet());
		aBuf[5] = nPort;
		aBuf[6] = 1;
		memcpy(&aBuf[7], pAdr, 6);
		aBuf[13] = 1;
		aBuf[14] = 1;
		evt_Save(35, aBuf, sizeof(aBuf), nAtt);
	}
}


//运行时间
void evt_RunTimeWrite(time_t tTime)
{

	sfs_Write(&evt_SfsDev, EVT_RUNTIME_ADDR, &tTime, sizeof(time_t));
}

int evt_RunTimeRead(time_t *pTime)
{
	sys_res res;

	res = sfs_Read(&evt_SfsDev, EVT_RUNTIME_ADDR, pTime);
	if (res == SYS_R_OK)
		return 1;
	return 0;
}


void evt_Init()
{

#if EVT_LOCK_ENABLE
	rt_sem_init(&evt_sem, "sem_evt", 1, RT_IPC_FLAG_FIFO);
#endif
	if (sfs_Read(&evt_SfsDev, EVT_MAGIC_WORD, NULL) != SYS_R_OK)
		evt_Format();
	//停上电事件
	evt_ERC14();
	//版本变更事件
	evt_ERC1(BITMASK(1));
}

void evt_Clear()
{

	evt_Lock();
	evt_Format();
	evt_Unlock();
}


uint_t evt_GetCount()
{
	uint_t nCnt;
	uint8_t aBuf[4];

	sfs_Read(&evt_SfsDev, EVT_CNT_ADDR, &aBuf[0]);
	sfs_Read(&evt_SfsDev, EVT_CNT_ADDR + 1, &aBuf[2]);
	aBuf[1] = aBuf[2];
	memcpy(&nCnt, aBuf, 2);
	return nCnt;
}

void evt_GetFlag(void *pBuf)
{

	sfs_Read(&evt_SfsDev, EVT_FLAG_ADDR, pBuf);
}

int evt_Read(buf b, uint_t nPm, uint_t nPn, uint_t nIsNormal)
{
	uint_t i, nCnt = 0, nCycle = 0, nStart, nLast, nLen;
	uint8_t aBuf[EVT_SIZE];

	evt_Lock();
	nLen = b->len;
	buf_PushData(b, 0, 2);
	if (nPm > nPn)
		nPn += 256;
	sfs_Read(&evt_SfsDev, EVT_CNT_ADDR + nIsNormal, &nCnt);
	if (nCnt > EVT_QTY) {
		nCnt &= (EVT_QTY - 1);
		nCycle = 1;
	}
	if (nCnt < 256) {
		if (nCycle) {
			nCnt += EVT_QTY;
			nStart = nCnt - 256;
		} else
			nStart = 0;
	} else {
		nStart = nCnt - 256;
	}
	i = nStart + nPm;
	b->p[nLen] = i;
	nIsNormal = EVT_DATA_BASE + nIsNormal * (EVT_QTY * EVT_SIZE);
	for (; (i < (nStart + nPn)) && (i < nCnt); i++) {
		spif_Read(nIsNormal + (i & (EVT_QTY - 1)) * EVT_SIZE, aBuf, sizeof(aBuf));
		buf_Push(b, aBuf, aBuf[1] + 2);
		nLast = i + 1;
	}
	b->p[nLen + 1] = nLast;
	evt_Unlock();
	return (i - (nStart + nPm));
}


