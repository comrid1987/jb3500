#ifndef __LIB_BUFFER_H__
#define __LIB_BUFFER_H__

//-------------------------------------------------------------------------
//Buffer Management
//-------------------------------------------------------------------------

//Public Typedefs
typedef struct {
	uint_t	len;
	uint8_t *p;
}buf[1];



//External Functions
#define buf_Unpush(b, l)		buf_Cut(b, (b)->len - (l), l)	//delete from (len - l) to len
#define buf_Remove(b, l)		buf_Cut(b, 0, l)				//delete from 0 to l


void buf_Init(void);
sys_res buf_Push(buf b, const void *pData, uint_t nLen);
sys_res buf_PushData(buf b, uint_t nData, uint_t nLen);
sys_res buf_Fill(buf b, uint_t nVal, uint_t nLen);
sys_res buf_Cut(buf b, uint_t nOffset, uint_t nLen);
void buf_Release(buf b);

#endif


