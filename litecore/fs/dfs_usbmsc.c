

//Private Typedef



//Private Defines
#define USB_LOCK_ENABLE			0

/* Disk Status Bits (DSTATUS) */
#define STA_NOINIT		0x01	/* Drive not initialized */
#define STA_NODISK		0x02	/* No medium in the drive */
#define STA_PROTECT		0x04	/* Write protected */


//Private Variables
#if USB_LOCK_ENABLE
static os_sem usbmsc_sem;
#endif
static struct rt_device usbmsc_device;

//Private Macros
#if USB_LOCK_ENABLE
#define usbmsc_lock()			rt_sem_take(&usbmsc_sem, RT_WAITING_FOREVER)
#define usbmsc_unlock()			rt_sem_release(&usbmsc_sem)
#else
#define usbmsc_lock()
#define usbmsc_unlock()
#endif



//-----Unfinished----------------------------------------------------
static uint_t usbmsc_state = STA_NOINIT;
//-----------------------------------------------------------------


static rt_err_t usbmsc_init(rt_device_t dev)
{

#if NFTL_LOCK_ENABLE
	rt_sem_init(&usbmsc_sem, "sem_u0", 1, RT_IPC_FLAG_FIFO);
#endif
	return RT_EOK;
}

static rt_err_t usbmsc_open(rt_device_t dev, rt_uint16_t oflag)
{
	extern void MSCCallback(unsigned long ulInstance, unsigned long ulEvent, void *pvData);

	dev->user_data = (void *)USBHMSCDriveOpen(0, MSCCallback);
	return RT_EOK;
}

static rt_err_t usbmsc_close(rt_device_t dev)
{

	USBHMSCDriveClose((unsigned long)dev->user_data);
	usbmsc_state |= STA_NOINIT;
	return RT_EOK;
}

static rt_size_t usbmsc_read(rt_device_t dev, rt_off_t pos, void* buffer, rt_size_t size)
{
	int res;

	usbmsc_lock();
	if (usbmsc_state & STA_NOINIT) {
		if (USBHMSCDriveReady((uint_t)dev->user_data)) {
			usbmsc_unlock();
			return 0;
		}
		usbmsc_state &= ~STA_NOINIT;
	}
	/* READ BLOCK */
	res = USBHMSCBlockRead((uint_t)dev->user_data, pos, buffer, size);
	usbmsc_unlock();
	if (res)
		return 0;
	return size;
}

static rt_size_t usbmsc_write (rt_device_t dev, rt_off_t pos, const void* buffer, rt_size_t size)
{
	int res;

    if (usbmsc_state & STA_NOINIT)
		return 0;
	if (usbmsc_state & STA_PROTECT)
		return 0;
	usbmsc_lock();
	/* WRITE BLOCK */
	res = USBHMSCBlockWrite((uint_t)dev->user_data, pos, (uint8_t *)buffer, size);
	usbmsc_unlock();
	if (res)
		return 0;
	return size;
}

static rt_err_t usbmsc_control(rt_device_t dev, rt_uint8_t cmd, void *args)
{

	struct rt_device_blk_geometry *p = (struct rt_device_blk_geometry *)args;

	switch (cmd) {
	case RT_DEVICE_CTRL_BLK_GETGEOME:
		p->sector_count = 512;
		p->bytes_per_sector = 512;
		p->block_size = 512;
		break;
	default:
		break;
	}
	return RT_EOK;
}

void usbmsc_dev_Init()
{
	usbmsc_device.type = RT_Device_Class_Block;

	/* register usbmsc device */
	usbmsc_device.init = usbmsc_init;
	usbmsc_device.open = usbmsc_open;
	usbmsc_device.close = usbmsc_close;
	usbmsc_device.read = usbmsc_read;
	usbmsc_device.write = usbmsc_write;
	usbmsc_device.control = usbmsc_control;

	/* no private */
	usbmsc_device.user_data = RT_NULL;

	rt_device_register(&usbmsc_device, "usbmsc0", RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_STANDALONE);
}



