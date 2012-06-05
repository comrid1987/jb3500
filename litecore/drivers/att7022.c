#if ATT7022_ENABLE
#include <drivers/att7022.h>


//Private Defines
#define ATT7022_DEBUG_ENABLE		1

#define ATT7022_PHASECALI_ENABLE	0

#define ATT7022_SAMPLEPOINT   		128
#define ATT7022_DATA_MASK			0x00FFFFFF
#define ATT7022_DATA_DEC			8192.0f

#define ATT7022_SPI_SPEED			200000



//Private Variables
static t_att7022 att7022;


//Private Macros
#define att7022_Sig()				sys_GpioRead(gpio_node(tbl_bspAtt7022, 1))

#define att7022_WriteEnable(p)		att7022_WriteReg(p, ATT7022_CALIDATA_WEN, 0)
#define att7022_WriteDisable(p)		att7022_WriteReg(p, ATT7022_CALIDATA_WEN, 1)
#define att7022_CaliClear(p)		att7022_WriteReg(p, ATT7022_CALIDATA_CLR, 0)
#define	att7022_SWRST(p)			att7022_WriteReg(p, ATT7022_SOFTRST_CMD, 0)









//Internal Functions
#if ATT7022_DEBUG_ENABLE
#define att7022_DbgOut				dbg_trace
#else
#define att7022_DbgOut(...)
#endif



static p_dev_spi att7022_SpiGet()
{
	p_dev_spi p;
	
	p = spi_Get(SPIF_COMID, OS_TMO_FOREVER);
	spi_Config(p, SPI_SCKIDLE_LOW, SPI_LATCH_2EDGE, ATT7022_SPI_SPEED);
#if SPI_SEL_ENABLE
	spi_CsSel(p, ATT7022_CSID);
#endif
	return p;
}


static sys_res att7022_WriteReg(p_att7022 p, uint_t nReg, uint32_t nData)
{
	uint32_t nCrc1, nCrc2, nTemp;

	p->spi = att7022_SpiGet();

	//Ğ´Êı¾İ¼Ä´æÆ÷
	nData &= ATT7022_DATA_MASK;
	reverse(&nData, 3);
	nTemp = nReg | 0x80 | (nData << 8);
	spi_Send(p->spi, &nTemp, 4);
	os_thd_Slp1Tick();
	//¶ÁĞ£Ñé1¼Ä´æÆ÷
	nTemp = ATT7022_REG_WSPIData1;
	spi_Transce(p->spi, &nTemp, 1, &nCrc1, 3);
	nCrc1 &= ATT7022_DATA_MASK;
	//¶ÁĞ£Ñé2¼Ä´æÆ÷
#if 0
	nTemp = ATT7022_REG_WSPIData2;
	spi_Transce(p->spi, &nTemp, 1, &nCrc2, 3);
	nCrc2 &= ATT7022_DATA_MASK;
#else
	nCrc2 = nCrc1;
#endif

	spi_Release(p->spi);

	if ((nData != nCrc1) || (nData != nCrc2)) {
		att7022_DbgOut("<ATT7022>WriteReg %02X Err %x %x %x", nReg, nData, nCrc1, nCrc2);
		return SYS_R_ERR;
	}
	return SYS_R_OK;
}

static uint32_t att7022_PhaseCaliData(float err)
{
	uint32_t phase_v = 0;
	float seta;

	if (err) {
		seta = acos((1 + err ) * 0.5);
		seta -= 3.14 / 3;
		if (seta >= 0)
			phase_v = seta * MAX_VALUE1;
		else
			phase_v = MAX_VALUE2 + seta * MAX_VALUE1;
	}	
	return phase_v;
}







//External Functions
void att7022_Init()
{
	p_gpio_def pDef;

	for (pDef = tbl_bspAtt7022[0]; pDef < tbl_bspAtt7022[1]; pDef++)
		sys_GpioConf(pDef);
}

uint32_t att7022_ReadReg(p_att7022 p, uint_t nReg)
{
	uint32_t nData, nCrc;

	p->spi = att7022_SpiGet();

	//¶ÁÊı¾İ¼Ä´æÆ÷
	spi_Transce(p->spi, &nReg, 1, &nData, 3);
	os_thd_Slp1Tick();
	reverse(&nData, 3);
	nData &= ATT7022_DATA_MASK;
	//¶ÁĞ£Ñé¼Ä´æÆ÷	
	nReg = ATT7022_REG_RSPIData;
	spi_Transce(p->spi, &nReg, 1, &nCrc, 3);
	reverse(&nCrc, 3);
	nCrc &= ATT7022_DATA_MASK;

	spi_Release(p->spi);

	if (nData != nCrc) {
		att7022_DbgOut("<ATT7022>ReadReg %02X Err %x %x", nReg, nData, nCrc);
		nData = 0;
	}
	return nData;
}


sys_res att7022_Reset(p_att7022 p, p_att7022_cali pCali)
{
	uint32_t nTemp;
	uint_t i;

	//¸´Î»ATT7022B
	sys_GpioSet(gpio_node(tbl_bspAtt7022, 0), 1);
	sys_Delay(3000);				//delay 30us
	sys_GpioSet(gpio_node(tbl_bspAtt7022, 0), 0);
	os_thd_Slp1Tick();

	for (i = 0; i < 3; i++) {
		if (att7022_WriteEnable(p) == SYS_R_OK)
			break;
	}
	if (i >= 3)
		return SYS_R_ERR;

	att7022_CaliClear(p);

	att7022_WriteReg(p, ATT7022_REG_UADCPga, 0);			//µçÑ¹Í¨µÀADCÔöÒæÉèÖÃÎª1
	att7022_WriteReg(p, ATT7022_REG_HFConst, 16);			//ÉèÖÃHFConst
	nTemp = IB_VO * ISTART_RATIO * CONST_G * MAX_VALUE1;
	att7022_WriteReg(p, ATT7022_REG_Istartup, nTemp);		//Æô¶¯µçÁ÷ÉèÖÃ
	att7022_WriteReg(p, ATT7022_REG_EnUAngle, 0x003584);	//Ê¹ÄÜµçÑ¹¼Ğ½Ç²âÁ¿
	att7022_WriteReg(p, ATT7022_REG_EnDtIorder, 0x005678);	//Ê¹ÄÜÏàĞò¼ì²â
	att7022_WriteReg(p, ATT7022_REG_EAddMode, 0);			//ºÏÏàÄÜÁ¿ÀÛ¼ÓÄ£Ê½	
	att7022_WriteReg(p, ATT7022_REG_GCtrlT7Adc, 5); 		//¿ªÆôÎÂ¶ÈºÍADC7²âÁ¿ 	
	att7022_WriteReg(p, ATT7022_REG_EnlineFreq, 0x000000);	//½ûÖ¹»ù²¨/Ğ³²¨²âÁ¿¹¦ÄÜ

	//Ğ´Èë¹¦ÂÊÔöÒæ
	for (i = 0; i < 3; i++) {
		att7022_WriteReg(p, ATT7022_REG_PgainA0 + i, pCali->Pgain0[i]);
		att7022_WriteReg(p, ATT7022_REG_PgainA1 + i, pCali->Pgain1[i]);
	}

	//Ğ´ÈëµçÑ¹ÔöÒæ
	for (i = 0; i < 3; i++)
		att7022_WriteReg(p, ATT7022_REG_UgainA + i, pCali->Ugain[i]);

	//Ğ´ÈëµçÁ÷ÔöÒæ
	for (i = 0; i < 3; i++)
		att7022_WriteReg(p, ATT7022_REG_IgainA + i, pCali->Igain[i]);

	for (i = 0; i < 5; i++) {
		att7022_WriteReg(p, ATT7022_REG_PhsregA0 + i, pCali->PhsregA[i]);
		att7022_WriteReg(p, ATT7022_REG_PhsregB0 + i, pCali->PhsregB[i]);
		att7022_WriteReg(p, ATT7022_REG_PhsregC0 + i, pCali->PhsregC[i]);
	}

	return att7022_WriteDisable(p);
}

