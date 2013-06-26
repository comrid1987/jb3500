#ifndef __SYS_GPIO_H__
#define __SYS_GPIO_H__


#ifdef __cplusplus
extern "C" {
#endif






//GPIO类型定义
#define GPIO_T_NULL				0
#define GPIO_T_INT				1
#define GPIO_T_HC138			2
#define GPIO_T_HC165			3
#define GPIO_T_HC595			4
#define GPIO_T_PCA955X			5
#define GPIO_T_SC16IS7X			6


//GPIO端口定义
#define GPIO_P0					0
#define GPIO_P1					1
#define GPIO_P2					2
#define GPIO_P3					3
#define GPIO_P4					4
#define GPIO_P5					5
#define GPIO_P6					6
#define GPIO_P7					7
#define GPIO_P8					8
#define GPIO_P9					9


//GPIO管脚功能定义
#define GPIO_M_OUT_MASK			4

//HIGH IMPEDANCE ANALOG INPUT TRISTATE
#define GPIO_M_IN_ANALOG		0
//INPUT FLOATING
#define GPIO_M_IN_FLOAT			1
//INPUT PULL DOWN
#define GPIO_M_IN_PD			2
//INPUT PULL UP
#define GPIO_M_IN_PU	 		3

//OUTPUT OPEN DRAIN
#define GPIO_M_OUT_OD	 		(GPIO_M_OUT_MASK | 0)
//OUTPUT PUSH PULL
#define GPIO_M_OUT_PP 			(GPIO_M_OUT_MASK | 1)
//ALTERNATE FUNCTION OPEN DRAIN 
#define GPIO_M_AF_OD 			(GPIO_M_OUT_MASK | 2)
//ALTERNATE FUNCTION PUSH PULL
#define GPIO_M_AF_PP			(GPIO_M_OUT_MASK | 3)

//GPIO初始电平
#define GPIO_INIT_NULL			0
#define GPIO_INIT_HIGH			1
#define GPIO_INIT_LOW			2


typedef const struct {
	uint8_t		type;
	uint8_t		port;
	uint8_t		pin;
	uint8_t		mode : 4,
				init : 4;
}t_gpio_def, *p_gpio_def, * const tbl_gpio_def;


#define gpio_node(n, i)			(n[0] + (i))

//External Functions
void sys_GpioConf(tbl_gpio_def p);
int sys_GpioRead(tbl_gpio_def p);
void sys_GpioSet(tbl_gpio_def p, uint_t nHL);

#define gpio_Read(n)			sys_GpioRead(gpio_node(tbl_bspGpio, n))
#define gpio_Set(n, v)			sys_GpioSet(gpio_node(tbl_bspGpio, n), v)


#ifdef __cplusplus
}
#endif

#endif

