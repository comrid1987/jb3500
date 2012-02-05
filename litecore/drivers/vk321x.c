#if VK321X_ENABLE
#include <drivers/vk321x.h>

//Public Variables


//Private Defines
#define VK321X_LOCK_ENABLE		1

#define VK312X_MFREQ			(VK321X_CRYSTAL / 1743800)
#define VK321X_TIMEOUT			10000		//(ms)


//Reg Table
#define VK321X_GCR				0x01
#define VK321X_GUCR				0x02 
#define VK321X_GIR				0x03
#define VK321X_GXOFF			0x04
#define VK321X_GX0N				0x05
#define VK321X_SCTLR			0x06
#define VK321X_SCONR			0x07
#define VK321X_SFWCR			0x08
#define VK321X_SFOCR			0x09
#define VK321X_SADR				0x0A
#define VK321X_SIER				0x0B
#define VK321X_SIFR				0x0C
#define VK321X_SSR				0x0D
#define VK321X_SFSR				0x0E
#define VK321X_SFDR				0x0F



//Private Macros
#define vk321x_Rst(x)			sys_GpioSet(&tbl_bspVk321x[0], (x))



//Private Variables
static p_dev_uart vk321x_port;
static p_dev_uart vk321x_uart_dev[4];
#if VK321X_LOCK_ENABLE
static os_sem vk321x_sem;
#endif

//-------------------------------------------------------------------
//Internal Functions
//-------------------------------------------------------------------
#if VK321X_LOCK_ENABLE
#define vk321x_Lock()			rt_sem_take(&vk321x_sem, RT_WAITING_FOREVER)
#define vk321x_Unlock()			rt_sem_release(&vk321x_sem)
#else
#define vk321x_Lock()
#define vk321x_Unlock()
#endif




#if 0
static sys_res vk321x_RegWrite(uint_t nChl, uint_t nReg, uint_t nData)
{

	nReg |= ((nData << 8) | 0x80 | (nChl << 4));
	return uart_Send(vk321x_port, &nReg, 2);
}

static int vk321x_RegRead(uint_t nChl, uint_t nReg)
{
	uint_t i;
	int res = -1;
	buf b = {0};

	nReg |= (nChl << 4);
	for (i = VK321X_TIMEOUT / 200; i; i--)
		if (uart_Send(vk321x_port, &nReg, 1) == SYS_R_OK)
			if (uart_RecData(vk321x_port, b, 200) == SYS_R_OK) {
				res = b->p[0];
				buf_Release(b);
				break;
			}
	return res;
}
#else
static sys_res vk321x_RegWrite(uint_t nChl, uint_t nReg, uint_t nData)
{

	nReg |= ((nData << 8) | 0x80 | (nChl << 4));
	uart_Send(vk321x_port, &nReg, 2);
	return SYS_R_OK;
}

static int vk321x_RegRead(uint_t nChl, uint_t nReg)
{

	nReg |= (nChl << 4);
	uart_Send(vk321x_port, &nReg, 1);
	return arch_UartGetChar(VK321X_COMID);
}
#endif

static uint_t vk321x_BaudReg(uint_t nBaud)
{
	uint_t n = 0, i;

	nBaud /= VK312X_MFREQ;
	for (n = 38400, i = 0; i < 16; i++) {
		if (i == 8)
			n = 115200;
		if (n == nBaud)
			break;
		n >>= 1;
	}
	return i;
}

static sys_res vk321x_Reset()
{
	int res;
	uint_t nReg;

	//Reset
	vk321x_Rst(0);
	sys_Delay(2000);
	vk321x_Rst(1);

	vk321x_Lock();
	uart_Config(vk321x_port, 4800 * VK312X_MFREQ, UART_PARI_NO, UART_DATA_8D, UART_STOP_1D);
	//全局中断使能
	vk321x_RegWrite(0, VK321X_GIR, 0xF0);
	//主UART口设置为高波特率
	nReg = vk321x_BaudReg(115200) << 4;
	vk321x_RegWrite(0, VK321X_GUCR, nReg);
	os_thd_Sleep(100);
	uart_Config(vk321x_port, 115200, UART_PARI_NO, UART_DATA_8D, UART_STOP_1D);
	res = vk321x_RegRead(0, VK321X_GUCR);
	vk321x_Unlock();
	if (res == nReg)
		return SYS_R_OK;
	return SYS_R_ERR;
}