//------------------------------------------------------------------------
//Ãû	³Æ:  att7022_GetFreq ()
//Éè	¼Æ: 
//Êä	Èë:  -
//Êä	³ö:  -
//·µ	»Ø:  ÆµÂÊÖµ
//¹¦	ÄÜ:  »ñÈ¡ÆµÂÊÖµ
//------------------------------------------------------------------------
float att7022_GetFreq(p_att7022 p)
{
	sint32_t nData;

	nData = att7022_ReadReg(p, ATT7022_REG_Freq);
	if (nData != (-1))
		return (nData / ATT7022_DATA_DEC);
	return 0;
}


//------------------------------------------------------------------------
//Ãû	³Æ: att7022_GetVoltage ()
//Éè	¼Æ: 
//Êä	Èë: nPhase - ÏàÎ»Öµ
//Êä	³ö: -
//·µ	»Ø: µçÑ¹ÓĞĞ§Öµ
//¹¦	ÄÜ: »ñÈ¡µ±Ç°µçÑ¹
//------------------------------------------------------------------------
float att7022_GetVoltage(p_att7022 p, uint_t nPhase)
{
	uint32_t nVoltage = 0;
	float fResult = 0.0;

	nVoltage = att7022_ReadReg(p, ATT7022_REG_URmsA + nPhase);
	fResult = (float)nVoltage / ATT7022_DATA_DEC;
	return fResult;
}



//------------------------------------------------------------------------
//Ãû	³Æ: att7022_GetCurrent ()
//Éè	¼Æ: 
//Êä	Èë: nPhase - ÏàÎ»Öµ
//Êä	³ö: -
//·µ	»Ø: µçÁ÷ÓĞĞ§Öµ
//¹¦	ÄÜ: »ñÈ¡µ±Ç°µçÁ÷
//------------------------------------------------------------------------
float att7022_GetCurrent(p_att7022 p, uint_t nPhase)
{
	uint32_t nCurrent = 0;
	float fResult = 0.0;

	nCurrent = att7022_ReadReg(p, ATT7022_REG_IRmsA + nPhase);
	fResult = (float)nCurrent / (ICALI_MUL * ATT7022_DATA_DEC);
	return fResult;
}


float att7022_GetPower(p_att7022 p, uint_t nReg, uint_t nPhase)
{
	int nP = 0;
	float fResult = 3200.0f / (float)ATT7022_CONST_EC; //Âö³åÊä³öÏµÊı

	nP = att7022_ReadReg(p, nReg + nPhase);
	if (nP > MAX_VALUE1)
		nP -= MAX_VALUE2;
	if (nPhase < 3) {
		//¶ÁÈ¡·ÖÏà¹¦ÂÊ
		fResult = (float)nP * fResult / 256000.0f; //×ª»»³É¹¤³ÌÁ¿
	} else {
		//¶ÁÈ¡ºÏÏà¹¦ÂÊ
		fResult = (float)nP * fResult / 64000.0f; //×ª»»³É¹¤³ÌÁ¿
	}
	return fResult;
}

uint32_t att7022_GetFlag(p_att7022 p) 
{
	
	return (att7022_ReadReg(p,ATT7022_REG_SFlag)); //¶ÁÈ¡×´Ì¬¼Ä´æÆ÷ 
}
//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
uint8_t att7022_GetPowerDir(p_att7022 p)
{

	//Bit0-3 ·Ö±ğ±íÊ¾A B  C  ºÏÏàµÄÓĞ¹¦¹¦ÂÊµÄ·½Ïò		0 ±íÊ¾ÎªÕı	1 ±íÊ¾Îª¸º 
	//Bit4-7 ·Ö±ğ±íÊ¾A B  C  ºÏÏàµÄÎŞ¹¦¹¦ÂÊµÄ·½Ïò 	0 ±íÊ¾ÎªÕı	1 ±íÊ¾Îª¸º 
	return (att7022_ReadReg(p, ATT7022_REG_PFlag)); //¶ÁÈ¡¹¦ÂÊ·½Ïò¼Ä´æÆ÷
}
//------------------------------------------------------------------------
//Ãû	³Æ:  att7022_GetSV ()
//Éè	¼Æ: 
//Êä	Èë:  nPhase - ÏàÎ»Öµ£¬0->A £¬1->B £¬2->C £¬3->ºÏÏà
//Êä	³ö:  -
//·µ	»Ø:  ÊÓÔÚ¹¦ÂÊÖµ
//¹¦	ÄÜ:  »ñÈ¡¸÷ÏàÊÓÔÚ¹¦ÂÊ¼°ºÏÏàÊÓÔÚ¹¦ÂÊ,
//------------------------------------------------------------------------
float att7022_GetSV(p_att7022 p, uint_t nPhase)
{
	float sv, eck;

	sv = att7022_ReadReg(p,ATT7022_REG_SA + nPhase);
	eck = 3200.0f / (float)ATT7022_CONST_EC; //Âö³åÊä³öÏµÊı
	if (sv > MAX_VALUE1)
		sv -= MAX_VALUE2;
	if (nPhase < 3) {
		//¶ÁÈ¡·ÖÏà¹¦ÂÊ
		return (sv / 256000.0f * eck); //×ª»»³É¹¤³ÌÁ¿
	} else {
		//¶ÁÈ¡ºÏÏà¹¦ÂÊ
		return (sv / 64000.0f * eck); //×ª»»³É¹¤³ÌÁ¿
	}
}
//------------------------------------------------------------------------
//Ãû	³Æ:  att7022_GetPFV ()
//Éè	¼Æ: 
//Êä	Èë:  nPhase - ÏàÎ»Öµ
//Êä	³ö:  -
//·µ	»Ø:  ¹¦ÂÊÒòÊı
//¹¦	ÄÜ:  »ñÈ¡¹¦ÂÊÒòÊı
//------------------------------------------------------------------------
float att7022_GetPFV(p_att7022 p, uint_t nPhase)
{
	int nData = 0;
	float f_data = 0.0f;

	nData = att7022_ReadReg(p, ATT7022_REG_PfA + nPhase);
	if (nData & 0x00800000) {
		nData &= 0x007FFFFF;
		nData = -nData;
	} else {
		nData &= 0x007FFFFF;
	}
	f_data = nData / MAX_VALUE1;
	return f_data;
}
//------------------------------------------------------------------------
//Ãû	³Æ:  att7022_GetPAG ()
//Éè	¼Æ: 
//Êä	Èë: nPhase - ÏàÎ»Öµ
//Êä	³ö: -
//·µ	»Ø: µçÁ÷ÓÚµçÑ¹µÄ¼Ğ½Ç
//¹¦	ÄÜ: »ñÈ¡½Ç¶È
//------------------------------------------------------------------------
float att7022_GetPAG(p_att7022 p, uint_t nPhase) 
{
	float sita;
	sita = att7022_ReadReg(p,ATT7022_REG_PgA + nPhase);
	if (sita > MAX_VALUE1) {
		sita -= MAX_VALUE2;
	}
	return (sita * 360 / MAX_VALUE1 / PI);
}

//-------------------------------------------------------------------------
//µçÑ¹Ïà½Ç
//-------------------------------------------------------------------------
float att7022_GetPVAG(p_att7022 p, uint_t nPhase) 
{

	return ((float)att7022_ReadReg(p,ATT7022_REG_YUaUb + nPhase) / 8192);
}

uint16_t att7022_GETQuanrant(p_att7022 p, uint_t Phase) 
{
	uint32_t pflag;
	uint32_t p_direction, q_direction;

	pflag = att7022_ReadReg(p,ATT7022_REG_PFlag); //ÏÈ¶ÁÈ¡¹¦ÂÊ·½Ïò¼Ä´æÆ÷(ÕıÏòÎª0,¸ºÏòÎª1)
	p_direction = ((pflag >> Phase) &0x00000001);
	q_direction = ((pflag >> (Phase + 4)) &0x00000001);
	if (p_direction) {
		if (q_direction) {
			//P- Q-
			return 3; //ATT7022B  3
		} else {
			//P- Q+
			return 2; //ATT7022B  2
		}
	} else {
		if (q_direction) {
			//P+ Q-
			return 4; //ATT7022B  4
		} else {
			//P+ Q+
			return 1; //ATT7022B  1
		}
	}
}

