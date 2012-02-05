//-----------------------------------------------------------------------------
//
//                           		红外处理程序         
//                                  头文件(*.h)
//
//
// 文件名    : ir_test.h
// 作者      : nmy
// 生成日期  : 2011-10-9
//
// ARM内核   : ARMv7M Cortex-M3
// 使用芯片  : LM3S5965
// 开发环境  : KEIL
//
// 版本记录  : V1.00  创建第一版   2011-10-9 15:30
//
//-----------------------------------------------------------------------------

// 使用驱动库所需要的头文件
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"

#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"   // 与系统配置有关
#include "driverlib/gpio.h"     // 与GPIO操作有关
#include "driverlib/timer.h"    // 与timer操作有关

#define ir_timer_base TIMER1_BASE
#define ir_pwm_timer_base TIMER2_BASE
//#define ir_timer_base TIMER1_BASE
//#define ir_pwm_timer_base TIMER2_BASE

#define ir_debug 0x00
#define ir_test 0x01
#define ir_receive_flag_reset 0x00
#define ir_receive_flag_fail 0xf0
#define ir_timer_on TimerEnable(TIMER1_BASE, TIMER_A) 
#define ir_timer_off TimerDisable(TIMER1_BASE, TIMER_A)
//#define ir_pwm_on TimerEnable(TIMER2_BASE, TIMER_A) 
//#define ir_pwm_off TimerDisable(TIMER2_BASE, TIMER_A)
#define ir_send_l TimerEnable(TIMER2_BASE, TIMER_A) 
#define ir_send_h TimerDisable(TIMER2_BASE, TIMER_A)


static unsigned char ir_receive_buf[25];    		//红外接收缓冲区,根据协议最大接收23个字节数据
static unsigned char ir_receive_flag=0x01;			//红外接收状态标志位,0表示接收失败
static unsigned char ir_receive_counter=0x00;		//红外接收计数器
static unsigned char ir_receive_length=0x0B;		//红外接收长度控制器
static unsigned char ir_temp_data[12];				//红外接收11位缓存+奇偶校验缓存位
static unsigned char ir_temp_byte;					//接收时八位有用数据缓存
static unsigned char ir_status=0x00;				//红外状态标志位,0x00-0x0f表示接收状态，0x10-0xf0表示发送状态
static unsigned char ir_send_buf[25];			   	//红外发送缓冲区,根据协议最大发送23个字节数据
static unsigned char ir_send_flag=0x00;				   	//红外发送启动标志位，0表示不发送
static unsigned char ir_send_length=0x00;			   	//红外发送帧长度（字节）



//struct meter_para
//{
// unsigned char factory_No[6];			//电表的表号
// unsigned char prg_password[6];		//电表的编程密码
// unsigned char pulse_constant[3];	//电表的脉冲常数
// unsigned char auto_bill_day[2];  //自动抄表日
//};
//
//typedef struct
//{               
//	unsigned char Program_permit         :1 ;		//为1代表当前处于允许编程状态		          
//	unsigned char Battery                :1 ;   //电池欠压标记，1为欠压 
//	unsigned char Battery_flag_write     :1 ;   //上电判断电池合格，写过一次次EEPROM后置一
//	unsigned char Receive_finish         :1 ; 	//485完成接收1帧数据
//	unsigned char receive_first_68H	     :1 ; 	//485接收到第1个0x68后置一
//	unsigned char I_receive_finish       :1 ;   //红外完成接收1帧数据                 		
//	unsigned char I_receive_first_68H 	 :1 ;   //红外接收到第1个0x68后置一
//	unsigned char I_transmit_one_byte 	 :1 ;   //红外1字节发送完毕标志
//	unsigned char Quarter_second				 :1 ;		//四分之一秒标记					
//	unsigned char Broadcast              :1 ;		//广播校时标记，为0可以广播校时，为1禁止	
//	unsigned char Zero_hour_energy       :1 ;	 	//零点是否已转存数据标记
//	unsigned char hour_energy  			     :1 ;		//整点是否已转存数据标记
//	unsigned char Month_energy           :1 ;   //结算日是否已转存数据标记
//	unsigned char Switch_order           :1 ;		//为1时有拉合闸命令未响应	
//	unsigned char Switch_on					     :1 ;		//为1代表合闸命令
//	unsigned char Switch_off             :1 ;		//为1代表拉闸命令
//	unsigned char Detect_time_enough     :1 ;   //为1代表已检测到足够长的时间判断是否拉闸
//	unsigned char Power_reverse					 :1 ;		//电能反向标记
//	unsigned char Less_half_IB           :1 ;   //为1代表电流小于IB
//	unsigned char More_than_255          :1 ;   //累计次数超过255时置1;
//	unsigned char New_pulse              :1 ;   //为1代表有新脉冲产生，要计算电流大小
//} METER_FLAG;
//
//
//
//
////unsigned char ir_rdata[11];								//红外发送1字节时需发送的11位  
//struct meter_para   meter_para;					//电表参数
//METER_FLAG   BitFlag;										//位标记
//









