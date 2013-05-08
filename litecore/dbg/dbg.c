#if 0

//这个用的时候必须要带参数，不然报错。。
#define PDEBUG(fmt, args...) fprintf(stderr, fmt, args##)

//这个可带可不带。。
#define LOGSTRINGS(fm, ...) printf(fm, ##__VA_ARGS__)
#endif

//External Consts
const char dbg_header[2] = {'\r', '\n'};



//External Functions
void dbg_printf(const char *fmt, ...)
{
	va_list args;
	char str[DBG_BUF_SIZE];

	va_start(args, fmt);
	vsnprintf(str, sizeof(str), fmt, args);
	va_end(args);

	dbg_trace(str);
}


