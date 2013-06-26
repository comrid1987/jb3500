#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <litecore.h>
#include <drivers/tdk6515.h>
#include "acm.h"
#include "alarm.h"


//Private Defines


//Pirvate Structs


//Pivate Consts



//Private Variables
static t_tdk6515 acm_xTdk6515;




//Public Variables
t_acm_rtdata acm_rtd;
t_acm_xbdata acm_uxb[3];
t_acm_xbdata acm_ixb[3];



//Internal Functions
static uint_t stat_RateGet(time_t tTime)
{
    uint8_t aBuf[52];
    uint_t nRate;

    nRate = ((tTime - 1) / (30 * 60)) % 48;
    icp_ParaRead(4, 21, TERMINAL, aBuf, 49);
    nRate = aBuf[nRate];
    if (nRate >= ECL_RATE_QTY)
        nRate = 2;
    return nRate;
}




//External Functions
void acm_Init()
{
    t_tdk6515 *p = &acm_xTdk6515;

    memset(&acm_rtd, 0, sizeof(acm_rtd));
    memset(&acm_uxb, 0, sizeof(acm_uxb));
    memset(&acm_ixb, 0, sizeof(acm_ixb));
    p->ste = 0;
    p->uart = uart_Get(TDK6515_COMID, OS_TMO_FOREVER);
    uart_Config(p->uart, 9600, UART_PARI_EVEN, UART_DATA_8D, UART_STOP_1D);
    tdk6515_Reset(p);
    p->ste = 1;
}


void acm_Balance(t_acm_rtdata *pa)
{
    float fTemp, fMax;
    uint_t i;

    //不平衡度
    fTemp = (pa->u[0] + pa->u[1] + pa->u[2]) / 3;
    fMax = pa->u[0];
    for (i = 1; i < 3; i++) {
        if (fMax < pa->u[i])
            fMax = pa->u[i];
    }
    pa->ub = (fMax - fTemp) / fTemp;
    fTemp = (pa->i[0] + pa->i[1] + pa->i[2]) / 3;
    fMax = pa->i[0];
    for (i = 1; i < 3; i++) {
        if (fMax < pa->i[i])
            fMax = pa->i[i];
    }
    pa->ib = (fMax - fTemp) / fTemp;
    fMax = pa->ui[1];
    fTemp = pa->ui[1];
    for (i = 2; i < 4; i++) {
        if (fMax < pa->ui[i])
            fMax = pa->ui[i];
        if (fTemp > pa->ui[i])
            fTemp = pa->ui[i];
    }
    pa->uib = (fMax - fTemp) / pa->ui[0];
}


void acm_JLRead()
{
    t_tdk6515 *p = &acm_xTdk6515;
    t_acm_rtdata *pa = &acm_rtd;
    t_tdk6515_rtdata *pT;
    uint8_t *pTemp;
    uint_t i, nCRC;
    buf b = {0};

    if (tdk6515_IsJLReady() == SYS_R_OK) {
        tdk6515_CmdSend(p, 0, 0x0000, 48);
        uart_RecLength(p->uart, b, 48 * 4 + 2, 2000);
        nCRC = crc16(b->p, 48 * 4);
        if (memcmp(&nCRC, &b->p[48 * 4], 2) == 0) {
            buf_Unpush(b, 2);
            for (pTemp = b->p; pTemp < &b->p[b->len]; pTemp += 4)
                reverse(pTemp, 4);
            pT = (t_tdk6515_rtdata *)(b->p);
            //频率
            pa->freq = pT->freq / 100.0f;
            //电压角度
            pa->ua[0] = 0;
            pa->ua[1] = pT->caangle / 10.0f;
            pa->ua[2] = pT->cbangle / 10.0f;
            for (i = 0; i < 3; i++) {
                //电压
                pa->u[i] = pT->u[i];
                //电流角度
                pa->ia[i] = pT->viangle[i + 1] + pa->ua[i];
                //角度校正
                if (pa->ua[i] >= 360)
                    pa->ua[i] -= 360;
                else if (pa->ua[i] <= -360)
                    pa->ua[i] += 360;
                if (pa->ua[i] < 0)
                    pa->ua[i] += 360;
                if (pa->ia[i] >= 360)
                    pa->ia[i] -= 360;
                else if (pa->ia[i] <= -360)
                    pa->ia[i] += 360;
                if (pa->ia[i] < 0)
                    pa->ia[i] += 360;
            }
            for (i = 0; i < 4; i++) {
                //电流
                pa->i[i] = pT->i[i];
                //功率
                pa->pp[i] = pT->p[i] / 1000.0f;
                pa->pq[i] = pT->q[i] / 1000.0f;
                pa->ui[i] = pT->ui[i] / 1000.0f;
                //功率因数
                pa->cos[i] = pT->cos[i];
            }
            acm_Balance(pa);
        }
        buf_Release(b);
    }        
}

