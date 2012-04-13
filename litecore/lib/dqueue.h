#ifndef __DQUEUE_H__
#define __DQUEUE_H__


#ifdef __cplusplus
extern "C" {
#endif


//是否使用动态内存分配数据队列存储区
#define DQUE_DYNCMEM_ENABLE		0

//块大小(<256)
#define DQUE_BLK_SIZE 			128

typedef struct {
	uint8_t	in;
	uint8_t	out;
	uint8_t	next;
	uint8_t	first : 1,
			chl : 7;
#if DQUE_DYNCMEM_ENABLE
	uint8_t *	p;
#else
	uint8_t	p[DQUE_BLK_SIZE];
#endif
}t_dque, *p_dque;

typedef const struct {
	uint_t	qty;
	p_dque	list;
}dque[1];





//External Functions
void dque_Init(dque pQue);
int dque_Pop(dque pQue, uint_t nChl, buf b);
int dque_PopChar(dque pQue, uint_t nChl);
int dque_Push(dque pQue, uint_t nChl, const void *pBuf, uint_t nLen);
int dque_IsNotEmpty(dque pQue, uint_t nChl);
void dque_Clear(dque pQue, uint_t nChl);


#ifdef __cplusplus
}
#endif

#endif 