/*=============================================================================
 * Ãû³Æ:			att7022_GetPosEP
 * ÃèÊö:			¶ÁÈ¡ÕıÏòÓĞ¹¦µçÄÜ
 * ´´½¨:			ÀµÏş·¼   2006Äê10ÔÂ23ÈÕ
 * ×îºóĞŞ¶©:		2006Äê10ÔÂ23ÈÕ 18:36:07
 * µ÷ÓÃ:			None
 * ÊäÈë²ÎÊı:		nPhase ÏàÎ»Öµ
 * ·µ»ØÖµ:		µçÄÜÖµ ÒÑÀÛ¼Ó(0.01kwhÎª»ùÊı)
 * ÌØÊâËµÃ÷:		RVMDK 3.01a
 *=============================================================================*/
uint32_t att7022_GetPosEP(p_att7022 p, uint_t nPhase)
{

	return (att7022_ReadReg(p,ATT7022_REG_PosEpA2 + nPhase)); //¶ÁÈ¡µÍÓÚ¼ÆÊı³£ÊıµÄ±£´æÖµ;
}

/*=============================================================================
 * Ãû³Æ:			att7022_GetNegEP
 * ÃèÊö:			¶ÁÈ¡·´ÏòÓĞ¹¦µçÄÜÖµ
 * ´´½¨:			ÀµÏş·¼   2006Äê10ÔÂ24ÈÕ
 * ×îºóĞŞ¶©:		2006Äê10ÔÂ24ÈÕ 9:00:20
 * µ÷ÓÃ:			None
 * ÊäÈë²ÎÊı:		nPhase ÏàÎ»Öµ
 * ·µ»ØÖµ:		µçÄÜÖµ ÒÑÀÛ¼Ó(0.01kwhÎª»ùÊı)
 * ÌØÊâËµÃ÷:		RVMDK 3.01a
 *=============================================================================*/
uint32_t att7022_GetNegEP(p_att7022 p, uint_t nPhase)
{

	return att7022_ReadReg(p,ATT7022_REG_NegEpA2 + nPhase); //¶ÁÈ¡µÍÓÚ¼ÆÊı³£ÊıµÄ±£´æÖµ;	
}

/*=============================================================================
 * Ãû³Æ:			att7022_GetPhaseEQ
 * ÃèÊö:			¶ÁÈ¡·ÖÏàÎŞ¹¦µçÄÜÁ¿
 * ´´½¨:			ÀµÏş·¼   2006Äê10ÔÂ27ÈÕ
 * ×îºóĞŞ¶©:		2006Äê10ÔÂ27ÈÕ 17:43:40
 * µ÷ÓÃ:			None
 * ÊäÈë²ÎÊı:		None
 * ·µ»ØÖµ:			None
 * ÌØÊâËµÃ÷:		RVMDK 3.01a
 *=============================================================================*/
uint32_t att7022_GetPhaseEQ(p_att7022 p, uint_t nQuad, uint_t nDir, uint_t nPhase) 
{
	uint32_t EQ;

	if (nQuad < 3) {
		EQ = att7022_ReadReg(p,ATT7022_REG_PosEqA2 + nPhase);
	} else {
		EQ = att7022_ReadReg(p,ATT7022_REG_NegEqA2 + nPhase);
	}

	return EQ;
}

/**************************
 * Ãû³Æ:			att7022_GetHarmonic
 * ÃèÊö:			¶ÁÈ¡¸÷Í¨µÀĞ³²¨
 * ÊäÈë²ÎÊı:		Ch(0--B)  ·Ö±ğ¶ÔÓ¦ÏÂÃæ¸÷Í¨µÀ£¬²ÉÑùÂÊ3.2k
 Ua\Ia\Ub\Ib\Uc\Ic\In\Ua+Ia\Ub+Ib\Uc+Ic\Ua+Ub+Uc\Ia+Ib+Ic
 * ·µ»ØÖµ:			
 * ÌØÊâËµÃ÷:		RVMDK 3.01a
***************************/
sys_res att7022_GetHarmonic(p_att7022 p, uint_t Ch, sint16_t *pbuf)
{
	uint_t i, nData, nReg = 0x7f;

	nData=0xccccc0|Ch;
	att7022_WriteReg(p, 0xc0, nData);
	for (i = 20; i; i--) {   //Ò»°ãÖØ¸´3´Î¾ÍĞĞÁË¡
		if(att7022_ReadReg(p, 0x7e)>=240)
			break;
		os_thd_Sleep(30);
	}
	if (i) {
		att7022_WriteReg(p, 0xc1, 0);
		for (i = 0; i < ATT7022_SAMPLEPOINT; i++) {
			spi_Transce(p->spi, &nReg, 1, &nData, 3);
			reverse(&nData, 3);
			nData &= ATT7022_DATA_MASK;		
			pbuf[i] = (sint16_t)(nData >> 8);
		}
		att7022_WriteReg(p, 0xC0, 0);
		return SYS_R_OK;
	}
	return SYS_R_ERR;
}

//------------------------------------------------------------------------ 
//Ãû ³Æ: att7022_UgainCalibration () 
//Éè ¼Æ: 
//Êä Èë: nPhase - ´ıĞ£×¼µÄÏà, (0 - AÏà, 1 - BÏà, 2 - CÏà) 
//Êä ³ö: - 
//·µ »Ø: gain - Ğ£×¼³É¹¦·µ»ØUgainµÄµ±Ç°Öµ 
//¹¦ ÄÜ: µçÑ¹²ÎÊıUgainĞ£×¼
//------------------------------------------------------------------------ 
uint32_t att7022_UgainCalibration(p_att7022 p, uint8_t nPhase )
{
	float urms = 0.0f; 
	float k = 0.0f; 
	sint32_t gain = 0; 

	//Ğ£±íÊ¹ÄÜ  
	att7022_WriteEnable(p);
	//ÉèÖÃµçÑ¹Ğ£Õı¼Ä´æÆ÷Îª0
	att7022_WriteReg(p, ATT7022_REG_UgainA + nPhase, 0 ); 
	//Ğ£±í½ûÖ¹ 
	att7022_WriteDisable(p); 
	//µÈ1Ãë
	os_thd_Sleep(1000);
	//¶ÁÈ¡¸ÃÏàµçÑ¹Öµ 
	urms = att7022_ReadReg(p, ATT7022_REG_URmsA + nPhase);
	
	if (urms )
	{
		 //¼ÆËãÊµ¼Ê²â³öµÄ¹¤³ÌÁ¿
		urms = urms / SYS_KVALUE; 
		//´Ë´¦ÒÔUCALI_CONSTÎª±ê×¼¼ÆËãUgain 
		k = UCALI_CONST / urms - 1; 

		if (k >= 0 )
		{
			//Ur / Urms ´óÓÚµÈÓÚ0 
			gain = k * MAX_VALUE1;
		}
		else
		{
			//Ur / Urms Ğ¡ÓÚ0 
			gain = k * MAX_VALUE1; 
			gain = MAX_VALUE2 + gain;
		} 

		//Ğ£±íÊ¹ÄÜ  
		att7022_WriteEnable(p);
		//Ğ´ÈëµçÑ¹Ğ£±íÊı¾İ
		att7022_WriteReg (p, ATT7022_REG_UgainA + nPhase, gain);
		//Ğ£±í½ûÖ¹
		att7022_WriteDisable(p); 
	}
	return (uint32_t)gain;
} 

