#if NRSEC3000_ENABLE
#include <drivers/nrsec3000.h>


//Private Defines


#define NRSEC3000_SPI_SPEED			330000
#define TR_INS                      0xAA



//Private Variables
static t_nrsec3000 nrsec3000;

const nrsec3000_cmd nrsec3000_GetVer = {0x00, 0x5B, 0x00, 0x00, 0x40};
const nrsec3000_cmd nrsec3000_Test = {0x80, 0xB2, 0x00, 0x00, 0x00};








//Internal Functions
#if NRSEC3000_DEBUG_ENABLE
#define nrsec3000_DbgOut				dbg_printf
#else
#define nrsec3000_DbgOut(...)
#endif


//External Functions
void nrsec3000_Init()
{
    
}


static p_dev_spi nrsec3000_SpiGet()
{
	p_dev_spi p;
	
	p = spi_Get(NRSEC3000_COMID, OS_TMO_FOREVER);
	spi_Config(p, SPI_SCKIDLE_LOW, SPI_LATCH_2EDGE, NRSEC3000_SPI_SPEED);
	return p;
}


static sys_res nrsec3000_WriteReg(p_nrsec3000 p, uint_t nReg, uint32_t nData)
{
	uint32_t nCrc1, nCrc2, nTemp;

	p->spi = nrsec3000_SpiGet();

	//Ð´Êý¾Ý¼Ä´æÆ÷
	reverse(&nData, 3);
	nTemp = nReg | 0x80 | (nData << 8);
	spi_Send(p->spi, &nTemp, 4);
	os_thd_Slp1Tick();
	//¶ÁÐ£Ñé1¼Ä´æÆ÷
	spi_Transce(p->spi, &nTemp, 1, &nCrc1, 3);
	//¶ÁÐ£Ñé2¼Ä´æÆ÷
#if 0
	nTemp = NRSEC3000_REG_WSPIData2;
	spi_Transce(p->spi, &nTemp, 1, &nCrc2, 3);
	nCrc2 &= NRSEC3000_DATA_MASK;
#else
	nCrc2 = nCrc1;
#endif

	spi_Release(p->spi);

	if ((nData != nCrc1) || (nData != nCrc2)) {
		nrsec3000_DbgOut("<NRSEC3000>WriteReg %02X Err %x %x %x", nReg, nData, nCrc1, nCrc2);
		return SYS_R_ERR;
	}
	return SYS_R_OK;
}




uint32_t nrsec3000_Read(p_nrsec3000 p, uint_t nReg)
{
	uint32_t nData, nCrc;

	p->spi = nrsec3000_SpiGet();

	//¶ÁÊý¾Ý¼Ä´æÆ÷
	spi_Transce(p->spi, &nReg, 1, &nData, 3);
	os_thd_Slp1Tick();
	reverse(&nData, 3);
	//¶ÁÐ£Ñé¼Ä´æÆ÷	
	spi_Transce(p->spi, &nReg, 1, &nCrc, 3);
	reverse(&nCrc, 3);

	spi_Release(p->spi);

	if (nData != nCrc) {
		nrsec3000_DbgOut("<NRSEC3000>ReadReg %02X Err %x %x", nReg, nData, nCrc);
		nData = 0;
	}
	return nData;
}

sys_res INS_NRSEC3000(p_nrsec3000 p, uint_t TX_INS, uint_t RX_INS, uint_t nRetry)
{
    uint8_t ntemp;
    for(; nRetry; nRetry--)
    {
        spi_Transce(p->spi, &TX_INS, 1, &ntemp, 1);
        if(ntemp == RX_INS)
            return SYS_R_OK;
    }
    return SYS_R_ERR;
}



sys_res nrsec3000_Getver(p_nrsec3000 p, uint_t nReg)
{
    uint32_t nData, nCrc;
    uint8_t ntemp;
    uint_t nRetry = 1000;
    uint_t tx_ins = 0xAA;
    p->spi = nrsec3000_SpiGet();


    spi_Send(p->spi, &nrsec3000_GetVer, 5);
    spi_Send(p->spi, &nrsec3000_GetVer, 5);    
    spi_Send(p->spi, &nrsec3000_GetVer, 5);    
    for(; nRetry; nRetry--)
    {
        spi_Transce(p->spi, &tx_ins, 1, &ntemp, 1);
        if(ntemp == 0x5B)
            return SYS_R_OK;
    }
//    INS_NRSEC3000(p, 0xAA, 0X5B,1000);
//    INS_NRSEC3000(p, 0xAA, 65, 1000);
//    INS_NRSEC3000(p, 0xAA, 0X5B,65);

    return SYS_R_ERR;
}


void nrsec3000_test(void)
{
    p_nrsec3000 p;
    uint32_t nData;
    nrsec3000_Getver(p, nData);
}




#endif

