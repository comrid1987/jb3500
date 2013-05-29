#if 0



extern p_gw3761_dt tbl_GW3761_AFN04[];
extern p_gw3761_dt tbl_GW3761_AFN05[];
extern p_gw3761_dt tbl_GW3761_AFN09[];
extern p_gw3761_dt tbl_GW3761_AFN0C[];
extern p_gw3761_dt tbl_GW3761_AFN0D[];





//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
static p_gw3761_dt gw3761_DataGetTable(uint_t nAfn, uint_t nDT)
{
	p_gw3761_dt pDT, pEnd;

	switch (nAfn) {
	case 0x04:
	case 0x0A:
		pDT = tbl_GW3761_AFN04[0];
		pEnd = tbl_GW3761_AFN04[1];
		break;
	case 0x05:
		pDT = tbl_GW3761_AFN05[0];
		pEnd = tbl_GW3761_AFN05[1];
		break;
	case 0x09:
		pDT = tbl_GW3761_AFN09[0];
		pEnd = tbl_GW3761_AFN09[1];
		break;
	case 0x0C:
		pDT = tbl_GW3761_AFN0C[0];
		pEnd = tbl_GW3761_AFN0C[1];
		break;
	case 0x0D:
		pDT = tbl_GW3761_AFN0D[0];
		pEnd = tbl_GW3761_AFN0D[1];
		break;
	default:
		return NULL;
	}
	for (; pDT < pEnd; pDT++)
		if (pDT->dtid == nDT)
			return pDT;
	return NULL;
}





