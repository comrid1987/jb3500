#include <string.h>


//Private Defines
#define DQUE_LOCK_ENABLE		(1 & OS_TYPE)

#define DQUE_INVALID_FLAG		0x7F


//Private Variables
#if DQUE_LOCK_ENABLE
static os_sem dque_sem;
#endif

//Private Macros
#if DQUE_LOCK_ENABLE
#define dque_Lock()				rt_sem_take(&dque_sem, RT_WAITING_FOREVER)
#define dque_Unlock()			rt_sem_release(&dque_sem)
#else
#define dque_Lock()
#define dque_Unlock()
#endif


//Internal Functions
//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
static void dque_Release(p_dque p)
{

#if DQUE_DYNCMEM_ENABLE
	mem_Free(p->p);
#endif
	bzero(p, sizeof(t_dque));
	p->chl = DQUE_INVALID_FLAG;
}




//External Functions
//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
void dque_Init(dque pQue)
{
	p_dque pEnd, p = pQue->list;

#if DQUE_LOCK_ENABLE
	rt_sem_init(&dque_sem, "dque", 1, RT_IPC_FLAG_FIFO);
#endif
	for (pEnd = p + pQue->qty; p < pEnd; p++) {
		dque_Release(p);
	}
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
int dque_Pop(dque pQue, uint_t nChl, buf b)
{
	uint_t i, nNext = 0, nOldLen = b->len;
	int nLen;
	p_dque p = pQue->list;

	dque_Lock();
	for (i = 0; i < pQue->qty; i++, p++) {
		if (p->first && (p->chl == nChl)) {
			for (nNext = i + 1; nNext; ) {
				p = &pQue->list[nNext - 1];
				while ((nLen = p->in - p->out) > 0) {
					buf_Push(b, &p->p[p->out], nLen);
					p->out += nLen;
				}
				nNext = p->next;
				if (p->out >= DQUE_BLK_SIZE)
					dque_Release(p);
			}
			p->first = 1;
			if (p->out >= p->in) {
				p->in = 0;
				p->out = 0;
			}
			break;
		}
	}
	dque_Unlock();
	return (b->len - nOldLen);
}


//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
int dque_PopChar(dque pQue, uint_t nChl)
{
	int nData = -1;
	p_dque p = pQue->list, pEnd;
	
	dque_Lock();
	for (pEnd = p + pQue->qty; p < pEnd; p++) {
		if (p->first && (p->chl == nChl) && (p->in > p->out)) {
			nData = p->p[p->out++];
			if (p->next) {
				if (p->out >= DQUE_BLK_SIZE) {
					pQue->list[p->next - 1].first = 1;
					dque_Release(p);
				}
			} else if (p->out >= p->in) {
				p->in = 0;
				p->out = 0;
			}
			break;
		}
	}
	dque_Unlock();
	return nData;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
int dque_Push(dque pQue, uint_t nChl, const void *pBuf, uint_t nLen)
{
	p_dque p;
	uint8_t *pData = (uint8_t *)pBuf;
	uint_t i, nMin, nNeedNew = 0;

	dque_Lock();
	for (p = pQue->list, i = 0; i < pQue->qty; i++, p++) {
		//ÕÒÒÑ¾­Ê¹ÓÃµÄ¿é
		if ((p->next == 0) && (p->chl == nChl)) {
			nMin = MIN(DQUE_BLK_SIZE - p->in, nLen);
			memcpy(&p->p[p->in], pData, nMin);
			pData += nMin;
			nLen -= nMin;
			p->in += nMin;
			//¿éÂú,ÉêÇëÐÂ¿é
			if (p->in >= DQUE_BLK_SIZE)
				nNeedNew = i + 1;
			break;
		}
	}
	for (p = pQue->list, i = 0; (nNeedNew | nLen) && (i < pQue->qty); i++, p++) {
		if ((p->chl == DQUE_INVALID_FLAG) && (p->in == 0)) {
			//·ÖÅä¿Õ¿é
			p->chl = nChl;
#if DQUE_DYNCMEM_ENABLE
			p->p = mem_Malloc(DQUE_BLK_SIZE);
#endif
			nMin = MIN(DQUE_BLK_SIZE, nLen);
			memcpy(p->p, pData, nMin);
			pData += nMin;
			nLen -= nMin;
			p->in += nMin;
			if (nNeedNew) {
				pQue->list[nNeedNew - 1].next = i + 1;
				nNeedNew = 0;
			} else
				p->first = 1;
			//¿éÂú,ÉêÇëÐÂ¿é
			if (p->in >= DQUE_BLK_SIZE)
				nNeedNew = i + 1;
		}
	}
	dque_Unlock();
	if (nLen == 0)
		return 1;
	return 0;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
int dque_IsNotEmpty(dque pQue, uint_t nChl)
{
	p_dque p = pQue->list, pEnd;

	for (pEnd = p + pQue->qty; p < pEnd; p++) {
		if ((p->chl == nChl) && (p->in > p->out))
			return 1;
	}
	return 0;
}


//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
void dque_Clear(dque pQue, uint_t nChl)
{
	p_dque p = pQue->list, pEnd;

	dque_Lock();
	for (pEnd = p + pQue->qty; p < pEnd; p++) {
		if ((p->chl == nChl) && p->in)
			dque_Release(p);
	}
	dque_Unlock();
}

