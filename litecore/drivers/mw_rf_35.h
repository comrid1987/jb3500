#ifndef __BSP_MW_RF_35_H__
#define __BSP_MW_RF_35_H__

//通讯字符定义
#define MW_STX          0x02
#define MW_ETX          0x03
#define MW_DLE          0x10
#define MW_NAK          0x15

//读卡器控制字定义
#define MW_COMMAND_LED             0x35     //led显示
#define MW_COMMAND_BUZZER          0x36     //蜂鸣器
#define MW_COMMAND_R_EE            0x37     //读EEPROM
#define MW_COMMAND_W_EE            0x38     //写EEPROM
#define MW_COMMAND_S_TM            0x39     //设置时间
#define MW_COMMAND_R_TM            0x3A     //读取时间
#define MW_COMMAND_S_CK            0x4D     //设置校验
#define MW_COMMAND_R_RF            0x4E     //复位RF模块
#define MW_COMMAND_R_ED            0x4F     //读读卡器版本
#define MW_COMMAND_W_RD            0x54     //写读卡器命令
#define MW_COMMAND_R_RD            0x55     //读读卡器值
#define MW_COMMAND_S_CD            0x60     //选择卡类型

//卡类型定义
#define MW_TYPEA                   0x00     //14433_TYPEA
#define MW_TYPEB                   0x01     //14433_TYPEA
#define MW_TYPEO                   0x02     //15693

//mf标准卡命令
#define MF_COMMAND_REQUEST         0x2F     //向天线操作范围内的卡片发出请求和检测信号
#define MF_COMMAND_REQUEST2        0x41     //向天线操作范围内的卡片发出请求和检测信号
#define MF_COMMAND_ANITI           0x42     //防冲突操作并返回卡片系列号
#define MF_COMMAND_GET_CAP         0x43     //从多个卡中选取一个给定序列号的卡，返回卡的容量
#define MF_COMMAND_ATTEST          0x44     //认证
#define MF_COMMAND_HALT            0x45     //将卡片置于暂停状态
#define MF_COMMAND_READ            0x46     //读一个数据块
#define MF_COMMAND_WRITE           0x47     //写一个数据块
#define MF_COMMAND_INCR            0x48     //对值操作的块进行增值操作并存储在卡的内部存储器中
#define MF_COMMAND_DECR            0x49     //对值操作的块进行减值操作并存储在卡的内部存储器中
#define MF_COMMAND_R_SEND_M        0x4A     //将某块的数据传入卡的内部寄存器中
#define MF_COMMAND_M_SEND_R        0x4B     //将内部寄存器的数据传送到某一块中
#define MF_COMMAND_LOAD_PASSWORD   0x4C     //装载密码

#define MF_COMMAND_RESET           0x4E     //reset ??

#define MF_COMMAND_OTHER           0x56     //其他验证命令
//MF pro卡命令
#define MF_COMMAND_CPU_RESET       0x5A     //复位非接触式CPU卡
#define MF_COMMAND_CPU_APDU        0x5B     //发送APDU命令给CPU卡
#define MF_COMMAND_CPU_END         0x5C     //结束CPU卡命令
//SAM卡命令
#define MH_SAM_RESET               0x3C     //复位SAM卡
#define MH_SAM_COMMAND             0x3D     //发送SAM命令


//明华读卡器数据发送帧
typedef struct
{
	uint8_t  cTxSeq;		//命令序号
	uint8_t  cCommand;	//命令字
	uint8_t  cLength;    //帧长度
	buf buf;	//数据内容
	uint8_t	cBcc;       //校验
}StructMWTxData,*pStructMWTxData;

//明华读卡器数据接收帧
typedef struct
{
	uint8_t  cRxSeq;		//命令序号
	uint8_t  cStatus;	//命令字
	uint8_t  cLength;    //帧长度
	buf buf;		//数据内容
}StructMWRxData,*pStructMWRxData;

