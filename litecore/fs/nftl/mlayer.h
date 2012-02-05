#ifndef _MLAYER_H_
#define _MLAYER_H_

/****************************************************************************
 *
 *            Copyright (c) 2005 by HCC Embedded 
 *
 * This software is copyrighted by and is the sole property of 
 * HCC.  All rights, title, ownership, or other interests
 * in the software remain the property of HCC.  This
 * software may only be used in accordance with the corresponding
 * license agreement.  Any unauthorized use, duplication, transmission,  
 * distribution, or disclosure of this software is expressly forbidden.
 *
 * This Copyright notice may not be removed or modified without prior
 * written consent of HCC.
 *
 * HCC reserves the right to modify this software without notice.
 *
 * HCC Embedded
 * Budapest 1132
 * Victor Hugo Utca 11-15
 * Hungary
 *
 * Tel:  +36 (1) 450 1302
 * Fax:  +36 (1) 450 1303
 * http: www.hcc-embedded.com
 * email: info@hcc-embedded.com
 *
 ***************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#if 1
typedef unsigned char t_bit;
#else
typedef bit t_bit;
#endif

typedef unsigned short t_ba; //typedef for block address
typedef unsigned char  t_po; //typedef for page offset

typedef struct {
t_ba pba;	    //physical address of bad block, where information is stored
t_po ppo;		//current page which holds current status
unsigned char index;	//in FLTable where this was allocated from
} ST_BADBLOCK;   //坏块表在内存中的具体位置


extern t_bit ml_init(void);
extern t_bit ml_format(void);

extern t_bit ml_open(unsigned long sector, unsigned long secnum, unsigned char mode);
extern t_bit ml_write(unsigned char *data);
extern t_bit ml_read(unsigned char *data);
extern t_bit ml_close(void);
extern unsigned long ml_getmaxsector(void);

#define MLAYER_SETS				4  //has meaning only if emlayer is used
#define MAX_BLOCK				(1024*3)//NAND_BLK_QTY//16384 //1024 //4096
#define MAX_PAGE_PER_BLOCK		NAND_BLK_PAGE//32
#define MAX_FREE_BLOCK			64	//(ori64)空闲块数 --其中5个块用来存放MAP表及坏块表
#define MAX_LOG_BLOCK			31	//(ori31)
#define MAX_RESERVED_BLOCK		32	//(ori32)预留块数，做为坏块的替代块


extern t_ba *gl_freetable;

#define	FT_ADDRMASK				((t_ba)0x3FFF)     //15位
#define	FT_MAP					0x8000   //标示某一块做为MAP表块
#define	FT_LOG					0x4000   //标示某一块做为LOG块
#define FT_BAD					(FT_MAP | FT_LOG)

typedef struct {
	unsigned long wear; /**< spare area 32-bit Wear Leveling Counter [byte 3] */

	union {
		unsigned char dummy[4]; //max 4 bytes

		struct {
			t_ba lba;
			t_po lpo;
		} log;

		struct {
		  unsigned long ref_count; /* 32bit MAP block reference counter */
		} map;			/* map block 1st entry spare area*/
	} u;

	unsigned char page_state;
	unsigned char frag;		/* fragment number if mappage*/

	unsigned char block_type;	/**< spare area Block Type (LOG, FREE, MAP, DATA) */
	unsigned char bad_tag;		/**< spare area offset of the bad block tag (0x0) */

	unsigned long ecc;
} ST_SPARE;

#define MAX_DATA_SIZE			NAND_PAGE_DATA
#define MAX_SPARE_SIZE			NAND_PAGE_SPARE
#if MAX_SPARE_SIZE < 16
#error Nand flash page spare area is too small!!!!!!
#endif

#define MAX_PAGE_SIZE			(MAX_DATA_SIZE + MAX_SPARE_SIZE)
#define MAX_BLOCK_SIZE			(MAX_PAGE_SIZE*MAX_PAGE_PER_BLOCK)
#define MAX_SIZE				(MAX_BLOCK * MAX_BLOCK_SIZE)
#define MAX_FRAGSIZE			((MAX_DATA_SIZE/2)/sizeof(t_ba))		//每个映射表可放映射关系数目
#define MAX_FRAGNUM				(MAX_BLOCK / MAX_FRAGSIZE)				//总共需要多少个映射表来放映射信息

