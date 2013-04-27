#ifndef __DBG_H__
#define __DBG_H__

#ifdef __cplusplus
extern "C" {
#endif




//External Functions
#if 1
void dbg_trace(const char *fmt, ...);
#else
void dbg_trace(const char *str);
#endif

#ifdef __cplusplus
}
#endif

#endif