void acm_XBRead()
{
    t_tdk6515 *p = &acm_xTdk6515;
    t_acm_xbdata *pD;
    t_tdk6515_xbdata *pT;
    uint8_t *pTemp;
    uint_t i, j, nCRC;
    buf b = {0};

    if (tdk6515_IsXBReady() == SYS_R_OK)      {
        tdk6515_CmdSend(p, 0, 0x011C, 24);      //起始地址为0x011C 长度24
        uart_RecLength(p->uart, b, 24 * 4 + 2, 2000);   //获得24*4+2个字节
        nCRC = crc16(b->p, 24 * 4);             //计算24*4个字节crc16值 从0x0011C开始
        if (memcmp(&nCRC, &b->p[24 * 4], 2) == 0) {
            buf_Unpush(b, 2);
            for (pTemp = b->p; pTemp < &b->p[b->len]; pTemp += 4)
                reverse(pTemp, 4);
            pT = (t_tdk6515_xbdata *)b->p;
            i = pT->curxbno;
            if (i < 6) {
                if (i & 1) {
                    switch (i) {
                    case 3:
                        pD = &acm_uxb[1];
                        break;
                    case 5:
                        pD = &acm_uxb[2];
                        break;
                    default:
                        pD = &acm_uxb[0];
                        break;
                    }
                } else
                    pD = &acm_ixb[i >> 1];
                pD->base = pT->xbbase;
                for (j = 0; j <= 10; j++)
                    pD->xbrate[j] = pT->xbrate[j * 2];
            }
        }
        buf_Release(b);
    }
}

void acm_MinSave(const uint8_t *pTime)
{
    t_acm_rtdata *pD = &acm_rtd;
    uint_t i;
    uint8_t *pTemp;
    t_data_min xData = {0};

    pTemp = xData.data;
    xData.time = rtc_GetTimet();
    for (i = 0; i < 3; i++)
        gw3761_ConvertData_07(&pTemp[ACM_MSAVE_VOL + i * 2], FLOAT2FIX(pD->u[i]));
    for (i = 0; i < 4; i++) {
        gw3761_ConvertData_25(&pTemp[ACM_MSAVE_CUR + i * 3], FLOAT2FIX(pD->i[i]), 1);
        gw3761_ConvertData_09(&pTemp[ACM_MSAVE_PP + i * 3], FLOAT2FIX(pD->pp[i]), 1);
        gw3761_ConvertData_09(&pTemp[ACM_MSAVE_PQ + i * 3], FLOAT2FIX(pD->pq[i]), 1);
        gw3761_ConvertData_05_Percent(&pTemp[ACM_MSAVE_COS + i * 2], FLOAT2FIX(pD->cos[i]), 1);
    }
    data_MinWrite(&pTime[1], &xData);
}

void acm_QuarterSave(const uint8_t *pTime)
{
    t_acm_rtdata *pD = &acm_rtd;
    t_acm_xbdata *pX;
    uint_t i, j;
    uint8_t *pTemp, aBuf[4];
    t_data_quarter xData = {0};

    pTemp = xData.data;
    xData.time = rtc_GetTimet();
    for (i = 0; i < 6; i++) {
        if (i < 3)
            pX = &acm_uxb[i];
        else
            pX = &acm_ixb[i - 3];
        for (j = 0; j < 11; j++) {
            gw3761_ConvertData_05(aBuf, FLOAT2FIX(pX->xbrate[j]), 0);
            memcpy(pTemp, aBuf, 2);
            pTemp += 2;
        }
    }
    for (i = 0; i < 3; i++) {
        gw3761_ConvertData_05(aBuf, FLOAT2FIX(pD->ua[i]), 0);
        memcpy(pTemp, aBuf, 2);
        pTemp += 2;
    }
    for (i = 0; i < 3; i++) {
        gw3761_ConvertData_05(aBuf, FLOAT2FIX(pD->ia[i]), 0);
        memcpy(pTemp, aBuf, 2);
        pTemp += 2;
    }
    gw3761_ConvertData_06(aBuf, FLOAT2FIX(pD->freq), 0);
    memcpy(pTemp, aBuf, 2);
    data_QuarterWrite(&pTime[1], &xData);
}



