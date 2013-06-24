#include <string.h>
#include <litecore.h>
#include "meter.h"


enum AFN10_nBaud
{
    nBaud_300,  nBaud_600,
    nBaud_1200, nBaud_2400,
    nBaud_4800, nBaud_7200,
    nBaud_9600, nBaud_19200
};

//External Functions
#if 1
extern t_ecl_task ecl_Task485;
sys_res AFN10_01_Passthrough(buf b, uint_t nBaud, uint_t nTmo)
{
    sys_res res;
    t_ecl_task *p = &ecl_Task485;
    
    res = ecl_485_Wait(nTmo);
    if (res != SYS_R_OK)
        return res;
    chl_rs232_Config(p->chl, nBaud, UART_PARI_EVEN, UART_DATA_8D, UART_STOP_1D);
    chl_Send(p->chl, b->p, b->len);
    buf_Release(b);
    for (nTmo /= OS_TICK_MS; nTmo; nTmo--) {
        chl_RecData(p->chl, b, OS_TICK_MS);
    }
    ecl_485_Release();
    return SYS_R_OK;
}


int gw3761_ResponseTransmit(p_gw3761 p, buf b, u_word2 *pDu, uint8_t **ppData)
{
    sys_res res;
    int nRelay;
    uint_t i, nPort, nBaud, nCtrl, nLen, nFn, nTemp, nMS, Temp_Baud;
    uint8_t *pAdr, *pTemp;
    uint32_t nDI;
    buf bTx = {0};
    
    for (i = 0; i < 8; i++) {
        if ((pDu->word[1] & BITMASK(i)) == 0)
            continue;
        nFn = gw3761_ConvertDt2Fn((pDu->word[1] & 0xFF00) | BITMASK(i));
        switch (nFn) {
        case 1:
            nPort = *(*ppData)++;
            nCtrl = *(*ppData)++;
            nRelay = **ppData;
            *ppData += 2;
            nLen = ((*ppData)[1] << 8) | (*ppData)[0];
            *ppData += 2;
            buf_Push(bTx, *ppData, nLen);
            *ppData += nLen;
            nTemp = nCtrl >> 5;
            if (nRelay & BITMASK(7))
                nMS = (nRelay & 0x7F) * 1000;
            else
                nMS = (nRelay & 0x7F) * 10;
            switch (nTemp) {
            case nBaud_300:     
                Temp_Baud = 300;
                break;
             case nBaud_600:        
                Temp_Baud = 600;
                break;
              case nBaud_1200:      
                Temp_Baud = 1200;
                break;             
                case nBaud_2400:        
                Temp_Baud = 2400;
                break;             
                case nBaud_4800:        
                Temp_Baud = 4800;
                break;             
                case nBaud_7200:        
                Temp_Baud = 7200;
                break;             
                case nBaud_9600:        
                Temp_Baud = 9600;
                break;             
                case nBaud_19200:       
                Temp_Baud = 19200;
                break;          
                default:
                break;  
            }
            res = AFN10_01_Passthrough(bTx, Temp_Baud, nMS);
            if (res == SYS_R_OK) {
                buf_PushData(b, nPort, 1);
                buf_PushData(b, bTx->len, 2);
                buf_Push(b, bTx->p, bTx->len);
            }
            buf_Release(bTx);
            break;
        case 9:
            nPort = *(*ppData)++;
            nRelay = *(*ppData)++;
            pTemp = *ppData;
            if (nRelay == 0xFF)
                nRelay = -1;
            else
                *ppData += nRelay * 6;
            pAdr = *ppData;
            *ppData += 6;
            nCtrl = *(*ppData)++;
            memcpy(&nDI, *ppData, 4);
            *ppData += 4;
            if (nCtrl) {
                //dlt645-07
                dlt645_Packet2Buf(bTx, pAdr, DLT645_CODE_READ07, &nDI, 4);
                nBaud = 2400;
            } else {
                //dlt645-97
                dlt645_Packet2Buf(bTx, pAdr, DLT645_CODE_READ97, &nDI, 2);
                nBaud = 1200;
            }
            buf_PushData(b, nPort, 1);
            buf_Push(b, pAdr, 6);
            if (ecl_485_RealRead(bTx, nBaud, 2000) == SYS_R_OK) {
                buf_PushData(b, 3, 1);
                buf_Push(b, &bTx->p[1], bTx->p[1] + 1);
            } else {
                buf_PushData(b, 0x000002, 3);
            }
            buf_Release(bTx);
            break;
        default:
            break;
        }
    }
    return 0;
}

