#ifndef __DEF_H__
#define __DEF_H__



//Archives Defines
#define ARCH_T_STM32F10X_HD	1
#define ARCH_T_STM32F10X_CL	2
#define ARCH_T_STM32F20X	3

#define ARCH_T_LPC176X		5

#define ARCH_T_LM3S5X		8
#define ARCH_T_LM3S9X		9

#define ARCH_T_M051X		16

#define ARCH_T_STR71X		20
#define ARCH_T_LPC22XX		21



//System Speed Defines
#define MCU_SPEED_FULL		1
#define MCU_SPEED_HALF		2
#define MCU_SPEED_LOW		3


//Operation System Type Defines
#define OS_T_NULL			0
#define OS_T_RTTHREAD		1
#define OS_T_KEILRTX		2
#define OS_T_FREERTOS		3
#define OS_T_UCOSII			4


//Buffer Type Defines
#define BUF_T_NULL			0
#define BUF_T_BUFFER		1
#define BUF_T_DQUEUE		2


//TCP Protocol Type Defines
#define TCPPS_T_NULL		0
#define TCPPS_T_LWIP		1
#define TCPPS_T_UIP			2
#define TCPPS_T_KEILTCP		3


//File System Type Defines
#define FS_T_NULL			0
#define FS_T_RTX			1
#define FS_T_FATFS			2
#define FS_T_EFSL			3


//System Return Defines
typedef enum {
	SYS_R_OK = 0,		/* 0 */
	SYS_R_NO,			/* 1 */
	SYS_R_ERR,			/* 2 */
	SYS_R_TMO,			/* 3 */
	SYS_R_FULL,			/* 4 */
	SYS_R_BUSY,			/* 5 */
	SYS_R_EMPTY,		/* 6 */
	SYS_R_EMEM,			/* 7 */
	SYS_R_RESET,		/* 8 */
}sys_res;





#endif

