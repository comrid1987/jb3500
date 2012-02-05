#ifndef __TDK6515_H__
#define __TDK6515_H__


#ifdef __cplusplus
extern "C" {
#endif




//Public Typedefs
typedef struct {
	uint_t		ste;
	p_dev_uart	uart;
}t_tdk6515;




// =============================================================
// 从计量IC中读取的原始值,最终会转换成相应的进制
// =============================================================
typedef struct {
    sint32_t	temp;		// 1. 温度 S32	0.1度
    uint32_t	freq;		// 2. 频率 U32	0.01HZ
    float		enp[4];		// 3. 合相ABC有功电能  Float
    float		enq[4];     // 7. 合相ABC无功电能  Float
    float		ahour[4];	// 11.ABCN相安培平方小时 Float
    float		vhour[3];	// 15.ABC相伏特平方小时  Float
    uint32_t	caangle;	// 18.CA相电压相间相角   U32		0.1DEC
    uint32_t	cbangle;	// 19.CB相电压相间相角	U32		0.1DEC
    uint32_t	reserve;	// 20
    uint32_t	vover0num;	// 21.电压过零次数	U32
    float		p[4];		// 22.合ABC相有功功率    Float
    float		q[4];		// 26.合ABC相无功功率	Float
    float		vi[4];		// 30.合ABC视在功率		Float
    float		viangle[4];	// 34.合ABC电压电流相角	Float
    float		cos[4];	// 38.合ABC功率因素	Float
    float		i[4];	// 42.ABCN电流有效值	Float
    float		v[3];	// 46.ABC相电压有效值	Float
    float		ptneed;	// 49.有功滑差需量		Float
    float		qtneed;	// 50.无功滑差需量		Float
    float		ppneed;	// 51.有功周期需量		Float
    float		qpneed;	// 52.无功周期需量		Float
    uint_t   workr;	// 53.计量工作状态寄存器	SW
    uint_t   powerdir;	// 54.功率方向寄存器		SW
    uint_t   netstatus;	// 55.电网运行状态字		SW
    uint_t   connstatus;// 56.电气接线状态字		SW
    uint_t   adjustsum;	// 57. 校表数据校验和寄存器		U32
    uint_t   lasttx;	// 58. 上一次TX发送值寄存器		
    uint_t   uab;	// 59. AB线电压有效值
    uint_t   ubc;	// 60. BC线电压有效值
    uint_t   uac;	// 61. AC线电压有效值
    uint_t   ppulse;	// 62. 有功脉冲计数器
    uint_t   qpulse;	// 63. 无功脉冲计数器
    uint_t   pulse1;	// 64. 远动脉冲计数器1
    uint_t   pulse2;	// 65. 远动脉冲计数器2
    uint_t   pulse3;	// 66. 远动脉冲计数器3
    uint_t   pulse4;	// 67. 远动脉冲计数器4
    uint_t   pulse5;	// 68. 远动脉冲计数器5
    uint_t   ua1min;	// 69. A相电压一分钟平均值
    uint_t   ub1min;	// 70. B相电压一分钟平均值
    uint_t   uc1min;	// 71. C相电压一分钟平均值
}t_tdk6515_rtdata;
	

// =============================================================
// 从计量IC中读取的原始值,最终转换成了浮点数和整型数
// =============================================================
typedef struct {
	uint_t	curxbno;	// 72.当前谐波分析通道号
	float	xbrate[21];	// 73.当前通道总2-21次谐波含有率
	float	xbbase;	// 94. 当前谐波通道基本有效值
	uint_t	xbbasefreq;// 95. 当前谐波基波频率
}t_tdk6515_xbdata;




//External Functions
sys_res tdk6515_IsJLReady(void);
sys_res tdk6515_IsXBReady(void);


#ifdef __cplusplus
}
#endif

#endif

