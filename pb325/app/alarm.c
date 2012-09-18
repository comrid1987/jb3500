#include <stdio.h>
#include <string.h>
#include <litecore.h>
#include "alarm.h"
#include "para.h"
#include "data.h"
#include "acm.h"



//Private Defines
#define EVT_LOCK_ENABLE			1
#define EVT_DEBUG_ENABLE		0
#define EVT_MAGIC_WORD			0x67527920

#define EVT_SIZE				128
#define EVT_QTY					2048
#define EVT_DATA_BASE			(196 * 0x1000)

#define EVT_CNT_ADDR			0xFFFF0001	// 4
#define EVT_OCCUR_ADDR			0xFFFF0010	// 8

#define EVT_FLAG_ADDR			0xFFFF0020	// 4
#define EVT_YX_ADDR				0xFFFF0021	// 4

#define EVT_RUNTIME_ADDR		0xFFFF0102	// 4

#define EVT_UABNORMAL_TIME		0xFFFF4100
#define EVT_IUP_TIME			0xFFFF4200

#define EVT_DLQ_QL_STE			0xFFFF7100
#define EVT_DLQ_QL_PARA			0xFFFF7200


#define EVT_FLAG_UBALANCE		0
#define EVT_FLAG_IBALANCE		1
#define EVT_FLAG_UOVER			2
#define EVT_FLAG_UUNDER			5
#define EVT_FLAG_IOVER			8
#define EVT_FLAG_IUP			11
#define EVT_FLAG_UIOVER			14
#define EVT_FLAG_UIUP			15
#define EVT_FLAG_UABNORMAL		16



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

#if EVT_DEBUG_ENABLE
#define evt_DbgOut				dbg_trace
#else
#define evt_DbgOut(...)
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
	time_t tTime;

	sfs_Init(&evt_SfsDev);
	sfs_Write(&evt_SfsDev, EVT_MAGIC_WORD, NULL, 0);
	tTime = rtc_GetTimet();
	sfs_Write(&evt_SfsDev, EVT_RUNTIME_ADDR, &tTime, sizeof(time_t));
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

	evt_Lock();
	nAtt -= 1;
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
	sfs_Read(&evt_SfsDev, EVT_OCCUR_ADDR, &nFlag);
	if ((nFlag & BITMASK(nERC - 1)) == 0) {
		SETBIT(nFlag, nERC - 1);
		sfs_Write(&evt_SfsDev, EVT_OCCUR_ADDR, &nFlag, sizeof(uint64_t));
	}
	evt_Unlock();
}


//版本变更事件
static void evt_ERC1(uint_t nVer)
{
	uint8_t aBuf[16];
	uint_t nAtt;

	nAtt = evt_Attrib(1);
	if (nAtt) {
		gw3761_ConvertData_15(aBuf, rtc_GetTimet());
		aBuf[5] = BITMASK(1);
		sprintf((char *)&aBuf[6], "%04X", nVer);
		sprintf((char *)&aBuf[10], "%04X", VER_SOFT);
		evt_Save(1, aBuf, sizeof(aBuf), nAtt);
	}
}

