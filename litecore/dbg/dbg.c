#define PDEBUG(fmt, args...) fprintf(stderr, fmt, args##) 这个用的时候必须要带参数，不然报错。。
#define LOGSTRINGS(fm, ...) printf(fm, ##__VA_ARGS__)  这个可带可不带。。