int acm_IsReady()
{

    return acm_xTdk6515.ste;
}


int acm_Rtd4timet(t_acm_rtdata *p, time_t tTime)
{
    uint8_t *pTemp, aTime[6];
    char str[10];
    uint_t i;
    t_data_min xMin;
    
    timet2array(tTime, aTime, 1);
    data_MinRead(&aTime[1], &xMin);
    if (xMin.time == GW3761_DATA_INVALID)
        return 0;
    for (i = 0; i < 3; i++) {
        pTemp = &xMin.data[ACM_MSAVE_VOL + i * 2];
        sprintf(str, "%02X%1X.%1X", pTemp[1], pTemp[0] >> 4, pTemp[0] & 0x0F);
        p->u[i] = atof(str);
    }
    for (i = 0; i < 4; i++) {
        pTemp = &xMin.data[ACM_MSAVE_CUR + i * 3];
        sprintf(str, "%02X%1X.%1X%02X", pTemp[2] & 0x7F, pTemp[1] >> 4, pTemp[1] & 0x0F, pTemp[0]);
        p->i[i] = atof(str);
        if (pTemp[2] & BITMASK(7))
            p->i[i] = 0.0f - p->i[i];
        pTemp = &xMin.data[ACM_MSAVE_PP + i * 3];
        sprintf(str, "%02X.%02X%02X", pTemp[2] & 0x7F, pTemp[1], pTemp[0]);
        p->pp[i] = atof(str);
        if (pTemp[2] & BITMASK(7))
            p->pp[i] = 0.0f - p->pp[i];
        pTemp = &xMin.data[ACM_MSAVE_PQ + i * 3];
        sprintf(str, "%02X.%02X%02X", pTemp[2] & 0x7F, pTemp[1], pTemp[0]);
        p->pq[i] = atof(str);
        if (pTemp[2] & BITMASK(7))
            p->pq[i] = 0.0f - p->pq[i];
        pTemp = &xMin.data[ACM_MSAVE_COS + i * 2];
        sprintf(str, "%1X.%1X%02X", (pTemp[1] >> 4) & 0x07, pTemp[1] & 0xF, pTemp[0]);
        p->cos[i] = atof(str);
        if (pTemp[1] & BITMASK(7))
            p->cos[i] = 0.0f - p->cos[i];
        //计算视在功率
        p->ui[i] = sqrtf(p->pp[i] * p->pp[i] + p->pq[i] * p->pq[i]);
    }
    //不平衡度
    acm_Balance(p);
    return 1;
}