#if 0
static void _vk321x_ItHandler()
{
	p_dev_uart pUart;
	p_uart_def pDef;
	int res;
	uint_t i, nLen, nGIR, nReg;
	buf b = {0};

	if ((res = vk321x_RegRead(0, VK321X_GIR)) == -1)
		return;
	nGIR = res;
	for (pDef = tbl_bspUartDef, i = 0; pDef < ARR_ENDADR(tbl_bspUartDef); pDef++, i++) {
		if (pDef->type != UART_T_VK321X)
			continue;
		pUart = uart_Dev(i);
		if (nGIR & BITMASK(pDef->id)) {
			while (1) {
				//Rx Fifo Not Empty
				if ((res = vk321x_RegRead(pDef->id, VK321X_SSR)) == -1)
					return;
				if (res & BITMASK(0))
					break;
				//Get rx fifo num
				if ((res = vk321x_RegRead(pDef->id, VK321X_SFSR)) == -1)
					return;
				nLen = (res - 1) & 0x0F;
				nReg = 0x40 | (pDef->id << 4) | nLen;
				if (uart_Send(vk321x_port, &nReg, 1) == SYS_R_OK)
					if (uart_RecLength(vk321x_port, b, nLen + 1, VK321X_TIMEOUT) == SYS_R_OK) {
#if IO_BUF_TYPE == BUF_T_BUFFER
						buf_Push(pUart->bufrx, b->p, nLen + 1);
#else
						dque_Push(dqueue, pUart->parent->id | UART_DQUE_RX_CHL, b->p, nLen + 1);
#endif
						buf_Release(b);
					}
			}
		}
	}
}
#else
static void _vk321x_ItHandler()
{
	p_dev_uart p;
	int res;
	uint_t i, nLen, nGIR, nReg;

	while(1) {
		if ((res = vk321x_RegRead(0, VK321X_GIR)) == -1)
			return;
		nGIR = res;
		if ((nGIR & 0x0F) == 0)
			break;
		for (i = 0; i < ARR_SIZE(vk321x_uart_dev); i++) {
			p = vk321x_uart_dev[i];
			if (nGIR & BITMASK(p->def->id)) {
				while (1) {
					//Rx Fifo Not Empty
					if ((res = vk321x_RegRead(p->def->id, VK321X_SSR)) == -1)
						return;
					if (res & BITMASK(0))
						break;
					//Get rx fifo num
					if ((res = vk321x_RegRead(p->def->id, VK321X_SFSR)) == -1)
						return;
					nLen = (res - 1) & 0x0F;
					nReg = 0x40 | (p->def->id << 4) | nLen++;
					uart_Send(vk321x_port, &nReg, 1);
					for (; nLen; nLen--) {
						res = arch_UartGetChar(VK321X_COMID);
#if IO_BUF_TYPE == BUF_T_BUFFER
						buf_Push(p->bufrx, &res, 1);
#else
						dque_Push(dqueue, p->parent->id | UART_DQUE_RX_CHL, &res, 1);
#endif
					}
				}
			}			
		}
	}
}
#endif



static void vk321x_ItHandler()
{

	vk321x_Lock();	
	_vk321x_ItHandler();
	vk321x_Unlock();
}



//-------------------------------------------------------------------
//External Functions
//-------------------------------------------------------------------
sys_res vk321x_Init()
{
	p_dev_uart pUart;

#if VK321X_LOCK_ENABLE
	rt_sem_init(&vk321x_sem, "vk321x", 1, RT_IPC_FLAG_FIFO);
#endif
	sys_GpioConf(&tbl_bspVk321x[0]);
	sys_GpioConf(&tbl_bspVk321x[1]);
	for (pUart = dev_Uart; pUart < ARR_ENDADR(dev_Uart); pUart++)
		if (pUart->def->type == UART_T_VK321X)
			vk321x_uart_dev[pUart->def->id] = pUart;
	while ((vk321x_port = uart_Get(VK321X_COMID, 1000)) == NULL);
	vk321x_Reset();
	return irq_ExtRegister(VK321X_INTID, tbl_bspVk321x[1].port, tbl_bspVk321x[1].pin, IRQ_TRIGGER_FALLING, vk321x_ItHandler, IRQ_MODE_HALF);
}

sys_res vk321x_UartConfig(uint_t nChl, t_uart_para *pPara)
{
	uint_t nReg = 0;

	switch (pPara->pari) {
	case UART_PARI_EVEN:
		nReg = BITMASK(6) | BITMASK(4);
		break;
	case UART_PARI_ODD:
		nReg = BITMASK(6) | BITMASK(3);
		break;
	default:
		break;
	}
	if (pPara->stop == UART_STOP_2D)
		nReg |= BITMASK(7);
	vk321x_Lock();
	vk321x_RegWrite(nChl, VK321X_SCONR, nReg);
	//波特率设置
	vk321x_RegWrite(nChl, VK321X_SCTLR, (vk321x_BaudReg(pPara->baud) << 4) | BITMASK(3));
	//发送接收FIFO使能
	vk321x_RegWrite(nChl, VK321X_SFOCR, BITMASK(2) | BITMASK(3));
	//接收FIFO中断使能
	vk321x_RegWrite(nChl, VK321X_SIER, BITMASK(0));
	vk321x_Unlock();
	return SYS_R_OK;
}

sys_res vk321x_UartSend(uint_t nChl, const void *pData, uint_t nLen)
{
	int nSR;
	uint_t i, n;
	const uint8_t *p = (const uint8_t *)pData;

	vk321x_Lock();
	for (; nLen; nLen -= n, p += n) {
		//wait tx fifo empty
		for (i = 600 / OS_TICK_MS; i; i--) {
			if ((nSR = vk321x_RegRead(nChl, VK321X_SSR)) == -1) {
				vk321x_Unlock();
				return SYS_R_TMO;
			}
			if (nSR & BITMASK(2))
				break;
			os_thd_Slp1Tick();
		}
		if (i == 0)
			break;
		n = MIN(nLen, 16);
		nSR = 0xC0 | (nChl << 4) | (n - 1);
		uart_Send(vk321x_port, &nSR, 1);
		uart_Send(vk321x_port, p, n);
	}
	vk321x_Unlock();
	if (nLen)
		return SYS_R_ERR;
	return SYS_R_OK;
}


#endif
