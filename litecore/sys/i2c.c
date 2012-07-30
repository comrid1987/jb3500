

//In litecore.c
#if IO_BUF_TYPE == BUF_T_DQUEUE
extern dque dqueue;
#endif


//Private Variables
static t_dev_i2c dev_I2c[BSP_I2C_QTY];






//-------------------------------------------------------------------------
//Function Name  : 
//Description    : 
//Input          : - GPIOx:
//                - GPIO_InitStruct: 
//Output         : None
//Return         : None
//-------------------------------------------------------------------------
p_dev_i2c i2c_Get(uint_t nId, int nTmo)
{
	p_dev_i2c p = &dev_I2c[nId];

	if (dev_Get(p->parent, nTmo) != SYS_R_OK)
		return NULL;
	return p;
}


//-------------------------------------------------------------------------
//Function Name  : 
//Description    : 
//Input          : - GPIOx:
//                - GPIO_InitStruct: 
//Output         : None
//Return         : None
//-------------------------------------------------------------------------
sys_res i2c_Release(p_dev_i2c p)
{

	return dev_Release(p->parent);
}


//-------------------------------------------------------------------------
//Function Name  : 
//Description    : 
//Input          : - GPIOx:
//                - GPIO_InitStruct: 
//Output         : None
//Return         : None
//-------------------------------------------------------------------------
sys_res i2c_Write(p_dev_i2c p, uint_t nDev, const void *pData, uint_t nLen)
{
	sys_res res;

	if (p->ste == I2C_S_IDLE) {
		p->adr = nDev;
		p->ste = I2C_S_W_SB;
#if I2C_SOFTWARE
		res = i2cbus_Write(p, p->adr, pData, nLen);
#else
#if I2C_IRQ_ENABLE
#if IO_BUF_TYPE == BUF_T_BUFFER
		buf_Push(&p->buftx, pData, nLen);
#elif IO_BUF_TYPE == BUF_T_DQUEUE
		dque_Push(dqueue, p->parent->id | I2C_DQUE_TX_CHL, pData, nLen);
#endif
		p->wlen = nLen;
		arch_I2cStart(p);
#else
		res = arch_I2cWrite(p, p->adr, pData, nLen);
#endif
#endif
		p->ste = I2C_S_IDLE;
		return res;
	}
	return SYS_R_BUSY;
}



//-------------------------------------------------------------------------
//Function Name  : 
//Description    : 
//Input          : - GPIOx:
//                - GPIO_InitStruct: 
//Output         : None
//Return         : None
//-------------------------------------------------------------------------
sys_res i2c_Read(p_dev_i2c p, uint_t nDev, void *pData, uint_t nLen)
{
	sys_res res;

	if (p->ste == I2C_S_IDLE) {
		p->adr = nDev;
		p->ste = I2C_S_R_SB;
#if I2C_SOFTWARE
		res = i2cbus_Read(p, p->adr, pData, nLen);
#else
#if I2C_IRQ_ENABLE

#else
		res = arch_I2cRead(p, p->adr, pData, nLen);
#endif
#endif
		p->ste = I2C_S_IDLE;
		return res;
	}
	return SYS_R_BUSY;
}


sys_res i2c_WriteByte(p_dev_i2c p, uint_t nDev, uint_t nAdr, uint_t nData)
{
	uint8_t aTemp[2];

	aTemp[0] = nAdr;
	aTemp[1] = nData;
	return i2c_Write(p, nDev, aTemp, 2);
}

sys_res i2c_WriteAdr(p_dev_i2c p, uint_t nDev, uint_t nAdr)
{

	return i2c_Write(p, nDev, &nAdr, 1);
}




