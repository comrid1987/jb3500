#include <litecore.h>
#include "ff.h"



//Private Defines
#define BOOT_SIZE		0x4000

#define SPIF_SEC_START	0
#define SECTOR_SIZE		0x1000

#define IAP_MAGIC_WORD	0x77982161


//Private Typedefs
struct iap_info {
	unsigned int	magic;
	unsigned int	size;
	unsigned char	reserve[120];
};

//Private Macros
#define BEEP(x) 		   		gpio_Set(0, x)			//高电平有效
#define LED_UDISK(x)    		gpio_Set(1, (x) ^ 1)	//低电平有效

//Private Variables
static FATFS g_s_fatfs;
static uint8_t spif_buf[SECTOR_SIZE];


int g_usbDC;






typedef void (*pFunction)(void);

pFunction Jump_To_Application;

__asm void __set_MSP(uint32_t mainStackPointer)
{
	msr	msp, r0
	bx	lr
}

void iap_EntryApp()
{

	/* Jump to user application */
	Jump_To_Application = (pFunction) *(volatile uint32_t*)((INTFLASH_BASE_ADR + BOOT_SIZE) + 4);
	/* Initialize user application's Stack Pointer */
	__set_MSP(*(volatile uint32_t*)(INTFLASH_BASE_ADR + BOOT_SIZE));
	Jump_To_Application();
}

__asm void rt_hw_interrupt_disable()
{
	MRS		r0, PRIMASK
	CPSID   I
	BX		LR
}





int iap_SpiFlashCheck()
{
	struct iap_info iap;
	
	spif_ReadLen(SPIF_SEC_START, 0, &iap, sizeof(iap));
	if (iap.magic == IAP_MAGIC_WORD)
		return iap.size;
	return 0;
}

void iap_Handle()
{
	FIL fp;
	int nIsUsb;
	uint_t i, j, nSize, nAdr;
	uint_t nRead;

	if (f_open(&fp, "/pb325.bin", FA_READ) == FR_OK) {
		nIsUsb = 1;
		nSize = fp.fsize;
	} else {
		nIsUsb = 0;
		nSize = iap_SpiFlashCheck();
	}
	if (nSize == 0)
		return;

	nAdr = (INTFLASH_BASE_ADR + BOOT_SIZE);
	nSize += nAdr;
	for (i = 0; nAdr < nSize; i++) {
		if (nIsUsb)
			f_read(&fp, spif_buf, SECTOR_SIZE, &nRead);
		else
			spif_ReadLen((SPIF_SEC_START + 1) + i, 0, spif_buf, SECTOR_SIZE);
		for (j = 0; j < (SECTOR_SIZE / INTFLASH_BLK_SIZE); j++, nAdr += INTFLASH_BLK_SIZE) {
			MAP_IntMasterDisable();
			LED_UDISK(1);
			arch_IntfErase(nAdr);
			LED_UDISK(0);
			arch_IntfProgram(nAdr, &spif_buf[j * INTFLASH_BLK_SIZE], INTFLASH_BLK_SIZE);
			MAP_IntMasterEnable();
		}
	}
	if (nIsUsb)
		f_close(&fp);
	spif_SecErase(SPIF_SEC_START);
}





extern void MSCCallback(unsigned long ulInstance, unsigned long ulEvent, void *pvData);
int main()
{
	uint_t i;

	sys_Start();

	g_usbDC = USBHMSCDriveOpen(0, MSCCallback);

	f_mount(0, &g_s_fatfs);

	for (i = 0; i < 500000; i++)
		USBHCDMain();

	iap_Handle();
	rt_hw_interrupt_disable();
	iap_EntryApp();
}

void lm3s_isr_usb0()
{
	extern void USB0HostIntHandler(void);

	USB0HostIntHandler();
}

