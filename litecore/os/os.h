#ifndef __OS_H__
#define __OS_H__


#ifdef __cplusplus
extern "C" {
#endif



#include <os/rtt/rthw.h>
#include <os/rtt/rtthread.h>

typedef struct rt_thread *		os_tid;
typedef struct rt_thread		os_thd;
typedef struct rt_semaphore		os_sem;
typedef struct rt_mutex			os_mutex;
typedef struct rt_mailbox		os_mbox;
typedef struct rt_messagequeue	os_queue;


//Thread operates
#define os_thd_declare(n, x)	struct rt_thread thd_##n;						\
								__align(8) uint8_t stk_##n[(x) & ~7]
#define os_thd_Create(n, p)																	\
								do {																	\
									rt_thread_init(&thd_##n, #n, tsk_##n, RT_NULL, stk_##n, sizeof(stk_##n), 					\
									RT_THREAD_PRIORITY_MAX - (p) / (256 / RT_THREAD_PRIORITY_MAX), 20);	\
									rt_thread_startup(&thd_##n);												\
								} while (0)

#define os_thd_Sleep(t)			rt_thread_delay((t) / OS_TICK_MS)
#define os_thd_Slp1Tick()		rt_thread_delay(1)
#define os_thd_IdSelf()			rt_thread_self()

#define os_thd_Lock()			rt_enter_critical()
#define os_thd_Unlock()			rt_exit_critical()

#define os_irq_Enter()			rt_interrupt_enter()
#define os_irq_Leave()			rt_interrupt_leave()

#define sys_jiffies()			rt_tick_get()


//External Functions
extern rt_base_t os_lock_level;
__inline void os_interrupt_Disable()
{

	os_lock_level = rt_hw_interrupt_disable();
}

__inline void os_interrupt_Enable()
{

	rt_hw_interrupt_enable(os_lock_level);
}



#include <os/rwlock.h>
#include <os/queue.h>




//External Functions
void os_Start(void);



#ifdef __cplusplus
}
#endif

#endif