const static unsigned char mf_trans_pass[16][6]={
      { 0xbd,0xde,0x6f,0x37,0x83,0x83 },
      { 0x14,0x8a,0xc5,0xe2,0x28,0x28 },
      { 0x7d,0x3e,0x9f,0x4f,0x95,0x95 },
      { 0xad,0xd6,0x6b,0x35,0xc8,0xc8 },
      { 0xdf,0xef,0x77,0xbb,0xe4,0xe4 },
      { 0x09,0x84,0x42,0x21,0xbc,0xbc },
      { 0x5f,0xaf,0xd7,0xeb,0xa5,0xa5 },
      { 0x29,0x14,0x8a,0xc5,0x9f,0x9f },
      { 0xfa,0xfd,0xfe,0x7f,0xff,0xff },
      { 0x73,0x39,0x9c,0xce,0xbe,0xbe },
      { 0xfc,0x7e,0xbf,0xdf,0xbf,0xbf },
      { 0xcf,0xe7,0x73,0x39,0x51,0x51 },
      { 0xf7,0xfb,0x7d,0x3e,0x5a,0x5a },
      { 0xf2,0x79,0x3c,0x1e,0x8d,0x8d },
      { 0xcf,0xe7,0x73,0x39,0x45,0x45 },
      { 0xb7,0xdb,0x6d,0xb6,0x7d,0x7d }
   };


uint8_t mw_35lt_beep(p_dev_uart rs485_3_dev,uint8_t *cTxS,uint16_t beep_time);
void send_35lt_start_byte(p_dev_uart rs485_3_dev);
uint8_t mw_35lt_set_time(p_dev_uart rs485_3_dev,uint8_t *cTxS);
uint8_t mw_sam_reset( p_dev_uart rs485_3_dev, uint8_t *cTxS ,uint8_t baud);
uint16_t mw_sam_get_random( p_dev_uart rs485_3_dev, uint8_t *cTxS ,uint8_t len,uint8_t * data);
uint8_t mw_35lt_requst_card( p_dev_uart rs485_3_dev, uint8_t *cTxS, uint8_t command );
uint8_t mw_35lt_anti_card( p_dev_uart rs485_3_dev, uint8_t *cTxS, uint32_t *cardnumber);
uint8_t mw_35lt_select_card( p_dev_uart rs485_3_dev, uint8_t *cTxS, uint32_t cardnumber);
uint8_t mw_35lt_reset_cpu( p_dev_uart rs485_3_dev, uint8_t *cTxS);
uint8_t mw_35lt_end_cpu( p_dev_uart rs485_3_dev, uint8_t *cTxS);

p_dev_uart init_rs485_3(void);
uint8_t mw_35lt_requst_cpu( p_dev_uart rs485_3_dev, uint8_t *cTxS, uint8_t command ,uint32_t * cardnumber);

uint8_t mw_35lt_reset( p_dev_uart rs485_3_dev, uint8_t *cTxS);
uint16_t mw_cpu_select_bin( p_dev_uart rs485_3_dev, uint8_t *cTxS ,uint8_t p1,uint8_t p2,uint8_t * pdata, uint8_t len);
uint16_t mw_cpu_read_bin( p_dev_uart rs485_3_dev, uint8_t *cTxS ,uint8_t p1,uint8_t p2, buf pdata, uint8_t len);
uint16_t mw_cpu_get_ramd( p_dev_uart rs485_3_dev, uint8_t *cTxS, void *data);
uint16_t mw_cpu_update_bin( p_dev_uart rs485_3_dev, uint8_t *cTxS ,uint8_t p1,uint8_t p2,buf pdata, uint8_t len);

uint8_t mw_35lt_init_cpu(p_dev_uart rs485_3_dev,uint8_t *cTxS,uint32_t *cardnumber);
uint8_t mw_35lt_check_card(p_dev_uart rs485_3_dev,uint8_t *cTxseq,uint32_t *cardnumber);

uint8_t mw_35lt_cpu_ext_auth(p_dev_uart rs485_3_dev,uint8_t *cTxseq,uint32_t *cardnumber);
uint8_t mw_35lt_int_auth(p_dev_uart rs485_3_dev, uint8_t *cTxseq,uint32_t *cardnumber);


#endif