#else
int gw3761_ResponseTransmit(p_gw3761 p, buf b, u_word2 *pDu, uint8_t **ppData)
{
    sys_res res;
    int nRelay;
    uint_t i, nPort, nBaud, nCtrl, nLen, nFn, nTemp, nMS;
    uint8_t *pAdr, *pTemp;
    uint32_t nDI;
    buf bTx = {0};
    
    for (i = 0; i < 8; i++) {
        if ((pDu->word[1] & BITMASK(i)) == 0)
            continue;
        nFn = gw3761_ConvertDt2Fn((pDu->word[1] & 0xFF00) | BITMASK(i));
        switch (nFn) {
        case 1:
            nPort = *(*ppData)++;
            nCtrl = *(*ppData)++;
            nRelay = **ppData;
            *ppData += 2;
            nLen = ((*ppData)[1] << 8) | (*ppData)[0];
            *ppData += 2;
            pTemp = dlt645_PacketAnalyze(*ppData, nLen);
            buf_Push(bTx, pTemp, nLen - (pTemp - *ppData));
            *ppData += nLen;
            nTemp = nCtrl >> 5;
            if (nRelay & BITMASK(7))
                nMS = (nRelay & 0x7F) * 1000;
            else
                nMS = (nRelay & 0x7F) * 10;
            if (2 == nTemp) {
                res = ecl_485_RealRead(bTx, 1200, nMS);
            } else {
                res = ecl_485_RealRead(bTx, 2400, nMS);
            }
            if (res == SYS_R_OK) {
                buf_PushData(b, nPort, 1);
                buf_PushData(b, bTx->p[1] + (DLT645_HEADER_SIZE + 2), 2);

                buf_Push(b, pTemp, DLT645_HEADER_SIZE - 2);
                byteadd(&bTx->p[2], 0x33, bTx->p[1]);
                buf_Push(b, bTx->p, bTx->p[1] + 4);
            }
            buf_Release(bTx);
            break;
        case 9:
            nPort = *(*ppData)++;
    nRelay = *(*ppData)++;
            pTemp = *ppData;
            if (nRelay == 0xFF)
                nRelay = -1;
            else
                *ppData += nRelay * 6;
            pAdr = *ppData;
            *ppData += 6;
            nCtrl = *(*ppData)++;
            memcpy(&nDI, *ppData, 4);
            *ppData += 4;
            if (nCtrl) {
                //dlt645-07
                dlt645_Packet2Buf(bTx, pAdr, DLT645_CODE_READ07, &nDI, 4);
                nBaud = 2400;
            } else {
                //dlt645-97
                dlt645_Packet2Buf(bTx, pAdr, DLT645_CODE_READ97, &nDI, 2);
                nBaud = 1200;
            }
            buf_PushData(b, nPort, 1);
            buf_Push(b, pAdr, 6);
            if (ecl_485_RealRead(bTx, nBaud, 2000) == SYS_R_OK) {
                buf_PushData(b, 3, 1);
                buf_Push(b, &bTx->p[1], bTx->p[1] + 1);
            } else {
                buf_PushData(b, 0x000002, 3);
            }
            buf_Release(bTx);
            break;
        default:
            break;
        }
    }
    return 0;
}
#endif



