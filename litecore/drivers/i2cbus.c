#include <drivers/i2cbus.h>







//Private Defines
#define I2CBUS_LOCK_ENABLE		0
#define I2CBUS_DELAY			50
#define I2CBUS_IOMODE_CHANGE	1

//Private Macros
#if I2CBUS_LOCK_ENABLE
#define i2cbus_Lock()			os_thd_Lock()
#define i2cbus_Unlock()			os_thd_Unlock()
#else
#define i2cbus_Lock()
#define i2cbus_Unlock()
#endif




//-------------------------------------------------------------------
//Internal Functions
//-------------------------------------------------------------------
static uint_t i2cbus_Ste(p_dev_i2c p)
{
	p_gpio_def pIO = tbl_bspI2cDef[p->parent->id] + 1;

	return arch_GpioRead(pIO->port, pIO->pin);
}

static void i2cbus_Scl(p_dev_i2c p, uint_t nHL)
{
	p_gpio_def pIO = tbl_bspI2cDef[p->parent->id];

	arch_GpioSet(pIO->port, pIO->pin, nHL);
	sys_Delay(I2CBUS_DELAY);
}

static void i2cbus_Sda(p_dev_i2c p, uint_t nHL)
{
	p_gpio_def pIO = tbl_bspI2cDef[p->parent->id] + 1;

	arch_GpioSet(pIO->port, pIO->pin, nHL);
}

static void i2cbus_IoInput(p_dev_i2c p)
{
	p_gpio_def pIO = tbl_bspI2cDef[p->parent->id] + 1;

	arch_GpioConf(pIO->port, pIO->pin, GPIO_M_IN_FLOAT, GPIO_INIT_HIGH);
}

static void i2cbus_IoConf(p_dev_i2c p)
{
	p_gpio_def pIO = tbl_bspI2cDef[p->parent->id] + 1;

	arch_GpioConf(pIO->port, pIO->pin, pIO->mode, pIO->init);
}

static sys_res i2cbus_Start(p_dev_i2c p)
{

	i2cbus_Sda(p, 1);
	i2cbus_Scl(p, 1);
	if (i2cbus_Ste(p) == 0)
		return SYS_R_ERR;
	i2cbus_Sda(p, 0);
	return SYS_R_OK;
}

static void i2cbus_Stop(p_dev_i2c p)
{

	i2cbus_Scl(p, 0);
	i2cbus_Sda(p, 0);
	i2cbus_Scl(p, 1);
	i2cbus_Sda(p, 1);
	i2cbus_Scl(p, 1);
}

static sys_res i2cbus_WaitAck(p_dev_i2c p)
{
	uint_t nSda;

	i2cbus_Scl(p, 0);
#if I2CBUS_IOMODE_CHANGE
	i2cbus_IoInput(p);
#else
	i2cbus_Sda(p, 1);
#endif
	i2cbus_Scl(p, 1);
	nSda = i2cbus_Ste(p);
#if I2CBUS_IOMODE_CHANGE
	i2cbus_IoConf(p);
#endif
	if (nSda)
		return SYS_R_ERR;
	return SYS_R_OK;
}

static void i2cbus_Ack(p_dev_i2c p)
{

	i2cbus_Scl(p, 0);
	i2cbus_Sda(p, 0);
	i2cbus_Scl(p, 1);
	i2cbus_Scl(p, 0);
	i2cbus_Sda(p, 1);
}

static void i2cbus_Nack(p_dev_i2c p)
{

	i2cbus_Scl(p, 0);
	i2cbus_Sda(p, 1);
	i2cbus_Scl(p, 1);
}

static sys_res i2cbus_Send(p_dev_i2c p, uint_t nData)
{
	uint_t i;

	for (i = 8; i; i--, nData <<= 1) {
		i2cbus_Scl(p, 0);
		i2cbus_Sda(p, nData & 0x80);
		i2cbus_Scl(p, 1);
	}
	return i2cbus_WaitAck(p);
}

static uint8_t i2cbus_Recv(p_dev_i2c p)
{
	uint_t i, nData = 0;

#if I2CBUS_IOMODE_CHANGE
	i2cbus_IoInput(p);
#endif
	for (i = 8; i; i--) {
		nData <<= 1;
		i2cbus_Scl(p, 0);
		i2cbus_Scl(p, 1);
		if (i2cbus_Ste(p))
			nData |= BITMASK(0);
	}
#if I2CBUS_IOMODE_CHANGE
	i2cbus_IoConf(p);
#endif
	return nData;
}

static sys_res _i2cbus_Write(p_dev_i2c p, uint_t nDev, const uint8_t *pData, uint_t nLen)
{

	if (i2cbus_Start(p) != SYS_R_OK)
		return SYS_R_ERR;
	if (i2cbus_Send(p, nDev & BITANTI(0)) != SYS_R_OK)
		return SYS_R_ERR;
	for (; nLen; nLen--)
		if (i2cbus_Send(p, *pData++) != SYS_R_OK)
			return SYS_R_ERR;
	i2cbus_Stop(p);
	return SYS_R_OK;
}

static sys_res _i2cbus_Read(p_dev_i2c p, uint_t nDev, uint8_t *pBuf, uint_t nLen)
{

	if (i2cbus_Start(p) != SYS_R_OK)
		return SYS_R_ERR;
	if (i2cbus_Send(p, nDev | BITMASK(0)) != SYS_R_OK)
		return SYS_R_ERR;
	for (; nLen; nLen--) {
		*pBuf++ = i2cbus_Recv(p);
		if (nLen == 1)
			i2cbus_Nack(p);
		else
			i2cbus_Ack(p);
	}
	i2cbus_Stop(p);
	return SYS_R_OK;
}



//-------------------------------------------------------------------
//External Functions
//-------------------------------------------------------------------
sys_res i2cbus_Init(p_dev_i2c p)
{
	tbl_gpio_def pScl = tbl_bspI2cDef[p->parent->id];
	uint_t i;

	sys_GpioConf(pScl);
	sys_GpioConf(pScl + 1);
	for (i = 10; i; i--) {
		i2cbus_Scl(p, 0);
		sys_Delay(0x200);
		i2cbus_Scl(p, 1);
		sys_Delay(0x200);
		if (i2cbus_Ste(p))
			break;
	}
	if (i == 0)
		return SYS_R_BUSY;
	return SYS_R_OK;
}

sys_res i2cbus_Write(p_dev_i2c p, uint_t nDev, const void *pData, uint_t nLen)
{
	sys_res res;

	i2cbus_Lock();
	if (_i2cbus_Write(p, nDev, pData, nLen) != SYS_R_OK) {
		i2cbus_Init(p);
		res = SYS_R_ERR;
	} else
		res = SYS_R_OK;
	i2cbus_Unlock();
	return res;
}


sys_res i2cbus_Read(p_dev_i2c p, uint_t nDev, void *pData, uint_t nLen)
{

	i2cbus_Lock();
	if (_i2cbus_Read(p, nDev, pData, nLen) != SYS_R_OK) {
		i2cbus_Init(p);
		if (_i2cbus_Read(p, nDev, pData, nLen) != SYS_R_OK) {
			i2cbus_Init(p);
			i2cbus_Unlock();
			return SYS_R_ERR;
		}
	}
	i2cbus_Unlock();
	return SYS_R_OK;
}




