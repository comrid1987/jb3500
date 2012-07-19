

//Private Variables
#if IRQ_HALF_ENABLE
static os_sem irq_sem;
#endif

#if IRQ_EXT_ENABLE
static t_irq_vector irq_aExt[ARCH_EXTIRQ_QTY];
#endif
#if IRQ_TIMER_ENABLE
static t_irq_vector irq_aTimer[ARCH_TIMER_QTY];
#endif




void irq_Init()
{

#if IRQ_HALF_ENABLE
	rt_sem_init(&irq_sem, "extirq", 0, RT_IPC_FLAG_FIFO);
#endif
}

void irq_VectorInit()
{
	t_irq_vector *p;

#if IRQ_EXT_ENABLE
	for (p = irq_aExt; p < ARR_ENDADR(irq_aExt); p++) {
		p->bottom = 0;
		p->handler = NULL;
	}
#endif
#if IRQ_TIMER_ENABLE
	for (p = irq_aTimer; p < ARR_ENDADR(irq_aTimer); p++) {
		p->bottom = 0;
		p->handler = NULL;
	}
#endif
}

#if IRQ_HALF_ENABLE
sys_res irq_Wait()
{

	if (rt_sem_trytake(&irq_sem) == RT_EOK)
		return SYS_R_OK;
	return SYS_R_TMO;
}

void irq_BottomHandler()
{
	t_irq_vector *p;

	for (p = irq_aExt; p < ARR_ENDADR(irq_aExt); p++)
		if (p->bottom) {
			p->bottom = 0;
			(p->handler)(p->args);
		}
	for (p = irq_aTimer; p < ARR_ENDADR(irq_aTimer); p++)
		if (p->bottom) {
			p->bottom = 0;
			(p->handler)(p->args);
		}
}
#endif


#if IRQ_EXT_ENABLE
int irq_ExtRegister(uint_t nPort, uint_t nPin, uint_t nTriggerMode, void (*pHandler)(void *), void *args, uint_t nHandlerMode)
{
	int nId;
	t_irq_vector *p;

	nId = arch_ExtIrqRegister(nPort, nPin, nTriggerMode);
	if (nId == -1)
		return -1;
	p = &irq_aExt[nId];
	p->mode = nHandlerMode;
	p->port = nPort;
	p->pin = nPin;
	p->handler = pHandler;
	p->args = args;
	return nId;
}

void irq_ExtEnable(uint_t nId)
{
	t_irq_vector *p = &irq_aExt[nId];

	arch_ExtIrqEnable(p->port, p->pin, p->mode);
}

void irq_ExtDisable(uint_t nId)
{
	t_irq_vector *p = &irq_aExt[nId];

	arch_ExtIrqDisable(p->port, p->pin, p->mode);
}

void irq_ExtISR(uint_t nId)
{
	t_irq_vector *p = &irq_aExt[nId];

#if IRQ_HALF_ENABLE
	if (p->mode == IRQ_MODE_NORMAL) {
		(p->handler)(p->args);
	} else {
		p->bottom = 1;
		rt_sem_release(&irq_sem);
	}
#else
	(p->handler)(p->args);
#endif
}
#endif


#if IRQ_TIMER_ENABLE
sys_res irq_TimerRegister(uint_t nId, void (*pHandler)(void *), void *args)
{
	t_irq_vector *p = &irq_aTimer[nId];

	arch_TimerInit(nId);
	p->handler = pHandler;
	p->args = args;
	return SYS_R_OK;
}

void irq_TimerHandler(uint_t nId, void (*pHandler)(void *), void *args)
{
	t_irq_vector *p = &irq_aTimer[nId];

	p->handler = pHandler;
	p->args = args;
}


void irq_TimerStart(uint_t nId, uint_t nValue)
{

	arch_TimerStart(nId, nValue);
}

void irq_TimerStop(uint_t nId)
{

	arch_TimerStop(nId);
}

void irq_TimerISR(uint_t nId)
{
	t_irq_vector *p = &irq_aTimer[nId];

#if IRQ_HALF_ENABLE
	if (p->mode == IRQ_MODE_NORMAL) {
		(p->handler)(p->args);
	} else {
		p->bottom = 1;
		rt_sem_release(&irq_sem);
	}
#else
	(p->handler)(p->args);
#endif
}
#endif