void stat_Handler(p_stat ps, t_acm_rtdata *pa, t_afn04_f26 *pF26, t_afn04_f28 *pF28, time_t tTime)
{
    uint_t i;
    uint32_t nData;
    float fData, fLow, fUp, fUnder, fOver;

    ps->run += 1;
    //电压
    fLow = (float)bcd2bin16(pF26->ulow) / 10.0f;
    fUnder = (float)bcd2bin16(pF26->uunder) / 10.0f;
    fUp = (float)bcd2bin16(pF26->uup) / 10.0f;
    fOver = (float)bcd2bin16(pF26->uover) / 10.0f;
    nData = 1;
    for (i = 0; i < 3; i++) {
        fData = pa->u[i];
        ps->usum[i] += fData;
        if (fData < fLow) {
            ps->ulow[i] += 1;
            nData = 0;
        }
        if (fData > fUp) {
            ps->uup[i] += 1;
            nData = 0;
        }
        if (fData < fUnder) {
            ps->uunder[i] += 1;
            nData = 0;
        }
        if (fData > fOver) {
            ps->uover[i] += 1;
            nData = 0;
        }
        if (ps->tumin[i] == 0)
            ps->umin[i] = fData;
        if (fData <= ps->umin[i]) {
            ps->umin[i] = fData;
            ps->tumin[i] = tTime;
        }
        if (fData >= ps->umax[i]) {
            ps->umax[i] = fData;
            ps->tumax[i] = tTime;
        }
    }       
    if (nData)
        ps->uok += 1;
    //电流
    nData = 0;
    memcpy(&nData, pF26->iup, 3);
    fUp = (float)bcd2bin32(nData) / 1000.0f;
    memcpy(&nData, pF26->iover, 3);
    fOver = (float)bcd2bin32(nData) / 1000.0f;
    for (i = 0; i < 3; i++) {
        fData = pa->i[i];
        if (fData > fUp)
            ps->iup[i] += 1;
        if (fData > fOver)
            ps->iover[i] += 1;
        if (fData >= ps->imax[i]) {
            ps->imax[i] = fData;
            ps->timax[i] = tTime;
        }
    }
    fData = pa->i[3];
    if (fData > fUp)
        ps->iup[3] += 1;
    if (fData >= ps->imax[3]) {
        ps->imax[3] = fData;
        ps->timax[3] = tTime;
    }
    //不平衡
    fUp = (float)bcd2bin16(pF26->ubalance) / 1000.0f;
    fData = pa->ub;
    if (fData > fUp)
        ps->ubalance += 1;
    if (fData >= ps->ubmax) {
        ps->ubmax = fData;
        ps->tubmax = tTime;
    }
    fUp = (float)bcd2bin16(pF26->ibalance) / 1000.0f;
    fData = pa->ib;
    if (fData > fUp)
        ps->ibalance += 1;
    if (fData >= ps->ibmax) {
        ps->ibmax = fData;
        ps->tibmax = tTime;
    }
    //功率
    memcpy(&nData, pF26->uiup, 3);
    fUp = (float)bcd2bin32(nData) / 10000.0f;
    memcpy(&nData, pF26->uiover, 3);
    fOver = (float)bcd2bin32(nData) / 10000.0f;
    for (i = 0; i < 4; i++) {
        fData = pa->ui[i];
        //总视在功率越限
        if (i == 3) {
            if (fData > fUp)
                ps->uiup += 1;
            if (fData > fOver)
                ps->uiover+= 1;
        }
        //功率为零时间
        if (fData < 0.1f)
            ps->p0[i] += 1;
        //视在功率最大值
        if (fData >= ps->uimax[i]) {
            ps->uimax[i] = fData;
            ps->tuimax[i] = tTime;
        }
        fData = pa->pp[i];
        //有功功率最大值
        if (fData >= ps->pmax[i]) {
            ps->pmax[i] = fData;
            ps->tpmax[i] = tTime;
        }
    }
    //视在不平衡度平均
    ps->uibsum[stat_RateGet(tTime)] += pa->uib;
    //功率因数平均值
    for (i = 0; i < 4; i++) {
        ps->cossum[i] += pa->cos[i];
    }
    //功率因数分段统计
    fLow = (float)bcd2bin16(pF28->low) / 1000.0f;
    fUp = (float)bcd2bin16(pF28->up) / 1000.0f;
    fData = pa->cos[3];
    if (fData < fLow) {
        ps->cos[0] += 1;
    } else {
        if (fData > fUp)
            ps->cos[2] += 1;
        else
            ps->cos[1] += 1;
    }
}


void tsk_Acm(void *args)
{
    time_t tTime;
    uint_t nCnt, nTemp;
    uint8_t nMin, aTime[6];
    t_afn04_f26 xF26;
    t_data_min xMin;
    
    acm_Init();
    nMin = 0xFF;
    for (nCnt = 0; ; os_thd_Slp1Tick()) {
        //秒count
        if (tTime == rtc_GetTimet())
            continue;
        tTime = rtc_GetTimet();
        timet2array(tTime, aTime, 1);
        if ((nCnt & 0x3F) == 0)
            icp_ParaRead(4, 26, TERMINAL, &xF26, sizeof(t_afn04_f26));
        if ((nCnt & 0x0F) == 0)
            acm_XBRead();
        if ((nCnt & 0x1F) == 0) {
            acm_JLRead();
            evt_Terminal(&xF26);
        }
        nCnt += 1;
        //遥信
        evt_YXRead();
        //分钟
        if (nMin != aTime[1]) {
            nMin = aTime[1];
            evt_RunTimeWrite(tTime);
            acm_MinSave(aTime);
            nTemp = bcd2bin8(aTime[1]);
            if ((nTemp % 15) == 0)
                acm_QuarterSave(aTime);
#if 1
            //补冻结
            timet2array(tTime - 60, aTime, 1);
            data_MinRead(aTime, &xMin);
            if (xMin.time == GW3761_DATA_INVALID) {
                acm_MinSave(aTime);
                nTemp = bcd2bin8(aTime[1]);
                if ((nTemp % 15) == 0)
                    acm_QuarterSave(aTime);
            }
#endif
        }
    }
}


