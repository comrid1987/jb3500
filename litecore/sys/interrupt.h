#ifndef __SYS_INTERRUPT_H__
#define __SYS_INTERRUPT_H__


#ifdef __cplusplus
extern "C" {
#endif





//Public Defines
#define IRQ_TRIGGER_FALLING		0
#define IRQ_TRIGGER_RISING		1

#define IRQ_MODE_NORMAL			0
#define IRQ_MODE_HALF			1


//Public Typedefs
typedef struct {
	uint8_t mode;
	uint8_t bottom;
	uint8_t port;
	uint8_t pin;
	void (*handler)(void *);
	void *args;
}t_irq_vector;





//External Functions
void irq_Init(void);
void irq_VectorInit(void);
sys_res irq_Wait(void);
int irq_ExtRegister(uint_t nPort, uint_t nPin, uint_t nTriggerMode, void (*pHandler)(void *), void *args, uint_t nHandlerMode);
void irq_ExtEnable(uint_t nId);
void irq_ExtDisable(uint_t nId);
void irq_ExtISR(uint_t nId);
sys_res irq_TimerRegister(uint_t nId, void (*pHandler)(void *), void *args);
void irq_TimerStart(uint_t nId, uint_t nValue);
void irq_TimerStop(uint_t nId);
void irq_TimerISR(uint_t nId);
void irq_BottomHandler(void);



#ifdef __cplusplus
}
#endif

#endif

