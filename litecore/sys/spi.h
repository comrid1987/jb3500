#ifndef __SYS_SPI_H__
#define __SYS_SPI_H__


#ifdef __cplusplus
extern "C" {
#endif





#define SPI_S_IDLE				0
#define SPI_S_BUSY				1

#define SPI_SCKIDLE_HIGH		0
#define SPI_SCKIDLE_LOW			1

#define SPI_LATCH_1EDGE			0
#define SPI_LATCH_2EDGE			1

#define SPI_CSID_INVALID		0xFF


typedef const struct {
	uint32_t	chipid : 3,
				sckport : 3,
				sckpin : 4,
				mosiport : 3,
				mosipin : 4,
				misoport : 3,
				misopin : 4,
				nssport : 3,
				nsspin : 4,
				outmode : 1;
}t_spi_def, *p_spi_def;


typedef struct {
	dev			parent;
	uint8_t		ste;
	uint8_t		sckmode;
	uint8_t		latchmode;
	uint8_t		csid;
	uint32_t	speed;
#if !SPI_SOFTWARE
	p_spi_def	def;
#endif
}t_dev_spi, *p_dev_spi;






//External Functions
p_dev_spi spi_Get(uint_t nId, int nTmo);
sys_res spi_Release(p_dev_spi p);
sys_res spi_Config(p_dev_spi p, uint_t nSckMode, uint_t nLatch, uint_t nSpeed);
void spi_CsSel(p_dev_spi p, uint_t nId);
sys_res spi_Start(p_dev_spi p);
sys_res spi_SendChar(p_dev_spi p, uint_t nData);
sys_res spi_End(p_dev_spi p);
sys_res spi_Send(p_dev_spi p, const void *pData, uint_t nLen);
sys_res spi_Recv(p_dev_spi p, void *pRec, uint_t nLen);
sys_res spi_Transce(p_dev_spi p, const void *pCmd, uint_t nCmdLen, void *pRec, uint_t nRecLen);


#ifdef __cplusplus
}
#endif

#endif

