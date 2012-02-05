#ifndef __LIB_MEMORY_H__
#define __LIB_MEMORY_H__



#ifdef __cplusplus
extern "C" {
#endif








//-------------------------------------------------------------------------
//Memory Management
//-------------------------------------------------------------------------
#if MEMORY_DEBUG_ENABLE
void *_mem_Malloc(uint_t nSize, const char *fn, const int line);
void *_mem_Realloc(void *pOld, uint_t nSize, const char *fn, const int line);
void _mem_Free(void *p, const char *fn, const int line);

#define mem_Malloc(s)			_mem_Malloc(s, __FUNCTION__, __LINE__)
#define mem_Realloc(p, s)		_mem_Realloc(p, s, __FUNCTION__, __LINE__)
#define mem_Free(p)				_mem_Free(p, __FUNCTION__, __LINE__)

#else
void *mem_Malloc(uint_t nSize);
void *mem_Realloc(void *p, uint_t nSize);
void mem_Free(void *p);

#endif



#ifdef __cplusplus
}
#endif


#endif

