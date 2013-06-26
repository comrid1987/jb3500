#ifndef __QUEUE_H__
#define __QUEUE_H__
#include <lib/buffer.h>


#define QUE_EVT_BUF_MASK            BITMASK(7)

#define QUE_EVT_KEYBOARD            0x00
#define QUE_EVT_PULSE               0x01

#define QUE_EVT_GDFTS_TRANS         (QUE_EVT_BUF_MASK | 0x00)
#define QUE_EVT_GDFTS_RESPOND       (QUE_EVT_BUF_MASK | 0x01)

#define QUE_EVT_USER_EVT            BITMASK(5)
#define QUE_EVT_USER_DATA           (QUE_EVT_BUF_MASK | BITMASK(6))


typedef struct {
    uint8_t     ste;
    uint8_t     evt;
    uint16_t    tmo;
    void *      dev;
    union {
        buf     b;
        uint_t  val;
    }data[1];
}t_os_que, *os_que;


//queue operate
void os_que_Release(os_que que);
void os_que_Init(void);
sys_res os_que_Send(uint_t nEvt, void *pDev, void *pData, uint_t nLen, int nTmo);
sys_res os_que_IsrSend(uint_t nEvt, void *pDev, void *pData, uint_t nLen, int nTmo);
os_que os_que_Wait(uint_t nEvt, void *pDev, int nTmo);
void os_que_Idle(void);


#endif

