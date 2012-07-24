#ifndef __ARCH_ARCH_H__
#define __ARCH_ARCH_H__

#ifdef __cplusplus
extern "C" {
#endif


//Header Files
#include <arch/arm7/lpc22xx/LPC22xx.H>

#include <arch/arm7/lpc22xx/gpio.h>
#include <arch/arm7/lpc22xx/timer.h>

#if OS_TYPE
#include <arch/arm7/lpc22xx/uart.h>
#endif

#include <arch/arm7/lpc22xx/it.h>


#if MCU_FREQUENCY == MCU_SPEED_LOW
#error "not support low speed yet"
#elif MCU_FREQUENCY == MCU_SPEED_HALF
#error "not support half speed yet"
#else
#define MCU_CLOCK			__CORE_CLK
#endif

#define __CORE_CLK			(11059200 * 5)			//系统频率，必须为OSC_CLK的整数倍(1~32)，且<=60MHZ
#define FCCO_CLK			(__CORE_CLK * 2)		//CCO频率，必须为__CORE_CLK的2、4、8、16倍，范围为156MHz~320MHz
#define PERI_CLOCK			(__CORE_CLK / 4)		//VPB时钟频率，只能为(__CORE_CLK / 4)的1 ~ 4倍




//Public Defines
#define __I		volatile const
#define __O		volatile
#define __IO	volatile


#if defined ( __CC_ARM   )
#pragma anon_unions
#endif

/*------------- General Purpose Input/Output (GPIO) --------------------------*/
typedef struct
{
	__I uint32_t FIOPIN;
	__O uint32_t FIOSET;
	__O uint32_t FIODIR;
	__O uint32_t FIOCLR;
} LPC_GPIO_TypeDef;

 
#define LPC_GPIO0             ((LPC_GPIO_TypeDef      *) 0xE0028000    )
#define LPC_GPIO1             ((LPC_GPIO_TypeDef      *) 0xE0028010    )
#define LPC_GPIO2             ((LPC_GPIO_TypeDef      *) 0xE0028020    )
#define LPC_GPIO3             ((LPC_GPIO_TypeDef      *) 0xE0028030    )



/*------------- Timer (TIM) --------------------------------------------------*/
typedef struct
{
	__IO uint32_t IR;
	__IO uint32_t TCR;
	__IO uint32_t TC;
	__IO uint32_t PR;
	__IO uint32_t PC;
	__IO uint32_t MCR;
	__IO uint32_t MR0;
	__IO uint32_t MR1;
	__IO uint32_t MR2;
	__IO uint32_t MR3;
	__IO uint32_t CCR0;
	__I  uint32_t CR0;
	__I  uint32_t CR1;
	__I  uint32_t CR2;
	__I  uint32_t CR3;
	__IO uint32_t EMR;
} LPC_TIM_TypeDef;
 

#define LPC_TIM0              ((LPC_TIM_TypeDef       *) 0xE0004000     )
#define LPC_TIM1              ((LPC_TIM_TypeDef       *) 0xE0008000     )
 


/*------------- Universal Asynchronous Receiver Transmitter (UART) -----------*/
typedef struct
{
  union {
  __I  uint8_t  RBR;
  __O  uint8_t  THR;
  __IO uint8_t  DLL;
       uint32_t RESERVED0;
  };
  union {
  __IO uint8_t  DLM;
  __IO uint32_t IER;
  };
  union {
  __I  uint32_t IIR;
  __O  uint8_t  FCR;
  };
  __IO uint8_t  LCR;
       uint8_t  RESERVED1[3];
  __IO uint8_t  MCR;
       uint8_t  RESERVED2[3];
  __I  uint8_t  LSR;
       uint8_t  RESERVED3[3];
  __I  uint8_t  MSR;
       uint8_t  RESERVED4[3];
  __IO uint8_t  SCR;
       uint8_t  RESERVED5[3];
  __IO uint32_t ACR;
       uint32_t RESERVED6;
  __IO uint32_t FDR;
       uint32_t RESERVED7;
  __IO uint8_t  TER;
       uint8_t  RESERVED8[3];
} LPC_UART_TypeDef;


#define LPC_UART0             ((LPC_UART_TypeDef     *) 0xE000C000    )
#define LPC_UART1             ((LPC_UART_TypeDef     *) 0xE0010000    )

#if defined ( __CC_ARM   )
#pragma no_anon_unions
#endif



/*------------- Interrupt (IRQ) -------------------------------------------------*/
#define VIC_BASE_ADDR	0xFFFFF000

enum LPC22xx_INT
{
	WDT_INT	= 0,
	SW_INT_reserved,
	DbgCommRx_INT,
	DbgCommTx_INT,
	TIMER0_INT,
	TIMER1_INT,
	UART0_INT,
	UART1_INT,
	PWM0_INT,
	I2C0_INT,
	SP0_INT,
	SP1_INT,
	PLL_INT,
	RTC_INT,
	EINT0_INT,
	EINT1_INT,
	EINT2_INT,
	EINT3_INT,
	ADC0_INT,
	I2C1_INT,
	BOD_INT,
	ADC1_INT,
	USB_INT
};




//External Functions
void arch_Init(void);
void arch_IdleEntry(void);
void arch_Reset(void);



#ifdef __cplusplus
}
#endif


#endif


