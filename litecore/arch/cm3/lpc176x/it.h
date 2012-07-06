#ifndef __ARCH_IT_H
#define __ARCH_IT_H


#ifdef __cplusplus
extern "C" {
#endif





/* Includes ------------------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
#define ARCH_EXTIRQ_QTY			32
#define ARCH_TIMER_QTY			4


/* Exported functions ------------------------------------------------------- */
int arch_ExtIrqRegister(uint_t nPort, uint_t nPin, uint_t nTriggerMode);
void arch_ExtIrqEnable(uint_t nPort, uint_t nPin, uint_t nMode);
void arch_ExtIrqDisable(uint_t nPort, uint_t nPin, uint_t nMode);



		


#ifdef __cplusplus
}
#endif

#endif

