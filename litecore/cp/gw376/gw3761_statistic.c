volatile U64 g_nTnValid = 0;
volatile U64 g_nTnDataValid = 0;

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
const uint16_t arrGroupSrcRegID[12] = {0xB630, 0xB630, 0xB640, 0xB640, 0x9210, 0x9220, 0x9230, 0x9240, 0x9310, 0x9320, 0x9330, 0x9340};
const uint16_t arrGroupDestRegID[6] = {0x8101, 0x8102, 0x8130, 0x8140, 0x8150, 0x8160};
void statistic_CalculateGroup(uint32_t nGroupID, uint32_t nSave) {
	uint32_t i, j, k, nRate, nTnQty;
	fixpoint fixTemp, fixData;
	t_stat_Group *pG, arrGroupParam[MAX_QTY_TN];
	nTnQty = reg_GetValue(nGroupID, 0x2511);
	reg_Get(nGroupID, 0x2512, (uint8_t *)arrGroupParam);
	for (i = 0; i < 6; ++i) {
		switch (i) {
			case 2:
			case 3:
			case 4:
			case 5:
				nRate = 14;
				break;
			case 0:
			case 1:
			default:
				nRate = 0;
				break;
		}
		for (j = 0; j <= nRate; ++j) {
			fixTemp = fixData = 0;
			for (pG = arrGroupParam, k = nTnQty; k; --k, ++pG){
				fixTemp = reg_GetValue(pG->nTn + 1, arrGroupSrcRegID[i * 2 + pG->nDir] + j);
				if (pG->nSign){
					fixData -= fixTemp;
				} else {
					fixData += fixTemp;
				}
			}
			reg_Set(nGroupID, arrGroupDestRegID[i] + j, (uint8_t *)&fixData, nSave);
		}
	}
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
void statistic_HourFreeze(uint32_t nDa, uint8_t nSaveID, uint8_t nHour, uint8_t nMin) {
	uint8_t nTimeSign;
	uint16_t nSaveIDRegID;
	uint32_t nDestReg, nSrcReg, nFreezeSpan, nFreezeID, nIsEnergy = 0;
	nSaveIDRegID = 0x0F00 | nSaveID;
	if ((nTimeSign = reg_GetValue(nDa, nSaveIDRegID)) == 0){
		return ;
	}
	nHour = bin2bcd8(nHour);
	switch (nTimeSign & 0xC0){
		case 0x40:
			// 1
			nFreezeSpan = 15;
			break;
		case 0x80:
			// 2
			nFreezeSpan = 30;
			break;
		case 0xC0:
			// 3
			nFreezeSpan = 60;
			break;
		default:
			return ; // 时标为0,不冻结
	}
	if (nMin % nFreezeSpan) {
		return;
	}
	// 冻结
	nFreezeID = nMin / nFreezeSpan - 1;
	nDestReg = ((nSaveID - 80) * 4 + nFreezeID) | 0x0D00;
	switch (nSaveID){
		case 81:
			nSrcReg = 0x8101;
			break;
		case 82:
			nSrcReg = 0x8102;
			break;
		case 83:
			nSrcReg = 0x8130;
			break;
		case 84:
			nSrcReg = 0x8140;
			break;
		case 89:
		case 90:
		case 91:
		case 92:
			nSrcReg = 0xB630 + nSaveID - 89;
			break;
		case 93:
		case 94:
		case 95:
		case 96:
			nSrcReg = 0xB640 + nSaveID - 93;
			break;
		case 97:
		case 98:
		case 99:
			nSrcReg = 0xB611 + nSaveID - 97;
			break;
		case 100:
		case 101:
		case 102:
			nSrcReg = 0xB621 + nSaveID - 100;
			break;
		case 103:
			nSrcReg = 0xB690;
			break;
		case 105:
			nSrcReg = 0x9010;
			nIsEnergy = 0x0D00 | 201;
			break;
		case 106:
			nSrcReg = 0x9110;
			nIsEnergy = 0x0D00 | 202;
			break;
		case 107:
			nSrcReg = 0x9020;
			nIsEnergy = 0x0D00 | 203;
			break;
		case 108:
			nSrcReg = 0x9120;
			nIsEnergy = 0x0D00 | 204;
			break;
		case 109:
			nSrcReg = 0x9010;
			break;
		case 110:
			nSrcReg = 0x9110;
			break;
		case 111:
			nSrcReg = 0x9020;
			break;
		case 112:
			nSrcReg = 0x9120;
			break;
		case 113:
		case 114:
		case 115:
		case 116:
			nSrcReg = 0xB650 + nSaveID - 113;
			break;
		default:
			nSrcReg = 0;
			break;
	}
	if (nSrcReg){
		if (nIsEnergy) {
			//电能量小时冻结
			fixpoint fixSavePower, fixPower;
			fixPower = reg_GetValue(nDa, nSrcReg);
			fixSavePower = reg_GetValue(nDa, nIsEnergy);
			if (fixPower > fixSavePower) {
				fixSavePower = fixPower - fixSavePower;
				reg_SetValue(nDa, nDestReg, fixSavePower, 0);
			}
			reg_SetValue(nDa, nIsEnergy, fixPower, 0);
		} else {
			reg_ArithmeticCopy(nDa, nDestReg, nDa, nSrcReg, 0, 0, 0);
			if (nMin == 60) {
				nTimeSign = (nTimeSign & 0xC0) | (nHour & 0x3F);
				reg_Set(nDa, nSaveIDRegID, &nTimeSign, 0);
			}
		}
	}
}

typedef struct STATISTIC_MAXMIN_STRUCT_ {
	uint16_t nDestID;
	uint16_t nSrcID;
	uint16_t nTimeID;
	uint16_t nType;
}MAXMIN_STRUCT;
const MAXMIN_STRUCT arrMaxMinID[] = {
	//最大有功功率
	0x8510, 0xB630, 0x8610, 0x01,
	0x8810, 0xB630, 0x8910, 0x01,
	0x8511, 0xB631, 0x8611, 0x01,
	0x8811, 0xB631, 0x8911, 0x01,
	0x8512, 0xB632, 0x8612, 0x01,
	0x8812, 0xB632, 0x8912, 0x01,
	0x8513, 0xB633, 0x8613, 0x01,
	0x8813, 0xB633, 0x8913, 0x01,
	//最大有功最大需量//////////Unfinished//////////
	0x8520, 0xA010, 0x8620, 0x01,
	0x8820, 0xA010, 0x8920, 0x01,
	0x8521, 0xA010, 0x8621, 0x01,
	0x8821, 0xA010, 0x8921, 0x01,
	0x8522, 0xA010, 0x8622, 0x01,
	0x8822, 0xA010, 0x8922, 0x01,
	0x8523, 0xA010, 0x8623, 0x01,
	0x8823, 0xA010, 0x8923, 0x01,
	//最大电压
	0x8531, 0xB611, 0x8631, 0x01,
	0x8831, 0xB611, 0x8931, 0x01,
	0x8532, 0xB612, 0x8632, 0x01,
	0x8832, 0xB612, 0x8932, 0x01,
	0x8533, 0xB613, 0x8633, 0x01,
	0x8833, 0xB613, 0x8933, 0x01,
	//最小电压
	0x8541, 0xB611, 0x8641, 0x02,
	0x8841, 0xB611, 0x8941, 0x02,
	0x8542, 0xB612, 0x8642, 0x02,
	0x8842, 0xB612, 0x8942, 0x02,
	0x8543, 0xB613, 0x8643, 0x02,
	0x8843, 0xB613, 0x8943, 0x02,
	//最大电流
	0x8560, 0xB690, 0x8660, 0x01,
	0x8860, 0xB690, 0x8960, 0x01,
	0x8561, 0xB621, 0x8661, 0x01,
	0x8861, 0xB621, 0x8961, 0x01,
	0x8562, 0xB622, 0x8662, 0x01,
	0x8862, 0xB622, 0x8962, 0x01,
	0x8563, 0xB623, 0x8663, 0x01,
	0x8863, 0xB623, 0x8963, 0x01,
};
void statistic_Extreme (uint8_t nDa, uint16_t nDestID, uint16_t nSrcID, uint16_t nTimeID, uint32_t nType) {//////////Unfinished//////////暂未考虑符号
	int nDestData, nSrcData;
	uint32_t nNeedSave = 0;

	// 读取原有记录
	reg_Get(nDa, nSrcID, (uint8_t *)&nSrcData);
	reg_Get(nDa, nDestID, (uint8_t *)&nDestData);
	switch (nType) {
		case 1://最大
			if (nSrcData > nDestData) {
				nNeedSave = 1;
			}
			break;
		case 2://最小
			if (nSrcData < nDestData) {
				nNeedSave = 1;
			}
			break;
		default:
			break;
	}
	if (nNeedSave) {
		reg_Set(nDa, nDestID, (uint8_t *)&nSrcData, 0);
		reg_Set(nDa, nTimeID, (uint8_t *)&g_tTime, 0);
	}
}

int statistic_OverTime (uint8_t nDa, int nSrcData, uint16_t nTimeID, int nThreshold, uint32_t nType) {//////////Unfinished//////////暂未考虑符号
	uint32_t nNeedSave = 0;
	switch (nType) {
		case 1://超上限
			if (nSrcData > nThreshold) {
				nNeedSave = 1;
			}
			break;
		case 2://超下限
			if (nSrcData < nThreshold) {
				nNeedSave = 1;
			}
			break;
		default:
			break;
	}
	if (nNeedSave) {
		reg_ArithmeticCopy(nDa, nTimeID, nDa, nTimeID, 1, 1, 0);
	}
	return nNeedSave;
}

uint32_t statistic_GetRateID(struct tm *pTm) {
	uint8_t arrRates[24];
	uint32_t nRateID, nRateQty;
	if ((nRateQty = reg_GetValue(TERMINAL, 0x0971)) == 0) {
		return 0;
	}
	reg_Get(TERMINAL, 0x0972, arrRates);
	if (pTm->tm_min < 30) {
		nRateID = (arrRates[pTm->tm_hour] & 0x0F) + 1;
	} else {
		nRateID = ((arrRates[pTm->tm_hour] & 0xF0) >> 4) + 1;
	}
	if ((nRateQty > 14) || (nRateID > nRateQty)) {
		//费率参数出错,考虑产生参数丢失事件//////////Unfinished//////////
		return 0;
	}
	return nRateID;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
typedef struct STATISTIC_MAXNEED_STRUCT_ {
	uint16_t nSaveID;
	uint16_t nRegID;
	uint16_t nMaxNeedID;
	uint16_t nTimeID;
}MAXNEED_STRUCT;
const MAXNEED_STRUCT arrMaxNeedID[] = {
	0x1171,	0x9010, 0xA010, 0xB010,
	0x1172,	0x9020, 0xA020, 0xB020,
	0x1181,	0x9110, 0xA110, 0xB110,
	0x1182,	0x9120, 0xA120, 0xB120,
};
void tsk_Statistic(void) {
	uint8_t nYear, nMon, nDay, nHour, nMin, nNum;
	uint32_t i, j, k, l, aVol[4], aCur[4];
	uint16_t nEventFlag;
	time_t tTime;
	struct tm tmTime;

	// 等待交采数据有效
	os_dly_wait(1000);
	while (1) {
		os_evt_wait_or(0x00FF, 0xFFFF);
		nEventFlag = os_evt_get();
		os_evt_clr (nEventFlag, os_tsk_self());

		tTime = g_tTime;
		// 统计处理(7秒)
		if (nEventFlag & EVT_STATIS) {
			uint8_t nOldState, nNewState, nChangeSign;
			// 统计文件系统信息
			flashFS_State();
			// 处理事件主动上报
			if (g_nIsLogin && reg_GetBit(TERMINAL, 0x0171, 0)) {
				i = reg_GetValue(TERMINAL, 0x0461);
				j = reg_GetValue(TERMINAL, 0x0465);
				if (j > i) {
					j = i;
				}
				if (j != i) {
					EVENT xEvent;
					BUFFER buf = {0};
					uint8_t *pBuf;
					event_Read(&buf, 0, j, i - j);
					pBuf = buf.p;
					while (((pBuf - buf.p) < buf.nLen) && g_nIsLogin) {
						memcpy(&xEvent, pBuf, sizeof(EVENT));
						if ((xEvent.nERC & 0x80) == 0){
							rcp_GDW_MsgEventReport(&xEvent, pBuf + sizeof(EVENT));
						}
						pBuf += ((sizeof(EVENT) + xEvent.nLen) + 3) & ~3;
						j++;
					}
					buffer_Free(&buf);
					reg_Set(TERMINAL, 0x0465, (uint8_t *)&j, 1);
				}
			}

			//状态量检测
			k = reg_GetValue(TERMINAL, 0x0891);
			nOldState = reg_GetValue(TERMINAL, 0x1391) & k;
			nNewState = sys_ReadState(k, reg_GetValue(TERMINAL, 0x0892));
			reg_Set(TERMINAL, 0x1391, (uint8_t *)&nNewState, 0);
			nChangeSign = (reg_GetValue(TERMINAL, 0x1392) | (nOldState ^ nNewState)) & k;
			reg_Set(TERMINAL, 0x1392, (uint8_t *)&nChangeSign, 0);
			if (nChangeSign) {
				//状态量变位事件
				rcp_GDW_Event_ERC4(nChangeSign, nNewState, tTime);
			}

#if TCPPS_FLOWCOUNT_ENABLE
			// 统计远程通讯流量
			reg_ArithmeticCopy(TERMINAL, 0x11E1, TERMINAL, 0x11E1, 1, tcpPs_PPP_GetDataFlow(0), 0);
			reg_ArithmeticCopy(TERMINAL, 0x11E2, TERMINAL, 0x11E2, 1, tcpPs_PPP_GetDataFlow(1), 0);// 和交采一起储存
#endif
		}

		// 跨分处理(1min)
		if (nEventFlag & EVT_STATIS_MIN) {
			uint32_t nHighHighThreshold, nLowLowThreshold, nHighThreshold, nLowThreshold, nEnergy;
			uint16_t arrThdTime[19], arrThd[19], arrVolThdThreshold[19], arrCurThdThreshold[19];
			const MAXMIN_STRUCT *pMaxMin;
#if (RCP_GDW_STATISTIC_DEBUG_ENABLE	== 1)
			rt_kprintf("STATISTIC: EVT_STATIS_MIN Start: %d\n", g_tTime);
#endif
			//谐波限值
			reg_Get(TERMINAL, 0x0861, (uint8_t *)arrVolThdThreshold);
			reg_Get(TERMINAL, 0x0862, (uint8_t *)arrCurThdThreshold);
			// 终端供电时间统计
			reg_ArithmeticCopy(TERMINAL, 0x11A1, TERMINAL, 0x11A1, 1, 1, 0);
			reg_ArithmeticCopy(TERMINAL, 0x11A9, TERMINAL, 0x11A9, 1, 1, 1);

			// 总加组统计
			for (i = (reg_GetValue(TERMINAL, 0x0844) & MAX_MASK_TN_GROUP); i; --i) {
				statistic_CalculateGroup(i, 0);
				//最大最小总加功率统计
				statistic_Extreme(i, 0x8211, 0x8101, 0x8212, 1);
				statistic_Extreme(i, 0x8221, 0x8101, 0x8222, 2);
				statistic_Extreme(i, 0x8311, 0x8101, 0x8312, 1);
				statistic_Extreme(i, 0x8321, 0x8101, 0x8322, 2);
				// 功率为0时间统计
				statistic_OverTime(i, reg_GetValue(i, 0x8101), 0x8231, 0x000000FF, 2);
				statistic_OverTime(i, reg_GetValue(i, 0x8101), 0x8331, 0x000000FF, 2);
			}

 			for (i = MAX_QTY_TN; i; --i) {
				if ((g_nTnDataValid & GETBIT(i - 1)) == 0) {
					continue;
				}
				// 最大最小统计
				for (j = 0, pMaxMin = arrMaxMinID; j < sizeof(arrMaxMinID) / sizeof(MAXMIN_STRUCT); ++j, ++pMaxMin) {
					statistic_Extreme(i, pMaxMin->nDestID, pMaxMin->nSrcID, pMaxMin->nTimeID, pMaxMin->nType);
				}
				for (j = 0; j < 4; ++j) {
					// 读取功率
					reg_Get(i, 0xB630 + j, (uint8_t *)&aVol[j]);
					// 功率为0时间统计
					statistic_OverTime(i, aVol[j], 0x8710 + j, 0x000000FF, 2);
					statistic_OverTime(i, aVol[j], 0x8A10 + j, 0x000000FF, 2);
				}
				// 电压平均值统计
				for (j = 3; j; --j) {
					// 读取电压电流
					aVol[j] = reg_GetValue(i, 0xB610 + j);
					aCur[j] = reg_GetValue(i, 0xB620 + j);
					// 读日统计次数
					k = reg_GetValue(i, 0x8558 + j);
					// 读日平均电压
					l = reg_GetValue(i, 0x8550 + j);
					l = (l * k++) + aVol[j];
					l /= k;
					reg_Set(i, 0x8558 + j, (uint8_t *)&k, 0);
					reg_Set(i, 0x8550 + j, (uint8_t *)&l, 0);
					// 读月统计次数
					k = reg_GetValue(i, 0x8858 + j);
					// 读月平均电压
					l = reg_GetValue(i, 0x8850 + j);
					l = (l * k++) + aVol[j];
					l /= k;
					reg_Set(i, 0x8858 + j, (uint8_t *)&k, 0);
					reg_Set(i, 0x8850 + j, (uint8_t *)&l, 0);
				}				
				// 电压电流不平衡度计算
				for (j = 0; j < 2; ++j) {
					uint32_t *pValue, nAvg, n1, n2, n3;
					if (j) {
						pValue = aCur;
					} else {
						pValue = aVol;
					}
					nAvg = (pValue[1] + pValue[2] + pValue[3]) / 3;
					n1 = abs(pValue[1] - nAvg);
					n2 = abs(pValue[2] - nAvg);
					n3 = abs(pValue[3] - nAvg);
					n1 = MAX(n1, n2);
					n1 = MAX(n1, n3);
					pValue[0] = FLOAT2FIX(n1 / nAvg);
					nHighThreshold = reg_GetValue(i, 0x2784 + j);
					if (pValue[0] > nHighThreshold) {
						// 不平衡度越限时间统计
						reg_ArithmeticCopy(i, 0x8791 + j, i, 0x8791 + j, 1, 1, 0);
						reg_ArithmeticCopy(i, 0x8A91 + j, i, 0x8A91 + j, 1, 1, 0);
						// 不平衡度越限事件
						rcp_GDW_Event_ERC17(i, j, 1, &aVol[0], &aCur[0], tTime);
					} else {
						rcp_GDW_Event_ERC17(i, j, 0, &aVol[0], &aCur[0], tTime);
					}
				}
				//电压越限统计
				nEnergy = reg_GetValue(i, 0x9010);
				nHighThreshold = reg_GetValue(i, 0x2771);
				nLowThreshold = reg_GetValue(i, 0x2772);
				nHighHighThreshold = reg_GetValue(i, 0x2774);
				nLowLowThreshold = reg_GetValue(i, 0x2775);
				for (j = 3; j; --j) {
					// 电压越上上限统计
					statistic_OverTime(i, aVol[j], 0x8720 + j, nHighHighThreshold, 1);
					if (statistic_OverTime(i, aVol[j], 0x8A20 + j, nHighHighThreshold, 1) == 0) {
						// 电压越上限统计
						statistic_OverTime(i, aVol[j], 0x8730 + j, nHighThreshold, 1);
						if (statistic_OverTime(i, aVol[j], 0x8A30 + j, nHighThreshold, 1) == 0) {
							// 电压越下下限统计
							statistic_OverTime(i, aVol[j], 0x8740 + j, nLowLowThreshold, 2);
							if (statistic_OverTime(i, aVol[j], 0x8A40 + j, nLowLowThreshold, 2) == 0) {
								// 电压越下限统计
								statistic_OverTime(i, aVol[j], 0x8750 + j, nLowThreshold, 2);
								if (statistic_OverTime(i, aVol[j], 0x8A50 + j, nLowThreshold, 2) == 0) {
									// 电压合格时间统计
									reg_ArithmeticCopy(i, 0x8760 + j, i, 0x8760 + j, 1, 1, 0);
									reg_ArithmeticCopy(i, 0x8A60 + j, i, 0x8A60 + j, 1, 1, 0);
								}
							}
						}
					}
				}
				// 电流越上上限统计
				nHighThreshold = reg_GetValue(i, 0x2776);
				for (j = 3; j; --j) {
					statistic_OverTime(i, aCur[j], 0x8770 + j, nHighThreshold, 1);
					statistic_OverTime(i, aCur[j], 0x8A70 + j, nHighThreshold, 1);
				}
				// 电流越上限统计
				nHighThreshold = reg_GetValue(i, 0x2777);
				for (j = 3; j; --j) {
					statistic_OverTime(i, aCur[j], 0x8780 + j, nHighThreshold, 1);
					statistic_OverTime(i, aCur[j], 0x8A80 + j, nHighThreshold, 1);
				}
				// 零序电流越上限统计
				nHighThreshold = reg_GetValue(i, 0x2778);
				aCur[0] = reg_GetValue(i, 0xB690);
				statistic_OverTime(i, aCur[0], 0x8780, nHighThreshold, 1);
				statistic_OverTime(i, aCur[0], 0x8A80, nHighThreshold, 1);
				// 视在功率越限时间统计
				k = reg_GetValue(i, 0xB6A0);
				nHighThreshold = reg_GetValue(i, 0x2782);
				nLowThreshold = reg_GetValue(i, 0x2783);
				if (k > nHighThreshold) {
					// 越上上限
					reg_ArithmeticCopy(i, 0x87A1, i, 0x87A1, 1, 1, 0);
					reg_ArithmeticCopy(i, 0x8AA1, i, 0x8AA1, 1, 1, 0);
					rcp_GDW_Event_ERC26(i, 1, k, nHighThreshold, nLowThreshold, tTime);
				}
				else if (k > nLowThreshold) {
					// 越上限
					reg_ArithmeticCopy(i, 0x87A2, i, 0x87A2, 1, 1, 0);
					reg_ArithmeticCopy(i, 0x8AA2, i, 0x8AA2, 1, 1, 0);
					rcp_GDW_Event_ERC26(i, 2, k, nHighThreshold, nLowThreshold, tTime);
				}
				else {
					// 正常状态
					rcp_GDW_Event_ERC26(i, 0, k, nHighThreshold, nLowThreshold, tTime);
				}
				//电压谐波越限统计
				for (j = 0; j < 3; ++j) {
					l = 0;
					reg_Get(i, 0xBD11 + j, (uint8_t *)arrThdTime);
					reg_Get(i, 0xBB21 + j, (uint8_t *)arrThd);
					for (k = 0; k < 19; ++k) {
						if (arrThd[k] > arrVolThdThreshold[k]) {
							arrThdTime[k]++;
							l |= GETBIT(k);
						}
					}
					reg_Set(i, 0xBD11 + j, (uint8_t *)arrThdTime, 0);
					//谐波越限事件
					rcp_GDW_Event_ERC15(i, j, l, arrThd, tTime);
				}
				//电流谐波越限统计
				for (j = 0; j < 3; ++j) {
					l = 0;
					reg_Get(i, 0xBD21 + j, (uint8_t *)arrThdTime);
					gw3761_ConvertData_THD(i, j + 3, (uint8_t *)arrThd);
					for (k = 0; k < 19; ++k) {
						if ((arrThd[k] & 0x7FFF) > (arrCurThdThreshold[k] & 0x7FFF)) {
							arrThdTime[k]++;
							l |= GETBIT(k);
						}
					}
					reg_Set(i, 0xBD21 + j, (uint8_t *)arrThdTime, 0);
					//谐波越限事件
					rcp_GDW_Event_ERC15(i, j + 3, l, arrThd, tTime);
				}
				// 电压回路异常事件
				rcp_GDW_Event_ERC10(i, &aVol[1], &aCur[1], nEnergy, tTime);
				// 电压越限事件
				rcp_GDW_Event_ERC24(i, &aVol[1], tTime);
				// 电流回路异常事件
				rcp_GDW_Event_ERC9(i, &aVol[1], &aCur[1], nEnergy, tTime);
				// 电流越限事件
				rcp_GDW_Event_ERC25(i, &aCur[1], tTime);
				// 相序异常事件
				rcp_GDW_Event_ERC11(i, nEnergy, tTime);
				// 读取上一次统计电量
				l = reg_GetValue(i, 0x8E27);
				// 电能表示度下降事件
				rcp_GDW_Event_ERC27(i, nEnergy, l, tTime);
				// 电能表飞走事件
				rcp_GDW_Event_ERC29(i, nEnergy, l, tTime);
				// 电能表停走事件
				rcp_GDW_Event_ERC30(i, nEnergy, l, tTime);
				// 电能表时间超差事件
				rcp_GDW_Event_ERC12(i, tTime);
				// 保存现在电量
				reg_Set(i, 0x8E27, (uint8_t *)&nEnergy, 0);
			}
			reg_StoreGroup(0x81);
			reg_StoreGroup(0x82);
			reg_StoreGroup(0x83);
			reg_StoreGroup(0x85);
			reg_StoreGroup(0x88);
			reg_StoreGroup(0x86);
			reg_StoreGroup(0x89);
			reg_StoreGroup(0x87);
			reg_StoreGroup(0x8A);
			reg_StoreGroup(0x8E);//事件统计数据
			reg_StoreGroup(0xBD);//谐波越限统计

			//抄表日冻结处理
			i = reg_GetValue(TERMINAL, 0x0821);
			time_Timet2Tm(tTime, &tmTime);
			nDay = tmTime.tm_mday;
			nHour = tmTime.tm_hour;
			nMin = tmTime.tm_min;
			if ((reg_GetValue(TERMINAL, 0x0826) == nMin) && (reg_GetValue(TERMINAL, 0x0825) == nHour)) {
				if (i & GETBIT(nDay - 1)) {
					rcp_GDW_Freeze(GW3761_DT_T_DATA_MDAY, tTime);
				}
			}

#if (RCP_GDW_STATISTIC_DEBUG_ENABLE	== 1)
			rt_kprintf("STATISTIC: EVT_STATIS_MIN End: %d\n", g_tTime);
#endif
		}

		// 15分钟
		if (nEventFlag & EVT_STATIS_QUARTER) {
			BUFFER buf;
			char strFile[12];
			t_stat_FreezeCurve stuCurveIndex;
			uint8_t arrGroup[12], arrTn[78];
			uint32_t nGroupQty;
			// 时间校正处理,当分为0时调整小时值为上一个小时
#if (RCP_GDW_STATISTIC_DEBUG_ENABLE	== 1)
			rt_kprintf("STATISTIC: EVT_STATIS_QUARTER Start: %d\n", g_tTime);
#endif
			tTime /= 900;
			tTime = tTime * 900 - 59;
			time_Timet2Tm(tTime, &tmTime);
			nYear = (uint8_t)tmTime.tm_year - 100;
			nMon = tmTime.tm_mon + 1;
			nDay = tmTime.tm_mday;
			nHour = tmTime.tm_hour;
			nMin = tmTime.tm_min + 1;
			nNum = nHour * 4 + nMin / 15;

			for (i = reg_GetValue(TERMINAL, 0x0844) & MAX_MASK_TN_GROUP; i; --i) {
				// 总加组小时冻结
				for (j = 81; j <= 84; ++j){
					statistic_HourFreeze(i, j, nHour, nMin);
				}
			}

			// 测量点小时冻结数据
			for (i = MAX_QTY_TN; i; --i) {
				if (reg_GetBit(i, 0xC001, 0) == 0) {
					continue;
				}
				for (j = 89; j <= 121; ++j) {
					statistic_HourFreeze(i, j, nHour, nMin);
				}
			}
			if (nMin == 60) {
				reg_GroupCopy(0xFF, 0x0C, 0xFF, 0x0D, 1);
				reg_DefaultGroup(0xFF, 0x0D, 0);
				reg_StoreGroup(0x0F);
			}
			reg_StoreGroup(0x0D);

			// 差动越限事件
			if (rcp_GDW_Event_Enable(22)) {
				for (i = reg_GetValue(TERMINAL, 0x0845) & 0x3F; i; --i) {
					j = reg_GetValue(i, 0x2654);
					switch (j & 3) {
						case 2:
							k = 1;
							break;
						case 1:
							k = ((nMin == 60) || (nMin == 30));
							break;
						case 0:
						default:
							k = (nMin == 60);
							break;
					}
					if (k) {
						rcp_GDW_Event_ERC22(i, j, tTime);
					}
				}
			}

			// 曲线冻结
			nGroupQty = reg_GetValue(TERMINAL, 0x0844) & MAX_MASK_TN_GROUP;
			buf.p = NULL;
			buf.nLen = 0;
			for (i = 1; i <= MAX_QTY_TN; ++i) {
				t_gw3761_dt *pDT2;
				uint32_t nTimeSign;
				stuCurveIndex.nDa = (uint8_t)i;
				stuCurveIndex.nSign = 0;
				stuCurveIndex.nLen = 0;
				// 判定测量点是否有效
				if (g_nTnValid & GETBIT(i - 1)) {
					stuCurveIndex.nSign |= 0x01;
					stuCurveIndex.nLen += 78;
				}
				if (i <= nGroupQty) {
					stuCurveIndex.nSign |= 0x02;
					stuCurveIndex.nLen += 12;
				}
				memset(arrGroup, 0xEE, 12);
				memset(arrTn, 0xEE, 78);
				if (stuCurveIndex.nSign) {
					for (j = rcp_GDW_GetTable_DT(0x0D, &pDT2); j; --j, ++pDT2) {
						if (pDT2->dttype == GW3761_DT_T_DATA_CURVE) {
							switch (pDT2->datype) {
								case DA_TYPE_TN:	// 测量点
									if (stuCurveIndex.nSign & 0x01) {
										if ((pDT2->item->reg & 0x0C00) == 0x0C00) {
											//小时冻结电量
											if ((nTimeSign = reg_GetValue(i, 0x0F00 | (pDT2->item->reg & 0x00FF))) != 0){
												switch (nTimeSign & 0xC0){
													case 0x40:
														// 1
														nTimeSign = 15;
														break;
													case 0x80:
														// 2
														nTimeSign = 30;
														break;
													case 0xC0:
														// 3
														nTimeSign = 60;
														break;
													default:
														nTimeSign = 0;
														break;
												}
												if ((nTimeSign != 0) && ((nMin % nTimeSign) == 0)){
													gw3761_DataGetRegValue(i, pDT2->item, arrTn + pDT2->item->para, nMin / nTimeSign - 1);
												}
											}
										}
										else {
											gw3761_DataGetRegValue(i, pDT2->item, arrTn + pDT2->item->para, 0);
										}
									}
									break;
								case DA_TYPE_GROUP:	// 总加组
									if (stuCurveIndex.nSign & 0x02) {
										gw3761_DataGetRegValue(i, pDT2->item, arrGroup + pDT2->item->para, 0);
									}
									break;
								default:
									break;
							}
						}
					}
				}
				if (stuCurveIndex.nSign) {
					buffer_Push(&buf, (uint8_t *)&stuCurveIndex, sizeof(stuCurveIndex));
				}
				if (stuCurveIndex.nSign & 0x01) {
					buffer_Push(&buf, arrTn, sizeof(arrTn));
				}
				if (stuCurveIndex.nSign & 0x02) {
					buffer_Push(&buf, arrGroup, sizeof(arrGroup));
				}
			}
			if (buf.nLen) {
				// 需要保存
				gw3761_FreezeName(strFile, nYear, nMon, nDay, nNum, GW3761_DT_T_DATA_CURVE);
				flashFS_Write(strFile, buf.p, buf.nLen);
			}
			buffer_Free(&buf);
#if (RCP_GDW_STATISTIC_DEBUG_ENABLE	== 1)
			rt_kprintf("STATISTIC: EVT_STATIS_QUARTER End: %d\n", g_tTime);
#endif
		}

		// 最大需量统计
		if (nEventFlag & EVT_STATIS_MAXNEED) {
			uint32_t nPoint, nHC, nCycle;
			int nRateID;
			fixpoint fixSaveData, fixData, fixOldData;
			const MAXNEED_STRUCT *p;
#if (RCP_GDW_STATISTIC_DEBUG_ENABLE	== 1)
			rt_kprintf("STATISTIC: S_TASK_MAXNEED Start: %d\n", g_tTime);
#endif
			reg_Get(TERMINAL, 0x0991, (uint8_t *)&nCycle);
			reg_Get(TERMINAL, 0x0992, (uint8_t *)&nHC);
			if (nHC > nCycle) {
				reg_DefaultReg(TERMINAL, 0x0991, 0);
				reg_DefaultReg(TERMINAL, 0x0992, 1);
				nHC = 60;
				nCycle = 900;
			}
			nPoint = nCycle / nHC;
			if ((nRateID = statistic_GetRateID((struct tm *)&g_tmTime)) == -1) {
				continue;
			}
			for (i = (sizeof(arrMaxNeedID) / sizeof(MAXNEED_STRUCT)), p = arrMaxNeedID; i; --i, ++p) {
				reg_Get(TERMINAL, p->nSaveID, (uint8_t *)&fixSaveData);
				reg_Get(SAMPLE, p->nRegID, (uint8_t *)&fixData);
				reg_Set(TERMINAL, p->nSaveID, (uint8_t *)&fixData, 0);
				if (fixData > fixSaveData) {
					fixData = (fixData - fixSaveData) / nPoint;
					// 总最大需量
					reg_Get(SAMPLE, p->nMaxNeedID, (uint8_t *)&fixOldData);
					if (fixData > fixOldData) {
						reg_Set(SAMPLE, p->nMaxNeedID, (uint8_t *)&fixData, 0);
						reg_Set(SAMPLE, p->nTimeID, (uint8_t *)&tTime, 0);
					}
					// 费率最大需量
					reg_Get(SAMPLE, p->nMaxNeedID + nRateID, (uint8_t *)&fixOldData);
					if (fixData > fixOldData) {
						reg_Set(SAMPLE, p->nMaxNeedID + nRateID, (uint8_t *)&fixData, 0);
						reg_Set(SAMPLE, p->nTimeID + nRateID, (uint8_t *)&tTime, 0);
					}
				}
			}
			// 保存处理后数据
			reg_StoreGroup(0x11);
			reg_StoreGroup(0xA0);
			reg_StoreGroup(0xA1);
			reg_StoreGroup(0xB0);
			reg_StoreGroup(0xB1);
#if (RCP_GDW_STATISTIC_DEBUG_ENABLE	== 1)
			rt_kprintf("STATISTIC: S_TASK_MAXNEED End: %d\n", g_tTime);
#endif
		}

		// 跨日处理
		if (nEventFlag & EVT_STATIS_DAY) {
			//日冻结
			rcp_GDW_Freeze(GW3761_DT_T_DATA_DAY, tTime);
			
			//清日供电信息
			reg_DefaultReg(TERMINAL, 0x11A1, 0);
			reg_DefaultReg(TERMINAL, 0x11A2, 1);
			
			//清日最大最小和累计时间
			reg_DefaultGroup(0xFF, 0x85, 1);
			reg_DefaultGroup(0xFF, 0x86, 1);
			reg_DefaultGroup(0xFF, 0x87, 1);

			//清日总加功率最大最小和累计时间
			reg_DefaultGroup(0xFF, 0x82, 1);

			//清日电能量
			reg_DefaultGroup(0xFF, 0x92, 1);

			//清日谐波统计数据
			reg_DefaultGroup(0xFF, 0xBC, 1);
			reg_DefaultGroup(0xFF, 0xBD, 1);
		}

		// 跨月处理
		if (nEventFlag & EVT_STATIS_MONTH) {
			// 月冻结
			rcp_GDW_Freeze(GW3761_DT_T_DATA_MONTH, tTime);
			
			// 清月供电信息
			reg_DefaultReg(TERMINAL, 0x11A9, 0);
			reg_DefaultReg(TERMINAL, 0x11AA, 1);
			
			// 清月最大最小和累计时间
			reg_DefaultGroup(0xFF, 0x88, 1);
			reg_DefaultGroup(0xFF, 0x89, 1);
			reg_DefaultGroup(0xFF, 0x8A, 1);

			//清月总加功率最大最小和累计时间
			reg_DefaultGroup(0xFF, 0x83, 1);

			// 清月电能量
			reg_DefaultGroup(0xFF, 0x93, 1);
			
			// 保存上月电能示值
			reg_GroupCopy(SAMPLE, 0x94, SAMPLE, 0x90, 1);
			reg_GroupCopy(SAMPLE, 0x95, SAMPLE, 0x91, 1);

			// 保存并清最大需量
			reg_GroupCopy(SAMPLE, 0xA4, SAMPLE, 0xA0, 1);
			reg_GroupCopy(SAMPLE, 0xA5, SAMPLE, 0xA1, 1);
			reg_GroupCopy(SAMPLE, 0xB4, SAMPLE, 0xB0, 1);
			reg_GroupCopy(SAMPLE, 0xB5, SAMPLE, 0xB1, 1);
			reg_DefaultGroup(SAMPLE, 0xA0, 1);
			reg_DefaultGroup(SAMPLE, 0xA1, 1);
			reg_DefaultGroup(SAMPLE, 0xB0, 1);
			reg_DefaultGroup(SAMPLE, 0xB1, 1);
		}
	}
}

