#ifndef __SYS_I2C_H__
#define __SYS_I2C_H__



#ifdef __cplusplus
extern "C" {
#endif





#define I2C_S_IDLE			0

#define I2C_S_W_SB			1
#define I2C_S_W_ADR			2
#define I2C_S_W_TXE			3
#define I2C_S_W_STOP		4
#define I2C_S_W_SUCCESS		5

#define I2C_S_R_SB			9
#define I2C_S_R_ADR			10
#define I2C_S_RXNE			11

#define I2C_S_BTF			16

#define I2C_ADR_7BIT		0
#define I2C_ADR_10BIT		1

typedef const struct {
	uint16_t	sclport : 3,
				sclpin : 4,
				sdaport : 3,
				sdapin : 4,
				adrmode : 2;
	uint32_t	speed;
}t_i2c_def, *p_i2c_def;


#if IO_BUF_TYPE == BUF_T_DQUEUE
#define I2C_DQUE_RX_CHL		0x10
#define I2C_DQUE_TX_CHL		0x18
#endif

typedef struct {
	dev			parent;
#if !I2C_SOFTWARE
	p_i2c_def	def;
#endif
	uint16_t	ste : 6,
				adr : 10;
#if !I2C_SOFTWARE
	uint16_t	wlen;
	uint16_t	rlen;
#if IO_BUF_TYPE == BUF_T_BUFFER
	buf			buftx;
	buf			bufrx;
#endif
#endif
}t_dev_i2c, *p_dev_i2c;





//External Functions
p_dev_i2c i2c_Get(uint_t nId, int nTmo);
sys_res i2c_Release(p_dev_i2c p);
sys_res i2c_Write(p_dev_i2c p, uint_t nDev, const void *pData, uint_t nLen);
sys_res i2c_Read(p_dev_i2c p, uint_t nDev, void *pData, uint_t nLen);
sys_res i2c_WriteByte(p_dev_i2c p, uint_t nDev, uint_t nAdr, uint_t nData);
sys_res i2c_WriteAdr(p_dev_i2c p, uint_t nDev, uint_t nAdr);



#ifdef __cplusplus
}
#endif

#endif


