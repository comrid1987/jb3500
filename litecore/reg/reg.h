#ifndef __REG_H__
#define __REG_H__


#ifdef __cplusplus
extern "C" {
#endif


//Register数据结构
typedef const struct {
	uint8_t		id;			//寄存器编号
	uint8_t		type;		//寄存器类型
	uint16_t	size;		//寄存器长度
}t_register, *p_register;

typedef const struct {
	uint8_t		id;			//寄存器组编号
	uint8_t		qty;		//寄存器元表项数
	uint16_t	type : 4,	//寄存器组类型
				size : 12;	//寄存器组大小
	p_register	tblreg;	//寄存器元表地址
}t_reg_grptbl, *p_reg_grptbl;




//寄存器数据类型定义
#define REG_TYPE_SINT 				0x01		//有符号 HEX
#define REG_TYPE_UINT 				0x02		//无符号 HEX
#define REG_TYPE_SBCD 				0x03		//有符号 BCD
#define REG_TYPE_UBCD 				0x04		//无符号 BCD
#define REG_TYPE_SDEC				0x05		//有符合fixpoint
#define REG_TYPE_UDEC				0x06		//无符号fixpoint
#define REG_TYPE_FLOAT				0x07		//float

#define REG_TYPE_STRING 			0x10		//字符型,末尾补0
#define REG_TYPE_OTHER 				0x11		//特殊类型
#define REG_TYPE_BIN 				0x12		//2进制类型,不做数值转换
#define REG_TYPE_TIME 				0x13		//时间
#define REG_TYPE_IP					0x14		//IP地址及端口
#define REG_TYPE_PASSWORD			0x15		//密码
#define REG_TYPE_BOOL				0x80		//有效/无效
#define REG_TYPE_METER				0x81		//电表类型
#define REG_TYPE_TNLINE				0x82		//测量点接线方式
#define REG_TYPE_COMMODE			0x83		//公网通讯模块工作方式 
#define REG_TYPE_UARTPARITY			0x84		//UART校验
#define REG_TYPE_UARTMODE			0x85		//UART模式
#define REG_TYPE_UARTSTOP			0x86		//UART停止位
#define REG_TYPE_RESET				0x87		//终端复位标志

//寄存器操作定义
#define REG_O_COPY					0x00
#define REG_O_ADD					0x01
#define REG_O_SUB					0x02
#define REG_O_MUL					0x03
#define REG_O_DIV					0x04
#define REG_O_OR					0x05
#define REG_O_AND					0x06
#define REG_O_SETBIT				0x07
#define REG_O_CLRBIT				0x08



//信息点号定义
#define REG_DA_ALL					0xFFFF

//AFN定义
#define GW3761_AFN_CONFIRM			0x00
#define GW3761_AFN_RESET			0x01
#define GW3761_AFN_LINKCHECK		0x02
#define GW3761_AFN_CMD_RELAY		0x03
#define GW3761_AFN_PARA_SET			0x04
#define GW3761_AFN_CMD_CTRL			0x05
#define GW3761_AFN_AUTHORITY		0x06
#define GW3761_AFN_CASCADE_QUERY	0x08
#define GW3761_AFN_CONFIG_GET		0x09
#define GW3761_AFN_PARA_GET			0x0A
#define GW3761_AFN_DATA_TASK		0x0B
#define GW3761_AFN_DATA_L1			0x0C
#define GW3761_AFN_DATA_L2			0x0D
#define GW3761_AFN_DATA_L3			0x0E
#define GW3761_AFN_FILE_TRANS		0x0F
#define GW3761_AFN_DATA_TRANS		0x10





//External Functions
int reg_Get(uint_t nDa, uint_t nID, void *pData);
uint32_t reg_GetValue(uint_t nDa, uint_t nID);
int reg_GetBit(uint_t nDa, uint_t nID, uint_t nBit);

int reg_Set(uint_t nDa, uint_t nID, const void *pData, uint_t nSave);
void reg_SetValue(uint_t nDa, uint_t nID, uint32_t nData, uint_t nSave);

int reg_GetRegSize(uint_t nID);
int reg_GetRegType(uint_t nID);

int reg_ArithmeticCopy(uint_t nDestDa, uint_t nDestReg, uint_t nSrcDa, uint_t nSrcReg, uint_t nType, uint32_t nParam, uint_t nSave);
int reg_OperateAdd(uint_t nDa, uint_t nReg, uint32_t nParam, uint_t nSave);


sys_res reg_GroupSave(uint_t nDa, uint_t nGrp);
sys_res reg_GroupCopy(uint_t nDestDa, uint_t nDestGrp, uint_t nSrcDa, uint_t nSrcGrp, uint_t nSave);

void reg_DefaultReg(uint_t nDa, uint_t nReg, uint_t nSave);
void reg_DefaultGroup(uint_t nDa, uint_t nGrp, uint_t nSave);

sys_res reg_Init(void);
sys_res reg_Maintain(void);


#ifdef __cplusplus
}
#endif

#endif