//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
int gw3761_DataGetRegValue(buf b, uint_t nDa, p_gw3761_item pItem, uint_t nOffset)
{
	uint8_t aTemp[40];
	uint32_t nData = 0, nTemp = 0;
	uint_t nRegID;
	int nLen;
	float fData;
	uint8_t *pTemp;

	nRegID = pItem->reg + nOffset;
	switch (pItem->type & 0x3F) {
	case GW3761_DATA_T_12:
	case GW3761_DATA_T_BIN:
		nLen = reg_GetRegSize(nRegID);
		if (nLen < 0)
			break;
		pTemp = mem_Malloc(nLen);
		if (pTemp == NULL)
			break;
		reg_Get(nDa, nRegID, pTemp);
		switch (reg_GetRegType(nRegID)) {
		case REG_TYPE_STRING:
			pTemp[pItem->size - 1] = 0;
			buf_Push(b, pTemp, pItem->size);
			break;
		case REG_TYPE_UINT:
			if (nLen <= 4) {
				memcpy(&nData, pTemp, nLen);
				if (pItem->type & GW3761_DATA_T_MULTI)
					nData /= pItem->para;
				buf_PushData(b, nData, pItem->size);
			} else
				buf_Push(b, pTemp, pItem->size);
			break;
		default:
			buf_Push(b, pTemp, pItem->size);
			break;
		}
		mem_Free(pTemp);
		break;
	default:
		switch (pItem->type) {
		case GW3761_DATA_T_01:
			switch (reg_GetRegType(nRegID)) {
			case REG_TYPE_TIME:
				gw3761_ConvertData_01(aTemp, reg_GetValue(nDa, nRegID));
				buf_Push(b, aTemp, pItem->size);
				break;
			default:
				break;
			}
			break;
		case GW3761_DATA_T_02: {
			float fE, fP;
			uint32_t nP;
			int nE, nSign = 0;
			nTemp = reg_GetValue(nDa, nRegID);
			switch (reg_GetRegType(nRegID)) {
			case REG_TYPE_SDEC:
				if ((sint32_t)nTemp < 0) {
					nTemp = abs((sint32_t)nTemp);
					nSign = 1;
				}
				break;
			case REG_TYPE_UDEC:
				nSign = 0;
				break;
			default:
				nSign = 2;
				break;
			}
			if (nSign == 2) {
				buf_PushData(b, 0, pItem->size);
				break;
			}
			fData = (float)nTemp * FIXP_EPSILON;
			fP = log10f(fData);
			fE = floorf(fP);
			nData = powf(10, fP - fE) * 100;
			nE = (int)fE + 1;
			nData = bin2bcd32(nData) &0x00000FFF;
			nP = ~(uint32_t)nE;
			nData |= int_setbits(nSign, 12, 1);
			nData |= int_setbits(nP, 13, 3);
			buf_PushData(b, nData, pItem->size);
			}break;
		case GW3761_DATA_T_03:
			nData = reg_GetValue(nDa, nRegID);
			if (pItem->type & GW3761_DATA_T_MULTI)
				nData /= pItem->para;
			switch (reg_GetRegType(nRegID)) {
			case REG_TYPE_SDEC:
				gw3761_ConvertData_03(aTemp, nData, 1);
				break;
			case REG_TYPE_UDEC:
				gw3761_ConvertData_03(aTemp, nData, 0);
				break;
			default:
				break;
			}
			buf_Push(b, aTemp, pItem->size);
			break;
		case GW3761_DATA_T_05:
			nData = reg_GetValue(nDa, nRegID);
			switch (reg_GetRegType(nRegID)) {
			case REG_TYPE_SDEC:
				gw3761_ConvertData_05(aTemp, nData, 1);
				break;
			case REG_TYPE_UDEC:
				gw3761_ConvertData_05(aTemp, nData, 0);
				break;
			default:
				break;
			}
			buf_Push(b, aTemp, pItem->size);
			break;
		case GW3761_DATA_T_PERCENT:
			nData = reg_GetValue(nDa, nRegID);
			switch (reg_GetRegType(nRegID)) {
			case REG_TYPE_SDEC:
				gw3761_ConvertData_05_Percent(aTemp, nData, 1);
				break;
			case REG_TYPE_UDEC:
				gw3761_ConvertData_05_Percent(aTemp, nData, 0);
				break;
			default:
				break;
			}
			buf_Push(b, aTemp, pItem->size);
			break;			
		case GW3761_DATA_T_06:
			nData = reg_GetValue(nDa, nRegID);
			if (pItem->type & GW3761_DATA_T_MULTI)
				nData /= pItem->para;
			switch (reg_GetRegType(nRegID)) {
			case REG_TYPE_SDEC:
				gw3761_ConvertData_06(aTemp, nData, 1);
				break;
			case REG_TYPE_UDEC:
				gw3761_ConvertData_06(aTemp, nData, 0);
				break;
			default:
				break;
			}
			buf_Push(b, aTemp, pItem->size);
			break;
		case GW3761_DATA_T_09:
			nData = reg_GetValue(nDa, nRegID);
			if (pItem->type & GW3761_DATA_T_MULTI)
				nData /= pItem->para;
			switch (reg_GetRegType(nRegID)) {
			case REG_TYPE_SDEC:
				gw3761_ConvertData_09(aTemp, nData, 1);
				break;
			case REG_TYPE_UDEC:
				gw3761_ConvertData_09(aTemp, nData, 0);
				break;
			default:
				break;
			}
			buf_Push(b, aTemp, pItem->size);
			break;
		case GW3761_DATA_T_07:
			nData = reg_GetValue(nDa, nRegID);
			if (pItem->type & GW3761_DATA_T_MULTI)
				nData /= pItem->para;
			switch (reg_GetRegType(nRegID)) {
			case REG_TYPE_SDEC:
			case REG_TYPE_UDEC:
				gw3761_ConvertData_07(aTemp, nData);
				buf_Push(b, aTemp, pItem->size);
				break;					
			default:
				break;
			}
			break;
		case GW3761_DATA_T_11:
			nData = reg_GetValue(nDa, nRegID);
			if (pItem->type & GW3761_DATA_T_MULTI)
				nData /= pItem->para;
			switch (reg_GetRegType(nRegID)) {
			case REG_TYPE_SDEC:
			case REG_TYPE_UDEC:
				gw3761_ConvertData_11(aTemp, nData);
				buf_Push(b, aTemp, pItem->size);
				break;
			default:
				break;
			}
			break;
		case GW3761_DATA_T_13:
			nData = reg_GetValue(nDa, nRegID);
			if (pItem->type & GW3761_DATA_T_MULTI)
				nData /= pItem->para;
			switch (reg_GetRegType(nRegID)) {
			case REG_TYPE_SDEC:
			case REG_TYPE_UDEC:
				gw3761_ConvertData_13(aTemp, nData);
				buf_Push(b, aTemp, pItem->size);
				break;					
			default:
				break;
			}
			break;
		case GW3761_DATA_T_14:
			nData = reg_GetValue(nDa, nRegID);
			if (pItem->type & GW3761_DATA_T_MULTI)
				nData /= pItem->para;
			switch (reg_GetRegType(nRegID)) {
			case REG_TYPE_SDEC:
			case REG_TYPE_UDEC:
				gw3761_ConvertData_14(aTemp, nData);
				buf_Push(b, aTemp, pItem->size);
				break;
			default:
				break;
			}
			break;
		case GW3761_DATA_T_22:
			nData = reg_GetValue(nDa, nRegID);
			if (pItem->type & GW3761_DATA_T_MULTI)
				nData /= pItem->para;
			switch (reg_GetRegType(nRegID)) {
			case REG_TYPE_SDEC:
			case REG_TYPE_UDEC:
				gw3761_ConvertData_22(aTemp, nData);
				buf_Push(b, aTemp, pItem->size);
				break;					
			default:
				break;
			}
			break;
		case GW3761_DATA_T_23:
			nData = reg_GetValue(nDa, nRegID);
			if (pItem->type & GW3761_DATA_T_MULTI)
				nData /= pItem->para;
			switch (reg_GetRegType(nRegID)) {
			case REG_TYPE_SDEC:
			case REG_TYPE_UDEC:
				gw3761_ConvertData_23(aTemp, nData);
				buf_Push(b, aTemp, pItem->size);
				break;					
			default:
				break;
			}
			break;
		case GW3761_DATA_T_25:
			nData = reg_GetValue(nDa, nRegID);
			if (pItem->type & GW3761_DATA_T_MULTI)
				nData /= pItem->para;
			switch (reg_GetRegType(nRegID)) {
			case REG_TYPE_SDEC:
				gw3761_ConvertData_25(aTemp, nData, 1);
				break;
			case REG_TYPE_UDEC:
				gw3761_ConvertData_25(aTemp, nData, 0);
				break;
			default:
				break;
			}
			buf_Push(b, aTemp, pItem->size);
			break;
		case GW3761_DATA_T_26:
			nData = reg_GetValue(nDa, nRegID);
			if (pItem->type & GW3761_DATA_T_MULTI)
				nData /= pItem->para;
			switch (reg_GetRegType(nRegID)) {
			case REG_TYPE_SDEC:
			case REG_TYPE_UDEC:
				gw3761_ConvertData_26(aTemp, nData);
				buf_Push(b, aTemp, pItem->size);
				break;
			default:
				break;
			}
			break;
		case GW3761_DATA_T_04:
		case GW3761_DATA_T_08:
		case GW3761_DATA_T_10:
			nData = reg_GetValue(nDa, nRegID);
			if (pItem->type & GW3761_DATA_T_MULTI)
				nData /= pItem->para;
			switch (reg_GetRegType(nRegID)) {
			case REG_TYPE_SINT:
			case REG_TYPE_SBCD:
				nData = abs(nData);
			case REG_TYPE_UINT:
			case REG_TYPE_UBCD:
				nData = bin2bcd32(nData);
				buf_PushData(b, nData, pItem->size);
			default:
				break;
			}
			break;
		case GW3761_DATA_T_15:
			nData = reg_GetValue(nDa, nRegID);
			if (pItem->type & GW3761_DATA_T_MULTI)
				nData /= pItem->para;
			switch (reg_GetRegType(nRegID)) {
			case REG_TYPE_TIME:
				gw3761_ConvertData_15(aTemp, nData);
				buf_Push(b, aTemp, pItem->size);
				break;
			default:
				break;
			}
			break;
		case GW3761_DATA_T_17:
			nData = reg_GetValue(nDa, nRegID);
			if (pItem->type & GW3761_DATA_T_MULTI)
				nData /= pItem->para;
			switch (reg_GetRegType(nRegID)) {
			case REG_TYPE_TIME:
				gw3761_ConvertData_17(aTemp, nData);
				buf_Push(b, aTemp, pItem->size);
				break;
			default:
				break;
			}
			break;
		case GW3761_DATA_T_18:
			nData = reg_GetValue(nDa, nRegID);
			if (pItem->type & GW3761_DATA_T_MULTI)
				nData /= pItem->para;
			switch (reg_GetRegType(nRegID)) {
			case REG_TYPE_TIME:
				gw3761_ConvertData_18(aTemp, nData);
				buf_Push(b, aTemp, pItem->size);
				break;
			default:
				break;
			}
			break;
		case GW3761_DATA_T_16:
		case GW3761_DATA_T_19:
		case GW3761_DATA_T_20:
		case GW3761_DATA_T_21:
			nData = reg_GetValue(nDa, nRegID);
			if (pItem->type & GW3761_DATA_T_MULTI)
				nData /= pItem->para;
			switch (reg_GetRegType(nRegID)) {
			case REG_TYPE_UINT:
				nData = bin2bcd32(nData);
				buf_PushData(b, nData, pItem->size);
				break;
			default:
				break;
			}
			break;
		default:
			nData = reg_GetValue(nDa, nRegID);
			if (pItem->type & GW3761_DATA_T_MULTI)
				nData /= pItem->para;
			buf_PushData(b, nData, pItem->size);
			break;
		}
	}
	return 1;
}


