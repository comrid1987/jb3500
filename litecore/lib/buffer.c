#include <string.h>


//Private Defines
#define BUF_DEBUG_ENABLE	0
#define BUF_LOCK_ENABLE		1

#define BUF_BLK_SIZE		(1 << 6)
#define BUF_BLK_MASK		(BUF_BLK_SIZE - 1)

#if BUF_DEBUG_ENABLE
#define buf_trace			rt_kprintf
#else
#define buf_trace(...)
#endif


//Private Variables
#if BUF_LOCK_ENABLE
static os_sem buf_sem;
#endif


//Private Macros
#if BUF_LOCK_ENABLE
#define buf_Lock()			rt_sem_take(&buf_sem, RT_WAITING_FOREVER)
#define buf_Unlock()		rt_sem_release(&buf_sem)
#else
#define buf_Lock()
#define buf_Unlock()
#endif




//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
void buf_Init()
{

#if BUF_LOCK_ENABLE
	rt_sem_init(&buf_sem, "buffer", 1, RT_IPC_FLAG_FIFO);
#endif
}


//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
sys_res buf_Push(buf b, const void *pData, uint_t nLen)
{
	sys_res res = SYS_R_OK;
	uint_t nNew, nAlloc;

	buf_Lock();
	nNew = b->len + nLen;
	nAlloc = (nNew + BUF_BLK_MASK) & ~BUF_BLK_MASK;
	if (((b->len + BUF_BLK_MASK) & ~BUF_BLK_MASK) != nAlloc) {
		b->p = mem_Realloc(b->p, nAlloc);
		if (b->p == NULL)
			res = SYS_R_ERR;
	}
	if (res == SYS_R_OK) {
		buf_trace("buf push: new=%d alloc=%d b->p=%08x b->len=%d len=%d\n", nNew, nAlloc, b->p, b->len, nLen);
		memcpy(b->p + b->len, pData, nLen);
		b->len = nNew;
	}
	buf_Unlock();
	return res;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
sys_res buf_PushData(buf b, uint_t nData, uint_t nLen)
{

	return buf_Push(b, &nData, nLen);
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
sys_res buf_Fill(buf b, uint_t nVal, uint_t nLen)
{
	sys_res res = SYS_R_ERR;
	uint_t nFill;
	uint8_t aBuf[64];

	memset(aBuf, nVal, sizeof(aBuf));
 	for (; nLen; nLen -= nFill) {
		nFill = MIN(sizeof(aBuf), nLen);
		res = buf_Push(b, aBuf, nFill);
		if (res != SYS_R_OK)
			break;
	}
	return res;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
sys_res buf_Cut(buf b, uint_t nOffset, uint_t nLen)
{
	uint_t nAlloc, nNew;
	
	buf_Lock();
	if (nLen > b->len)
		nNew = nOffset;
	else
		nNew = b->len - nLen;
	if (nNew == 0) {
		mem_Free(b->p);
		b->p = NULL;
	} else {
		nAlloc = (nNew + BUF_BLK_MASK) & ~BUF_BLK_MASK;
		memmove(b->p + nOffset, b->p + nOffset + nLen, nNew - nOffset);
		if (((b->len + BUF_BLK_MASK) & ~BUF_BLK_MASK) != nAlloc) {
			b->p = mem_Realloc(b->p, nAlloc);
			if (b->p == NULL) {
				buf_Unlock();
				return SYS_R_ERR;
			}
		}
	}
	buf_trace("buf cut: new=%d alloc=%d b->p=%08x b->len=%d len=%d\n", nNew, nAlloc, b->p, b->len, nLen);
	b->len = nNew;
	buf_Unlock();
	return SYS_R_OK;
}


//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
void buf_Release(buf b)
{

	if (b->p == NULL)
		return;
	buf_Lock();
	mem_Free(b->p);
	buf_trace("buf release: b->p=%08x b->len=%d\n", b->p, b->len);
	b->p = NULL;
	b->len = 0;
	buf_Unlock();
}