//------------------------------------------------------------------------ 
//Ãû ³Æ: att7022_IgainCalibration () 
//Éè ¼Æ: 
//Êä Èë: nPhase - ´ıĞ£×¼µÄÏà, (0 - AÏà, 1 - BÏà, 2 - CÏà) 
//Êä ³ö: - 
//·µ »Ø: Igain - Ğ£×¼³É¹¦·µ»ØIgainµÄµ±Ç°Öµ 
//¹¦ ÄÜ: µçÑ¹²ÎÊıIgainĞ£×¼
//------------------------------------------------------------------------ 
uint32_t att7022_IgainCalibration(p_att7022 p, uint32_t nPhase)
{
	float irms = 0.0f; 
	float k = 0.0f; 
	float gain = 0.0f; 

	//Ğ£±íÊ¹ÄÜ  
	att7022_WriteEnable(p); 
	//ÉèÖÃµçÑ¹Ğ£Õı¼Ä´æÆ÷Îª0
	att7022_WriteReg(p, ATT7022_REG_IgainA + nPhase, 0 ); 
	//Ğ£±í½ûÖ¹ 
	att7022_WriteDisable(p);  
	//µÈ1Ãë
	os_thd_Sleep(1000);
	//¶ÁÈ¡¸ÃÏàµçÑ¹Öµ 
	irms = att7022_ReadReg(p, ATT7022_REG_IRmsA + nPhase);

	if (irms )
	{
		irms = irms / SYS_KVALUE; 
		//´Ë´¦ÒÔICALI_CONSTÎª±ê×¼¼ÆËãIgain	
		k = (ICALI_CONST * ICALI_MUL ) / irms - 1;	

		if (k >= 0 )
		{	
			//Ir / Irms ´óÓÚµÈÓÚ0 
			gain = k * MAX_VALUE1;
		}
		else
		{
			//Ir / Irms Ğ¡ÓÚ0 
			gain = k * MAX_VALUE1; 
			gain = MAX_VALUE2 + gain;
		} 
		//Ğ£±íÊ¹ÄÜ  
		att7022_WriteEnable(p); 
		//Ğ´ÈëµçÑ¹Ğ£±íÊı¾İ
		att7022_WriteReg (p, ATT7022_REG_IgainA + nPhase, (uint32_t)gain);
		//Ğ£±í½ûÖ¹
		att7022_WriteDisable(p); 
	}
	return (uint32_t)gain;
}

//------------------------------------------------------------------------
//Ãû	³Æ: att7022_PgainCalibration () 
//Éè	¼Æ: 
//Êä	Èë: nPhase- ´ıĞ£×¼µÄÏà, (0 - AÏà, 1 - BÏà, 2 - CÏà)
//Êä	³ö: -
//·µ	»Ø:  pgain - Ğ£×¼³É¹¦·µ»ØpgainµÄµ±Ç°Öµ 
//¹¦	ÄÜ: ¹¦ÂÊ²ÎÊıpgainĞ£×¼
//------------------------------------------------------------------------
uint32_t att7022_PgainCalibration(p_att7022 p, uint8_t nPhase )
{
	float pvalue = 0.0f;
	float err = 0.0f;
	uint32_t pgain = 0.0f;
	float eck = 0.0f;

	//Âö³åÊä³öÏµÊı
	eck = 3200.0f / (float)ATT7022_CONST_EC;
	//Ğ£±íÊ¹ÄÜ  
	att7022_WriteEnable(p); 
	//ÏÈÉèÖÃPgainÎª0
	att7022_WriteReg(p, ATT7022_REG_PgainA0 + nPhase, 0);
	att7022_WriteReg(p, ATT7022_REG_PgainA1 + nPhase, 0);
	//Ğ£±í½ûÖ¹ 
	att7022_WriteDisable(p); 						
	//µÈ´ıÊıÖµÎÈ¶¨
	os_thd_Sleep(1000);
	//¶Á³ö²âÁ¿µ½µÄ¹¦ÂÊ
	pvalue = att7022_ReadReg(p, ATT7022_REG_PA + nPhase);	

	if (pvalue > MAX_VALUE1 )
	{
		pvalue = pvalue - MAX_VALUE2;
	}
	//×ª»»³É¹¤³ÌÁ¿
	pvalue = (pvalue / 256.0f) * eck;		
	//Îó²î¼ÆËã
	err = (pvalue - (float)PCALI_CONST ) / (float)PCALI_CONST ;					
	if (err )
	{
		err = -err / (1 + err);

		if (err >= 0 )
		{
			//¼ÆËãPgain
			pgain = err * MAX_VALUE1;				
		}
		else
		{
			//pgain Îª¸ºÖµ												
			pgain = MAX_VALUE2 + err * MAX_VALUE1;			//¼ÆËãPgain
		}
		//Ğ£±íÊ¹ÄÜ  
		att7022_WriteEnable(p); 
		att7022_WriteReg(p, ATT7022_REG_PgainA0 + nPhase, pgain);
		att7022_WriteReg(p, ATT7022_REG_PgainA1 + nPhase, pgain);
		//Ğ£±í½ûÖ¹
		att7022_WriteDisable(p); 
	}
			
	return (uint32_t)pgain;
}

//------------------------------------------------------------------------
//Ãû	³Æ: BSP_ATT7022B_UIP_gainCalibration () 
//Éè	¼Æ: 
//Êä	Èë: -
//Êä	³ö: -
//·µ	»Ø: -
//¹¦	ÄÜ: Ğ£ÕıU£¬I£¬PÔöÒæ
//------------------------------------------------------------------------
void att7022_UIP_gainCalibration(p_att7022 p, p_att7022_cali pCali)
{
	uint_t i;

	for(i = 0; i < 3; i++)
	{
		//µçÑ¹Í¨µÀĞ£Õı
		pCali->Ugain [i] = att7022_UgainCalibration(p, PHASE_A + i);   
		
		//µçÁ÷Í¨µÀĞ£Õı
		pCali->Igain[i] = att7022_IgainCalibration(p, PHASE_A + i );   
		
		//¹¦ÂÊĞ£Õı
		pCali->Pgain0[i] = att7022_PgainCalibration(p, PHASE_A + i );   
	}
}

uint32_t att7022_Status(p_att7022 p)
{

	return att7022_ReadReg(p, ATT7022_REG_SFlag);
}

uint32_t att7022_CheckSum1(p_att7022 p)
{

	return att7022_ReadReg(p, ATT7022_REG_ChkSum1);
}

uint32_t att7022_CheckSum2(p_att7022 p)
{

	return att7022_ReadReg(p, ATT7022_REG_ChkSum2);
}



#if 0

/*=============================================================================
 * Ãû³Æ:			att7022_GetTemp
 * ÃèÊö:			ÎÂ¶È¶ÁÈ¡
 * ´´½¨:			ÀµÏş·¼   2006Äê10ÔÂ23ÈÕ
 * ×îºóĞŞ¶©:		2006Äê10ÔÂ23ÈÕ 18:11:28
 * µ÷ÓÃ:			None
 * ÊäÈë²ÎÊı:		cali: 1--¶ÁÈ¡Î´Ğ£ÕıµÄÖµ 0--¶ÁÈ¡Ğ£ÕıºóµÄÖµ
 * ·µ»ØÖµ:		ÎÂ¶ÈÖµ(À©´ó10±¶)
 * ÌØÊâËµÃ÷:		RVMDK 3.01a
 *=============================================================================*/
float att7022_GetTemp(p_att7022 p, uint8_t nCali) {
	float fTemp;
	uint16_t nTempCali;
	fTemp = att7022_ReadReg(ATT7022_REG_TempD); //¶ÁÈ¡ÎÂ¶ÈÖµ
	if (fTemp > 128) {
		fTemp -= 256;
	}
	if (!nCali) {
		ATT7022_REG_Get(TERMINAL, 0x1061, &nTempCali);
		fTemp = nTempCali - fTemp;
	}
	return fTemp;
}


