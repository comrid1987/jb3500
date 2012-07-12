#ifndef __APP_PARA_H__
#define __APP_PARA_H__

#ifdef __cplusplus
extern "C" {
#endif



//Public Defines
#define ECL_SN_MAX					4
#define ECL_TN_MAX					ECL_SN_MAX




//Public Typedefs
//终端上行通讯口参数
typedef __packed struct {
	uint8_t		rts;
	uint8_t		delay;
	uint16_t	tmo;
	uint8_t		con;
	uint8_t		span;
}t_afn04_f1;

//主站IP地址和端口
typedef __packed struct {
	uint8_t		ip1[4];
	uint16_t	port1;
	uint8_t		ip2[4];
	uint16_t	port2;
	char		apn[16];
}t_afn04_f3;

//终端事件配置参数
typedef __packed struct {
	uint64_t	valid;
	uint64_t	imp;
}t_afn04_f9;

//测量点参数
typedef __packed struct {
	uint16_t	tn;			//所属测量点号
	uint8_t		port : 5,	//通信速率及端口号 D7～D5编码表示电能表、交流采样装置与终端的通信波特率， 1～7依次表示600、1200、2400、4800、7200、9600、19200；0：表示无需设置或使用默认的，例如本通信端口号的接口及通道类型为2时（即本端口是以串行接口与窄带低压载波通信模块相连接，参见本部分5.9.2.4.2中相关内容），本数值无意义，设为0，实际使用的通信速率按本部分5.5.1.3.32的设置执行。
				baud : 3;	//D4～D0编码表示电能表、交流采样装置与终端连接所对应的终端通信端口号，数值范围1～31，其他值无效。
	uint8_t		prtl; 		//通信规约类型通信协议类型：数值范围0～255，其中0：表示终端无需对本序号的电能表/交流采样装置进行抄表；1：DL/T 645-1997.；2：交流采样装置通信协议；30：DL/T 645-2007；31："串行接口连接窄带低压载波通信模块"接口协议；其他：备用。
	uint8_t		madr[6];	//表地址数值范围0～999999999999。
	uint8_t		pwd[6];		//表密码终端与电能表通信的密码。
	uint8_t		rate;		//电表费率个数
	uint8_t		energy;		//有功电能示值整数位及小数位个数● D3～D2：编码表示通信接入的电能表的有功电能示值的整数位个数，数值范围0～3依次表示4～7位整数
							//D1～D0：编码表示通信接入的电能表的有功电能示值的小数位个数，数值范围0～3依次表示1～4位小数。
	uint8_t		tadr[6];	//对应的表的终端地址  
	uint8_t		usertype;	//用户大类号及用户小类号   高高4位大类别，低4位为小类别 0缺省1，单相 2三相 
							// D7～D4：编码表示本电能表所属的用户大类号，数值范围0～15，依次表示16个用户大类号。
							// D3～D0：编码表示本电能表所属的用户小类号，数值范围0～15，依次表示16套1类和2类数据项的配置（应在终端支持的1类和2类数据配置范围内）
}t_afn04_f10;

//总加组配置参数
typedef __packed struct {
	uint8_t		qty;
	uint8_t		tn[64];
}t_afn04_f14;

//费率参数
typedef __packed struct {
	uint8_t		qty;
	uint32_t	rate[16];
}t_afn04_f22;

//测量点基本参数
typedef __packed struct {
	uint16_t	PT;
	uint16_t	CT;
	uint8_t		RatedU[2];
	uint8_t		RatedI;
	uint8_t		RatedCP[3];
	uint8_t		LineStyle : 2,
				SinglePhasic : 2,
				reserve : 4;
}t_afn04_f25;

//测量点限值参数
typedef __packed struct {
	uint16_t	uup;				//电压合格上限
	uint16_t	ulow;				//电压合格下限
	uint16_t	ubreak;				//电压断相门限
	uint16_t	uover;				//电压上上限（过压门限）
	uint8_t		uovertime;			//越限持续时间 
	uint16_t	uoverresume;		//越限恢复系数
	uint16_t	uunder;				//电压下下限（欠压门限）
	uint8_t		uundertime;			//越限持续时间 
	uint16_t	uunderresume;		//越限恢复系数
	uint8_t		iover[3];			//相电流上上限（过流门限）
	uint8_t		iovertime;			//越限持续时间 
	uint16_t	ioverresume;		//越限恢复系数
	uint8_t		iup[3];				//相电流上限（额定电流门限）
	uint8_t		iuptime;			//越限持续时间 
	uint16_t	iupresume;			//越限恢复系数
	uint8_t		i0up[3];			//零序电流上限
	uint8_t		i0time;				//越限持续时间 
	uint16_t	i0resume;			//越限恢复系数
	uint8_t		uiover[3];			//视在功率上上限
	uint8_t		uiovertime;			//越限持续时间 
	uint16_t	uioverresume;		//越限恢复系数
	uint8_t		uiup[3];			//视在功率上限
	uint8_t		uiuptime;			//越限持续时间 
	uint16_t	uiupresume;			//越限恢复系数
	uint16_t	ubalance;			//三相电压不平衡限值
	uint8_t		ubalancetime;		//越限持续时间 
	uint16_t	ubalanceresume;		//越限恢复系数
	uint16_t	ibalance;			//三相电流不平衡限值
	uint8_t		ibalancetime;		//越限持续时间 
	uint16_t	ibalanceresume;		//越限恢复系数
	uint8_t		losttime;			//连续失压时间限值
}t_afn04_f26;

//功率因数分段限值
typedef __packed struct {
	uint16_t	low;				//限值1
	uint16_t	up;					//限值2
}t_afn04_f28;

//终端抄表运行参数
typedef __packed struct {
	uint16_t	flag;
	uint32_t	date;
	uint8_t		time[2];
	uint8_t		span;
	uint8_t		checktime[3];
	uint8_t		qty;
	uint32_t	timeinfo[24];
}t_afn04_f33;

//终端地址
typedef __packed struct {
	uint16_t	mfcode;
	uint16_t	area;
	uint16_t	addr;
}t_afn04_f85;







//External Functions
int icp_ParaRead(uint_t nAfn, uint_t nFn, uint_t nPn, void *pBuf, uint_t nLen);
int icp_ParaWrite(uint_t nAfn, uint_t nFn, uint_t nPn, const void *pBuf, uint_t nLen);
int icp_MeterRead(uint_t nSn, t_afn04_f10 *p);
void icp_MeterWrite(uint_t nSn, const t_afn04_f10 *p);
int icp_Meter4Tn(uint_t nTn, t_afn04_f10 *p);
void icp_Clear(void);
void icp_Init(void);
void icp_UdiskLoad(void);


#ifdef __cplusplus
}
#endif

#endif


