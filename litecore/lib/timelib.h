#ifndef __LIB_TIME_H__
#define __LIB_TIME_H__


#ifdef __cplusplus
extern "C" {
#endif

#include <time.h>


//Pulic typedef









//External Functions
time_t bin2timet(uint_t nSec, uint_t nMin, uint_t nHour, uint_t nDay, uint_t nMonth, uint_t nYear, uint_t nIsBcd);
time_t array2timet(uint8_t *p, uint_t nIsBcd);
int timet2array(time_t tTime, uint8_t *p, uint_t nIsBcd);
void day4timet(time_t tTime, int nDay, uint8_t *p, uint_t nIsBcd);
void monthprev(time_t tTime, int nMon, uint8_t *p, uint_t nIsBcd);


#ifdef __cplusplus
}
#endif

#endif