//------------------------------------------------------------------------
//Ãû	³Æ: att7022_PgainCalibration () 
//Éè	¼Æ: 
//Êä	Èë: nPhase - ´ıĞ£×¼µÄÏà, (0 - AÏà, 1 - BÏà, 2 - CÏà)
//Êä	³ö: -
//·µ	»Ø: __FALSE - Ğ£×¼Ê§°Ü, pgain - Ğ£×¼³É¹¦·µ»ØpgainµÄµ±Ç°Öµ 
//¹¦	ÄÜ: ²ÎÊıpgainĞ£×¼
//------------------------------------------------------------------------
uint32_t att7022_PgainCalibration(p_att7022 p, uint8_t nPhase) {
	float att7022_pvalue, err, eck;
	float pgain = 0;
	if (nPhase > 2) {
		return __FALSE;
	}
	eck = 3200.0f / (float)ATT7022_CONST_EC; //Âö³åÊä³öÏµÊı
	att7022_WriteEnable(); //Ğ£±íÊ¹ÄÜ
	att7022_WriteReg(ATT7022_REG_PgainA0 + nPhase, 0); //ÏÈÉèÖÃPgainÎª0
	att7022_WriteReg(ATT7022_REG_PgainA1 + nPhase, 0);
	att7022_WriteDisable(); //Ğ£±í½ûÖ¹ 
	sys_Delay(5000000); //µÈ´ıÊıÖµÎÈ¶¨
	att7022_pvalue = att7022_ReadReg(ATT7022_REG_PA + nPhase); //¶Á³ö²âÁ¿µ½µÄ¹¦ÂÊ
	if (att7022_pvalue > MAX_VALUE1) {
		att7022_pvalue -= MAX_VALUE2;
	}
	att7022_pvalue = (att7022_pvalue / 256) *eck; //×ª»»³É¹¤³ÌÁ¿
	err = (att7022_pvalue - PCALI_CONST) / PCALI_CONST; //Îó²î¼ÆËã
	if (err) {
		pgain = 0-(err / (1+err));
		if (pgain < 0) {
			//pgain Îª¸ºÖµ												
			pgain = MAX_VALUE2 + pgain * MAX_VALUE1; //¼ÆËãPgain
		} else {
			pgain *= MAX_VALUE1; //¼ÆËãPgain
		}
		att7022_WriteEnable(); //Ğ£±íÊ¹ÄÜ
		att7022_WriteReg(ATT7022_REG_PgainA0 + nPhase, (uint32_t)pgain); //¸üĞÂPgain_0²ÎÊı
		att7022_WriteReg(ATT7022_REG_PgainA1 + nPhase, (uint32_t)pgain); //¸üĞÂPgain_1²ÎÊı
		att7022_WriteDisable();
	}
	return (uint32_t)pgain;
}


//------------------------------------------------------------------------ 
//Ãû ³Æ: att7022_IgainCalibration () 
//Éè ¼Æ: 
//Êä Èë: nPhase - ´ıĞ£×¼µÄÏà, (0 - AÏà, 1 - BÏà, 2 - CÏà) 
//Êä ³ö: - 
//·µ »Ø: __FALSE - Ğ£×¼Ê§°Ü, Igain - Ğ£×¼³É¹¦·µ»ØIgainµÄµ±Ç°Öµ 
//¹¦ ÄÜ: ²ÎÊıIgainĞ£×¼ 
//------------------------------------------------------------------------ 
uint32_t att7022_IgainCalibration(p_att7022 p, uint8_t nPhase) {
	float irms, k, gain;
	if (nPhase > 2) {
		if (nPhase != PHASE_ADC7) {
			return __FALSE;
		}
	} //²ÎÊı´íÎó·µ»Ø 
	att7022_WriteEnable(); //Ğ£±íÊ¹ÄÜ 
	if (nPhase != PHASE_ADC7) {
		att7022_WriteReg(ATT7022_REG_IgainA + nPhase, 0);
	} //ÏÈÉèÖÃÔöÒæ·Å´óÆ÷Îª0 
	else {
		att7022_WriteReg(ATT7022_REG_gainADC7, 0);
	} //ÏÈÉèÖÃÔöÒæ·Å´óÆ÷Îª0 
	att7022_WriteDisable(); //Ğ£±í½ûÖ¹ 
	sys_Delay(5000000);
	if (nPhase != PHASE_ADC7) {
		irms = att7022_ReadReg(ATT7022_REG_IRmsA + nPhase);
	} //¶ÁÈ¡¸ÃÏàµçÁ÷Öµ 
	else {
		irms = att7022_ReadReg(ATT7022_REG_RmsADC7);
	} //¶ÁÈ¡¸ÃÏàµçÁ÷Öµ 
	if (irms) {
		irms = irms / SYS_KVALUE;
		k = (ICALI_CONST * ICALI_MUL) / irms - 1; //´Ë´¦ÒÔICALI_CONSTÎª±ê×¼¼ÆËãIgain	
		gain = k * MAX_VALUE1;
		if (k < 0) {
			gain += MAX_VALUE2;
		}
		att7022_WriteEnable(); //Ğ£±íÊ¹ÄÜ 
		if (nPhase != PHASE_ADC7) {
			att7022_WriteReg(ATT7022_REG_IgainA + nPhase, (uint32_t)gain);	//Ğ´ÈëµçÑ¹Ğ£±íÊı¾İ 
		} else {
			att7022_WriteReg(ATT7022_REG_gainADC7, (uint32_t)gain);			//Ğ´ÈëµçÑ¹Ğ£±íÊı¾İ 
		}
		att7022_WriteDisable();
		return (uint32_t)gain;
	}
	return __FALSE;
}

