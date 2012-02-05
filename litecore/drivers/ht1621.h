#ifndef __HT1621_H__
#define __HT1621_H__


#ifdef __cplusplus
extern "C" {
#endif



// LCD所在端口及引脚
#define GPIO_LCD_SYSCTL_PERIPH  SYSCTL_PERIPH_GPIOE

#define GPIO_LCD_PORT_BASE      GPIO_PORTE_BASE

#define HT1621_CS			GPIO_PIN_0

#define HT1621_RD			GPIO_PIN_1

#define HT1621_WR			GPIO_PIN_2

#define HT1621_DATA			GPIO_PIN_3

#define high_val			0xFF

//CS端置1
#define HT1621_CS_H GPIOPinWrite(GPIO_LCD_PORT_BASE,HT1621_CS,high_val)

//CS端清零
#define HT1621_CS_L GPIOPinWrite(GPIO_LCD_PORT_BASE,HT1621_CS,~high_val)

//RD端置1
#define HT1621_RD_H GPIOPinWrite(GPIO_LCD_PORT_BASE,HT1621_RD,high_val)

//RD端清零
#define HT1621_RD_L GPIOPinWrite(GPIO_LCD_PORT_BASE,HT1621_RD,~high_val)
 
//WR端置1
#define HT1621_WR_H GPIOPinWrite(GPIO_LCD_PORT_BASE,HT1621_WR,high_val)

//WR端置1
#define HT1621_WR_L GPIOPinWrite(GPIO_LCD_PORT_BASE,HT1621_WR,~high_val)

//DATA端清零
#define HT1621_DATA_H GPIOPinWrite(GPIO_LCD_PORT_BASE,HT1621_DATA,high_val)

//DATA端清零
#define HT1621_DATA_L GPIOPinWrite(GPIO_LCD_PORT_BASE,HT1621_DATA,~high_val)

//读DATA端的引脚状态
#define HT1621_DATA_READ GPIOPinRead(GPIO_LCD_PORT_BASE, HT1621_DATA)

//设置DATA端的为输出
#define HT1621_DATA_OUTPUT GPIOPinTypeGPIOOutput(GPIO_LCD_PORT_BASE,HT1621_DATA )

//设置DATA端的为输入
#define HT1621_DATA_INPUT 	GPIOPinTypeGPIOInput(GPIO_LCD_PORT_BASE,HT1621_DATA);

//设置DATA端的弱上拉
#define DATA_INPUT_CONFIG 	GPIOPadConfigSet(GPIO_LCD_PORT_BASE, HT1621_DATA, GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPU);;

//设置引脚的为输出
#define HT1621_OUTPUT(x) 	GPIOPinTypeGPIOOutput(GPIO_LCD_PORT_BASE,x )

//定义延时
#define Ctl_Delay(x) 	SysCtlDelay(x * (SysCtlClockGet()/16000000))





//External Functions
void ht1621_Init(void);
void ht1621_Write(uint_t nIcon, uint_t nMask);

#ifdef __cplusplus
}
#endif

#endif