//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
static void gw3761_DataGetOther(buf b, uint_t nDa, uint_t nReg, uint8_t **ppData)
{
	uint_t i, j, nTemp;
	uint8_t aTemp[40];
	uint32_t nData;

	////////////////////Unfinished////////////////////////
	switch (nReg) {
	case 0x4020:
		nData = reg_GetValue(TERMINAL, 0x0108);//nID_TransfNum
		if (reg_GetValue(TERMINAL, 0x0107))//nID_TransfEnable))
			buf_PushData(b, nData | BITMASK(7), 1);
		else
			buf_PushData(b, nData, 1);
		reg_Get(TERMINAL, 0x0109, aTemp);//nID_TransfAdr
		buf_Push(b, aTemp, nData * 2);
		break;
	case 0x4070:
		reg_Get(TERMINAL, 0x01d1, aTemp);//nID_VpnUser
		nData = MIN(strlen((char *)aTemp), 20);
		buf_PushData(b, nData, 1);
		buf_Push(b, aTemp, nData);
		reg_Get(TERMINAL, 0x01d2, aTemp);//nID_VpnPasswd
		nData = MIN(strlen((char *)aTemp), 20);
		buf_PushData(b, nData, 1);
		buf_Push(b, aTemp, nData);
		break;
	case 0x4100:
		buf_PushData(b, (reg_GetValue(nDa, 0x0413) << 5) | reg_GetValue(nDa, 0x0414), 1);//nID_MeterSpeed//nID_MeterPort
		break;
	case 0x4101:
		buf_PushData(b, (reg_GetValue(nDa, 0x0453) << 2) | reg_GetValue(nDa, 0x0454), 1);//nID_MeterIntNum//nID_MeterFraNum
		break;
	case 0x4102:
		buf_PushData(b, (reg_GetValue(nDa, 0x0456) << 4) | reg_GetValue(nDa, 0x0457), 1);//nID_MeterBigCla//nID_MeterSmaCla
		break;
	case 0x4370:
		if (reg_GetValue(TERMINAL, 0x0a36)) {//nID_CascadeMode
			//被级联
			buf_PushData(b, BITMASK(7) | 1, 1);
			reg_Get(TERMINAL, 0x0a38, aTemp);//nID_CascadeAdr
			buf_Push(b, aTemp, 4);
		} else {
			//级联
			nData = reg_GetValue(TERMINAL, 0x0a37);//nID_CascadeQty
			buf_PushData(b, nData, 1);
			reg_Get(TERMINAL, 0x0a38, aTemp);//nID_CascadeAdr
			buf_Push(b, aTemp, 4 * nData);
		}
		break;
	case 0x4380:
	case 0x4390:
		nDa = *(*ppData)++;
		buf_PushData(b, nDa, 1);
		nDa <<= 8;
		nTemp = *(*ppData)++;
		buf_PushData(b, nTemp, 1);
		for (i = 0; i < nTemp; i++) {
			j = *(*ppData)++;
			if (nReg == 0x4380) {
				nData = reg_GetValue(nDa | j, 0x0d10);//nID_Data1Qty
				reg_Get(nDa | j, 0x0d11, aTemp);//nID_Data1Param
			} else {
				nData = reg_GetValue(nDa | j, 0x0d20);//nID_Data2Qty
				reg_Get(nDa | j, 0x0d21, aTemp);//nID_Data2Param
			}
			buf_PushData(b, j, 1);
			buf_PushData(b, nData, 1);
			buf_Push(b, aTemp, nData);
		}
		break;
	case 0x4600: {
		//谐波限值
		//电压谐波限值
		reg_Get(TERMINAL, 0x0C60, aTemp);//nID_ULTDisV
		buf_Push(b, aTemp, 2);
		reg_Get(TERMINAL, 0x0C61, aTemp);//nID_ULODDDisV
		buf_Push(b, aTemp, 2);
		reg_Get(TERMINAL, 0x0C62, aTemp);//nID_ULEVEDisV
		buf_Push(b, aTemp, 2);
		for (i = 1; i < 18; i += 2)
			buf_Push(b, &aTemp[i * 2], 2);
		for (i = 2; i < 19; i += 2)
			buf_Push(b, &aTemp[i * 2], 2);
		//电流谐波限值
		reg_Get(TERMINAL, 0x0C70, aTemp);//nID_ULTDisC
		buf_Push(b, aTemp, 2);
		for (i = 1; i < 18; i += 2)
			buf_Push(b, &aTemp[i * 2], 2);
		for (i = 2; i < 19; i += 2)
			buf_Push(b, &aTemp[i * 2], 2);
		}break;
	case 0xC570:
		//谐波有效值
		for (nData = 0; nData < 6; ++nData) {
			gw3761_ConvertData_THD(nDa, nData, aTemp);
			buf_Push(b, aTemp + 2, 36);
		}
		break;
	case 0xC580:
		//谐波含有率
		///////////////////////////Unfinished///////////////////////////////
		for (nData = 0; nData < 3; ++nData) {
			reg_Get(nDa, 0x2131 + nData, aTemp);		//nID_HarPVolPHA	//A相电压谐波含有率
			buf_Push(b, aTemp, 38);
		}
		for (nData = 3; nData < 6; ++nData) {
			reg_Get(nDa, 0x2131 + nData, aTemp);		//nID_HarPVolPHA	//A相电压谐波含有率
			buf_Push(b, aTemp + 2, 36);
		}
		//////////////////////////////////////////////////////////////////
		break;
	////////////////////////////////////////////////////
#if 0
	case 0xFF01:
		//终端等待从动站响应的超时时间和重发次数
		reg_Get(nDa, 0x0103, &nData);
		nData = int_setbits(nData, 0, 12);
		reg_Get(nDa, 0x0104, &nData);
		nData |= int_setbits(nData, 12, 2);
		buf_PushData(b, nData, pItem->size);
		break;
	case 0xFF02:
		//终端中继转发设置
		reg_Get(nDa, 0x0107, &nData);
		nData = int_setbits(nData, 0, 1);
		reg_Get(nDa, 0x0108, &nData);
		nData |= int_setbits(nData, 1, 7);
		buf_PushData(b, nData, pItem->size);
		break;
	case 0xFF04:
		//电表通信速率及端口号
		reg_Get(nDa, 0x2214, &nData);
		nData = int_setbits(nData, 0, 5);
		reg_Get(nDa, 0x2213, &nData);
		switch (nData) {
		case 600:
			nData = 1;
			break;
		case 1200:
			nData = 2;
			break;
		case 2400:
			nData = 3;
			break;
		case 4800:
			nData = 4;
			break;
		case 7200:
			nData = 5;
			break;
		case 9600:
			nData = 6;
			break;
		case 19200:
			nData = 7;
			break;
		default:
			nData = 0;
			break;
		}
		nData |= int_setbits(nData, 5, 3);
		buf_PushData(b, nData, pItem->size);
		break;
	case 0xFF05:
		//电表小数位数
		reg_Get(nDa, 0x2254, &nData);
		nData = int_setbits(nData - 1, 0, 2);
		reg_Get(nDa, 0x2253, &nData);
		nData |= int_setbits(nData - 4, 2, 2);
		reg_Get(nDa, 0x2251, &nData);
		nData |= int_setbits(nData, 4, 4);
		buf_PushData(b, nData, pItem->size);
		break;
#endif		
	default:
		break;
	}			
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
static void gw3761_DataSetOther(uint_t nDa, uint_t nReg, uint8_t **ppData)
{
	uint_t i, j, k, nUser1, nUser2;
	uint8_t aTemp[40];

	////////////////Unfinished/////////////////
	switch (nReg) {
	case 0x4020:
		i = *(*ppData)++;
		if (i & BITMASK(7))
			reg_SetValue(TERMINAL, 0x0107, 1, 0);//nID_TransfEnable
		else
			reg_SetValue(TERMINAL, 0x0107, 0, 0);//nID_TransfEnable
		CLRBIT(i, 7);
		reg_Set(TERMINAL, 0x0108, &i, 0);//nID_TransfNum
		reg_Set(TERMINAL, 0x0109, *ppData, 1);//nID_TransfAdr
		*ppData += i * 2;
		break;
	case 0x4070:
		i = *(*ppData)++;
		memcpy(aTemp, *ppData, i);
		aTemp[i] = '\0';
		reg_Set(TERMINAL, 0x01d1, aTemp, 1);//nID_VpnUser
		*ppData += i;
		i = *(*ppData)++;
		memcpy(aTemp, *ppData, i);
		aTemp[i] = '\0';
		reg_Set(TERMINAL, 0x01d2, aTemp, 1);//nID_VpnPasswd
		*ppData += i;
		break;
	case 0x4100:
		i = *(*ppData)++;
		reg_SetValue(nDa, 0x0413, i >> 5, 0);//nID_MeterSpeed
		reg_SetValue(nDa, 0x0414, i & 0x1F, 1);//nID_MeterPort
		break;
	case 0x4101:
		i = *(*ppData)++;
		reg_SetValue(nDa, 0x0453, i >> 2, 0);//nID_MeterIntNum
		reg_SetValue(nDa, 0x0454, i & 0x03, 1);//nID_MeterFraNum
		break;
	case 0x4102:
		i = *(*ppData)++;
		reg_SetValue(nDa, 0x0456, i >> 4, 0);//nID_MeterBigCla
		reg_SetValue(nDa, 0x0457, i & 0x0F, 1);//nID_MeterSmaCla
		break;
	case 0x4370:
		i = *(*ppData)++;
		if (i & BITMASK(7)) {
			//被级联
			reg_SetValue(TERMINAL, 0x0a36, 1, 1);//
			reg_Set(TERMINAL, 0x0a38, *ppData, 1);//nID_CascadeAdr
			*ppData += 4;
		} else {
			//级联
			i &= 3;
			reg_Set(TERMINAL, 0x0a37, &i, 1);//nID_CascadeQty
			reg_Set(TERMINAL, 0x0a38, *ppData, 1);//nID_CascadeAdr
			*ppData += (4 * i);
		}
		break;
	case 0x4380:
	case 0x4390:
		nUser1 = *(*ppData)++;
		j = *(*ppData)++;
		for (i = 0; i < j; i++) {
			nUser2 = *(*ppData)++;
			k = *(*ppData)++;
			if (nReg == 0x4380) {
				reg_Set((nUser1 << 8) | nUser2, 0x0d10, &k, 0);//nID_Data1Qty
				reg_Set((nUser1 << 8) | nUser2, 0x0d11, *ppData, 1);//nID_Data1Param
			} else {
				reg_Set((nUser1 << 8) | nUser2, 0x0d20, &k, 0);//nID_Data2Qty
				reg_Set((nUser1 << 8) | nUser2, 0x0d21, *ppData, 1);//nID_Data2Param
			}
			*ppData += k;
		}
		break;
	case 0x4600: {
		//谐波限值
		//电压谐波限值
		memcpy(&aTemp[0], *ppData, 2);
		*ppData += 2;
		reg_Set(TERMINAL, 0x0C61, *ppData, 0);//nID_ULODDDisV
		*ppData += 2;
		reg_Set(TERMINAL, 0x0C62, *ppData, 0);//nID_ULEVEDisV
		*ppData += 2;
		for (i = 1; i < 18; i += 2, *ppData += 2)
			memcpy(&aTemp[i * 2], *ppData, 2);
		for (i = 2; i < 19; i += 2, *ppData += 2)
			memcpy(&aTemp[i * 2], *ppData, 2);
		reg_Set(TERMINAL, 0x0C60, aTemp, 0);//nID_ULTDisV
		//电流谐波限值
		memcpy(&aTemp[0], *ppData, 2);
		*ppData += 2;
		for (i = 1; i < 18; i += 2, *ppData += 2)
			memcpy(&aTemp[i * 2], *ppData, 2);
		for (i = 2; i < 19; i += 2, *ppData += 2)
			memcpy(&aTemp[i * 2], *ppData, 2);
		reg_Set(TERMINAL, 0x0C70, aTemp, 1);//nID_ULTDisC
		}break;
	case 0x5010:
		break;
	default:
		break;
	}
	/////////////////////////////////////////
#if 0
		case 0xFF01:
			//终端等待从动站响应的超时时间和重发次数
			memcpy(&nData, pData, pItem->size);
			nTemp = int_getbits(nData, 0, 12);
			reg_Set(nDa, 0x0103, &nTemp, 0);
			nTemp = int_getbits(nData, 12, 2);
			reg_Set(nDa, 0x0104, &nTemp, 1);
			break;
		case 0xFF02:
			//终端中继转发设置
			memcpy(&nData, pData, pItem->size);
			nTemp = int_getbits(nData, 0, 7);
			reg_Set(nDa, 0x0108, &nTemp, 0);
			nTemp = int_getbits(nData, 7, 1);
			reg_Set(nDa, 0x0107, &nTemp, 1);
			break;
		case 0xFF04:
			//电表通信速率及端口号
			memcpy(&nData, pData, pItem->size);
			nTemp = int_getbits(nData, 0, 5);
			reg_Set(nDa, 0x2214, &nTemp, 0);
			nTemp = int_getbits(nData, 5, 3);
			switch (nTemp) {
			case 1:
				nTemp = 600;
				break;
			case 2:
				nTemp = 1200;
				break;
			case 3:
				nTemp = 2400;
				break;
			case 4:
				nTemp = 4800;
				break;
			case 5:
				nTemp = 7200;
				break;
			case 6:
				nTemp = 9600;
				break;
			case 7:
				nTemp = 19200;
				break;
			default:
				break;
			}
			reg_Set(nDa, 0x2213, &nTemp, 1);
			break;
		case 0xFF05:
			//电表小数位数
			memcpy(&nData, pData, pItem->size);
			nTemp = int_getbits(nData, 0, 2) + 1;
			reg_Set(nDa, 0x2254, &nTemp, 0);
			nTemp = int_getbits(nData, 2, 2) + 4;
			reg_Set(nDa, 0x2253, &nTemp, 0);
			nTemp = int_getbits(nData, 4, 4);
			reg_Set(nDa, 0x2251, &nTemp, 1);
			break;
#endif
}



//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
#if 0
int gw3761_ResponseData1(buf b, u_word2 *pDu, uint_t nAfn, uint8_t **ppData)
{
#if GW3761_TYPE < GW3761_T_GWFK2004
	uint16_t aDa[8];
#else
	uint8_t aDa[64];
#endif
	int nLen, res = 0;
	uint_t i, j, k, l, nDa, nDaQty, nLoop, nDataValid, nRateQty = 0, nRate, nOffset = 80;
	uint8_t *pTemp;
	uint32_t nMulti;
	p_gw3761_item pItem, pItemEnd;
	p_gw3761_dt pDT;

	nDaQty = gw3761_ConvertDa2Map(pDu->word[0], aDa);
	for (i = 0; i < nDaQty; i++) {
		nDa = aDa[i];
		for (j = 0; j < 8; j++) {
			if ((pDu->word[1] & BITMASK(j)) == 0)
				continue;
			if ((pDT = gw3761_DataGetTable(nAfn, (pDu->word[1] & 0xFF00) | BITMASK(j))) == NULL)
				continue;
			pItem = pDT->item;
			nDataValid = 1;
			if (nAfn == GW3761_AFN_DATA_L1) {
				// 1类数据检查有效性
				nLoop = 1;
				switch (pDT->datype) {
				case GW3761_DA_T_TN:
					///////////////////Unfinished/////////////////////////
					//if ((g_nTnDataValid & BITMASK(nDa)) == 0)
					//	nDataValid = 0;
					////////////////////////////////////////////////////
					break;
				case GW3761_DA_T_GROUP:
					///////////////////Unfinished/////////////////////////
					if (reg_GetValue(nDa, 0x0711) == 0)//nID_GroupTP
						nDataValid = 0;
					////////////////////////////////////////////////////
					break;
				default:
					break;
				}
			}
			if (nDataValid)
				res += 1;
			nLoop = 0;
			switch (pDT->dttype) {
			case GW3761_DT_T_PARAM_METER:
				memcpy(&nLoop, *ppData, 2);
				*ppData += 2;
				buf_PushData(b, nLoop, 2);
				break;
			case GW3761_DT_T_PARAM_GROUP:
			case GW3761_DT_T_PARAM_ANA:
			case GW3761_DT_T_PARAM_PULSE:
			case GW3761_DT_T_PARAM_DIFF:
			case GW3761_DT_T_PARAM_PORT:
				nLoop = *(*ppData)++;
				buf_PushData(b, nLoop, 1);
				break;
			case GW3761_DT_T_PARAM_FREEZE:
				nLoop = 32;
				buf_PushData(b, nLoop, 1);
				break;
			case GW3761_DT_T_DATA_HOUR: {
				uint_t nTimeSign, nFn;
				nLoop = 0;
				nFn = gw3761_ConvertDt2Fn((pDu->word[1] & 0xFF00) | BITMASK(j));
				reg_Get(nDa, 0x0F00 | nFn, &nTimeSign);
				buf_PushData(b, nTimeSign, 1);
				switch (nTimeSign & 0xC0) {
				case 0x40:
					nTimeSign = 4;
					break;
				case 0x80:
					nTimeSign = 2;
					break;
				case 0xC0://3
					nTimeSign = 1;
					break;
				default:
					nTimeSign = 0;
					break;
				}
				for (k = 0; k < nTimeSign; ++k) {
					if (nDataValid)
						gw3761_DataGetRegValue(b, nDa, pItem, (nFn - 80) * 4 + k);
					else
						buf_PushData(b, GW3761_DATA_INVALID, pItem->size);
				}
				}break;
			default:
				nLoop = 1;
				break;
			}
			for (; nLoop; nLoop--) {
				nMulti = 0x80000000;
				nRate = 0;
				for (pItem = pDT->item, pItemEnd = pDT->item + pDT->qty ; pItem < pItemEnd; pItem++) {
					switch (pItem->type) {
					case GW3761_DATA_T_OTHER:
						gw3761_DataGetOther(b, nDa, pItem->reg, ppData);
						break;
					case GW3761_DATA_T_INDEX:
						nDa = 0;
						memcpy(&nDa, *ppData, pItem->size);
						*ppData += pItem->size;
						buf_PushData(b, nDa, pItem->size);
						break;
					case GW3761_DATA_T_FREEZEPARAM: {
						uint_t nTimeSign= 0;
						while (nOffset++ < 122)
							if (reg_Get(nDa, pItem->reg | nOffset, &nTimeSign) != NULL) {
								nTimeSign >>= 6;
								buf_PushData(b, nOffset, 1);
								buf_PushData(b, nTimeSign, pItem->size - 1);
								break;
							}
						}break;
					case GW3761_DATA_T_CONST:
						buf_PushData(b, pItem->reg, pItem->size);
						break;
					case GW3761_DATA_T_REPEAT:
						nMulti = reg_GetValue(nDa, pItem->reg);
						buf_PushData(b, nMulti, pItem->size);
						break;
					case GW3761_DATA_T_RDONLY:
						pTemp = mem_Malloc(pItem->size);
						if (pTemp == NULL)
							break;
						reg_Get(nDa, pItem->reg, pTemp);
						buf_Push(b, pTemp, pItem->size);
						mem_Free(pTemp);
						reg_DefaultReg(nDa, pItem->reg, pItem->para);
						break;
					case GW3761_DATA_T_RATE:
						nRateQty = reg_GetValue(nDa, pItem->reg);
						nRate = pItem->size;
						buf_PushData(b, nRateQty, 1);
						break;
					case GW3761_DATA_T_DYNAMIC:
					default:
						if (nMulti == 0x80000000) {
							if (nRate) {
								nRate -= 1;
								l = nRateQty;
							} else {
								l = 0;
							}
							for (k = 0; k <= l; k++) {
								if (nDataValid)
									gw3761_DataGetRegValue(b, nDa, pItem, k);
								else
									buf_Fill(b, GW3761_DATA_INVALID, pItem->size);
							}
						} else {
							nLen = reg_GetRegSize(pItem->reg);
							if (nLen != -1) {
								pTemp = mem_Malloc(nLen);
								if (pTemp != NULL) {
									reg_Get(nDa, pItem->reg, pTemp);
									buf_Push(b, pTemp, pItem->size * nMulti);
									mem_Free(pTemp);
								}
							}
						}
						break;
					}
				}
			}
		}
	}
	return res;
}
#endif

#if 0 //Unfinished
int gw3761_ResponseData2(buf b, uint_t nDa, uint_t nDt, uint8_t **ppData, uint_t nIsReport)
{
#if GW3761_TYPE < GW3761_T_GWFK2004
	uint16_t aDa[8];
#else
	uint8_t aDa[64];
#endif
	uint_t i, j, nQty, nDaQty, nReadQty = 0, nInterval = 0;
	time_t tTime;
	p_gw3761_dt pDt;
	char filename[STAT_FREEZE_FILE_OFFSET + 8 + 1 + 8 + 1];

	nDaQty = gw3761_ConvertDa2Map(nDa, aDa);
	for (i = 0; i < nDaQty; i++) {
		nDa = aDa[i];
		for (j = 0; j < 8; j++) {
			if ((nDt & BITMASK(j)) == 0)
				continue;
			if ((pDt = gw3761_DataGetTable(GW3761_AFN_DATA_L2, (nDt & 0xFF00) | BITMASK(j))) == NULL)
				continue;
			switch (pDt->dttype) {
			case GW3761_DT_T_DATA_CURVE:
				tTime = bin2timet(0, (*ppData)[0], (*ppData)[1], (*ppData)[2], (*ppData)[3], (*ppData)[4], 1);
				nInterval = (*ppData)[5];
				nQty = (*ppData)[6];
				switch (nInterval) {
				case 0:
					nInterval = 0;
					break;
				case 1:
					nInterval = 900;
					break;
				case 2:
					nInterval = 1800;
					break;
				case 254:
					nInterval = 300;
					break;
				case 255:
					nInterval = 60;
					break;
				case 3:
				default:
					nInterval = 3600;
					break;
				}
				buf_Push(b, *ppData, 7);
				*ppData += 7;
				for (; nQty; nQty--, tTime += nInterval) {
					rcp_GDW_FreezePathCurve(filename, tTime);
					rcp_GDW_FreezeName(filename, nDa, pDt->dtid, pDt->dttype);
					nReadQty += rcp_GDW_FreezeRead(filename, b);
				}
				break;
			case GW3761_DT_T_DATA_DAY:
			case GW3761_DT_T_DATA_MDAY:
				buf_Push(b, *ppData, 3);
				rcp_GDW_FreezePath(filename, (*ppData)[2], (*ppData)[1], (*ppData)[0], pDt->dttype);
				*ppData += 3;
				rcp_GDW_FreezeName(filename, nDa, pDt->dtid, pDt->dttype);
				nReadQty += rcp_GDW_FreezeRead(filename, b);
				break;
			case GW3761_DT_T_DATA_MONTH:
				buf_Push(b, *ppData, 2);
				rcp_GDW_FreezePath(filename, (*ppData)[1], (*ppData)[0], 0, pDt->dttype);
				*ppData += 2;
				rcp_GDW_FreezeName(filename, nDa, pDt->dtid, pDt->dttype);
				nReadQty += rcp_GDW_FreezeRead(filename, b);
			default:
				break;
			}
		}
	}
	return nReadQty;
}
#endif

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
#if 0 //Unfinished
int gw3761_ResponseEventQuery(buf b, uint_t nDa, uint_t nDt, uint8_t **ppData)
{
	int res = 0;
	uint_t nEventType, nQty, nPm, nPn;
	uint8_t *pTemp;

	nPm = (*ppData)[0];
	nPn = (*ppData)[1];
	*ppData += 2;
	switch (nDt) {
	case 0x0001:
		//重要事件
		nEventType = 1;
		break;
	case 0x0002:
		//一般事件
		nEventType = 2;
		break;
	default:
		nEventType = 0;
		break;
	}
	if (nEventType) {
		////////////////Unfinished///////////////
		buf_PushData(b, (reg_GetValue(TERMINAL, 0x1009) << 8) | reg_GetValue(TERMINAL, 0x1008), 2);
		/////////////////////////////////////////
		buf_PushData(b, nPm, 2);
		pTemp = &b->p[b->len - 2];
		if (nPm > nPn) {
			nQty = event_Read(b, nEventType, nPn, 256 - nPn);
			res += nQty;
			nQty = event_Read(b, nEventType, 0, nPm);
			if (res) {
				if (nQty)
					pTemp[1] = nQty;
				else
					pTemp[1] = nPn + res;
			} else {
				pTemp[0] = 0;
				pTemp[1] = nQty;
			}
			res += nQty;
		} else {
			nQty = event_Read(b, nEventType, nPm, nPn - nPm);
			pTemp[1] = nPm + nQty;
			res += nQty;
		}
		return res;
	}
	return 0;
}
#endif





//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
int gw3761_ResponseSetParam(p_gw3761 p)
{
#if GW3761_TYPE < GW3761_T_GWFK2004
	uint16_t aDa[8];
#else
	uint8_t aDa[64];
#endif
	int res = 0, nLen;
	uint8_t *pTemp, *pData, *pEnd;
	uint32_t nData, nTemp;
	uint_t i, j, nMulti, nDa, nDaQty, nLoop;
	float fData;
	time_t tTime;
	u_word2 uDu;
	p_gw3761_item pItem, pItemEnd;
	p_gw3761_dt pDT;

	pData = p->rmsg.data->p;
	pEnd = pData + p->rmsg.data->len;
	for (; (pData + 4) <= pEnd; ) {
		memcpy(&uDu, pData, 4);
		buf_Push(b, pData, 4);
		pData += 4;
		nDaQty = gw3761_ConvertDa2Map(uDu.word[0], aDa);
		for (i = 0; i < nDaQty; i++) {
			for (j = 0; j < 8; j++) {
				if ((uDu.word[1] & BITMASK(j)) == 0)
					continue;
				if ((pDT = gw3761_DataGetTable(p->rmsg.afn, (uDu.word[1] & 0xFF00) | BITMASK(j))) == NULL)
					continue;
				nLoop = 0;
				switch (pDT->dttype) {
				case GW3761_DT_T_PARAM_METER:
					memcpy(&nLoop, pData, 2);
					pData += 2;
					break;
				case GW3761_DT_T_PARAM_GROUP:
				case GW3761_DT_T_PARAM_ANA:
				case GW3761_DT_T_PARAM_PULSE:
				case GW3761_DT_T_PARAM_FREEZE:
				case GW3761_DT_T_PARAM_PORT:
					nLoop = *pData++;
					break;
				case GW3761_DT_T_PARAM_DIFF:
					nLoop = *pData++;
					reg_Set(TERMINAL, 0x0845, &nLoop, 1);
					break;
				default:
					nLoop = 1;
					break;
				}
				for (; nLoop; nLoop--) {
					nDa = aDa[i];
					nMulti = 1;
					pItem = pDT->item;
					pItemEnd = pDT->item + pDT->qty;
					for (; pItem < pItemEnd; pItem++) {
						nData = 0;
						nTemp = 0;
						switch (pItem->type & ~GW3761_DATA_T_MULTI) {
						case GW3761_DATA_T_OTHER:
							gw3761_DataSetOther(nDa, pItem->reg, &pData);
							break;
						case GW3761_DATA_T_INDEX:
							nDa = 0;
							memcpy(&nDa, pData, pItem->size);
							if (nDa > pItem->para)
								nDa = pItem->para;
							break;
						case GW3761_DATA_T_FREEZEPARAM:
							reg_SetValue(nDa, pItem->reg | pData[0], pData[1] << 6, 1);
							break;
						case GW3761_DATA_T_REPEAT:
							memcpy(&nMulti, pData, pItem->size);
							if (pItem->reg)
								reg_Set(nDa, pItem->reg, &nMulti, 1);
							//////////////////Unfinished//////////tbl_GW3761表中未配置pItem->para//////////////
							//if (nMulti > pItem->para)
							//	nMulti = pItem->para;
							///////////////////////////////////////////////////////////////////////////////////
							break;
						case GW3761_DATA_T_CONTROL:
							reg_SetValue(nDa, pItem->reg, pItem->para, 1);
							break;
						case GW3761_DATA_T_DYNAMIC:
							reg_Set(nDa, pItem->reg, pData, 1);
							break;
						case GW3761_DATA_T_12:
						case GW3761_DATA_T_BIN:
							nLen = reg_GetRegSize(pItem->reg);
							if (nLen < 0)
								break;
							if (nLen <= 4) {
								memcpy(&nData, pData, pItem->size);
								if (pItem->type & GW3761_DATA_T_MULTI)
									nData *= pItem->para;
								reg_SetValue(nDa, pItem->reg, nData, 1);
							} else if (nLen <= pItem->size) {
								reg_Set(nDa, pItem->reg, pData, 1);
							} else {
								//寄存器长度大于报文数据长度,需补0
								pTemp = mem_Malloc(nLen);
								if (pTemp == NULL)
									break;
								memset(pTemp, 0, nLen);
								memcpy(pTemp, pData, pItem->size);
								reg_Set(nDa, pItem->reg, pTemp, 1);
								mem_Free(pTemp);
							}
							break;
						case GW3761_DATA_T_01:
							switch (reg_GetRegType(pItem->reg)) {
							case REG_TYPE_TIME:
								tTime = bin2timet(pData[0], pData[1], pData[2], pData[3], pData[4] & 0x1F, pData[5], 1);
								reg_Set(nDa, pItem->reg, &tTime, 1);
								break;
							default:
								break;
							}
							break;
						case GW3761_DATA_T_18:
							switch (reg_GetRegType(pItem->reg)) {
							case REG_TYPE_TIME:
								tTime = bin2timet(0, pData[0], pData[1], pData[2], 1, 0, 1);
								reg_Set(nDa, pItem->reg, &tTime, 1);
								break;
							default:
								break;
							}
							break;
						case GW3761_DATA_T_03:
							memcpy(&nTemp, pData, pItem->size);
							fData = bcd2bin32(nTemp & 0x0FFFFFFF);
							if (nTemp & 0x40000000)
								fData *= 1000.0f;
							if (nTemp & 0x10000000)
								fData = 0.0f - fData;
							reg_SetValue(nDa, pItem->reg, FLOAT2FIX(fData), 1);
							break;
						case GW3761_DATA_T_05:
							memcpy(&nTemp, pData, 2);
							fData = bcd2bin32((nTemp & 0x00007FF0) >> 4) + (float)bcd2bin8(nTemp & 0x0000000F) / 10.0f;
							if (nTemp & 0x00008000)
								fData = 0.0f - fData;
							reg_SetValue(nDa, pItem->reg, FLOAT2FIX(fData), 1);
							break;
						case GW3761_DATA_T_PERCENT:
							memcpy(&nTemp, pData, pItem->size);
							fData = bcd2bin32((nTemp & 0x00007FF0) >> 4) + (float)bcd2bin8(nTemp & 0x0000000F) / 10.0f;
							if (nTemp & 0x00008000)
								fData = 0.0f - fData;
							reg_SetValue(nDa, pItem->reg, FLOAT2FIX(fData / 100.0f), 1);
							break;
						case GW3761_DATA_T_06:
							memcpy(&nTemp, pData, 2);
							fData = bcd2bin32((nTemp & 0x00007F00) >> 8) + (float)bcd2bin8(nTemp) / 100.0f;
							if (nTemp & 0x00008000)
								fData = 0.0f - fData;
							nData = FLOAT2FIX(fData);
							reg_Set(nDa, pItem->reg, &nData, 1);
							break;
						case GW3761_DATA_T_07:
							memcpy(&nTemp, pData, 2);
							nData = bcd2bin32((nTemp & 0x0000FFF0) >> 4) << EXP;
							nTemp = (bcd2bin8(nTemp & 0x0000000F) << EXP) / 10;
							nData |= nTemp;
							reg_Set(nDa, pItem->reg, &nData, 1);
							break;
						case GW3761_DATA_T_13:
						case GW3761_DATA_T_14:
						case GW3761_DATA_T_23:
							memcpy(&nTemp, pData, 2);
							nTemp = (bcd2bin32(nTemp) << EXP) / 10000;
							memcpy(&nData, pData + 2, pItem->size - 2);
							nData = bcd2bin32(nData) << EXP | nTemp;
							reg_Set(nDa, pItem->reg, &nData, 1);
							break;
						case GW3761_DATA_T_22:
							memcpy(&nTemp, pData, 1);
							nData = bcd2bin8((nTemp & 0x000000F0) >> 4) << EXP;
							nTemp = (bcd2bin8(nTemp & 0x0000000F) << EXP) / 10;
							nData |= nTemp;
							reg_Set(nDa, pItem->reg, &nData, 1);
							break;
						case GW3761_DATA_T_25:
							memcpy(&nTemp, pData, 3);
							fData = bcd2bin32((nTemp & 0x007FF000) >> 12) + (float)bcd2bin16(nTemp & 0x00000FFF) / 1000.0f;
							if (nTemp & 0x00800000)
								fData = 0.0f - fData;
							nData = FLOAT2FIX(fData);
							reg_Set(nDa, pItem->reg, &nData, 1);
							break;
						case GW3761_DATA_T_26:
							memcpy(&nTemp, pData, 2);
							nData = bcd2bin32((nTemp & 0x0000F000) >> 12) << EXP;
							nTemp = (bcd2bin32(nTemp & 0x00000FFF) << EXP) / 1000;
							nData |= nTemp;
							reg_Set(nDa, pItem->reg, &nData, 1);
							break;
						case GW3761_DATA_T_02:
						case GW3761_DATA_T_04:
						case GW3761_DATA_T_08:
						case GW3761_DATA_T_09:
						case GW3761_DATA_T_10:
						case GW3761_DATA_T_11:
						case GW3761_DATA_T_15:
						case GW3761_DATA_T_16:
						case GW3761_DATA_T_17:
						case GW3761_DATA_T_19:
						case GW3761_DATA_T_20:
						case GW3761_DATA_T_21:
						default: {
							uint64_t nLong = 0;
							if (pItem->size > 8)
								memcpy(&nLong, pData, 8);
							else
								memcpy(&nLong, pData, pItem->size);
							nLong = bcd2bin64(nLong);
							reg_Set(nDa, pItem->reg, &nLong, 1);
							}break;
						}
						switch (pItem->type) {
						case GW3761_DATA_T_REPEAT:
							pData += pItem->size;
							break;
						case GW3761_DATA_T_OTHER:
							break;
						default:
							pData += nMulti * pItem->size;
							break;
						}
					}
				}
				res += 1;
			}
		}
	}
	if (res) {
		evt_ERC3(p->parent.msta, &uDu);
		gw3761_TmsgConfirm(p);
	} else {
		gw3761_TmsgReject(p);
	}
	return res;
}

#endif



