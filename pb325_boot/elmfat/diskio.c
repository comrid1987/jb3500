/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2007        */
/*-----------------------------------------------------------------------*/
/* This is a stub disk I/O module that acts as front end of the existing */
/* disk I/O modules and attach it to FatFs module with common interface. */
/*-----------------------------------------------------------------------*/

#include "diskio.h"
#include "ff.h"

/*-----------------------------------------------------------------------*/
/* Correspondence between physical drive number and physical drive.      */

#define NAND	0
#define ATA		1
#define MMC		2
#define USB		3


static DSTATUS USBStat = STA_NOINIT;    /* Disk status */


extern int g_usbDC;

/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */

DSTATUS disk_initialize (
	BYTE drv				/* Physical drive nmuber (0..) */
)
{

    /* Set the not initialized flag again. If all goes well and the disk is */
    /* present, this will be cleared at the end of the function.            */
    USBStat |= STA_NOINIT;

    /* Find out if drive is ready yet. */
    if (USBHMSCDriveReady(g_usbDC)) return(FR_NOT_READY);

    /* Clear the not init flag. */
    USBStat &= ~STA_NOINIT;

    return 0;
}



/*-----------------------------------------------------------------------*/
/* Return Disk Status                                                    */

DSTATUS disk_status (
	BYTE drv		/* Physical drive nmuber (0..) */
)
{

	return USBStat;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */

DRESULT disk_read (
	BYTE drv,		/* Physical drive nmuber (0..) */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address (LBA) */
	BYTE count		/* Number of sectors to read (1..255) */
)
{

	if (USBStat & STA_NOINIT)
		return RES_NOTRDY;

	/* READ BLOCK */
	if (USBHMSCBlockRead(g_usbDC, sector, buff, count) == 0)
		return RES_OK;
	
	return RES_ERROR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */

#if _READONLY == 0
DRESULT disk_write (
	BYTE drv,			/* Physical drive nmuber (0..) */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Sector address (LBA) */
	BYTE count			/* Number of sectors to write (1..255) */
)
{
	if (ucDrive || !count) return RES_PARERR;
	if (USBStat & STA_NOINIT) return RES_NOTRDY;
	if (USBStat & STA_PROTECT) return RES_WRPRT;
	
	/* WRITE BLOCK */
	if (USBHMSCBlockWrite(g_usbDC, sector, (unsigned char *)buff, count) == 0)
		return RES_OK;
	
	return RES_ERROR;
}
#endif /* _READONLY */



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */

DRESULT disk_ioctl (
	BYTE drv,		/* Physical drive nmuber (0..) */
	BYTE ctrl,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	

	switch (ctrl) {
	case GET_SECTOR_COUNT:
		*(DWORD *)buff = 0;
		break;
	case GET_SECTOR_SIZE:
		*(WORD *)buff = 512;
		break;
	case GET_BLOCK_SIZE:
		*(DWORD *)buff = 1;
		break;
	case CTRL_SYNC:
	default:
		break;
	}
	return RES_OK;
}

DWORD get_fattime (void)
{

	return 0;
}
