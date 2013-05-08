#ifndef __DBG_H__
#define __DBG_H__

#ifdef __cplusplus
extern "C" {
#endif


//External Defines
#define DBG_BUF_SIZE			200



//External Consts
extern const char dbg_header[2];



//External Functions
void dbg_trace(const char *str);
void dbg_printf(const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif


