//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
volatile EVENT g_xEvent;
volatile uint8_t *g_pEvent;
void event_WaitReady(uint8_t nERC)
{
	if (g_xEvent.nERC == nERC) {
		return ;
	}
	while (g_xEvent.nERC) {
		// 等待其它事件处理完成
		os_dly_wait(1);
	}
	g_xEvent.nERC = nERC;
	g_xEvent.nLen = 0;
	return ;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------

void event_Finish(uint8_t nERC)
{
	if (g_xEvent.nERC == nERC) {
		g_xEvent.nERC = 0;
	}
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
int rcp_GDW_Event_Enable(uint_t nERC)
{
	if (reg_GetBit(TERMINAL, 0x0410, nERC - 1)) {
		return 1;
	}
	return 0;
}

//-------------------------------------------------------------------------
// 数据初始化和版本变更记录
//-------------------------------------------------------------------------
int rcp_GDW_Event_ERC1(uint8_t nERC, uint8_t *pData, time_t tTime) 
{
	EVENT xEvent;
	int nResult;
	uint8_t arrData[14] = {0};

	xEvent.nERC = 1;
	if (rcp_GDW_Event_Enable(xEvent.nERC) == 0) {
		return -1;
	}
	xEvent.nLen = 14;
	gw3761_ConvertData_15(arrData, tTime);
	arrData[5] = nERC & 0x03;
	if (nERC & 0x02) {
		memcpy(&arrData[6], pData, 8);
	}
	nResult = event_Save(&xEvent, arrData);
	return nResult;
}

//-------------------------------------------------------------------------
// 参数变更记录
//-------------------------------------------------------------------------
int rcp_GDW_Event_ERC3(uint8_t *pData, uint32_t nLen, uint8_t nAddress, time_t tTime)
{
	EVENT xEvent;
	int nResult;
	uint8_t *pBuf;

	xEvent.nERC = 3;
	if (rcp_GDW_Event_Enable(xEvent.nERC) == 0) {
		return -1;
	}
	xEvent.nLen = 6 + nLen;
	pBuf = mem_Malloc(xEvent.nLen);
	if (pBuf != NULL) {
		gw3761_ConvertData_15(pBuf, tTime);
		pBuf[5] = nAddress;
		memcpy(&pBuf[6], pData, nLen);
		nResult = event_Save(&xEvent, pBuf);
		mem_Free(pBuf);
	}
	return nResult;
}

//-------------------------------------------------------------------------
// 状态量变位记录
//-------------------------------------------------------------------------
int rcp_GDW_Event_ERC4(uint8_t nChangeSign, uint8_t nNewState, time_t tTime)
{
	EVENT xEvent;
	int nResult;
	uint8_t arrData[7];

	xEvent.nERC = 4;
	if (rcp_GDW_Event_Enable(xEvent.nERC) == 0) {
		return -1;
	}
	xEvent.nLen = 7;
	gw3761_ConvertData_15(arrData, tTime);
	arrData[5] = nChangeSign;
	arrData[6] = nNewState;
	nResult = event_Save(&xEvent, arrData);
	return nResult;
}

//-------------------------------------------------------------------------
// 电能表参数变更记录
//-------------------------------------------------------------------------
int rcp_GDW_Event_ERC8(uint8_t nDa, uint8_t nSign, time_t tTime)
{
	EVENT xEvent;
	uint8_t arrData[7] = {0};

	xEvent.nERC = 8;
	if (rcp_GDW_Event_Enable(xEvent.nERC) == 0) {
		return -1;
	}
	xEvent.nLen = 7;
	gw3761_ConvertData_15(arrData, tTime);
	arrData[5] = nDa;
	arrData[6] = nSign;
	return (event_Save(&xEvent, arrData));
}


//-------------------------------------------------------------------------
// 电流回路异常
//-------------------------------------------------------------------------
int rcp_GDW_Event_ERC9(uint8_t nDa, uint32_t *pVol, uint32_t *pCurrent, uint32_t nEnergy, time_t tTime)
{
	EVENT xEvent;
	int nResult = 0;
	uint8_t arrState[3], arrData[24];
	uint32_t i, nState;
	S32 nData;

	xEvent.nERC = 9;
	if (rcp_GDW_Event_Enable(xEvent.nERC) == 0) {
		return -1;
	}
	xEvent.nLen = 24;
	reg_Get(nDa, 0x8E09, arrState);
	for (i = 0; i < 3; ++i) {
		nData = reg_GetValue(nDa, 0xB681 + i);
		if (nData < 0) {
			// 电流反极性
			nState = 3;
		} else {
			nState = 0;
		}
		switch (nState) {
			case 3:
				if (arrState[i] != nState) {
					arrState[i] = nState;
					nResult = 3;
				}
				break;
			case 0:
			default:
				// 正常状态,判恢复
				if (arrState[i] == 3) {
					nResult = 6;
				}
				arrState[i] = 0;
				break;
		}
		switch (nResult) {
			case 6:// 反向恢复
				arrData[5] = nDa;
				arrData[6] = 0xA0 | GETBIT(i);
				break;
			case 3:// 反向
				arrData[5] = 0x80 | nDa;
				arrData[6] = 0xA0 | GETBIT(i);
				break;
			default:
				break;
		}
		if (nResult != 0) {
			gw3761_ConvertData_15(arrData, tTime);
			for (i = 0; i < 3; ++i) {
				gw3761_ConvertData_07(&arrData[7 + 2 * i], pVol[i]);
				gw3761_ConvertData_06(&arrData[13 + 2 * i], pCurrent[i], 1);
			}
			gw3761_ConvertData_14(&arrData[19], nEnergy);
			nResult = event_Save(&xEvent, arrData);
		}
	}
	reg_Set(nDa, 0x8E09, arrState, 0);
	return nResult;
}

//-------------------------------------------------------------------------
// 电压回路异常
//-------------------------------------------------------------------------
int rcp_GDW_Event_ERC10(uint8_t nDa, uint32_t *pVol, uint32_t *pCurrent, uint32_t nEnergy, time_t tTime)
{
	EVENT xEvent;
	int nResult = 0, nThreshold;
	uint8_t arrState[3], arrData[24];
	uint32_t i, nState;

	xEvent.nERC = 10;
	if (rcp_GDW_Event_Enable(xEvent.nERC) == 0) {
		return -1;
	}
	xEvent.nLen = 24;
	reg_Get(nDa, 0x8E10, arrState);
	nThreshold = reg_GetValue(nDa, 0x2773);
	for (i = 0; i < 3; ++i) {
		if (pVol[i] > nThreshold) {
			// 正常状态
			nState = 0;
		} else if (pVol[i] < (100 << EXP)) {
			// 失压状态
			nState = 2;
		} else {
			// 断相状态
			nState = 1;
		}
		switch (nState) {
			case 2:
				if (arrState[i] & 0x80) {
					// 已经进入失压状态,等超时
					if ((arrState[i] & 0x7F) != 0) {
						if ((--arrState[i] & 0x7F) == 0) {
							nResult = 2;
						}
					}
				} else {
					// 进入失压状态
					arrState[i] = 0x80 | reg_GetValue(nDa, 0x2791);
				}
				break;
			case 1:
				// 断相状态
				if (arrState[i] != nState) {
					nResult = 1;
					arrState[i] = 1;
				}
				break;
			case 0:
			default:
				// 正常状态,判恢复
				if (arrState[i] & 0x80) {
					if ((arrState[i] & 0x7F) == 0) {
						nResult = 4;
					}
				} else if (arrState[i] == 1) {
					nResult = 3;
				}
				arrState[i] = 0;
				break;
		}
		switch (nResult) {
			case 4:// 失压恢复
				arrData[5] = nDa;
				arrData[6] = 0x80 | GETBIT(i);
				break;
			case 3:// 断相恢复
				arrData[5] = nDa;
				arrData[6] = 0x40 | GETBIT(i);
				break;
			case 2:// 失压
				arrData[5] = 0x80 | nDa;
				arrData[6] = 0x80 | GETBIT(i);
				break;
			case 1:// 断相
				arrData[5] = 0x80 | nDa;
				arrData[6] = 0x40 | GETBIT(i);
				break;
			default:
				break;
		}
		if (nResult != 0) {
			gw3761_ConvertData_15(arrData, tTime);
			for (i = 0; i < 3; ++i) {
				gw3761_ConvertData_07(&arrData[7 + 2 * i], pVol[i]);
				gw3761_ConvertData_06(&arrData[13 + 2 * i], pCurrent[i], 1);
			}
			gw3761_ConvertData_14(&arrData[19], nEnergy);
			nResult = event_Save(&xEvent, arrData);
		}
	}
	reg_Set(nDa, 0x8E10, arrState, 0);
	return nResult;
}

//-------------------------------------------------------------------------
// 相序异常
//-------------------------------------------------------------------------
int rcp_GDW_Event_ERC11(uint8_t nDa, uint32_t nEnergy, time_t tTime)
{
	EVENT xEvent;
	int nResult = 0, i, nTemp;
	uint8_t arrData[23];

	xEvent.nERC = 11;
	if (rcp_GDW_Event_Enable(xEvent.nERC) == 0) {
		return -1;
	}
	if (reg_GetBit(nDa, 0xC091, 0) || reg_GetBit(nDa, 0xC092, 0)) {
		if (reg_GetBit(nDa, 0x8E11, 0) == 0) {
			// 发生
			arrData[5] = 0x80 | nDa;
			reg_SetValue(nDa, 0x8E11, 1, 0);
			nResult = 1;
		}
	} else {
		if (reg_GetBit(nDa, 0x8E11, 0)) {
			// 恢复
			arrData[5] = nDa;
			reg_SetValue(nDa, 0x8E11, 0, 0);
			nResult = 1;
		}
	}
	if (nResult) {
		xEvent.nLen = 23;
		gw3761_ConvertData_15(arrData, tTime);
		for (i = 0; i < 3; ++i) {
			reg_Get(nDa, 0xB671 + i, (uint8_t *)&nTemp);
			gw3761_ConvertData_05(&arrData[6 + 2 * i], nTemp, 1);
			reg_Get(nDa, 0xB681 + i, (uint8_t *)&nTemp);
			gw3761_ConvertData_05(&arrData[12 + 2 * i], nTemp, 1);
		}
		gw3761_ConvertData_14(&arrData[18], nEnergy);
		nResult = event_Save(&xEvent, arrData);
	}
	return nResult;
}

//-------------------------------------------------------------------------
// 电能表时间超差
//-------------------------------------------------------------------------
int rcp_GDW_Event_ERC12(uint8_t nDa, time_t tTime)
{
	EVENT xEvent;
	int nResult = 0;
	uint8_t arrData[6];
	uint32_t nTemp;
	time_t tMeterTime;

	xEvent.nERC = 12;
	if (rcp_GDW_Event_Enable(xEvent.nERC) == 0) {
		return -1;
	}
	tMeterTime = reg_GetValue(nDa, 0xC010);
	nTemp = reg_GetValue(TERMINAL, 0x0854);
	if ((tMeterTime > (tTime + nTemp)) || (tMeterTime < (tTime - nTemp))) {
		if (reg_GetBit(nDa, 0x8E12, 0) == 0) {
			// 发生
			arrData[5] = 0x80 | nDa;
			reg_SetValue(nDa, 0x8E12, 1, 0);
			nResult = 1;
		}
	} else {
		if (reg_GetBit(nDa, 0x8E12, 0)) {
			// 恢复
			arrData[5] = nDa;
			reg_SetValue(nDa, 0x8E12, 0, 0);
			nResult = 1;
		}
	}
	if (nResult) {
		xEvent.nLen = 6;
		gw3761_ConvertData_15(arrData, tTime);
		nResult = event_Save(&xEvent, arrData);
	}
	return nResult;
}

//-------------------------------------------------------------------------
// 终端停/上电事件
//-------------------------------------------------------------------------
int rcp_GDW_Event_ERC14(uint32_t nState, time_t tTime)
{
	EVENT xEvent;
	int nResult = 0;
	uint8_t arrData[10] = {0};

	xEvent.nERC = 14;
	if (rcp_GDW_Event_Enable(xEvent.nERC) == 0) {
		return -1;
	}
	xEvent.nLen = 10;
	nState &= 0x00000001;
	if (nState != reg_GetBit(TERMINAL, 0x0414, 0)) {
		reg_Set(TERMINAL, 0x0414, (uint8_t *)&nState, 0);
		gw3761_ConvertData_15(arrData + nState * 5, tTime);
		nResult = event_Save(&xEvent, arrData);
	}
	return nResult;
}

//-------------------------------------------------------------------------
// 谐波越限事件
//-------------------------------------------------------------------------
int rcp_GDW_Event_ERC15(uint8_t nDa, uint32_t nPulse, uint32_t nNewSign, uint16_t *pThd, time_t tTime)
{
	EVENT xEvent;
	uint8_t arrData[48] = {0};
	uint32_t nChangeSign, nOldSign = 0, nOver = 0, nRestore = 0, i;
	int nResult = 0;

	xEvent.nERC = 15;
	if (rcp_GDW_Event_Enable(xEvent.nERC) == 0) {
		return -1;
	}
	xEvent.nLen = 48;
	reg_Get(nDa, 0x8E15, arrData);
	memcpy((uint8_t *)&nOldSign, &arrData[3 * nPulse], 3);
	memcpy(&arrData[3 * nPulse], (uint8_t *)&nNewSign, 3);
	reg_Set(nDa, 0x8E15, arrData, 0);
	nChangeSign = nNewSign ^ nOldSign;
	if (nChangeSign) {
		for (i = 0; i < 19; ++i) {
			if (nChangeSign & GETBIT(i)) {
				if (nNewSign & GETBIT(i)) {
					nOver = 1;
				} else {
					nRestore = 1;
				}
			}
		}
		gw3761_ConvertData_15(arrData, tTime);
		if (nPulse < 3) {
			arrData[6] = GETBIT(nPulse);
		} else {
			arrData[6] = 0x80 | GETBIT(nPulse - 3);
		}
		memcpy(&arrData[7], (uint8_t *)&nNewSign, 3);
		memcpy(&arrData[10], pThd, 38);
		if (nOver) {
			arrData[5] = 0x80 | nDa;
			nResult = event_Save(&xEvent, arrData);
		}
		if (nRestore) {
			arrData[5] = nDa;
			nResult = event_Save(&xEvent, arrData);
		}
	}
	return nResult;
}

//-------------------------------------------------------------------------
// 不平衡度越限
//-------------------------------------------------------------------------
int rcp_GDW_Event_ERC17(uint8_t nDa, uint32_t nType, uint32_t nState, uint32_t *pVol, uint32_t *pCurrent, time_t tTime)
{
	EVENT xEvent;
	int nResult = 0;
	uint8_t nOldState, arrData[23];

	xEvent.nERC = 17;
	if (rcp_GDW_Event_Enable(xEvent.nERC) == 0) {
		return -1;
	}
	xEvent.nLen = 23;
	nOldState = reg_GetValue(nDa, 0x8E17);
	if (nState) {
		// 越限
		if ((nOldState & GETBIT(nType)) == 0) {
			nOldState |= GETBIT(nType);
			nResult = 1;
		}
	} else {
		// 越限恢复
		if (nOldState & GETBIT(nType)) {
			nOldState &= BITMASK(nType);
			nResult = 2;
		}
	}
	switch (nResult) {
		case 2:
			// 越限恢复
			arrData[5] = nDa;
			arrData[6] = nOldState;
			break;
		case 1:
			// 越限
			arrData[5] = 0x80 | nDa;
			arrData[6] = nOldState;
			break;
		default:
			break;
	}
	if (nResult != 0) {
		gw3761_ConvertData_15(arrData, tTime);
		gw3761_ConvertData_05(&arrData[7], pVol[0], 0);
		gw3761_ConvertData_05(&arrData[9], pCurrent[0], 0);
		for (nType = 3; nType; --nType) {
			gw3761_ConvertData_07(&arrData[9 + 2 * nType], pVol[nType]);
			gw3761_ConvertData_06(&arrData[15 + 2 * nType], pCurrent[nType], 1);
		}
		nResult = event_Save(&xEvent, arrData);
	}
	reg_Set(nDa, 0x8E17, (uint8_t *)&nOldState, 0);
	return nResult;
}

//-------------------------------------------------------------------------
// 终端故障记录
//-------------------------------------------------------------------------
int rcp_GDW_Event_ERC21(uint32_t nType, time_t tTime)
{
	EVENT xEvent;
	uint8_t arrData[6];

	xEvent.nERC = 21;
	if (rcp_GDW_Event_Enable(xEvent.nERC) == 0) {
		return -1;
	}
	if (reg_GetBit(TERMINAL, 0x0455, 20)) {
		// 该事件已发生
		return 0;
	}
	xEvent.nLen = 6;
	gw3761_ConvertData_15(arrData, tTime);
	arrData[5] = nType;
	return (event_Save(&xEvent, arrData));
}

//-------------------------------------------------------------------------
// 差动越限记录
//-------------------------------------------------------------------------
int rcp_GDW_Event_ERC22(uint32_t nDa, uint32_t nSign, time_t tTime)
{
	EVENT xEvent;
	int nResult = 0;
	uint8_t *pTemp, arrData[64];
	uint32_t nState, i, nRelativeMargin, nRelativeThreshold, nQid, nqid, nTemp, nOffset, nQQty, nqQty, q, Q, nAbsoluteMargin, nAbsoluteThreshold;

	xEvent.nERC = 22;
	nQid = reg_GetValue(nDa, 0x2652);
	Q = reg_GetValue(nQid, 0x8130);
	nqid = reg_GetValue(nDa, 0x2653);
	q = reg_GetValue(nqid, 0x8130);
	nAbsoluteMargin = abs(Q - q);
	nRelativeMargin = nAbsoluteMargin * 100 / q;
	nAbsoluteThreshold = reg_GetValue(nDa, 0x2656);
	nRelativeThreshold = reg_GetValue(nDa, 0x2655);
	if (nSign & 0x80){
		// 绝对对比
		nState = (nAbsoluteMargin > nAbsoluteThreshold) ? 1 : 0;
	} else {
		// 相对对比
		nState = (nRelativeMargin > nRelativeThreshold) ? 1 : 0;
	}
	if (nState) {
		if (reg_GetValue(nDa, 0x8E22) == 0) {
			nResult = 1;//发生
		}
	} else {
		if (reg_GetValue(nDa, 0x8E22)) {
			nResult = 2;//恢复
		}
	}
	reg_Set(nDa, 0x8E22, (uint8_t *)&nState, 0);
	if (nResult) {
		if (nResult == 1) {
			nSign = nDa;
		} else {
			nSign = 0x80 | nDa;
		}
		if ((nQQty = reg_GetValue(nQid, 0x2511)) > 23) {
			nQQty = 23;// xEvent.nLen < 256
		}
		if ((nqQty = reg_GetValue(nqid, 0x2511)) > 23) {
			nqQty = 23;// xEvent.nLen < 256
		}
		xEvent.nLen = 21 + 5 * (nQQty + nqQty);
		pTemp = mem_Malloc(xEvent.nLen);
		if (pTemp != NULL) {
			gw3761_ConvertData_15(pTemp, tTime);
			pTemp[5] = nSign;
			gw3761_ConvertData_03(&pTemp[6], Q, 0);
			gw3761_ConvertData_03(&pTemp[10], q, 0);
			pTemp[11] = nRelativeThreshold;
			gw3761_ConvertData_03(&pTemp[12], nAbsoluteThreshold, 0);
			pTemp[16] = nQQty;
			reg_Get(nQid, 0x2512, arrData);
			for (i = 0; i < nQQty; ++i) {
				nTemp = reg_GetValue(arrData[i] & 0x3F, 0x9010);
				gw3761_ConvertData_14(&pTemp[17 + i * 5], nTemp);
			}
			nOffset = 18 + i * nQQty;
			pTemp[nOffset - 1] = nqQty;
			reg_Get(nqid, 0x2512, arrData);
			for (i = 0; i < nqQty; ++i) {
				nTemp = reg_GetValue(arrData[i] & 0x3F, 0x9010);
				gw3761_ConvertData_14(&pTemp[nOffset + i * 5], nTemp);
			}
			nResult = event_Save(&xEvent, pTemp);
			mem_Free(pTemp);
		}
	}
	return nResult;
}

//-------------------------------------------------------------------------
// 电压越限
//-------------------------------------------------------------------------
int rcp_GDW_Event_ERC24(uint8_t nDa, uint32_t *pVol, time_t tTime)
{
	EVENT xEvent;
	int nResult = 0, nHighThreshold, nLowThreshold;
	uint8_t arrState[3], arrData[13];
	uint32_t i, nState;

	xEvent.nERC = 24;
	if (rcp_GDW_Event_Enable(xEvent.nERC) == 0) {
		return -1;
	}
	xEvent.nLen = 13;
	reg_Get(nDa, 0x8E24, arrState);
	nHighThreshold = reg_GetValue(nDa, 0x2774);
	nLowThreshold = reg_GetValue(nDa, 0x2775);
	for (i = 0; i < 3; ++i) {
		if (pVol[i] < nLowThreshold) {
			// 越下下限
			nState = 2;
		}
		else if (pVol[i] > nHighThreshold) {
			// 越上上限
			nState = 1;
		}
		else {
			// 正常状态
			nState = 0;
		}
		if (arrState[i] != nState) {
			switch (nState) {
				case 2:
					arrData[5] = 0x80 | nDa;
					arrData[6] = 0x80 | GETBIT(i);
					break;
				case 1:
					arrData[5] = 0x80 | nDa;
					arrData[6] = 0x40 | GETBIT(i);
					break;
				case 0:
				default:
					arrData[5] = nDa;
					if (arrState[i] == 1) {
						arrData[6] = 0x40 | GETBIT(i);
					} else {
						arrData[6] = 0x80 | GETBIT(i);
					}
					break;
			}
			arrState[i] = nState;
			gw3761_ConvertData_15(arrData, tTime);
			for (i = 0; i < 3; ++i) {
				gw3761_ConvertData_07(&arrData[7 + 2 * i], pVol[i]);
			}
			nResult = event_Save(&xEvent, arrData);
		}
	}
	reg_Set(nDa, 0x8E24, arrState, 0);
	return nResult;
}

//-------------------------------------------------------------------------
// 电流越限
//-------------------------------------------------------------------------
int rcp_GDW_Event_ERC25(uint8_t nDa, uint32_t *pCurrent, time_t tTime)
{
	EVENT xEvent;
	int nResult = 0, nHighThreshold, nLowThreshold;
	uint8_t arrState[3], arrData[13];
	uint32_t i, nState;

	xEvent.nERC = 25;
	if (rcp_GDW_Event_Enable(xEvent.nERC) == 0) {
		return -1;
	}
	xEvent.nLen = 13;
	reg_Get(nDa, 0x8E25, arrState);
	nHighThreshold = reg_GetValue(nDa, 0x2776);
	nLowThreshold = reg_GetValue(nDa, 0x2777);
	for (i = 0; i < 3; ++i) {
		if (pCurrent[i] > nHighThreshold) {
			// 越上上限
			nState = 1;
		}
		else if (pCurrent[i] > nLowThreshold) {
			// 越上限
			nState = 2;
		}
		else {
			// 正常状态
			nState = 0;
		}
		if (arrState[i] != nState) {
			switch (nState) {
				case 2:
					arrData[5] = 0x80 | nDa;
					arrData[6] = 0x80 | GETBIT(i);
					break;
				case 1:
					arrData[5] = 0x80 | nDa;
					arrData[6] = 0x40 | GETBIT(i);
					break;
				case 0:
				default:
					arrData[5] = nDa;
					if (arrState[i] == 1) {
						arrData[6] = 0x40 | GETBIT(i);
					} else {
						arrData[6] = 0x80 | GETBIT(i);
					}
					break;
			}
			arrState[i] = nState;
			gw3761_ConvertData_15(arrData, tTime);
			for (i = 0; i < 3; ++i) {
				gw3761_ConvertData_06(&arrData[7 + 2 * i], pCurrent[i], 1);
			}
			nResult = event_Save(&xEvent, arrData);
		}
	}
	reg_Set(nDa, 0x8E25, arrState, 0);
	return nResult;
}

//-------------------------------------------------------------------------
// 视在功率越限
//-------------------------------------------------------------------------
int rcp_GDW_Event_ERC26(uint8_t nDa, uint32_t nState, uint32_t nData, uint32_t nHighThreshold, uint32_t nLowThreshold, time_t tTime)
{
	EVENT xEvent;
	int nResult = 0;
	uint8_t nOldState, arrData[13];

	xEvent.nERC = 26;
	if (rcp_GDW_Event_Enable(xEvent.nERC) == 0) {
		return -1;
	}
	xEvent.nLen = 13;
	nOldState = reg_GetValue(nDa, 0x8E26);
	reg_Set(nDa, 0x8E26, (uint8_t *)&nState, 0);
	if (nOldState != nState) {
		switch (nState) {
			case 2:// 越上限
				arrData[5] = 0x80 | nDa;
				arrData[6] = 0x80;
				nHighThreshold = nLowThreshold;
				break;
			case 1:// 越上上限
				arrData[5] = 0x80 | nDa;
				arrData[6] = 0x40;
				nHighThreshold = nHighThreshold;
				break;
			case 0:// 正常
			default:
				arrData[5] = nDa;
				if (nOldState == 1) {
					arrData[6] = 0x40;
					nHighThreshold = nHighThreshold;
				} else {
					arrData[6] = 0x80;
					nHighThreshold = nLowThreshold;
				}
				break;
		}
		gw3761_ConvertData_15(arrData, tTime);
		gw3761_ConvertData_23(&arrData[7], nData);
		gw3761_ConvertData_23(&arrData[10], nHighThreshold);
		nResult = event_Save(&xEvent, arrData);
	}
	return nResult;
}

//-------------------------------------------------------------------------
// 电能表示度下降
//-------------------------------------------------------------------------
int rcp_GDW_Event_ERC27(uint8_t nDa, uint32_t nNowEnergy, uint32_t nOldEnergy, time_t tTime)
{
	EVENT xEvent;
	int nResult = 0;
	uint8_t arrData[16];

	xEvent.nERC = 27;
	if (rcp_GDW_Event_Enable(xEvent.nERC) == 0) {
		return -1;
	}
	xEvent.nLen = 16;
	if (nNowEnergy < nOldEnergy) {
		gw3761_ConvertData_15(arrData, tTime);
		arrData[5] = nDa;
		gw3761_ConvertData_14(&arrData[6], nOldEnergy);
		gw3761_ConvertData_14(&arrData[11], nNowEnergy);
		nResult = event_Save(&xEvent, arrData);
	}
	return nResult;
}

//-------------------------------------------------------------------------
// 电能量超差
//-------------------------------------------------------------------------
int rcp_GDW_Event_ERC28(uint8_t nDa, uint32_t nNowEnergy, uint32_t nOldEnergy, time_t tTime)
{
	EVENT xEvent;
	int nResult = 0;
	uint8_t arrData[17];
	uint32_t nThreshold;

	xEvent.nERC = 28;
	if (rcp_GDW_Event_Enable(xEvent.nERC) == 0) {
		return -1;
	}
	xEvent.nLen = 17;
	nThreshold = reg_GetValue(TERMINAL, 0x0851);
	if ((nNowEnergy - nOldEnergy) > nThreshold) {
		if (reg_GetBit(nDa, 0x8E28, 0) == 0) {
			gw3761_ConvertData_15(arrData, tTime);
			arrData[5] = nDa;
			gw3761_ConvertData_14(&arrData[6], nOldEnergy);
			gw3761_ConvertData_14(&arrData[11], nNowEnergy);
			gw3761_ConvertData_22(&arrData[16], nThreshold);
			nResult = event_Save(&xEvent, arrData);
		}
	} else {
		reg_SetValue(nDa, 0x8E28, 0, 0);
	}
	return nResult;
}

//-------------------------------------------------------------------------
// 电能表飞走
//-------------------------------------------------------------------------
int rcp_GDW_Event_ERC29(uint8_t nDa, uint32_t nNowEnergy, uint32_t nOldEnergy, time_t tTime)
{
	EVENT xEvent;
	int nResult = 0;
	uint8_t arrData[17];
	uint32_t nThreshold;

	xEvent.nERC = 29;
	if (rcp_GDW_Event_Enable(xEvent.nERC) == 0)
		return -1;
	xEvent.nLen = 17;
	nThreshold = reg_GetValue(TERMINAL, 0x0852);
	if ((nNowEnergy - nOldEnergy) > nThreshold) {
		if (reg_GetBit(nDa, 0x8E29, 0) == 0) {
			gw3761_ConvertData_15(arrData, tTime);
			arrData[5] = nDa;
			gw3761_ConvertData_14(&arrData[6], nOldEnergy);
			gw3761_ConvertData_14(&arrData[11], nNowEnergy);
			gw3761_ConvertData_22(&arrData[16], nThreshold);
			nResult = event_Save(&xEvent, arrData);
			reg_SetValue(nDa, 0x8E29, 1, 0);
		}
	} else
		reg_SetValue(nDa, 0x8E29, 0, 0);
	return nResult;
}

//-------------------------------------------------------------------------
// 电能表停走
//-------------------------------------------------------------------------
int rcp_GDW_Event_ERC30(uint8_t nDa, uint32_t nNowEnergy, uint32_t nOldEnergy, time_t tTime)
{
	EVENT xEvent;
	int nResult = 0;
	uint8_t arrData[12];
	uint32_t nState, nThreshold;
	xEvent.nERC = 30;
	if (rcp_GDW_Event_Enable(xEvent.nERC) == 0)
		return -1;
	xEvent.nLen = 12;
	if (nNowEnergy == nOldEnergy) {
		nState = reg_GetValue(nDa, 0x8E30);
		nThreshold = reg_GetValue(TERMINAL, 0x0853);
		if (nState == 0) {
			nState = 0x8000 | nThreshold;
		} else {
			if (nState & 0x7FFF) {
				if ((--nState & 0x7FFF) == 0) {
					gw3761_ConvertData_15(arrData, tTime);
					arrData[5] = nDa;
					gw3761_ConvertData_14(&arrData[6], nNowEnergy);
					arrData[11] = nThreshold / 15;
					nResult = event_Save(&xEvent, arrData);
				}
			}
		}
	} else {
		nState = 0;
	}
	reg_Set(nDa, 0x8E30, (uint8_t *)&nState, 0);
	return nResult;
}

//-------------------------------------------------------------------------
// 
//-------------------------------------------------------------------------
int event_Save(EVENT *pEvent, uint8_t *pData)
{
	uint32_t nNeedReport = 0;
	int nResult;

	if (reg_GetValue(TERMINAL, 0x0461) == reg_GetValue(TERMINAL, 0x0465))
		nNeedReport = 1;
	if ((nResult = event_Append(pEvent, pData)) != 0)
		nResult = 1;
	if (nNeedReport || (nResult == 0)) {
		if (reg_GetBit(TERMINAL, 0x0411, pEvent->nERC - 1)) {
			if (g_nIsLogin && reg_GetBit(TERMINAL, 0x0171, 0)) {
				rcp_GDW_MsgEventReport(pEvent, pData);
				nNeedReport = 1;
			} else {
				nNeedReport = 0;
			}
		} else {
			nNeedReport = 1;
		}
		if (nNeedReport)
			reg_ArithmeticCopy(TERMINAL, 0x0465, TERMINAL, 0x0465, 1, nResult, 0);
	}
	reg_StoreGroup(0x04);
	return nResult;
}

//-------------------------------------------------------------------------
// 
//-------------------------------------------------------------------------
int event_Append(EVENT *pEvent, uint8_t *pData)
{
	uint32_t nTotalLen, nECRegID, nCurrentQty, nFileNum;
	uint8_t *pTemp;
	int nResult = 0;
	char strFile[4] = FILENAME_EVENT;

	nFileNum = reg_GetValue(TERMINAL, 0x0422);
	nCurrentQty = reg_GetValue(TERMINAL, 0x0423);
	if (nCurrentQty >= 64){
		uint8_t nStartFileNum;
		nStartFileNum = reg_GetValue(TERMINAL, 0x0421);
		nCurrentQty = 0;
		reg_SetValue(TERMINAL, 0x0422, ++nFileNum, 0);
		if ((nFileNum - nStartFileNum) >= 8){
			// 文件满,删除起始文件
			lib_Int2Str(nStartFileNum, &strFile[1], 2, 0);
			fdelete(strFile);
			reg_SetValue(TERMINAL, 0x0421, ++nStartFileNum, 0);
			// 更新保存总数量
			reg_ArithmeticCopy(TERMINAL, 0x0461, TERMINAL, 0x0461, 2, 64, 0);
			// 更新上报处理
			if ((nTotalLen = reg_GetValue(TERMINAL, 0x0465)) > 64) {
				nTotalLen -= 64;
			} else {
				nTotalLen = 0;
			}
			reg_Set(TERMINAL, 0x0465, (uint8_t *)&nTotalLen, 0);
		}
	}
	lib_Int2Str(nFileNum, &strFile[1], 2, 0);
	// 追加事件记录
	nTotalLen = ((sizeof(EVENT) + pEvent->nLen) + 3) & ~3;
	// 将记录拼合,以避免分两次写文件的风险
	pTemp = mem_Malloc(nTotalLen);
	if (pTemp != NULL) {
		if (reg_GetBit(TERMINAL, 0x0411, pEvent->nERC - 1)){
			nECRegID = 0x0415;
		} else {
			nECRegID = 0x0416;
			pEvent->nERC |= 0x80;
		}
		memcpy(pTemp, pEvent, sizeof(EVENT));
		memcpy(pTemp + sizeof(EVENT), pData, pEvent->nLen);
		if (flashFS_Append(strFile, pTemp, nTotalLen) == nTotalLen) {
			// 成功
			nResult = 1;
			reg_ArithmeticCopy(TERMINAL, nECRegID, TERMINAL, nECRegID, 1, 1, 0); // 置事件计数器
			reg_ArithmeticCopy(TERMINAL, 0x0455, TERMINAL, 0x0455, 7, pEvent->nERC - 1 & 0x7F, 0); // 置状态
			reg_ArithmeticCopy(TERMINAL, 0x0461, TERMINAL, 0x0461, 1, 1, 0); // 更新保存总数量
			++nCurrentQty;
			reg_Set(TERMINAL, 0x0423, &nCurrentQty, 0);
		}
		mem_Free(pTemp);
	}
	return nResult;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
int event_Read(BUFFER *pBuf, uint32_t nEventType, uint8_t nIndex, uint8_t nReadQty){
	uint32_t nCurrentIndex, nFileNum, nSaveQty, nTypeMask, nReadLen;
	int nResult = 0;
	nSaveQty = reg_GetValue(TERMINAL, 0x0461);
	nFileNum = reg_GetValue(TERMINAL, 0x0421);
	nCurrentIndex = nIndex % 64;
	nFileNum += (nIndex / 64);
	if (nSaveQty > 255){
		uint8_t nCurrentQty;
		reg_Get(TERMINAL, 0x0423, &nCurrentQty);
		nCurrentIndex += nCurrentQty;
		if (nCurrentIndex >= 64){
			nCurrentIndex -= 64;
			++nFileNum;
		}
		nResult = nSaveQty - nIndex - nCurrentQty;
	} else {
		if (nIndex < nSaveQty){
			nResult = nSaveQty - nIndex;
		}
	}
	if (nResult > nReadQty){
		nResult = nReadQty;
	}
	if (nResult){
		FILE *file = NULL;
		EVENT xEvent;
		uint8_t *pTemp = mem_Malloc(256);
		uint32_t j = nCurrentIndex, nReadError = 0;
		char strFile[4] = FILENAME_EVENT;
		nReadQty = nResult;
		for (; nReadQty; --nReadQty){
			if (file == NULL){
				lib_Int2Str(nFileNum, &strFile[1], 2, 0);
				if ((file = flashFS_Open(strFile, FILE_OPEN_READ)) == NULL){
					break;
				}
			}
			for (; j; --j){
				nReadError = 1;
				if (fs_read(&xEvent, 1, sizeof(EVENT), file) == sizeof(EVENT)){
					nReadLen = ((xEvent.nLen + sizeof(EVENT) + 3) & ~3) - sizeof(EVENT);
					if (fs_read(pTemp, 1, nReadLen, file) == nReadLen){
						nReadError = 0;
					}
				}
				if (nReadError){
					break;
				}
			}
			if (nReadError){
				break;
			}
			nReadError = 1;
			if (fs_read(&xEvent, 1, sizeof(EVENT), file) == sizeof(EVENT)){
				nReadLen = ((xEvent.nLen + sizeof(EVENT) + 3) & ~3) - sizeof(EVENT);
				switch (nEventType) {
					case 1:
						// Important
						if (xEvent.nERC & 0x80) {
							nTypeMask = 0;
						} else {
							nTypeMask = 1;
						}
						break;
					case 2:
						// Normal
						if (xEvent.nERC & 0x80) {
							nTypeMask = 1;
						} else {
							nTypeMask = 0;
						}
						break;
					default:
						nTypeMask = 1;
						break;
				}
				if (nTypeMask) {
					if (fs_read(pTemp, 1, nReadLen, file) == nReadLen){
						buffer_Push(pBuf, (uint8_t *)&xEvent, sizeof(EVENT));
						buffer_Push(pBuf, pTemp, xEvent.nLen);
						nReadError = 0;
						++nCurrentIndex;
					}
				} else {
					if (fseek (file, nReadLen, SEEK_CUR) == 0) {
						nReadError = 0;
						++nCurrentIndex;
						++nReadQty;
					}
				}
				if (nCurrentIndex >= 64){
					nCurrentIndex = 0;
					++nFileNum;
					flashFS_Close(file);
					file = NULL;
				}
			}
			if (nReadError){
				break;
			}
		}
		flashFS_Close(file);
		mem_Free(pTemp);
		nResult -= nReadQty;
	}
	return nResult;
}


