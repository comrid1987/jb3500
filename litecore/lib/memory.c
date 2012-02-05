

//Private Defines
#ifndef __MICROLIB
#pragma import (__use_two_region_memory)
#pragma import (__use_realtime_heap)
#endif

#define MEMORY_LOCK_ENABLE		0


//Private Macros
#if MEMORY_LOCK_ENABLE
#define mem_Lock()				os_thd_Lock()
#define mem_Unlock()			os_thd_Unlock()
#else
#define mem_Lock()
#define mem_Unlock()
#endif


#if MEMORY_DEBUG_ENABLE
extern void list_memdebug(int nStart, int nEnd);

typedef struct {
	uint16_t	size;
	uint16_t	linemalloc;
	void *	p;
	const char *fnmalloc;
}t_mem_debug;


static int mem_nCnt = 0;
static t_mem_debug mem_debug[64];


static void mem_DebugMalloc(void *p, uint_t nSize, const char *fn, const int line)
{
	t_mem_debug *pDebug;

	if (p == NULL)
		return;
	mem_Lock();
	for (pDebug = &mem_debug[0]; pDebug < &mem_debug[mem_nCnt]; pDebug++)
		if (pDebug->p == p) {
			list_memdebug(0, ARR_SIZE(mem_debug));
			rt_kprintf("memory error:%X no malloc to free\n", (uint_t)p);
		}
	if (mem_nCnt < ARR_SIZE(mem_debug)) {
		pDebug = &mem_debug[mem_nCnt];
		pDebug->p = p;
		pDebug->size = nSize;
		pDebug->linemalloc = line;
		pDebug->fnmalloc = fn;
	}
	mem_nCnt += 1;
	RT_ASSERT(mem_nCnt < ARR_SIZE(mem_debug));
	mem_Unlock();
}

static void mem_DebugFree(void *p, const char *fn, const int line)
{
	t_mem_debug *pDebug;

	if (p == NULL)
		return;
	mem_Lock();
	for (pDebug = &mem_debug[0]; pDebug < &mem_debug[mem_nCnt]; pDebug++)
		if (pDebug->p == p) {
			rt_memmove(pDebug, pDebug + 1, (uint_t)&mem_debug[mem_nCnt] - (uint_t)pDebug);
			break;
		}
	if (pDebug == &mem_debug[mem_nCnt]) {
		list_memdebug(0, ARR_SIZE(mem_debug));
		rt_kprintf("memory error:%X no malloc to free\n", (uint_t)p);
	}
	mem_nCnt -= 1;
	RT_ASSERT(mem_nCnt >= 0);
	mem_Unlock();
}

static void mem_DebugRealloc(void *p, void *pOld, const char *fn, const int line)
{
	t_mem_debug *pDebug;

	mem_Lock();
	for (pDebug = &mem_debug[0]; pDebug < &mem_debug[mem_nCnt]; pDebug++)
		if (pDebug->p == pOld) {
			pDebug->p = p;
			pDebug->linemalloc = line;
			pDebug->fnmalloc = fn;
			break;
		}
	if (pDebug == &mem_debug[mem_nCnt]) {
		list_memdebug(0, ARR_SIZE(mem_debug));
		rt_kprintf("memory error:%X no malloc to realloc\n", (uint_t)pOld);
	}
	RT_ASSERT(mem_nCnt >= 0);
	mem_Unlock();
}
#endif


//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
#if MEMORY_DEBUG_ENABLE
void *_mem_Malloc(uint_t nSize, const char *fn, const int line)
#else
void *mem_Malloc(uint_t nSize)
#endif
{
	void *p;

	mem_Lock();
#if MEMORY_DEBUG_ENABLE
	p = _rt_malloc(nSize);
#else
	p = rt_malloc(nSize);
#endif
	mem_Unlock();
#if MEMORY_DEBUG_ENABLE
	mem_DebugMalloc(p, nSize, fn, line);
#endif
	return p;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
#if MEMORY_DEBUG_ENABLE
void *_mem_Realloc(void *pOld, uint_t nSize, const char *fn, const int line)
#else
void *mem_Realloc(void *pOld, uint_t nSize)
#endif
{
	void *p;

	if (pOld == NULL)
#if MEMORY_DEBUG_ENABLE
		return _mem_Malloc(nSize, fn, line);
#else
		return mem_Malloc(nSize);
#endif
	if (nSize == 0) {
#if MEMORY_DEBUG_ENABLE
		_mem_Free(pOld, fn, line);
#else
		mem_Free(pOld);
#endif
		return NULL;
	}
	mem_Lock();
#if MEMORY_DEBUG_ENABLE
	p = _rt_realloc(pOld, nSize);
#else
	p = rt_realloc(pOld, nSize);
#endif
	mem_Unlock();
#if MEMORY_DEBUG_ENABLE
	mem_DebugRealloc(p, pOld, fn, line);
#endif
	return p;
}


//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
#if MEMORY_DEBUG_ENABLE
void _mem_Free(void *p, const char *fn, const int line)
#else
void mem_Free(void *p)
#endif
{

	mem_Lock();
#if MEMORY_DEBUG_ENABLE
	_rt_free(p);
#else
	rt_free(p);
#endif
	mem_Unlock();
#if MEMORY_DEBUG_ENABLE
	mem_DebugFree(p, fn, line);
#endif
}


#if MEMORY_DEBUG_ENABLE
void list_memdebug(int nStart, int nEnd)
{
#ifdef RT_USING_FINSH
	extern void list_mem(void);
#endif
	t_mem_debug *p;
	uint_t nSize = 0;

	os_thd_Lock();
	if (nEnd == 0)
		nEnd = mem_nCnt;
	for (p = &mem_debug[nStart]; p < &mem_debug[nEnd]; p++) {
		rt_kprintf("%2d %8X %4d %s %d\n", p - &mem_debug[0], (uint_t)p->p, p->size, p->fnmalloc, p->linemalloc);
		if (p->size < 12)//MIN_SIZE = 12, defined in mem.c
			nSize += (12 + 0x0c);
		else
			nSize += (p->size + 0x0c);
	}
	rt_kprintf("total size %d\n", nSize);
#ifdef RT_USING_FINSH
	list_mem();
#endif
	os_thd_Unlock();
}
#ifdef RT_USING_FINSH
#include <hi/finsh/finsh.h>
FINSH_FUNCTION_EXPORT(list_memdebug, list memory debug information)
FINSH_VAR_EXPORT(mem_nCnt, finsh_type_int, memory alloced counter)
#endif

#endif