//------------------------------------------------------------------------ 
//Ãû ³Æ: att7022_UgainCalibration () 
//Éè ¼Æ: 
//Êä Èë: nPhase - ´ıĞ£×¼µÄÏà, (0 - AÏà, 1 - BÏà, 2 - CÏà) 
//Êä ³ö: - 
//·µ »Ø: __FALSE - Ğ£×¼Ê§°Ü, gain - Ğ£×¼³É¹¦·µ»ØUgainµÄµ±Ç°Öµ 
//¹¦ ÄÜ: ²ÎÊıUgainĞ£×¼ 
//------------------------------------------------------------------------ 
uint32_t att7022_UgainCalibration(p_att7022 p, uint8_t nPhase)
{
	float urms, k, gain;
	if (nPhase > 2) {
		if (nPhase != PHASE_ADC7) {
			return __FALSE;
		}
	} //²ÎÊı´íÎó·µ»Ø 
	att7022_WriteEnable(); //Ğ£±íÊ¹ÄÜ 
	if (nPhase != PHASE_ADC7) {
		att7022_WriteReg(ATT7022_REG_UgainA + nPhase, 0);
	} //ÏÈÉèÖÃÔöÒæ·Å´óÆ÷Îª0 
	else {
		att7022_WriteReg(ATT7022_REG_gainADC7, 0);
	} //ÏÈÉèÖÃÔöÒæ·Å´óÆ÷Îª0 
	att7022_WriteDisable(); //Ğ£±í½ûÖ¹ 
	sys_Delay(5000000);
	if (nPhase != PHASE_ADC7) {
		urms = att7022_ReadReg(ATT7022_REG_URmsA + nPhase);
	} //¶ÁÈ¡¸ÃÏàµçÑ¹Öµ 
	else {
		urms = att7022_ReadReg(ATT7022_REG_RmsADC7);
	} //¶ÁÈ¡¸ÃÏàµçÑ¹Öµ 
	if (urms) {
		urms = urms / SYS_KVALUE; //¼ÆËãÊµ¼Ê²â³öµÄ¹¤³ÌÁ¿ 
		k = UCALI_CONST / urms - 1; //´Ë´¦ÒÔUCALI_CONSTÎª±ê×¼¼ÆËãUgain 
		gain = k * MAX_VALUE1;
		if (k < 0) {
			gain += MAX_VALUE2;
		}
		att7022_WriteEnable(); //Ğ£±íÊ¹ÄÜ 
		if (nPhase != PHASE_ADC7) {
			att7022_WriteReg(ATT7022_REG_UgainA + nPhase, (uint32_t)gain);
		} //Ğ´ÈëµçÑ¹Ğ£±íÊı¾İ 
		else {
			att7022_WriteReg(ATT7022_REG_gainADC7, (uint32_t)gain);
		} //Ğ´ÈëµçÑ¹Ğ£±íÊı¾İ 
		att7022_WriteDisable();
		return (uint32_t)gain;
	}
	return __FALSE;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
void att7022_GainAD7(uint8_t channel) {
#ifdef ATT7022B_ADC7_CALI
	uint32_t gain;
	att7022_WriteEnable();
	//Ğ´ÈëÔöÒæ
//	att7022_WriteGain();
	if (channel < 6) {
		//Ğ´ÈëAD7µçÑ¹ÔöÒæ
		ATT7022_REG_Get(0xFF, 0x1020 + channel, (uint8_t*) &gain);
		att7022_WriteReg(ATT7022_REG_GainAdc7, (uint32_t)gain);
		if (channel < 3) {
			ATT7022_REG_Get(0xFF, 0x1004 + channel, (uint8_t*) &gain);
			att7022_WriteReg(ATT7022_REG_UgainA + channel, (uint32_t)gain);
			ATT7022_REG_Get(0xFF, 0x1026 + channel, (uint8_t*) &gain);
			att7022_WriteReg(ATT7022_REG_PgainA0 + channel, (uint32_t)gain);
			att7022_WriteReg(ATT7022_REG_PgainA1 + channel, (uint32_t)gain);
		} else {
			ATT7022_REG_Get(0xFF, 0x1010 + (channel - 3), (uint8_t*) &gain);
			att7022_WriteReg(ATT7022_REG_IgainA + (channel - 3), (uint32_t)gain);
			ATT7022_REG_Get(0xFF, 0x1029 + (channel - 3), (uint8_t*) &gain);
			att7022_WriteReg(ATT7022_REG_PgainA0 + (channel - 3), (uint32_t)gain);
			att7022_WriteReg(ATT7022_REG_PgainA1 + (channel - 3), (uint32_t)gain);
		}
	}
	att7022_WriteDisable();
#endif 
}


#if 0
/*=============================================================================
 * Ãû³Æ:			att7022_ADC_ChannelSelect
 * ÃèÊö:			Ñ¡ÔñADC7µÄ²ÉÑùÍ¨µÀ
 * ´´½¨:			ÀµÏş·¼   2007Äê1ÔÂ20ÈÕ
 * ×îºóĞŞ¶©:		2007Äê1ÔÂ20ÈÕ 17:25:34
 * µ÷ÓÃ:			None
 * ÊäÈë²ÎÊı:		None
 * ·µ»ØÖµ:			None
 * ÌØÊâËµÃ÷:		RVMDK 3.03a + RTL-ARM 3.03a
 *=============================================================================*/
int att7022_ADC_ChannelSelect(uint32_t nChl)
{
	uint32_t nFlag;
	int nResult = 1;

	nFlag = att7022_GetFlag();
	switch (nChl){
		case 0:
			if (nFlag & BITMASK(0)){
				//< 20 ) AÏàÊ§Ñ¹
				nResult = 0;
			} else {
				att7022_AD_Channel_Ua();
			}
			break;
		case 1:
			if (nFlag & BITMASK(1)){
				//< 20 )	bÏàÊ§Ñ¹
				nResult = 0;
			} else {
				att7022_AD_Channel_Ub();
			}
			break;
		case 2:
			if (nFlag & BITMASK(2)){
				//< 20 )	cÏàÊ§Ñ¹
				nResult = 0;
			} else {
				att7022_AD_Channel_Uc();
			}
			break;
		case 3:
			if (nFlag & BITMASK(9)){
				//< 20 )	aÏàÇ±¶¯
				nResult = 0;
			} else {
				att7022_AD_Channel_Ia();
			}
			break;
		case 4:
			if (nFlag & BITMASK(10)){
				//< 20 ) bÏàÇ±¶¯
				nResult = 0;
			} else {
				att7022_AD_Channel_Ib();
			}
			break;
		case 5:
			if (nFlag & BITMASK(11)){
				//< 20 ) cÏàÇ±¶¯
				nResult = 0;
			} else {
				att7022_AD_Channel_Ic();
			}
			break;
		default:
			nResult = 0;
			break;
	}
	return nResult;
}



//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
S16 att7022_Getadc7(void) {
	return ((S16)att7022_ReadReg(ATT7022_REG_InstADC7));
}






//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
uint16_t att7022_GETQuanrant(uint8_t Phase) {
	uint32_t pflag;
	uint32_t p_direction, q_direction;
	pflag = att7022_ReadReg(ATT7022_REG_PFlag); //ÏÈ¶ÁÈ¡¹¦ÂÊ·½Ïò¼Ä´æÆ÷(ÕıÏòÎª0,¸ºÏòÎª1)
	p_direction = ((pflag >> Phase) &0x00000001);
	q_direction = ((pflag >> (Phase + 4)) &0x00000001);
	if (p_direction) {
		if (q_direction) {
			//P- Q-
			return 3; //ATT7022B  3
		} else {
			//P- Q+
			return 2; //ATT7022B  2
		}
	} else {
		if (q_direction) {
			//P+ Q-
			return 4; //ATT7022B  4
		} else {
			//P+ Q+
			return 1; //ATT7022B  1
		}
	}
}



//------------------------------------------------------------------------
//Ãû	³Æ: att7022_IBlanceCalc ()
//Éè	¼Æ: 
//Êä	Èë: -
//Êä	³ö: -
//·µ	»Ø: ÈıÏàµçÁ÷²»Æ½ºâÂÊ(%)
//¹¦	ÄÜ: µçÁ÷²»Æ½ºâÂÊ¼ÆËã£¬Êµ¼ÊÖ±À©´ó10±¶
//------------------------------------------------------------------------
float att7022_IBlanceCalc(void) {
	float iin[3];
	float temp1;
	float temp2;
	uint32_t i;
	//¶Á³öÈıÏàµçÁ÷Öµ
	for (i = 0; i < 3; i++) {
		iin[i] = att7022_GetCurrent(PHASE_A + i) / 1000;
	} 
	temp1 = iin[0];
	if (iin[0] < iin[1]) {
		temp1 = iin[1];
	}
	if (temp1 < iin[2]) {
		temp1 = iin[2];
	} 
	//ÈıÏà×î´ó
	temp2 = (iin[0] + iin[1] + iin[2]) / 3; //ÈıÏàÆ½¾ù	
	iin[0] = (temp1 - temp2) / temp2;
	return iin[0];
}

//------------------------------------------------------------------------
//Ãû	³Æ: att7022_VBlanceCalc ()
//Éè	¼Æ: 
//Êä	Èë: -
//Êä	³ö: -
//·µ	»Ø: µçÑ¹²»Æ½ºâÂÊ(%)
//¹¦	ÄÜ: µçÑ¹²»Æ½ºâÂÊ¼ÆËã£¬Êµ¼ÊÖ±À©´ó10±¶
//------------------------------------------------------------------------
float att7022_VBlanceCalc(void) {
	int vin[3];
	int temp;
	uint32_t i;
	//¶Á³öÈıÏàµçÑ¹Öµ
	for (i = 0; i < 3; i++) {
		vin[i] = att7022_GetVoltage(PHASE_A + i);
	}
	temp = (vin[0] + vin[1] + vin[2]) / 3; //ÈıÏàÆ½¾ù
	vin[0] = fabsf(temp - vin[0]);
	vin[1] = fabsf(temp - vin[1]);
	vin[2] = fabsf(temp - vin[2]);
	if (vin[1] < vin[2]) {
		if (vin[0] < vin[2]) {
			vin[0] = vin[2];
		}
	} else {
		if (vin[0] < vin[1]) {
			vin[0] = vin[1];
		}
	} //ÈıÏà×î´ó
	vin[0] = vin[0] *100 / temp;
	return vin[0];
}

//------------------------------------------------------------------------
//Ãû	³Æ: void att7022_IstartupSet () 
//Éè	¼Æ: 
//Êä	Èë: -
//Êä	³ö: -
//·µ	»Ø: -
//¹¦	ÄÜ: ATT7022B Æô¶¯µçÁ÷ÉèÖÃ£¬Ib = 5A, Æô¶¯µçÁ÷0.2%Ib
//------------------------------------------------------------------------
void att7022_IstartupSet(void) {
	float isv, temp;
	temp = IB_VO * ISTART_RATIO;
	att7022_WriteEnable();
	isv = temp * CONST_G * MAX_VALUE1;
	att7022_WriteReg(ATT7022_REG_Istartup, (uint32_t)isv);
	att7022_WriteDisable();
}


//------------------------------------------------------------------------
//Ğ£Õıº¯Êı
//------------------------------------------------------------------------
//------------------------------------------------------------------------
//Ãû	³Æ: att7022_HFConstSet()
//Éè	¼Æ: 
//Êä	Èë: 
//Êä	³ö: 
//·µ	»Ø: 
//¹¦	ÄÜ:ÉèÖÃÂö³å³£ÊıĞ£ÍêµçÑ¹µçÁ÷ºóµ÷ÓÃ£¬
//			×°ÖÃĞèÒªÌá¹©µçÑ¹220µçÁ÷5A¡£
//------------------------------------------------------------------------
uint32_t att7022_HFConstSet(void) {
	float urms, irms, hfcost;
	urms = att7022_ReadReg(ATT7022_REG_URmsA); //¶ÁÈ¡µçÑ¹Öµ 
	irms = att7022_ReadReg(ATT7022_REG_IRmsA); //¶ÁÈ¡µçÁ÷Öµ 
	irms = irms / ICALI_MUL;
	hfcost = CONST_HF * CONST_G * CONST_G * urms * irms / (UCALI_CONST *ICALI_CONST * ATT7022_CONST_EC);
	att7022_WriteEnable();
	att7022_WriteReg(ATT7022_REG_HFConst, (uint32_t)hfcost); //ÉèÖÃHFConst
	att7022_WriteDisable();
	return ((uint32_t)hfcost);
}

//------------------------------------------------------------------------
//Ãû	³Æ: att7022_PhaseCali2Seg () 
//Éè	¼Æ: 
//Êä	Èë: nPhase - ´ıĞ£ÕıµÄÏàÎ»
//			 cali_point - Ğ£Õıµã(0 - 4)
//Êä	³ö: 
//·µ	»Ø: 
//¹¦	ÄÜ: ÏàÎ»Ğ£Õı(0.5L´¦Ğ£Õı), ´Ë´¦·ÖÁ½¶ÎĞ£Õı
//------------------------------------------------------------------------
void att7022_PhaseCali2Seg(uint8_t nPhase) {
	uint32_t phv[2];
	phv[1] = att7022_PhaseCalibration(PHASE_A, 1); //7.5A´¦Ğ£Õı
	att7022_WriteEnable(); //ATT7022BĞ£ÕıÊ¹ÄÜ
	att7022_WriteReg(ATT7022_REG_PhsregA0 + nPhase * 5, phv[1]);
	att7022_WriteReg(ATT7022_REG_PhsregA1 + nPhase * 5, phv[1]);
	att7022_WriteReg(ATT7022_REG_PhsregA2 + nPhase * 5, phv[1]);
	att7022_WriteReg(ATT7022_REG_PhsregA3 + nPhase * 5, phv[1]);
	att7022_WriteReg(ATT7022_REG_PhsregA4 + nPhase * 5, phv[1]);
	att7022_WriteDisable(); //ATT7022BĞ£Õı½ûÖ¹
}

//------------------------------------------------------------------------
//Ãû	³Æ: att7022_PhaseCalibration () 
//Éè	¼Æ: 
//Êä	Èë: nPhase - ´ıĞ£ÕıµÄÏàÎ»
//			cali_point - Ğ£Õıµã(0 - 4)
//Êä	³ö: -
//·µ	»Ø: phase_v - ·µ»Ø¼ÆËã³öµÄÏàÎ»Ğ£ÕıÖµ(³¤ÕûĞÎ) 
//¹¦	ÄÜ:ÏàÎ»Ğ£Õı, ´Ë´¦·ÖÁ½¶ÎĞ£Õı
//------------------------------------------------------------------------
uint32_t att7022_PhaseCalibration(uint8_t nPhase, uint8_t nCali) {
	uint32_t i;
	float fPhase = 0;
	switch (nCali) {
		case 0:
			att7022_WriteEnable();
			for (i = 0; i < 5; i++) {
				//Çå³ıËùÓĞĞ£Õı¼Ä´æÆ÷Öµ
				att7022_WriteReg(ATT7022_REG_PhsregA0 + (nPhase *5) + i, 0);
			}
			for (i = 0; i < 4; i++) {
				//Çå³ı¼Ä´æÆ÷Öµ
				att7022_WriteReg(ATT7022_REG_Irgion1 + i, 0);
			}
			att7022_WriteDisable();
			sys_Delay(50000);
			fPhase = att7022_FPhaseCaliData(nPhase, nCali);
			break;
		case 1:
		case 2:
		case 3:
		case 4:
			fPhase = att7022_FPhaseCaliData(nPhase, nCali);
			break;
		default:
			break;
	}
	return (uint32_t)fPhase;
}

//------------------------------------------------------------------------
//Ãû	³Æ: att7022_PhaseCaliData () 
//Éè	¼Æ: 
//Êä	Èë: nPhase - ´ıĞ£ÕıµÄÏàÎ»
//			cali_point - Ğ£Õıµã(0 - 4)
//Êä	³ö: -
//·µ	»Ø: phase_v - ·µ»Ø¼ÆËã³öµÄÏàÎ»Ğ£ÕıÖµ(Ë«¾«¶È¸¡µãÊı) 
//¹¦	ÄÜ: ÏàÎ»Ğ£ÕıÊı¾İ¼ÆËã
//------------------------------------------------------------------------

float att7022_FPhaseCaliData(uint8_t nPhase, uint8_t cali_point) {
	float phase_v = 0, att7022_pvalue, seta, err, pcali_value, eck;
	if (nPhase > 2) {
		return __FALSE;
	}
	eck = 3200.0f / (float)ATT7022_CONST_EC; //Âö³åÊä³öÏµÊı
	pcali_value = 0; //phiconst_tab[cali_point];				//ÔØÈëĞ£ÕıµãµÄÓĞ¹¦¹¦ÂÊ³£Êı
	att7022_pvalue = att7022_ReadReg(ATT7022_REG_PA + nPhase); //¶ÁÈ¡ÓĞ¹¦¹¦ÂÊÖµ
	if (att7022_pvalue > MAX_VALUE1) {
		att7022_pvalue -= MAX_VALUE2;
	}
	att7022_pvalue = (att7022_pvalue / 256) *eck; //×ª»»³É¹¤³ÌÁ¿
	err = (att7022_pvalue - pcali_value) / pcali_value; //Îó²î¼ÆËã
	if (err) {
		seta = acosf((1 + err) * 0.5);
		seta -= PI / 3;
		if (seta < 0) {
			phase_v = MAX_VALUE2 + seta * MAX_VALUE1;
		} else {
			phase_v = seta * MAX_VALUE1;
		}
	}
	return phase_v;
}

//------------------------------------------------------------------------
//Ãû	³Æ: att7022_IregionSet () 
//Éè	¼Æ: 
//Êä	Èë: currunt_value - µçÁ÷Öµ(Ò»¸ö¸¡µãÊı)
//			 field_num - ÇøÓòºÅ(0 - 3)
//Êä	³ö: -
//·µ	»Ø: __TRUE - ÉèÖÃ³É¹¦, __FALSE - ÉèÖÃÊ§°Ü
//¹¦	ÄÜ: ÏàÎ»²¹³¥ÇøÓò²ÎÊıIregionÉèÖÃ, 
//			 ¿ÉÒÔ·Ö³É5¶Î½øĞĞÏàÎ»²¹³¥,
//			 ĞèÒª ¶ÔÃ¿¸öÇøÓòµÄµçÁ÷Öµ½øĞĞÉèÖÃ, 
//			 ÇøÓòºÅÓÉfield_numÖ¸¶¨  
//------------------------------------------------------------------------
uint8_t att7022_IregionSet(float current_value, uint8_t field_num) {
	float Iregion;
	if (field_num > 3) {
		return __FALSE;
	}
	att7022_WriteEnable();
	Iregion = CONST_G * current_value * MAX_VALUE1;
	att7022_WriteReg(ATT7022_REG_Irgion1 + field_num, (uint32_t)Iregion);
	att7022_WriteDisable();
	return __TRUE;
}


//------------------------------------------------------------------------
//Ãû	³Æ: att7022_UIP_GainCalibration () 
//Éè	¼Æ: 
//Êä	Èë: -
//Êä	³ö: -
//·µ	»Ø: -
//¹¦	ÄÜ: Ğ£ÕıU£¬I£¬PÔöÒæ
//------------------------------------------------------------------------
void att7022_UIP_GainCalibration(void) {
	uint32_t nGain;
#ifdef ATT7022B_ADC7_CALI
	//AD7 Ğ£Õı
	att7022_AD_Channel_Ua();
	nGain = att7022_UgainCalibration(PHASE_ADC7);
	ATT7022_REG_Set(TERMINAL, 0x1020, (uint8_t*) &nGain, 0);
	nGain = att7022_UgainCalibration(PHASE_A);
	ATT7022_REG_Set(TERMINAL, 0x1004, (uint8_t*) &nGain, 0);
	nGain = att7022_PgainCalibration(PHASE_A);
	ATT7022_REG_Set(TERMINAL, 0x1026, (uint8_t*) &nGain, 0);
	att7022_AD_Channel_Ub();
	nGain = att7022_UgainCalibration(PHASE_ADC7);
	ATT7022_REG_Set(TERMINAL, 0x1021, (uint8_t*) &nGain, 0);
	nGain = att7022_UgainCalibration(PHASE_B);
	ATT7022_REG_Set(TERMINAL, 0x1005, (uint8_t*) &nGain, 0);
	nGain = att7022_PgainCalibration(PHASE_B);
	ATT7022_REG_Set(TERMINAL, 0x1027, (uint8_t*) &nGain, 0);
	att7022_AD_Channel_Uc();
	nGain = att7022_UgainCalibration(PHASE_ADC7);
	ATT7022_REG_Set(TERMINAL, 0x1022, (uint8_t*) &nGain, 0);
	nGain = att7022_UgainCalibration(PHASE_C);
	ATT7022_REG_Set(TERMINAL, 0x1006, (uint8_t*) &nGain, 0);
	nGain = att7022_PgainCalibration(PHASE_C);
	ATT7022_REG_Set(TERMINAL, 0x1028, (uint8_t*) &nGain, 0);
	att7022_AD_Channel_Ia();
	nGain = att7022_IgainCalibration(PHASE_ADC7);
	ATT7022_REG_Set(TERMINAL, 0x1023, (uint8_t*) &nGain, 0);
	nGain = att7022_IgainCalibration(PHASE_A);
	ATT7022_REG_Set(TERMINAL, 0x1010, (uint8_t*) &nGain, 0);
	nGain = att7022_PgainCalibration(PHASE_A);
	ATT7022_REG_Set(TERMINAL, 0x1029, (uint8_t*) &nGain, 0);
	att7022_AD_Channel_Ib();
	nGain = att7022_IgainCalibration(PHASE_ADC7);
	ATT7022_REG_Set(TERMINAL, 0x1024, (uint8_t*) &nGain, 0);
	nGain = att7022_IgainCalibration(PHASE_B);
	ATT7022_REG_Set(TERMINAL, 0x1011, (uint8_t*) &nGain, 0);
	nGain = att7022_PgainCalibration(PHASE_B);
	ATT7022_REG_Set(TERMINAL, 0x102a, (uint8_t*) &nGain, 0);
	att7022_AD_Channel_Ic();
	nGain = att7022_IgainCalibration(PHASE_ADC7);
	ATT7022_REG_Set(TERMINAL, 0x1025, (uint8_t*) &nGain, 0);
	nGain = att7022_IgainCalibration(PHASE_C);
	ATT7022_REG_Set(TERMINAL, 0x1012, (uint8_t*) &nGain, 0);
	nGain = att7022_PgainCalibration(PHASE_C);
	ATT7022_REG_Set(TERMINAL, 0x102b, (uint8_t*) &nGain, 0);
#endif 
	att7022_AD_Channel_None();
	//µçÑ¹Í¨µÀĞ£Õı
	nGain = att7022_UgainCalibration(PHASE_A);
	ATT7022_REG_Set(TERMINAL, 0x1001, (uint8_t*) &nGain, 0);
	nGain = att7022_UgainCalibration(PHASE_B);
	ATT7022_REG_Set(TERMINAL, 0x1002, (uint8_t*) &nGain, 0);
	nGain = att7022_UgainCalibration(PHASE_C);
	ATT7022_REG_Set(TERMINAL, 0x1003, (uint8_t*) &nGain, 0);
	//µçÁ÷Í¨µÀĞ£Õı
	nGain = att7022_IgainCalibration(PHASE_A);
	ATT7022_REG_Set(TERMINAL, 0x1007, (uint8_t*) &nGain, 0);
	nGain = att7022_IgainCalibration(PHASE_B);
	ATT7022_REG_Set(TERMINAL, 0x1008, (uint8_t*) &nGain, 0);
	nGain = att7022_IgainCalibration(PHASE_C);
	ATT7022_REG_Set(TERMINAL, 0x1009, (uint8_t*) &nGain, 0);
	//¹¦ÂÊĞ£Õı
	nGain = att7022_PgainCalibration(PHASE_A);
	ATT7022_REG_Set(TERMINAL, 0x1013, (uint8_t*) &nGain, 0);
	nGain = att7022_PgainCalibration(PHASE_B);
	ATT7022_REG_Set(TERMINAL, 0x1014, (uint8_t*) &nGain, 0);
	nGain = att7022_PgainCalibration(PHASE_C);
	ATT7022_REG_Set(TERMINAL, 0x1015, (uint8_t*) &nGain, 1);
}



//------------------------------------------------------------------------
//Ãû	³Æ: att7022_PhaseABC_Calibration () 
//Éè	¼Æ: 
//Êä	Èë: -
//Êä	³ö: -
//·µ	»Ø: -
//¹¦	ÄÜ: Ğ£ÕıÏàÎ»
//------------------------------------------------------------------------
void att7022_PhaseABC_Calibration(void) {
	float err;
	int *p1,  *p2;
	p1 = (int*) &err; //CommDataArea[7];
	p2 = (int*) &err; //CommDataArea[9];
	err = (float)((*p1) / (*p2));
}

//ÔİÊ±Î´Ê¹ÓÃµÄº¯Êı
/*
float att7022_GetAD7(uint32 fat) {
	return ((float)att7022_ReadReg(ATT7022_REG_RmsADC7) / 8192 * fat);
}

//------------------------------------------------------------------------
//Ãû	³Æ: att7022_IronlossCalc ()
//Éè	¼Æ: 
//Êä	Èë: -
//Êä	³ö: -
//·µ	»Ø: ÌúËğÖµ
//¹¦	ÄÜ: ÌúËğÖµ¼ÆËã
//------------------------------------------------------------------------
float att7022_IronlossCalc(void) {
	float iron_loss, vin[3];
	uint32 i;
	//¶Á³öÈıÏàµçÑ¹Öµ
	for (i = 0; i < 3; i++) {
		vin[i] = att7022_GetVoltage(PHASE_A + i) / 10;
	}
	iron_loss = vin[0] *vin[0] + vin[1] *vin[1] + vin[2] *vin[2];
	return iron_loss;
}

//------------------------------------------------------------------------
//Ãû	³Æ: att7022_CopperlossCalc ()
//Éè	¼Æ: 
//Êä	Èë: -
//Êä	³ö: -
//·µ	»Ø: Í­ËğÖµ
//¹¦	ÄÜ: Í­ËğÖµ¼ÆËã
//------------------------------------------------------------------------
float att7022_CopperlossCalc(void) {
	float copper_loss, iin[3];
	uint32 i;
	//¶Á³öÈıÏàµçÁ÷Öµ
	for (i = 0; i < 3; i++) {
		iin[i] = att7022_GetCurrent(PHASE_A + i) / 1000;
	}
	copper_loss = iin[0] *iin[0] + iin[1] *iin[1] + iin[2] *iin[2];
	copper_loss = copper_loss * 10;
	return copper_loss;
}
*/

#endif

#endif

#endif