//电压回路异常
static void evt_ERC10(const void *pData)
{
	uint8_t aBuf[27];
	uint_t nAtt;

	nAtt = evt_Attrib(10);
	if (nAtt) {
		gw3761_ConvertData_15(aBuf, rtc_GetTimet());
		memcpy(&aBuf[5], pData, 22);
		evt_Save(10, aBuf, sizeof(aBuf), nAtt);
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

//不平衡度越限事件
static void evt_ERC17(const void *pData)
{
	uint8_t aBuf[27];
	uint_t nAtt;

	nAtt = evt_Attrib(17);
	if (nAtt) {
		gw3761_ConvertData_15(aBuf, rtc_GetTimet());
		memcpy(&aBuf[5], pData, 22);
		evt_Save(17, aBuf, sizeof(aBuf), nAtt);
	}
}

//电容器投切事件
static void evt_ERC18(uint_t nId, uint_t nIsOccur)
{
	uint8_t aBuf[10];
	uint_t nAtt, nTn;

	nAtt = evt_Attrib(18);
	if (nAtt) {
		gw3761_ConvertData_15(aBuf, rtc_GetTimet());
		nTn = TERMINAL;
		if (nIsOccur)
			SETBIT(nTn, 15);
		memcpy(&aBuf[5], &nTn, 2);
		aBuf[7] = 0;
		nTn = 0;
		SETBIT(nTn, nId);
		memcpy(&aBuf[8], &nTn, 2);
		evt_Save(18, aBuf, sizeof(aBuf), nAtt);
	}
}

static void evt_ERC24(const void *pData)
{
	uint8_t aBuf[14];
	uint_t nAtt;

	nAtt = evt_Attrib(24);
	if (nAtt) {
		gw3761_ConvertData_15(aBuf, rtc_GetTimet());
		memcpy(&aBuf[5], pData, 9);
		evt_Save(24, aBuf, sizeof(aBuf), nAtt);
	}
}

static void evt_ERC25(const void *pData)
{
	uint8_t aBuf[33];
	uint_t nAtt;

	nAtt = evt_Attrib(25);
	if (nAtt) {
		gw3761_ConvertData_15(aBuf, rtc_GetTimet());
		memcpy(&aBuf[5], pData, 28);
		evt_Save(25, aBuf, sizeof(aBuf), nAtt);
	}
}

static void evt_ERC26(const void *pData)
{
	uint8_t aBuf[14];
	uint_t nAtt;

	nAtt = evt_Attrib(26);
	if (nAtt) {
		gw3761_ConvertData_15(aBuf, rtc_GetTimet());
		memcpy(&aBuf[5], pData, 9);
		evt_Save(26, aBuf, sizeof(aBuf), nAtt);
	}
}

static void evt_IupMax(uint8_t *pData, time_t tTime, time_t tEnd)
{
	uint_t i;
	float fData, fMax[4] = {0};
	t_acm_rtdata xRtd;

	tTime /= 60;
	tTime *= 60;
	for (; tTime < tEnd; tTime += 60) {
		if (acm_Rtd4timet(&xRtd, tTime) == 0)
			continue;
		for (i = 0; i < 4; i++) {
			fData = xRtd.i[i];
			if (fData >= fMax[i])
				fMax[i] = fData;
		}
	}
	gw3761_ConvertData_25(&pData[0], FLOAT2FIX(fMax[3]), 1);
	for (i = 0; i < 3; i++) {
		gw3761_ConvertData_25(&pData[3 + i * 3], FLOAT2FIX(fMax[i]), 1);
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

//总保跳闸事件
void evt_ERC5(uint_t nTn, const void *pOld, const void *pNew)
{
	uint8_t aBuf[13];
	uint_t nAtt;

	nAtt = evt_Attrib(5);
	evt_DbgOut("<ERC5> Attrib %d", nAtt);
	if (nAtt) {
		gw3761_ConvertData_15(aBuf, rtc_GetTimet());
		memcpy(&aBuf[5], &nTn, 2);
		memcpy(&aBuf[7], pNew, 3);
		memcpy(&aBuf[10], pOld, 3);
		evt_Save(5, aBuf, sizeof(aBuf), nAtt);
	}
}

//总保参数变更事件
void evt_ERC8(uint_t nTn, const void *pOld, const void *pNew)
{
	uint8_t aBuf[29];
	uint_t nAtt;

	nAtt = evt_Attrib(8);
	if (nAtt) {
		gw3761_ConvertData_15(aBuf, rtc_GetTimet());
		memcpy(&aBuf[5], &nTn, 2);
		memcpy(&aBuf[7], pNew, 11);
		memcpy(&aBuf[18], pOld, 11);
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

void evt_Terminal(t_afn04_f26 *pF26)
{
	uint_t i, j, nValid, nTn;
	float fData, fTemp;
	t_acm_rtdata *pa = &acm_rtd;
	uint8_t aBuf[28];
	uint32_t nFlagOld, nFlag, nData;
	time_t tTime;

	nFlagOld = 0;
	sfs_Read(&evt_SfsDev, EVT_FLAG_ADDR, &nFlagOld);
	nFlag = nFlagOld;

#if 0
	//不平衡越限
	fTemp = (float)bcd2bin16(pF26->ubalance) / 1000.0f;
	nValid = 0;
	fData = pa->ub;
	if (GETBIT(nFlagOld, EVT_FLAG_UBALANCE)) {
		//恢复
		if (fData < fTemp) {
			nTn = TERMINAL;
			nValid = 1;
			CLRBIT(nFlag, EVT_FLAG_UBALANCE);
		}
	} else {
		//发生
		if (fData > fTemp) {
			nTn = TERMINAL | BITMASK(15);
			nValid = 1;
			SETBIT(nFlag, EVT_FLAG_UBALANCE);
		}
	}
	if (nValid) {
		memcpy(&aBuf[0], &nTn, 2);
		aBuf[2] = BITMASK(0);
		gw3761_ConvertData_05_Percent(&aBuf[3], FLOAT2FIX(pa->ub), 0);
		gw3761_ConvertData_05_Percent(&aBuf[5], FLOAT2FIX(pa->ib), 0);
		for (j = 0; j < 3; j++) {
			gw3761_ConvertData_07(&aBuf[7 + j * 2], FLOAT2FIX(pa->u[j]));
		}
		for (j = 0; j < 3; j++) {
			gw3761_ConvertData_25(&aBuf[13 + j * 3], FLOAT2FIX(pa->i[j]), 1);
		}
		evt_ERC17(aBuf);
	}

	fTemp = (float)bcd2bin16(pF26->ibalance) / 1000.0f;
	nValid = 0;
	fData = pa->ib;
	if (GETBIT(nFlagOld, EVT_FLAG_IBALANCE)) {
		//恢复
		if (fData < fTemp) {
			nTn = TERMINAL;
			nValid = 1;
			CLRBIT(nFlag, EVT_FLAG_IBALANCE);
		}
	} else {
		//发生
		if (fData > fTemp) {
			nTn = TERMINAL | BITMASK(15);
			nValid = 1;
			SETBIT(nFlag, EVT_FLAG_IBALANCE);
		}
	}
	if (nValid) {
		memcpy(&aBuf[0], &nTn, 2);
		aBuf[2] = BITMASK(1);
		gw3761_ConvertData_05_Percent(&aBuf[3], FLOAT2FIX(pa->ub), 0);
		gw3761_ConvertData_05_Percent(&aBuf[5], FLOAT2FIX(pa->ib), 0);
		for (j = 0; j < 3; j++) {
			gw3761_ConvertData_07(&aBuf[7 + j * 2], FLOAT2FIX(pa->u[j]));
		}
		for (j = 0; j < 3; j++) {
			gw3761_ConvertData_25(&aBuf[13 + j * 3], FLOAT2FIX(pa->i[j]), 1);
		}
		evt_ERC17(aBuf);
	}

	//电压越上限
	fTemp = (float)bcd2bin16(pF26->uover) / 10.0f;
	for (i = 0; i < 3; i++) {
		nValid = 0;
		fData = pa->u[i];
		if (GETBIT(nFlagOld, EVT_FLAG_UOVER + i)) {
			//恢复
			if (fData < fTemp) {
				nTn = TERMINAL;
				nValid = 1;
				CLRBIT(nFlag, EVT_FLAG_UOVER + i);
			}
		} else {
			//发生
			if (fData > fTemp) {
				nTn = TERMINAL | BITMASK(15);
				nValid = 1;
				SETBIT(nFlag, EVT_FLAG_UOVER + i);
			}
		}
		if (nValid) {
			memcpy(&aBuf[0], &nTn, 2);
			aBuf[2] = BITMASK(6) | BITMASK(i);
			for (j = 0; j < 3; j++) {
				gw3761_ConvertData_07(&aBuf[3 + j * 2], FLOAT2FIX(pa->u[j]));
			}
			evt_ERC24(aBuf);
		}
	}

	//电压越下限
	fTemp = (float)bcd2bin16(pF26->uunder) / 10.0f;
	for (i = 0; i < 3; i++) {
		nValid = 0;
		fData = pa->u[i];
		if (GETBIT(nFlagOld, EVT_FLAG_UUNDER + i)) {
			//恢复
			if (fData > fTemp) {
				nTn = TERMINAL;
				nValid = 1;
				CLRBIT(nFlag, EVT_FLAG_UUNDER + i);
			}
		} else {
			//发生
			if (fData < fTemp) {
				nTn = TERMINAL | BITMASK(15);
				nValid = 1;
				SETBIT(nFlag, EVT_FLAG_UUNDER + i);
			}
		}
		if (nValid) {
			memcpy(&aBuf[0], &nTn, 2);
			aBuf[2] = BITMASK(7) | BITMASK(i);
			for (j = 0; j < 3; j++) {
				gw3761_ConvertData_07(&aBuf[3 + j * 2], FLOAT2FIX(pa->u[j]));
			}
			evt_ERC24(aBuf);
		}
	}

	//电流越限
	memcpy(&nData, pF26->iover, 3);
	fTemp = (float)bcd2bin32(nData) / 1000.0f;
	for (i = 0; i < 3; i++) {
		nValid = 0;
		fData = pa->i[i];
		if (GETBIT(nFlagOld, EVT_FLAG_IOVER + i)) {
			//恢复
			if (fData < fTemp) {
				nTn = TERMINAL;
				nValid = 1;
				CLRBIT(nFlag, EVT_FLAG_IOVER + i);
			}
		} else {
			//发生
			if (fData > fTemp) {
				nTn = TERMINAL | BITMASK(15);
				nValid = 1;
				SETBIT(nFlag, EVT_FLAG_IOVER + i);
			}
		}
		if (nValid) {
			memcpy(&aBuf[0], &nTn, 2);
			aBuf[2] = BITMASK(6) | BITMASK(i);
			for (j = 0; j < 3; j++) {
				gw3761_ConvertData_25(&aBuf[3 + j * 3], FLOAT2FIX(pa->i[j]), 1);
			}
			evt_ERC25(aBuf);
		}
	}
#endif

	//电压异常
	fTemp = (float)bcd2bin16(pF26->ubreak) / 10.0f;
	for (i = 0; i < 3; i++) {
		nValid = 0;
		fData = pa->u[i];
		if (GETBIT(nFlagOld, EVT_FLAG_UABNORMAL + i)) {
			//恢复
			if (fData > fTemp) {
				if (sfs_Read(&evt_SfsDev, EVT_UABNORMAL_TIME + i, &tTime) != SYS_R_OK)
					tTime = rtc_GetTimet();
				nTn = TERMINAL;
				if (rtc_GetTimet() > tTime)
					nData = (rtc_GetTimet() - tTime) / 60;
				else
					nData = 0;
				nValid = 1;
				CLRBIT(nFlag, EVT_FLAG_UABNORMAL + i);
				sfs_Delete(&evt_SfsDev, EVT_UABNORMAL_TIME + i);
			}
		} else {
			//发生
			if (fData < fTemp) {
				if (sfs_Read(&evt_SfsDev, EVT_UABNORMAL_TIME + i, &tTime) == SYS_R_OK) {
					nTn = TERMINAL | BITMASK(15);
					nData = 0;
					nValid = 1;
					SETBIT(nFlag, EVT_FLAG_UABNORMAL + i);
				} else {
					tTime = rtc_GetTimet();
					sfs_Write(&evt_SfsDev, EVT_UABNORMAL_TIME + i, &tTime, 4);
				}
			}
		}
		if (nValid) {
			memcpy(&aBuf[0], &nTn, 2);
			aBuf[2] = BITMASK(6) | BITMASK(i);
			for (j = 0; j < 3; j++) {
				gw3761_ConvertData_07(&aBuf[3 + j * 2], FLOAT2FIX(pa->u[j]));
			}
			for (j = 0; j < 3; j++) {
				gw3761_ConvertData_25(&aBuf[9 + j * 3], FLOAT2FIX(pa->i[j]), 1);
			}
			memcpy(&aBuf[18], &nData, 4);
			evt_ERC10(aBuf);
		}
	}

	//电流越限
	nData = 0;
	memcpy(&nData, pF26->iup, 3);
	fTemp = (float)bcd2bin32(nData) / 1000.0f;
	for (i = 0; i < 3; i++) {
		nValid = 0;
		fData = pa->i[i];
		if (GETBIT(nFlagOld, EVT_FLAG_IUP + i)) {
			//恢复
			if (fData < fTemp) {
				if (sfs_Read(&evt_SfsDev, EVT_IUP_TIME + i, &tTime) != SYS_R_OK)
					tTime = rtc_GetTimet();
				nTn = TERMINAL;
				if (rtc_GetTimet() > tTime)
					nData = (rtc_GetTimet() - tTime) / 60;
				else
					nData = 0;
				nValid = 1;
				CLRBIT(nFlag, EVT_FLAG_IUP + i);
			}
		} else {
			//发生
			if (fData > fTemp) {
				nTn = TERMINAL | BITMASK(15);
				nValid = 1;
				SETBIT(nFlag, EVT_FLAG_IUP + i);
				tTime = rtc_GetTimet();
				sfs_Write(&evt_SfsDev, EVT_IUP_TIME + i, &tTime, 4);
			}
		}
		if (nValid) {
			memcpy(&aBuf[0], &nTn, 2);
			aBuf[2] = BITMASK(7) | BITMASK(i);
			for (j = 0; j < 3; j++) {
				gw3761_ConvertData_25(&aBuf[3 + j * 3], FLOAT2FIX(pa->i[j]), 1);
			}
			memcpy(&aBuf[12], &nData, 4);
			if (nTn &  BITMASK(15))
				memset(&aBuf[16], 0, 12);
			else
				evt_IupMax(&aBuf[16], tTime, rtc_GetTimet());
			evt_ERC25(aBuf);
		}
	}

#if 0
	//视在功率越限
	memcpy(&nData, pF26->uiover, 3);
	fTemp = (float)bcd2bin32(nData) / 10000.0f;
	nValid = 0;
	fData = pa->ui[3];
	if (GETBIT(nFlagOld, EVT_FLAG_UIOVER)) {
		//恢复
		if (fData < fTemp) {
			nTn = TERMINAL;
			nValid = 1;
			CLRBIT(nFlag, EVT_FLAG_UIOVER);
		}
	} else {
		//发生
		if (fData > fTemp) {
			nTn = TERMINAL | BITMASK(15);
			nValid = 1;
			SETBIT(nFlag, EVT_FLAG_UIOVER);
		}
	}
	if (nValid) {
		memcpy(&aBuf[0], &nTn, 2);
		aBuf[2] = BITMASK(6);
		gw3761_ConvertData_23(&aBuf[3], FLOAT2FIX(fData));
		memcpy(&aBuf[6], pF26->uiup, 3);
		evt_ERC26(aBuf);
	}

	memcpy(&nData, pF26->uiup, 3);
	fTemp = (float)bcd2bin32(nData) / 10000.0f;
	nValid = 0;
	fData = pa->ui[3];
	if (GETBIT(nFlagOld, EVT_FLAG_UIUP)) {
		//恢复
		if (fData < fTemp) {
			nTn = TERMINAL;
			nValid = 1;
			CLRBIT(nFlag, EVT_FLAG_UIUP);
		}
	} else {
		//发生
		if (fData > fTemp) {
			nTn = TERMINAL | BITMASK(15);
			nValid = 1;
			SETBIT(nFlag, EVT_FLAG_UIUP);
		}
	}
	if (nValid) {
		memcpy(&aBuf[0], &nTn, 2);
		aBuf[2] = BITMASK(7);
		gw3761_ConvertData_23(&aBuf[3], FLOAT2FIX(fData));
		memcpy(&aBuf[6], pF26->uiup, 3);
		evt_ERC26(aBuf);
	}
#endif

	if (nFlag != nFlagOld)
		sfs_Write(&evt_SfsDev, EVT_FLAG_ADDR, &nFlag, 4);
}

uint_t evt_YXRead()
{
	uint_t i, nValid, nFlag, nRead;

	nRead = 0;
	for (i = 0; i < 3; i++) {
		if (gpio_Read(4 + i))
			SETBIT(nRead, i);
	}
	nFlag = 0;
	nValid = 0;
	if (sfs_Read(&evt_SfsDev, EVT_YX_ADDR, &nFlag) != SYS_R_OK)
		nValid = 1;
	for (i = 0; i < 3; i++) {
		if (nFlag & BITMASK(i)) {
			if ((nRead & BITMASK(i)) == 0) {
				evt_ERC18(i, 0);
				nValid = 1;
				CLRBIT(nFlag, i);
			}
		} else {
			if (nRead & BITMASK(i)) {
				evt_ERC18(i, 1);
				nValid = 1;
				SETBIT(nFlag, i);
			}
		}
	}
	if (nValid)
		sfs_Write(&evt_SfsDev, EVT_YX_ADDR, &nRead, 2);
	return nRead;
}

//运行时间
int evt_RunTimeRead(time_t *pTime)
{

	if (sfs_Read(&evt_SfsDev, EVT_RUNTIME_ADDR, pTime) == SYS_R_OK)
		return 1;
	return 0;
}

void evt_RunTimeWrite(time_t tTime)
{

	evt_Lock();
	sfs_Write(&evt_SfsDev, EVT_RUNTIME_ADDR, &tTime, sizeof(time_t));
	evt_Unlock();
}

void evt_Init()
{
	uint_t nVer, nInit = 0;

#if EVT_LOCK_ENABLE
	rt_sem_init(&evt_sem, "sem_evt", 1, RT_IPC_FLAG_FIFO);
#endif
	if (sfs_Read(&evt_SfsDev, EVT_MAGIC_WORD, NULL) != SYS_R_OK)
		nInit = 1;
	nVer = icp_GetVersion();
	if (nVer < 0x0095) {
		nInit = 1;
		icp_ParaWrite(0x04, 1, TERMINAL, "\x14\x02\x1E\x30\x00\x05", 6);
	}
	if (nInit)
		evt_Format();
	if (nVer != VER_SOFT) {
		//版本变更事件
		evt_ERC1(nVer);
		icp_SetVersion();
	}
	//停上电事件
	evt_ERC14();
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
	uint8_t aBuf[4] = {0};

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
			nStart = nCnt & (~0xFF);
		} else
			nStart = 0;
	} else
		nStart = nCnt & (~0xFF);
	i = nStart + nPm;
	buf_PushData(b, nPm, 2);
	nLen = b->len - 1;
	nIsNormal = EVT_DATA_BASE + nIsNormal * (EVT_QTY * EVT_SIZE);
	for (nLast = i; (i < (nStart + nPn)) && (i < nCnt); i++) {
		spif_Read(nIsNormal + (i & (EVT_QTY - 1)) * EVT_SIZE, aBuf, sizeof(aBuf));
		buf_Push(b, aBuf, aBuf[1] + 2);
		nLast = i + 1;
	}
	b->p[nLen] = nLast - nStart;
	return (i - (nStart + nPm));
}

sys_res evt_DlqQlStateGet(uint_t nSn, void *pBuf)
{

	return sfs_Read(&evt_SfsDev, EVT_DLQ_QL_STE | nSn, pBuf);
}

void evt_DlqQlStateSet(uint_t nSn, const void *pBuf)
{

	evt_Lock();
	sfs_Write(&evt_SfsDev, EVT_DLQ_QL_STE | nSn, pBuf, 3);
	evt_Unlock();
}

sys_res evt_DlqQlParaGet(uint_t nSn, void *pBuf)
{

	return sfs_Read(&evt_SfsDev, EVT_DLQ_QL_PARA | nSn, pBuf);
}

void evt_DlqQlParaSet(uint_t nSn, const void *pBuf)
{

	evt_Lock();
	sfs_Write(&evt_SfsDev, EVT_DLQ_QL_PARA | nSn, pBuf, 11);
	evt_Unlock();
}



