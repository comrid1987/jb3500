


//Private Defines
#define NFTL_LOCK_ENABLE		0


//Private Variables
#if NFTL_LOCK_ENABLE
static os_sem nftl_sem;
#endif
static struct rt_device nftl_device;

//Private Macros
#if NFTL_LOCK_ENABLE
#define nftl_lock()				rt_sem_take(&nftl_sem, RT_WAITING_FOREVER)
#define nftl_unlock()			rt_sem_release(&nftl_sem)
#else
#define nftl_lock()
#define nftl_unlock()
#endif

static rt_err_t nftl_init(rt_device_t dev)
{
	int i = 0;

#if NFTL_LOCK_ENABLE
	rt_sem_init(&nftl_sem, "sem_nf0", 1, RT_IPC_FLAG_FIFO);
#endif
/////////////Unfinished//////////////////
	if (i)
		ml_format();
//////////////////////////////////////
	if (ml_init())
		return RT_ERROR;
	return RT_EOK;
}

static rt_err_t nftl_open(rt_device_t dev, rt_uint16_t oflag)
{

	return RT_EOK;
}

static rt_err_t nftl_close(rt_device_t dev)
{

	return RT_EOK;
}

static rt_size_t nftl_read(rt_device_t dev, rt_off_t pos, void* buffer, rt_size_t size)
{
	uint8_t *p, *pEnd;

	nftl_lock();
	if (ml_open(pos, size, ML_READ) == 0) {
		p = (uint8_t *)buffer;
		pEnd = p + size * _MAX_SS;
		for (; p < pEnd; p += _MAX_SS)
			if (ml_read(p))
				break;
		ml_close();
	}
	nftl_unlock();
	return (p - (uint8_t *)buffer);
}

static rt_size_t nftl_write (rt_device_t dev, rt_off_t pos, const void* buffer, rt_size_t size)
{
	uint8_t *p, *pEnd;

	nftl_lock();
	if (ml_open(pos, size, ML_WRITE) == 0) {
		p = (uint8_t *)buffer;
		pEnd = p + size * _MAX_SS;
		for (; p < pEnd; p += _MAX_SS)
			if (ml_write(p))
				break;
		ml_close();
	}
	nftl_unlock();
	return (p - (uint8_t *)buffer);
}

static rt_err_t nftl_control(rt_device_t dev, rt_uint8_t cmd, void *args)
{
	struct rt_device_blk_geometry *p = (struct rt_device_blk_geometry *)args;

	switch (cmd) {
	case RT_DEVICE_CTRL_BLK_GETGEOME:
		p->sector_count = ml_getmaxsector() * MAX_PAGE_PER_BLOCK;
		p->bytes_per_sector = MAX_DATA_SIZE;
		p->block_size = MAX_PAGE_PER_BLOCK * MAX_DATA_SIZE;
		break;
	default:
		break;
	}
	return RT_EOK;
}

void nftl_dev_Init()
{

	nftl_device.type = RT_Device_Class_Block;

	/* register sdcard device */
	nftl_device.init = nftl_init;
	nftl_device.open = nftl_open;
	nftl_device.close = nftl_close;
	nftl_device.read = nftl_read;
	nftl_device.write = nftl_write;
	nftl_device.control = nftl_control;

	/* no private */
	nftl_device.user_data = RT_NULL;

	rt_device_register(&nftl_device, "nftl0", RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_STANDALONE);
}