#define MAX_NUM_OF_DIF_MAP		4										//总共分4个块来存放映射表
#define MAX_FRAG_PER_BLK		(MAX_FRAGNUM/MAX_NUM_OF_DIF_MAP)		//每块映射表的个数

#define BLK_TYPE_MAP_00			0x10									//映射块0    //存放映射关系
#define BLK_TYPE_MAP_01			(BLK_TYPE_MAP_00+0x01)					//映射块1
#define BLK_TYPE_MAP_02			(BLK_TYPE_MAP_00+0x02)					//映射块2
#define BLK_TYPE_MAP_03			(BLK_TYPE_MAP_00+0x03)					//映射块3

#define BLK_TYPE_DAT			0x00
#define BLK_TYPE_BAD			0x80

#define STA_ORIGFRAG			0x01   /* if page contains original fragment info only */
#define STA_MAPPAGE				0x02   /* if page contains FLT, a fragment, and other information */

#define MAX_MAP_BLK				0x03    //可以同时存在的map block数

#define GET_SPARE_AREA(_buf_) ((ST_SPARE*)(((unsigned char*)(_buf_)) + MAX_DATA_SIZE))

typedef struct {
	t_ba pba;
	t_po ppo;
	unsigned char index;   //两个映射表占一个page，index：1，前半部分；0，后半部分
} ST_MAPDIR;   //每个映射表存放的具体位置



typedef struct {
	t_ba pba;		/* current map block address; BLK_NA if we don't have one (fatal error) */
	t_po ppo;		/* current page offset in map */
	t_ba last_pba;
	t_po last_ppo;				/*last good written map situated here */
	unsigned long ref_count;	/* last written counter in MAP block */
	unsigned char start_frag;   /* start fragment number in this MAP */
	unsigned char end_frag;     /* end fragment number in this MAP */
	unsigned char block_type;  /*type in the spare area of this block*/

	ST_MAPDIR *mapdir;		   /* start entry in mg_mapdir*/
	unsigned char index[MAX_MAP_BLK];
	unsigned char indexcou;	   /* current number of MAP blocks */
	//only at start up
	unsigned long mappagecou_hi;
	unsigned long mappagecou_lo; //searrching for the latest correct
} ST_MAPBLOCK;   //4个存放映射表的物理块的信息

typedef struct {
	t_ba last_pba;
	t_po last_ppo;
	ST_BADBLOCK badblock;
	unsigned long mappagecou_hi;
	unsigned long mappagecou_lo; //current counter
	unsigned char index[MAX_MAP_BLK]; //common!!!
} ST_MAPINFO; //用于存贮的信息块

#define MAX_CACHEFRAG 4          //可以缓存几个映射表

typedef struct {
	unsigned char indexes[MAX_CACHEFRAG]; //存放了第几个映射表
	t_ba *ppbas[MAX_CACHEFRAG];           //每个映射表在内存中的位置
	unsigned char current;                //当前操作哪个映射表
	unsigned char pos;                    //当前映射表号=indexes[pos]
	t_ba *ppba;                           //当前操作映射表在内存中的位置
} ST_FRAG;    //存放在内存中映射表信息

enum {
	ML_CLOSE,
	ML_PENDING_READ,
	ML_PENDING_WRITE,
	ML_READ,
	ML_WRITE,
	ML_ABORT,
	ML_INIT
};

typedef struct {
	unsigned long wear;
	t_ba lba;		//which logical is this
	t_ba pba;		//what its phisical
	unsigned char ppo[MAX_PAGE_PER_BLOCK];
	unsigned char lastppo;
	unsigned char index;
	unsigned char switchable;
} ST_LOG;  //记录当前正在更新的若干个块信息


#ifdef __cplusplus
}
#endif


/****************************************************************************
 *
 * end of mlayer.h
 *
 ***************************************************************************/

#endif	//_MLAYER_H_
