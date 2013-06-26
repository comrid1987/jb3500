void gw3761_FreezeName(char *p, uint8_t nYear, uint8_t nMon, uint8_t nDay, uint8_t nNum, uint32_t nType){
    switch(nType) {
        case GW3761_DT_T_DATA_CURVE:
            strcpy(p, FILENAME_CURVE);
            break;
        case GW3761_DT_T_DATA_DAY:
            strcpy(p, FILENAME_DAY);
            break;
        case GW3761_DT_T_DATA_MDAY:
            strcpy(p, FILENAME_MDAY);
            break;
        case GW3761_DT_T_DATA_MONTH:
            strcpy(p, FILENAME_MONTH);
            break;
        default:
            return;
    }
    lib_Int2Str(nYear, &p[1], 2, 1);
    lib_Int2Str(nMon, &p[3], 2, 1);
    if (nType != GW3761_DT_T_DATA_MONTH) {
        lib_Int2Str(nDay, &p[5], 2, 1);
    }
    if (nType == GW3761_DT_T_DATA_CURVE) {
        lib_Int2Str(nNum, &p[7], 2, 1);
    }
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
int rcp_GDW_CurveRead (uint8_t nDa, BUFFER *pBuf, t_gw3761_dt *pDT, uint32_t nQty, uint32_t nInterval, time_t tTime){
    FILE *file;
    uint8_t arrGroup[12], arrTn[78];
    uint32_t nYear, nMon, nDay, nHour, nMin, i, nSucceed = 0;
    int nValidQty = 0;
    struct tm tmTime;
    t_stat_FreezeCurve stuIndex;
    char strFile[12];

    if (pDT->dttype != GW3761_DT_T_DATA_CURVE) {
        return 0;
    }
    for (tTime -= 30; nQty; --nQty, tTime += nInterval){
        time_Timet2Tm(tTime, &tmTime);
        nYear = (uint8_t)tmTime.tm_year - 100;
        nMon = tmTime.tm_mon + 1;
        nDay = tmTime.tm_mday;
        nHour = tmTime.tm_hour;
        nMin = tmTime.tm_min + 1;
        nMin = nHour * 4 + nMin / 15;
        gw3761_FreezeName(strFile, nYear, nMon, nDay, nMin, GW3761_DT_T_DATA_CURVE);
        nSucceed = 0;
        if ((file = flashFS_Open(strFile, FILE_OPEN_READ)) != NULL){
            for (i = MAX_QTY_TN; i; --i) {
                if (fs_read(&stuIndex, 1, sizeof(stuIndex), file) == sizeof(stuIndex)){
                    if (stuIndex.nDa < nDa) {
                        // 没有有效数据(因为记录按Da从小到大排列)
                        break;
                    }
                    if (stuIndex.nDa == nDa) {
                        if (stuIndex.nSign & 0x01) {
                            if (fs_read(arrTn, 1, sizeof(arrTn), file) != sizeof(arrTn)) {
                                break;
                            }
                        }
                        if (stuIndex.nSign & 0x02) {
                            if (fs_read(arrGroup, 1, sizeof(arrGroup), file) != sizeof(arrGroup)) {
                                break;
                            }
                        }
                        switch (pDT->datype) {
                            case DA_TYPE_TN:
                                buffer_Push(pBuf, arrTn + pDT->item->para, pDT->item->size);
                                nSucceed = 1;
                                nValidQty++;
                                break;
                            case DA_TYPE_GROUP:
                                buffer_Push(pBuf, arrGroup + pDT->item->para, pDT->item->size);
                                nSucceed = 1;
                                nValidQty++;
                                break;
                            default:
                                break;
                        }
                        break;
                    } else {
                        if (fseek(file, stuIndex.nLen, SEEK_CUR)) {
                            break;
                        }
                    }
                }
            }
            flashFS_Close(file);
        }
        if (nSucceed == 0) {
            // 失败,填EEH
            memset(arrTn, 0xEE, pDT->item->size);
            buffer_Push(pBuf, arrTn, pDT->item->size);
        }
    }
    return nValidQty;
}

int rcp_GDW_FreezeRead (char *strFile, uint8_t nDa, uint16_t nDT, BUFFER *pBuf){
    FILE *file;
    uint8_t nReadDa, *pTemp;
    uint16_t nReadDT;
    uint32_t i, nHandleLen, nDaLen, nDTLen;
    int nValidQty = 0;

    if ((file = flashFS_Open(strFile, FILE_OPEN_READ)) != NULL){
        for (i = MAX_QTY_TN; i; --i) {
            if (fs_read(&nDaLen, 1, sizeof(nDaLen), file) == sizeof(nDaLen)){
                nReadDa = nDaLen;
                nDaLen >>= 8;
                if (nReadDa < nDa) {
                    // 没有有效数据(因为记录按Da从小到大排列)
                    break;
                }
                if (nReadDa == nDa) {
                    for (nHandleLen = 0; nHandleLen < nDaLen; ) {
                        if (fs_read(&nDTLen, 1, sizeof(nDTLen), file) != sizeof(nDTLen)){
                            break;
                        }
                        nReadDT = nDTLen;
                        nDTLen >>= 16;
                        if (nReadDT == nDT) {
                            pTemp = mem_Malloc(nDTLen);
                            if (fs_read(pTemp, 1, nDTLen, file) == nDTLen) {
                                // 正确读取一个点
                                buffer_Push(pBuf, pTemp, nDTLen);
                                nValidQty++;
                            }
                            mem_Free(pTemp);
                            break;
                        } else {
                            if (fseek(file, nDTLen, SEEK_CUR)) {
                                break;
                            }
                        }
                        nHandleLen += nDTLen;
                    }
                    break;
                } else {
                    // 下一个Da
                    if (fseek(file, nDaLen, SEEK_CUR)) {
                        break;
                    }
                }
            }
        }
        flashFS_Close(file);
    }
    return nValidQty;
}

void rcp_GDW_Freeze(uint32_t nType, time_t tTime)
{
    uint8_t nYear, nMon, nDay;
    uint32_t i, j, nGroupQty, nTnValid, nValid;
    BUFFER buf = {0}, bufDt = {0}, bufDa = {0};
    struct tm tmTime;
    char strFile[8];

    switch (nType) {
        case GW3761_DT_T_DATA_DAY:  // 调整为上一日
            tTime -= (60 * 60 * 24);
            break;
        case GW3761_DT_T_DATA_MONTH:    // 调整为上一月
            tTime -= (60 * 60 * 24 * 28);
            break;
        case GW3761_DT_T_DATA_MDAY:
        default:
            break;
    }
    time_Timet2Tm(tTime, &tmTime);
    nYear = (uint8_t)tmTime.tm_year - 100;
    nMon = tmTime.tm_mon + 1;
    nDay = tmTime.tm_mday;
    nGroupQty = reg_GetValue(TERMINAL, 0x0844) & MAX_MASK_TN_GROUP;
    for (i = 1; i <= MAX_QTY_TN; ++i) {
        t_gw3761_dt *pDT2;
        // 判定测量点是否有效
        if (g_nTnValid & GETBIT(i - 1)){
            nTnValid = 1;
        } else {
            nTnValid = 0;
        }
        if (nTnValid || (i <= nGroupQty)){
            for (j = rcp_GDW_GetTable_DT(0x0D, &pDT2); j; --j, ++pDT2) {
                nValid = 0;
                if (pDT2->dttype == nType) {
                    switch (pDT2->datype) {
                        case DA_TYPE_TER:   // 终端
                            if (i == 1) {
                                nValid = 1;
                            }
                            break;
                        case DA_TYPE_TN:    // 测量点
                            if (nTnValid) {
                                nValid = 1;
                            }
                            break;
                        case DA_TYPE_GROUP: // 总加组
                            if (i <= nGroupQty) {
                                nValid = 1;
                            }
                            break;
                        default:
                            break;
                    }
                }
                if (nValid) {
                    if (rcp_GDW_DataQuery(&bufDt, rcp_GDW_Da2DA(i), pDT2->dtid, 0x0D)) {
                        buffer_Push(&bufDa, (uint8_t *)&pDT2->dtid, 2);
                        buffer_Push(&bufDa, (uint8_t *)&bufDt.nLen, 2);
                        buffer_Push(&bufDa, bufDt.p, bufDt.nLen);
                    }
                    buffer_Free(&bufDt);
                }
            }
        }
        if (bufDa.nLen) {
            buffer_Push(&buf, (uint8_t *)&i, 1);
            buffer_Push(&buf, (uint8_t *)&bufDa.nLen, 3);
            buffer_Push(&buf, bufDa.p, bufDa.nLen);
            buffer_Free(&bufDa);
        }
    }
    if (buf.nLen) {
        // 需要保存
        gw3761_FreezeName(strFile, nYear, nMon, nDay, 0, nType);
        flashFS_Write(strFile, buf.p, buf.nLen);
    }
    buffer_Free(&buf);
}


int rcp_GDW_FreezeInquire(uint8_t nAFN, BUFFER *pBuf, uint8_t **ppRecData) {
    uint8_t nDT1Mask;
    uint16_t nDT;
    uint32_t i, j, k, nQty, nDtQty, nDaQty, nReadQty = 0, nInterval = 0;
    U64 nDaMask, nDaBits = 0;
    time_t tTime;
    char strFile[12];
    t_gw3761_dt *pDT;

    nDaQty = rcp_GDW_DA2Map(**ppRecData, *(*ppRecData + 1), &nDaBits);
    memcpy(&nDT, *ppRecData + 2, 2);
    *ppRecData += 4;
    for (nDaMask = 1, i = 0; (i < 64) && (nDaQty != 0); ++i, --nDaQty, nDaMask <<= 1) {
        if (nDaBits & nDaMask) {
            for (j = 8, nDT1Mask = 1; j; --j, nDT1Mask <<= 1) {
                if ((uint8_t)nDT & nDT1Mask) {
                    if ((nDtQty = rcp_GDW_GetTable_DT(nAFN, &pDT)) == 0) {
                        continue;
                    }
                    for (k = nDtQty; k; --k, ++pDT) {
                        if (pDT->dtid == nDT) {
                            break;
                        }
                    }
                    if (k == 0) {
                        return 0;
                    }
                    switch (pDT->dttype) {
                        case GW3761_DT_T_DATA_CURVE:
                            tTime = time_Bin2Timet(0, (*ppRecData)[0], (*ppRecData)[1], (*ppRecData)[2], (*ppRecData)[3], (*ppRecData)[4], 1);
                            nInterval = *(*ppRecData + 5);
                            nQty = *(*ppRecData + 6);
                            switch (nInterval) {
                                case 0:
                                case 1:
                                case 2:
                                    nInterval *= 900;
                                case 3:
                                default:
                                    nInterval = 3600;
                                    break;
                            }
                            tTime = (tTime - 1 + nInterval) / nInterval;
                            tTime *= nInterval;
                            gw3761_ConvertData_15(*ppRecData, tTime);
                            buffer_Push(pBuf, *ppRecData, 7);
                            *ppRecData += 7;
                            nReadQty += rcp_GDW_CurveRead(i + 1, pBuf, pDT, nQty, nInterval, tTime);
                            break;
                        case GW3761_DT_T_DATA_DAY:
                        case GW3761_DT_T_DATA_MDAY:
                            buffer_Push(pBuf, *ppRecData, 3);
                            gw3761_FreezeName(strFile, bcd2bin8((*ppRecData)[2]), bcd2bin8((*ppRecData)[1]), bcd2bin8((*ppRecData)[0]), 0, pDT->dttype);
                            *ppRecData += 3;
                            nReadQty += rcp_GDW_FreezeRead(strFile, i + 1, pDT->dtid, pBuf);
                            break;
                        case GW3761_DT_T_DATA_MONTH:
                            buffer_Push(pBuf, *ppRecData, 2);
                            gw3761_FreezeName(strFile, bcd2bin8((*ppRecData)[1]), bcd2bin8((*ppRecData)[0]), 0, 0, pDT->dttype);
                            *ppRecData += 2;
                            nReadQty += rcp_GDW_FreezeRead(strFile, i + 1, pDT->dtid, pBuf);
                        default:
                            break;
                    }
                }
            }
        }
    }
    return nReadQty;
}


