
//-------------------------------------------------------------------------
//转换日期时间为time_t
//-------------------------------------------------------------------------
time_t bin2timet(uint_t nSec, uint_t nMin, uint_t nHour, uint_t nDay, uint_t nMonth, uint_t nYear, uint_t nIsBcd)
{
	struct tm tmTime;

	if (nIsBcd) {
		nSec = bcd2bin8(nSec);
		nMin = bcd2bin8(nMin);
		nHour = bcd2bin8(nHour);
		nDay = bcd2bin8(nDay);
		nMonth = bcd2bin8(nMonth);
		nYear = bcd2bin8(nYear);
	}
	tmTime.tm_sec = nSec;
	tmTime.tm_min = nMin;
	tmTime.tm_hour = nHour;
	tmTime.tm_mday = nDay;
	tmTime.tm_mon = nMonth - 1;
	tmTime.tm_year = nYear + 100;
	return mktime(&tmTime);
}

time_t array2timet(uint8_t *p, uint_t nIsBcd)
{
	struct tm tmTime;

	if (nIsBcd) {
		tmTime.tm_sec = bcd2bin8(*p++);
		tmTime.tm_min = bcd2bin8(*p++);
		tmTime.tm_hour = bcd2bin8(*p++);
		tmTime.tm_mday = bcd2bin8(*p++);
		tmTime.tm_mon = bcd2bin8(*p++) - 1;
		tmTime.tm_year = bcd2bin8(*p) + 100;
	} else {
		tmTime.tm_sec = *p++;
		tmTime.tm_min = *p++;
		tmTime.tm_hour = *p++;
		tmTime.tm_mday = *p++;
		tmTime.tm_mon = *p++ - 1;
		tmTime.tm_year = *p + 100;
	}
	return mktime(&tmTime);
}

int timet2array(time_t tTime, uint8_t *p, uint_t nIsBcd)
{
	struct tm tmTime;

	localtime_r(&tTime, &tmTime);
	if (nIsBcd) {
		*p++ = bin2bcd8(tmTime.tm_sec);
		*p++ = bin2bcd8(tmTime.tm_min);
		*p++ = bin2bcd8(tmTime.tm_hour);
		*p++ = bin2bcd8(tmTime.tm_mday);
		*p++ = bin2bcd8(tmTime.tm_mon + 1);
		*p = bin2bcd8(tmTime.tm_year - 100);
	} else {
		*p++ = tmTime.tm_sec;
		*p++ = tmTime.tm_min;
		*p++ = tmTime.tm_hour;
		*p++ = tmTime.tm_mday;
		*p++ = tmTime.tm_mon + 1;
		*p = tmTime.tm_year - 100;
	}
	return 1;
}

void day4timet(time_t tTime, int nDay, uint8_t *p, uint_t nIsBcd)
{
	uint8_t aTime[6];

	timet2array(tTime + nDay * (60 * 60 * 24), aTime, nIsBcd);
	memcpy(p, &aTime[3], 3);
}

void monthprev(time_t tTime, uint8_t *p, uint_t nIsBcd)
{
	uint8_t aTime[6];

	timet2array(tTime, aTime, nIsBcd);
	if (aTime[4] <= 1) {
		if (nIsBcd) {
			aTime[4] = 0x12;
			aTime[5] = bin2bcd8(bcd2bin8(aTime[1]) - 1);
		} else {
			aTime[4] = 12;
			aTime[5] -= 1;
		}
	} else {
		if (nIsBcd)
			aTime[4] = bin2bcd8(bcd2bin8(aTime[0]) - 1);
		else
			aTime[4] -= 1;
	}
	memcpy(p, &aTime[4], 2);
}




