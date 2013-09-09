#include <os/rtt/clock.c>
#include <os/rtt/device.c>
#include <os/rtt/idle.c>
#include <os/rtt/ipc.c>
#include <os/rtt/irq.c>
#include <os/rtt/kservice.c>
#if DEBUG_MEMORY_ENABLE
#include <os/rtt/mem_debug.c>
#else
#include <os/rtt/mem.c>
#endif
#include <os/rtt/mempool.c>
#include <os/rtt/object.c>
#include <os/rtt/scheduler.c>
#include <os/rtt/slab.c>
#include <os/rtt/thread.c>
#include <os/rtt/timer.c>

#ifdef RT_USING_FINSH
#include <hi/finsh/cmd.c>
#include <hi/finsh/finsh_compiler.c>
#include <hi/finsh/finsh_error.c>
#include <hi/finsh/finsh_heap.c>
#include <hi/finsh/finsh_init.c>
#include <hi/finsh/finsh_node.c>
#include <hi/finsh/finsh_ops.c>
#include <hi/finsh/finsh_parser.c>
#include <hi/finsh/finsh_token.c>
#include <hi/finsh/finsh_var.c>
#include <hi/finsh/finsh_vm.c>
#include <hi/finsh/shell.c>
#include <hi/finsh/symbol.c>
#endif


//Public Variables
rt_base_t os_lock_level;



//----------------------------------------------------------------------------
//Function Name  :tsk_OsEntry
//Description    :   进入操作系统
//Input            : None
//Output         :  None
//Return          : None
//----------------------------------------------------------------------------
os_thd_declare(OsEntry, 1024);
void tsk_OsEntry(void *args)
{

	sys_Init();
}


#if OS_QUEUE_QTY
#include <os/queue.c>
#endif

#if OS_RWL_ENABLE
#include <os/rwlock.c>
#endif


//----------------------------------------------------------------------------
//Function Name  :os_Start
//Description    :   启动操作系统
//Input            : None
//Output         :  None
//Return          : None
//----------------------------------------------------------------------------
#ifdef __CC_ARM
#if EXTSRAM_ENABLE
extern int Image$$RW_RAM1$$ZI$$Limit;
#else
extern int Image$$RW_IRAM1$$ZI$$Limit;
#endif
#elif __ICCARM__
#pragma section = "HEAP"
#else
extern int __bss_end;
#endif
void os_Start()
{

	/* disable interrupt first */
	os_interrupt_Disable();
	/* init tick */
	rt_system_tick_init();
	/* init kernel object */
	rt_system_object_init();
	/* init timer system */
	rt_system_timer_init();
#ifdef RT_USING_HEAP
#if EXTSRAM_ENABLE
	#ifdef __CC_ARM
		rt_system_heap_init((void*)&Image$$RW_RAM1$$ZI$$Limit, (void*)(EXTSRAM_BASE_ADR + EXTSRAM_SIZE));
	#elif __ICCARM__
		rt_system_heap_init(__segment_end("HEAP"), (void*)(EXTSRAM_BASE_ADR + EXTSRAM_SIZE));
	#else
		/* init memory system */
		rt_system_heap_init((void*)&__bss_end, (void*)(EXTSRAM_BASE_ADR + EXTSRAM_SIZE));
	#endif
#else	
	#ifdef __CC_ARM
		rt_system_heap_init((void*)&Image$$RW_IRAM1$$ZI$$Limit, (void*)(MCU_SRAM_BASE_ADR + MCU_SRAM_SIZE));
	#elif __ICCARM__
		rt_system_heap_init(__segment_end("HEAP"), (void*)(MCU_SRAM_BASE_ADR + MCU_SRAM_SIZE));
	#else
		/* init memory system */
		rt_system_heap_init((void*)&__bss_end, (void*)(MCU_SRAM_BASE_ADR + MCU_SRAM_SIZE));
	#endif
#endif
#endif
	/* init scheduler system */
	rt_system_scheduler_init();
	/* init console to support rt_kprintf */
#ifdef RT_USING_CONSOLE
#ifdef RT_USING_DEVICE
	rt_hw_console_init();
#endif
	rt_console_set_device("uart1");
	/* show version */
	rt_show_version();
#endif
#ifdef RT_USING_DEVICE
	/* init hardware serial device */
	rt_device_init_all();
#endif
#ifdef RT_USING_FINSH
	/* init finsh */
	finsh_system_init();
#ifdef RT_USING_DEVICE
	finsh_set_device("uart1");
#endif
#endif
	/* init timer thread */
	rt_system_timer_thread_init();
	/* init idle thread */
	rt_thread_idle_init();
	/* init application */
	os_thd_Create(OsEntry, 200);
	/* start scheduler */
	rt_system_scheduler_start();

}






