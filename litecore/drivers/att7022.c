#if ATT7022_ENABLE
#include <drivers/att7022.h>


//Private Defines
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
#define att7022_DbgOut				dbg_printf
#else
#define att7022_DbgOut(...)
#endif



static p_dev_spi att7022_SpiGet()
{
	p_dev_spi p;
	
	p = spi_Get(ATT7022_COMID, OS_TMO_FOREVER);
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

	//写数据寄存器
	nData &= ATT7022_DATA_MASK;
	reverse(&nData, 3);
	nTemp = nReg | 0x80 | (nData << 8);
	spi_Send(p->spi, &nTemp, 4);
	os_thd_Slp1Tick();
	//读校验1寄存器
	nTemp = ATT7022_REG_WSPIData1;
	spi_Transce(p->spi, &nTemp, 1, &nCrc1, 3);
	nCrc1 &= ATT7022_DATA_MASK;
	//读校验2寄存器
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

	//读数据寄存器
	spi_Transce(p->spi, &nReg, 1, &nData, 3);
	os_thd_Slp1Tick();
	reverse(&nData, 3);
	nData &= ATT7022_DATA_MASK;
	//读校验寄存器	
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

	//复位ATT7022B
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

	att7022_WriteReg(p, ATT7022_REG_UADCPga, 0);			//电压通道ADC增益设置为1
	att7022_WriteReg(p, ATT7022_REG_HFConst, pCali->HFConst);	//设置HFConst
	nTemp = IB_VO * ISTART_RATIO * CONST_G * MAX_VALUE1;
	att7022_WriteReg(p, ATT7022_REG_Istartup, nTemp);		//启动电流设置
	att7022_WriteReg(p, ATT7022_REG_EnUAngle, 0x003584);	//使能电压夹角测量
	att7022_WriteReg(p, ATT7022_REG_EnDtIorder, 0x005678);	//使能相序检测
	att7022_WriteReg(p, ATT7022_REG_EAddMode, 0);			//合相能量累加模式	
	att7022_WriteReg(p, ATT7022_REG_GCtrlT7Adc, 0); 		//关闭温度和ADC7测量 	
	att7022_WriteReg(p, ATT7022_REG_EnlineFreq, 0x007812);	//基波/谐波测量功能
	att7022_WriteReg(p, ATT7022_REG_EnHarmonic, 0x0055AA);	//基波/谐波测量功能

	//写入功率增益
	for (i = 0; i < 3; i++) {
		att7022_WriteReg(p, ATT7022_REG_PgainA0 + i, pCali->Pgain0[i]);
		att7022_WriteReg(p, ATT7022_REG_PgainA1 + i, pCali->Pgain1[i]);
	}

	//写入电压增益
	for (i = 0; i < 3; i++) {
		att7022_WriteReg(p, ATT7022_REG_UgainA + i, pCali->Ugain[i]);
	}

	//写入电流增益
	for (i = 0; i < 3; i++) {
		att7022_WriteReg(p, ATT7022_REG_IgainA + i, pCali->Igain[i]);
	}

	//写入相位角增益
	for (i = 0; i < 5; i++) {
		att7022_WriteReg(p, ATT7022_REG_PhsregA0 + i, pCali->PhsregA[i]);
		att7022_WriteReg(p, ATT7022_REG_PhsregB0 + i, pCali->PhsregB[i]);
		att7022_WriteReg(p, ATT7022_REG_PhsregC0 + i, pCali->PhsregC[i]);
	}

	return att7022_WriteDisable(p);
}

//------------------------------------------------------------------------
//名	称:  att7022_GetFreq ()
//设	计: 
//输	入:  -
//输	出:  -
//返	回:  频率值
//功	能:  获取频率值
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
//名	称: att7022_GetVoltage ()
//设	计: 
//输	入: nPhase - 相位值
//输	出: -
//返	回: 电压有效值
//功	能: 获取当前电压
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
//名	称: att7022_GetCurrent ()
//设	计: 
//输	入: nPhase - 相位值
//输	出: -
//返	回: 电流有效值
//功	能: 获取当前电流
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
	float fResult,eck;
	
	eck = 3200.0f / (float)ATT7022_CONST_EC; //脉冲输出系数
	fResult = att7022_ReadReg(p, nReg + nPhase);
	if (fResult > MAX_VALUE1)
		fResult -= MAX_VALUE2;
	if (nPhase < 3) {
		//读取分相功率
		fResult = fResult * eck / 256000.0f; //转换成工程量
	} else {
		//读取合相功率
		fResult = fResult * eck / 64000.0f; //转换成工程量
	}
	return fResult;
}

uint32_t att7022_GetFlag(p_att7022 p) 
{
	
	return (att7022_ReadReg(p,ATT7022_REG_SFlag)); //读取状态寄存器 
}
//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
uint32_t att7022_GetPowerDir(p_att7022 p)
{

	//Bit0-3 分别表示A B  C  合相的有功功率的方向		0 表示为正	1 表示为负 
	//Bit4-7 分别表示A B  C  合相的无功功率的方向 	0 表示为正	1 表示为负 
	return (att7022_ReadReg(p, ATT7022_REG_PFlag)); //读取功率方向寄存器
}
//------------------------------------------------------------------------
//名	称:  att7022_GetSV ()
//设	计: 
//输	入:  nPhase - 相位值，0->A ，1->B ，2->C ，3->合相
//输	出:  -
//返	回:  视在功率值
//功	能:  获取各相视在功率及合相视在功率,
//------------------------------------------------------------------------
float att7022_GetSV(p_att7022 p, uint_t nPhase)
{
	float sv, eck;

	sv = att7022_ReadReg(p,ATT7022_REG_SA + nPhase);
	eck = 3200.0f / (float)ATT7022_CONST_EC; //脉冲输出系数
	if (sv > MAX_VALUE1)
		sv -= MAX_VALUE2;
	if (nPhase < 3) {
		//读取分相功率
		return (sv / 256000.0f * eck); //转换成工程量
	} else {
		//读取合相功率
		return (sv / 64000.0f * eck); //转换成工程量
	}
}
//------------------------------------------------------------------------
//名	称:  att7022_GetPFV ()
//设	计: 
//输	入:  nPhase - 相位值
//输	出:  -
//返	回:  功率因数
//功	能:  获取功率因数
//------------------------------------------------------------------------
float att7022_GetPFV(p_att7022 p, uint_t nPhase)
{
	int nData = 0;
	float f_data = 0.0f;

	nData = att7022_ReadReg(p, ATT7022_REG_PfA + nPhase);
	if (nData > MAX_VALUE1)
		nData -= MAX_VALUE2;
	f_data = nData / MAX_VALUE1;
	return f_data;
}
//------------------------------------------------------------------------
//名	称:  att7022_GetPAG ()
//设	计: 
//输	入: nPhase - 相位值
//输	出: -
//返	回: 电流于电压的夹角
//功	能: 获取角度
//------------------------------------------------------------------------
float att7022_GetPAG(p_att7022 p, uint_t nPhase) 
{
	float sita;
	
	sita = att7022_ReadReg(p,ATT7022_REG_PgA + nPhase);
	if (sita > MAX_VALUE1)
		sita -= MAX_VALUE2;
	return (sita * 360 / MAX_VALUE1 / PI);
}

//-------------------------------------------------------------------------
//电压相角
//-------------------------------------------------------------------------
float att7022_GetPVAG(p_att7022 p, uint_t nPhase) 
{

	return ((float)att7022_ReadReg(p, ATT7022_REG_YUaUb + nPhase) / 8192.0f);
}

uint16_t att7022_GetQuanrant(p_att7022 p, uint_t Phase) 
{
	uint32_t pflag;
	uint32_t p_direction, q_direction;

	pflag = att7022_ReadReg(p, ATT7022_REG_PFlag); //先读取功率方向寄存器(正向为0,负向为1)
	p_direction = ((pflag >> Phase) & 0x00000001);
	q_direction = ((pflag >> (Phase + 4)) & 0x00000001);
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
 * 名称:			att7022_GetPosEP
 * 描述:			读取正向有功电能
 * 创建:			赖晓芳   2006年10月23日
 * 最后修订:		2006年10月23日 18:36:07
 * 调用:			None
 * 输入参数:		nPhase 相位值
 * 返回值:		电能值 已累加(0.01kwh为基数)
 * 特殊说明:		RVMDK 3.01a
 *=============================================================================*/
uint32_t att7022_GetPosEP(p_att7022 p, uint_t nPhase)
{

	return (att7022_ReadReg(p,ATT7022_REG_PosEpA2 + nPhase)); //读取低于计数常数的保存值;
}

/*=============================================================================
 * 名称:			att7022_GetNegEP
 * 描述:			读取反向有功电能值
 * 创建:			赖晓芳   2006年10月24日
 * 最后修订:		2006年10月24日 9:00:20
 * 调用:			None
 * 输入参数:		nPhase 相位值
 * 返回值:		电能值 已累加(0.01kwh为基数)
 * 特殊说明:		RVMDK 3.01a
 *=============================================================================*/
uint32_t att7022_GetNegEP(p_att7022 p, uint_t nPhase)
{

	return att7022_ReadReg(p,ATT7022_REG_NegEpA2 + nPhase); //读取低于计数常数的保存值;	
}

/*=============================================================================
 * 名称:			att7022_GetPhaseEQ
 * 描述:			读取分相无功电能量
 * 创建:			赖晓芳   2006年10月27日
 * 最后修订:		2006年10月27日 17:43:40
 * 调用:			None
 * 输入参数:		None
 * 返回值:			None
 * 特殊说明:		RVMDK 3.01a
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
 * 名称:			att7022_GetHarmonic
 * 描述:			读取各通道谐波
 * 输入参数:		Ch(0--B)  分别对应下面各通道，采样率3.2k
 Ua\Ia\Ub\Ib\Uc\Ic\In\Ua+Ia\Ub+Ib\Uc+Ic\Ua+Ub+Uc\Ia+Ib+Ic
 * 返回值:			
 * 特殊说明:		RVMDK 3.01a
***************************/
sys_res att7022_GetHarmonic(p_att7022 p, uint_t Ch, sint16_t *pbuf)
{
	uint_t i, nData, nReg = 0x7F;

	//开启采样功能
	nData=0xCCCCC0|Ch;
	att7022_WriteReg(p, 0xC0, nData);//启动波形数据缓存
	//等待采样数据完成
#if 0
	os_thd_Sleep(200);
#else
	for (i = 20; i; i--) {   //一般重复3次就行了?
		if(att7022_ReadReg(p, 0x7E) >= 240)//下一个写数据的位置
			break;
		os_thd_Sleep(20);
	}
#endif
	if (i) {
		//设置用户读取指针的起始位置
		att7022_WriteReg(p, 0xC1, 0);
		for (i = 0; i < 240; i++) {
			spi_Transce(p->spi, &nReg, 1, &nData, 3);//读取数据
			reverse(&nData, 3);
			nData &= ATT7022_DATA_MASK;
			if (i < ATT7022_SAMPLEPOINT)
				pbuf[i] = (sint16_t)(nData >> 8);
		}
		return SYS_R_OK;
	}
	return SYS_R_ERR;
}

//------------------------------------------------------------------------ 
//名 称: att7022_UgainCalibration () 
//设 计: 
//输 入: nPhase - 待校准的相, (0 - A相, 1 - B相, 2 - C相) 
//输 出: - 
//返 回: gain - 校准成功返回Ugain的当前值 
//功 能: 电压参数Ugain校准
//------------------------------------------------------------------------ 
uint32_t att7022_UgainCalibration(p_att7022 p, uint8_t nPhase )
{
	float urms, k, gain = 0;

	//校表使能  
	att7022_WriteEnable(p);
	//设置电压校正寄存器为0
	att7022_WriteReg(p, ATT7022_REG_UgainA + nPhase, 0); 
	//校表禁止 
	att7022_WriteDisable(p); 
	//等1秒
	os_thd_Sleep(1000);
	//读取该相电压值 
	urms = att7022_ReadReg(p, ATT7022_REG_URmsA + nPhase);
	
	if(urms)
	{
		 //计算实际测出的工程量
		urms = urms / SYS_KVALUE;
		//此处以UCALI_CONST为标准计算Ugain 
		k = UCALI_CONST / urms - 1;

		gain = k * MAX_VALUE1;
		if (k < 0)
			gain = MAX_VALUE2 + gain;

		//校表使能  
		att7022_WriteEnable(p);
		//写入电压校表数据
		att7022_WriteReg (p, ATT7022_REG_UgainA + nPhase, (uint32_t)gain);
		//校表禁止
		att7022_WriteDisable(p); 
	}
	return (uint32_t)gain;
} 

//------------------------------------------------------------------------ 
//名 称: att7022_IgainCalibration () 
//设 计: 
//输 入: nPhase - 待校准的相, (0 - A相, 1 - B相, 2 - C相) 
//输 出: - 
//返 回: Igain - 校准成功返回Igain的当前值 
//功 能: 电压参数Igain校准
//------------------------------------------------------------------------ 
uint32_t att7022_IgainCalibration(p_att7022 p, uint32_t nPhase)
{
	float irms, k, gain = 0;

	//校表使能  
	att7022_WriteEnable(p); 
	//设置电压校正寄存器为0
	att7022_WriteReg(p, ATT7022_REG_IgainA + nPhase, 0); 
	//校表禁止 
	att7022_WriteDisable(p);  
	//等1秒
	os_thd_Sleep(1000);
	//读取该相电压值 
	irms = att7022_ReadReg(p, ATT7022_REG_IRmsA + nPhase);

	if (irms )
	{
		irms = irms / SYS_KVALUE; 
		//此处以ICALI_CONST为标准计算Igain	
		k = (ICALI_CONST * ICALI_MUL) / irms - 1;	

		gain = k * MAX_VALUE1;
		if (k < 0)
			gain = MAX_VALUE2 + gain;

		//校表使能  
		att7022_WriteEnable(p); 
		//写入电压校表数据
		att7022_WriteReg (p, ATT7022_REG_IgainA + nPhase, (uint32_t)gain);
		//校表禁止
		att7022_WriteDisable(p); 
	}
	return (uint32_t)gain;
}

//------------------------------------------------------------------------
//名	称: att7022_PgainCalibration () 
//设	计: 
//输	入: nPhase- 待校准的相, (0 - A相, 1 - B相, 2 - C相)
//输	出: -
//返	回:  pgain - 校准成功返回pgain的当前值 
//功	能: 功率参数pgain校准
//------------------------------------------------------------------------
uint32_t att7022_PgainCalibration(p_att7022 p, uint8_t nPhase)
{
	float pvalue, err, eck, pgain = 0;

	//脉冲输出系数
	eck = 3200.0f / (float)ATT7022_CONST_EC;
	//校表使能  
	att7022_WriteEnable(p); 
	//先设置Pgain为0
	att7022_WriteReg(p, ATT7022_REG_PgainA0 + nPhase, 0);
	att7022_WriteReg(p, ATT7022_REG_PgainA1 + nPhase, 0);
	//校表禁止 
	att7022_WriteDisable(p); 						
	//等待数值稳定
	os_thd_Sleep(1000);
	//读出测量到的功率
	pvalue = att7022_ReadReg(p, ATT7022_REG_PA + nPhase);	

	if (pvalue > MAX_VALUE1 )
	{
		pvalue = pvalue - MAX_VALUE2;
	}
	//转换成工程量
	pvalue = (pvalue / 256.0f) * eck;		
	//误差计算
	err = (pvalue - (float)PCALI_CONST) / (float)PCALI_CONST;					
	if (err )
	{
		err = -err / (1 + err);

		pgain = err * MAX_VALUE1;
		if (err < 0 )
			pgain = MAX_VALUE2 + pgain;			//计算Pgain

		//校表使能  
		att7022_WriteEnable(p); 
		att7022_WriteReg(p, ATT7022_REG_PgainA0 + nPhase, pgain);
		att7022_WriteReg(p, ATT7022_REG_PgainA1 + nPhase, pgain);
		//校表禁止
		att7022_WriteDisable(p); 
	}
			
	return (uint32_t)pgain;
}

//------------------------------------------------------------------------
//名	称: BSP_ATT7022B_UIP_gainCalibration () 
//设	计: 
//输	入: -
//输	出: -
//返	回: -
//功	能: 校正U，I，P增益
//------------------------------------------------------------------------
void att7022_UIP_gainCalibration(p_att7022 p, p_att7022_cali pCali)
{
	uint_t i;

	for (i = 0; i < 3; i++)
	{
		//电压通道校正
		pCali->Ugain [i] = att7022_UgainCalibration(p, PHASE_A + i);   
		
		//电流通道校正
		pCali->Igain[i] = att7022_IgainCalibration(p, PHASE_A + i);   
		
		//功率校正
		pCali->Pgain0[i] = att7022_PgainCalibration(p, PHASE_A + i);   
		pCali->Pgain1[i] = pCali->Pgain0[i];
	}
}
//------------------------------------------------------------------------
//名	称: BSP_ATT7022B_Phase_gainCalibration () 
//设	计: 
//输	入: -
//输	出: -
//返	回: -
//功	能: 相位角校正,功率因数为  0.5L  的条件下
//------------------------------------------------------------------------
void att7022_Phase_GainCalibration(p_att7022 p, p_att7022_cali pCali)
{
	uint_t i;
	uint32_t phv;

	phv = att7022_PhaseCalibration(p, PHASE_A, 1); //1.5A处校正
	for(i = 0; i < 5; i++) {
		pCali->PhsregA[i] = phv;   
	}
	phv = att7022_PhaseCalibration(p, PHASE_B, 1); //1.5A处校正
	for(i = 0; i < 5; i++) {
		pCali->PhsregB[i] = phv;  
	}
	phv = att7022_PhaseCalibration(p, PHASE_B, 1); //1.5A处校正
	for(i = 0; i < 5; i++) {
		pCali->PhsregC[i] = phv;  
	}
}
//------------------------------------------------------------------------
//名	称: att7022_PhaseCalibration () 
//设	计: 
//输	入: nPhase - 待校正的相位
//			cali_point - 校正点(0 - 4)
//输	出: -
//返	回: phase_v - 返回计算出的相位校正值(长整形) 
//功	能:相位校正, 此处分两段校正
//------------------------------------------------------------------------
uint32_t att7022_PhaseCalibration(p_att7022 p, uint8_t nPhase, uint8_t nCali)
{
	uint32_t i;
	float fPhase = 0;

	switch (nCali) {
	case 0:
		att7022_WriteEnable(p);
		for (i = 0; i < 5; i++) {
			//清除所有校正寄存器值
			att7022_WriteReg(p,ATT7022_REG_PhsregA0 + (nPhase *5) + i, 0);
		}
		for (i = 0; i < 4; i++) {
			//清除寄存器值
			att7022_WriteReg(p,ATT7022_REG_Irgion1 + i, 0);
		}
		att7022_WriteDisable(p);
		sys_Delay(50000);
		fPhase = att7022_FPhaseCaliData(p, nPhase, nCali);
		break;
	case 1:
	case 2:
	case 3:
	case 4:
		fPhase = att7022_FPhaseCaliData(p, nPhase, nCali);
		break;
	default:
		break;
	}
	return (uint32_t)fPhase;
}

float att7022_FPhaseCaliData(p_att7022 p,uint8_t nPhase, uint8_t cali_point) 
{
	float phase_v = 0, att7022_pvalue, seta, err, pcali_value = 0, eck;
	uint_t nTmp = 0;
	
	eck = 3200.0f / (float)ATT7022_CONST_EC;			//脉冲输出系数
	pcali_value = PCALI_CONST * 0.5f;					//载入校正点的有功功率常数
	nTmp = att7022_ReadReg(p, ATT7022_REG_PA + nPhase);	//读取有功功率值
 	if (nTmp > MAX_VALUE1) {
 		nTmp -= MAX_VALUE2;
 	}
	att7022_pvalue = ((float)nTmp / 256.0f) *eck; //转换成工程量
	err = (att7022_pvalue - pcali_value) / pcali_value; //误差计算
	if (err) {
		seta = acosf((1 + err) * 0.5f);
		seta -= PI / 3.0f;
		phase_v = seta * MAX_VALUE1;
		if (seta < 0)
			phase_v = MAX_VALUE2 + phase_v;
	}
	return phase_v;
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
 * 名称:			att7022_GetTemp
 * 描述:			温度读取
 * 创建:			赖晓芳   2006年10月23日
 * 最后修订:		2006年10月23日 18:11:28
 * 调用:			None
 * 输入参数:		cali: 1--读取未校正的值 0--读取校正后的值
 * 返回值:		温度值(扩大10倍)
 * 特殊说明:		RVMDK 3.01a
 *=============================================================================*/
float att7022_GetTemp(p_att7022 p, uint8_t nCali) {
	float fTemp;
	uint16_t nTempCali;
	fTemp = att7022_ReadReg(ATT7022_REG_TempD); //读取温度值
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
//名	称: att7022_PgainCalibration () 
//设	计: 
//输	入: nPhase - 待校准的相, (0 - A相, 1 - B相, 2 - C相)
//输	出: -
//返	回: __FALSE - 校准失败, pgain - 校准成功返回pgain的当前值 
//功	能: 参数pgain校准
//------------------------------------------------------------------------
uint32_t att7022_PgainCalibration(p_att7022 p, uint8_t nPhase) {
	float att7022_pvalue, err, eck;
	float pgain = 0;
	if (nPhase > 2) {
		return __FALSE;
	}
	eck = 3200.0f / (float)ATT7022_CONST_EC; //脉冲输出系数
	att7022_WriteEnable(); //校表使能
	att7022_WriteReg(ATT7022_REG_PgainA0 + nPhase, 0); //先设置Pgain为0
	att7022_WriteReg(ATT7022_REG_PgainA1 + nPhase, 0);
	att7022_WriteDisable(); //校表禁止 
	sys_Delay(5000000); //等待数值稳定
	att7022_pvalue = att7022_ReadReg(ATT7022_REG_PA + nPhase); //读出测量到的功率
	if (att7022_pvalue > MAX_VALUE1) {
		att7022_pvalue -= MAX_VALUE2;
	}
	att7022_pvalue = (att7022_pvalue / 256) *eck; //转换成工程量
	err = (att7022_pvalue - PCALI_CONST) / PCALI_CONST; //误差计算
	if (err) {
		pgain = 0-(err / (1+err));
		if (pgain < 0) {
			//pgain 为负值												
			pgain = MAX_VALUE2 + pgain * MAX_VALUE1; //计算Pgain
		} else {
			pgain *= MAX_VALUE1; //计算Pgain
		}
		att7022_WriteEnable(); //校表使能
		att7022_WriteReg(ATT7022_REG_PgainA0 + nPhase, (uint32_t)pgain); //更新Pgain_0参数
		att7022_WriteReg(ATT7022_REG_PgainA1 + nPhase, (uint32_t)pgain); //更新Pgain_1参数
		att7022_WriteDisable();
	}
	return (uint32_t)pgain;
}


//------------------------------------------------------------------------ 
//名 称: att7022_IgainCalibration () 
//设 计: 
//输 入: nPhase - 待校准的相, (0 - A相, 1 - B相, 2 - C相) 
//输 出: - 
//返 回: __FALSE - 校准失败, Igain - 校准成功返回Igain的当前值 
//功 能: 参数Igain校准 
//------------------------------------------------------------------------ 
uint32_t att7022_IgainCalibration(p_att7022 p, uint8_t nPhase) {
	float irms, k, gain;
	if (nPhase > 2) {
		if (nPhase != PHASE_ADC7) {
			return __FALSE;
		}
	} //参数错误返回 
	att7022_WriteEnable(); //校表使能 
	if (nPhase != PHASE_ADC7) {
		att7022_WriteReg(ATT7022_REG_IgainA + nPhase, 0);
	} //先设置增益放大器为0 
	else {
		att7022_WriteReg(ATT7022_REG_gainADC7, 0);
	} //先设置增益放大器为0 
	att7022_WriteDisable(); //校表禁止 
	sys_Delay(5000000);
	if (nPhase != PHASE_ADC7) {
		irms = att7022_ReadReg(ATT7022_REG_IRmsA + nPhase);
	} //读取该相电流值 
	else {
		irms = att7022_ReadReg(ATT7022_REG_RmsADC7);
	} //读取该相电流值 
	if (irms) {
		irms = irms / SYS_KVALUE;
		k = (ICALI_CONST * ICALI_MUL) / irms - 1; //此处以ICALI_CONST为标准计算Igain	
		gain = k * MAX_VALUE1;
		if (k < 0) {
			gain += MAX_VALUE2;
		}
		att7022_WriteEnable(); //校表使能 
		if (nPhase != PHASE_ADC7) {
			att7022_WriteReg(ATT7022_REG_IgainA + nPhase, (uint32_t)gain);	//写入电压校表数据 
		} else {
			att7022_WriteReg(ATT7022_REG_gainADC7, (uint32_t)gain);			//写入电压校表数据 
		}
		att7022_WriteDisable();
		return (uint32_t)gain;
	}
	return __FALSE;
}

//------------------------------------------------------------------------ 
//名 称: att7022_UgainCalibration () 
//设 计: 
//输 入: nPhase - 待校准的相, (0 - A相, 1 - B相, 2 - C相) 
//输 出: - 
//返 回: __FALSE - 校准失败, gain - 校准成功返回Ugain的当前值 
//功 能: 参数Ugain校准 
//------------------------------------------------------------------------ 
uint32_t att7022_UgainCalibration(p_att7022 p, uint8_t nPhase)
{
	float urms, k, gain;
	if (nPhase > 2) {
		if (nPhase != PHASE_ADC7) {
			return __FALSE;
		}
	} //参数错误返回 
	att7022_WriteEnable(); //校表使能 
	if (nPhase != PHASE_ADC7) {
		att7022_WriteReg(ATT7022_REG_UgainA + nPhase, 0);
	} //先设置增益放大器为0 
	else {
		att7022_WriteReg(ATT7022_REG_gainADC7, 0);
	} //先设置增益放大器为0 
	att7022_WriteDisable(); //校表禁止 
	sys_Delay(5000000);
	if (nPhase != PHASE_ADC7) {
		urms = att7022_ReadReg(ATT7022_REG_URmsA + nPhase);
	} //读取该相电压值 
	else {
		urms = att7022_ReadReg(ATT7022_REG_RmsADC7);
	} //读取该相电压值 
	if (urms) {
		urms = urms / SYS_KVALUE; //计算实际测出的工程量 
		k = UCALI_CONST / urms - 1; //此处以UCALI_CONST为标准计算Ugain 
		gain = k * MAX_VALUE1;
		if (k < 0) {
			gain += MAX_VALUE2;
		}
		att7022_WriteEnable(); //校表使能 
		if (nPhase != PHASE_ADC7) {
			att7022_WriteReg(ATT7022_REG_UgainA + nPhase, (uint32_t)gain);
		} //写入电压校表数据 
		else {
			att7022_WriteReg(ATT7022_REG_gainADC7, (uint32_t)gain);
		} //写入电压校表数据 
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
	//写入增益
//	att7022_WriteGain();
	if (channel < 6) {
		//写入AD7电压增益
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
 * 名称:			att7022_ADC_ChannelSelect
 * 描述:			选择ADC7的采样通道
 * 创建:			赖晓芳   2007年1月20日
 * 最后修订:		2007年1月20日 17:25:34
 * 调用:			None
 * 输入参数:		None
 * 返回值:			None
 * 特殊说明:		RVMDK 3.03a + RTL-ARM 3.03a
 *=============================================================================*/
int att7022_ADC_ChannelSelect(uint32_t nChl)
{
	uint32_t nFlag;
	int nResult = 1;

	nFlag = att7022_GetFlag();
	switch (nChl){
		case 0:
			if (nFlag & BITMASK(0)){
				//< 20 ) A相失压
				nResult = 0;
			} else {
				att7022_AD_Channel_Ua();
			}
			break;
		case 1:
			if (nFlag & BITMASK(1)){
				//< 20 )	b相失压
				nResult = 0;
			} else {
				att7022_AD_Channel_Ub();
			}
			break;
		case 2:
			if (nFlag & BITMASK(2)){
				//< 20 )	c相失压
				nResult = 0;
			} else {
				att7022_AD_Channel_Uc();
			}
			break;
		case 3:
			if (nFlag & BITMASK(9)){
				//< 20 )	a相潜动
				nResult = 0;
			} else {
				att7022_AD_Channel_Ia();
			}
			break;
		case 4:
			if (nFlag & BITMASK(10)){
				//< 20 ) b相潜动
				nResult = 0;
			} else {
				att7022_AD_Channel_Ib();
			}
			break;
		case 5:
			if (nFlag & BITMASK(11)){
				//< 20 ) c相潜动
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
uint16_t att7022_GetQuanrant(uint8_t Phase) {
	uint32_t pflag;
	uint32_t p_direction, q_direction;
	pflag = att7022_ReadReg(ATT7022_REG_PFlag); //先读取功率方向寄存器(正向为0,负向为1)
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
//名	称: att7022_IBlanceCalc ()
//设	计: 
//输	入: -
//输	出: -
//返	回: 三相电流不平衡率(%)
//功	能: 电流不平衡率计算，实际直扩大10倍
//------------------------------------------------------------------------
float att7022_IBlanceCalc(void) {
	float iin[3];
	float temp1;
	float temp2;
	uint32_t i;
	//读出三相电流值
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
	//三相最大
	temp2 = (iin[0] + iin[1] + iin[2]) / 3; //三相平均	
	iin[0] = (temp1 - temp2) / temp2;
	return iin[0];
}

//------------------------------------------------------------------------
//名	称: att7022_VBlanceCalc ()
//设	计: 
//输	入: -
//输	出: -
//返	回: 电压不平衡率(%)
//功	能: 电压不平衡率计算，实际直扩大10倍
//------------------------------------------------------------------------
float att7022_VBlanceCalc(void) {
	int vin[3];
	int temp;
	uint32_t i;
	//读出三相电压值
	for (i = 0; i < 3; i++) {
		vin[i] = att7022_GetVoltage(PHASE_A + i);
	}
	temp = (vin[0] + vin[1] + vin[2]) / 3; //三相平均
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
	} //三相最大
	vin[0] = vin[0] *100 / temp;
	return vin[0];
}

//------------------------------------------------------------------------
//名	称: void att7022_IstartupSet () 
//设	计: 
//输	入: -
//输	出: -
//返	回: -
//功	能: ATT7022B 启动电流设置，Ib = 5A, 启动电流0.2%Ib
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
//校正函数
//------------------------------------------------------------------------
//------------------------------------------------------------------------
//名	称: att7022_HFConstSet()
//设	计: 
//输	入: 
//输	出: 
//返	回: 
//功	能:设置脉冲常数校完电压电流后调用，
//			装置需要提供电压220电流5A。
//------------------------------------------------------------------------
uint32_t att7022_HFConstSet(void) {
	float urms, irms, hfcost;
	urms = att7022_ReadReg(ATT7022_REG_URmsA); //读取电压值 
	irms = att7022_ReadReg(ATT7022_REG_IRmsA); //读取电流值 
	irms = irms / ICALI_MUL;
	hfcost = CONST_HF * CONST_G * CONST_G * urms * irms / (UCALI_CONST *ICALI_CONST * ATT7022_CONST_EC);
	att7022_WriteEnable();
	att7022_WriteReg(ATT7022_REG_HFConst, (uint32_t)hfcost); //设置HFConst
	att7022_WriteDisable();
	return ((uint32_t)hfcost);
}

//------------------------------------------------------------------------
//名	称: att7022_PhaseCali2Seg () 
//设	计: 
//输	入: nPhase - 待校正的相位
//			 cali_point - 校正点(0 - 4)
//输	出: 
//返	回: 
//功	能: 相位校正(0.5L处校正), 此处分两段校正
//------------------------------------------------------------------------
void att7022_PhaseCali2Seg(uint8_t nPhase) {
	uint32_t phv[2];
	phv[1] = att7022_PhaseCalibration(PHASE_A, 1); //7.5A处校正
	att7022_WriteEnable(); //ATT7022B校正使能
	att7022_WriteReg(ATT7022_REG_PhsregA0 + nPhase * 5, phv[1]);
	att7022_WriteReg(ATT7022_REG_PhsregA1 + nPhase * 5, phv[1]);
	att7022_WriteReg(ATT7022_REG_PhsregA2 + nPhase * 5, phv[1]);
	att7022_WriteReg(ATT7022_REG_PhsregA3 + nPhase * 5, phv[1]);
	att7022_WriteReg(ATT7022_REG_PhsregA4 + nPhase * 5, phv[1]);
	att7022_WriteDisable(); //ATT7022B校正禁止
}

//------------------------------------------------------------------------
//名	称: att7022_PhaseCalibration () 
//设	计: 
//输	入: nPhase - 待校正的相位
//			cali_point - 校正点(0 - 4)
//输	出: -
//返	回: phase_v - 返回计算出的相位校正值(长整形) 
//功	能:相位校正, 此处分两段校正
//------------------------------------------------------------------------
uint32_t att7022_PhaseCalibration(uint8_t nPhase, uint8_t nCali) {
	uint32_t i;
	float fPhase = 0;
	switch (nCali) {
		case 0:
			att7022_WriteEnable();
			for (i = 0; i < 5; i++) {
				//清除所有校正寄存器值
				att7022_WriteReg(ATT7022_REG_PhsregA0 + (nPhase *5) + i, 0);
			}
			for (i = 0; i < 4; i++) {
				//清除寄存器值
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
//名	称: att7022_PhaseCaliData () 
//设	计: 
//输	入: nPhase - 待校正的相位
//			cali_point - 校正点(0 - 4)
//输	出: -
//返	回: phase_v - 返回计算出的相位校正值(双精度浮点数) 
//功	能: 相位校正数据计算
//------------------------------------------------------------------------

float att7022_FPhaseCaliData(uint8_t nPhase, uint8_t cali_point) {
	float phase_v = 0, att7022_pvalue, seta, err, pcali_value, eck;
	if (nPhase > 2) {
		return __FALSE;
	}
	eck = 3200.0f / (float)ATT7022_CONST_EC; //脉冲输出系数
	pcali_value = 0; //phiconst_tab[cali_point];				//载入校正点的有功功率常数
	att7022_pvalue = att7022_ReadReg(ATT7022_REG_PA + nPhase); //读取有功功率值
	if (att7022_pvalue > MAX_VALUE1) {
		att7022_pvalue -= MAX_VALUE2;
	}
	att7022_pvalue = (att7022_pvalue / 256) *eck; //转换成工程量
	err = (att7022_pvalue - pcali_value) / pcali_value; //误差计算
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
//名	称: att7022_IregionSet () 
//设	计: 
//输	入: currunt_value - 电流值(一个浮点数)
//			 field_num - 区域号(0 - 3)
//输	出: -
//返	回: __TRUE - 设置成功, __FALSE - 设置失败
//功	能: 相位补偿区域参数Iregion设置, 
//			 可以分成5段进行相位补偿,
//			 需要 对每个区域的电流值进行设置, 
//			 区域号由field_num指定  
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
//名	称: att7022_UIP_GainCalibration () 
//设	计: 
//输	入: -
//输	出: -
//返	回: -
//功	能: 校正U，I，P增益
//------------------------------------------------------------------------
void att7022_UIP_GainCalibration(void) {
	uint32_t nGain;
#ifdef ATT7022B_ADC7_CALI
	//AD7 校正
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
	//电压通道校正
	nGain = att7022_UgainCalibration(PHASE_A);
	ATT7022_REG_Set(TERMINAL, 0x1001, (uint8_t*) &nGain, 0);
	nGain = att7022_UgainCalibration(PHASE_B);
	ATT7022_REG_Set(TERMINAL, 0x1002, (uint8_t*) &nGain, 0);
	nGain = att7022_UgainCalibration(PHASE_C);
	ATT7022_REG_Set(TERMINAL, 0x1003, (uint8_t*) &nGain, 0);
	//电流通道校正
	nGain = att7022_IgainCalibration(PHASE_A);
	ATT7022_REG_Set(TERMINAL, 0x1007, (uint8_t*) &nGain, 0);
	nGain = att7022_IgainCalibration(PHASE_B);
	ATT7022_REG_Set(TERMINAL, 0x1008, (uint8_t*) &nGain, 0);
	nGain = att7022_IgainCalibration(PHASE_C);
	ATT7022_REG_Set(TERMINAL, 0x1009, (uint8_t*) &nGain, 0);
	//功率校正
	nGain = att7022_PgainCalibration(PHASE_A);
	ATT7022_REG_Set(TERMINAL, 0x1013, (uint8_t*) &nGain, 0);
	nGain = att7022_PgainCalibration(PHASE_B);
	ATT7022_REG_Set(TERMINAL, 0x1014, (uint8_t*) &nGain, 0);
	nGain = att7022_PgainCalibration(PHASE_C);
	ATT7022_REG_Set(TERMINAL, 0x1015, (uint8_t*) &nGain, 1);
}



//------------------------------------------------------------------------
//名	称: att7022_PhaseABC_Calibration () 
//设	计: 
//输	入: -
//输	出: -
//返	回: -
//功	能: 校正相位
//------------------------------------------------------------------------
void att7022_PhaseABC_Calibration(void) {
	float err;
	int *p1,  *p2;
	p1 = (int*) &err; //CommDataArea[7];
	p2 = (int*) &err; //CommDataArea[9];
	err = (float)((*p1) / (*p2));
}

//暂时未使用的函数
/*
float att7022_GetAD7(uint32 fat) {
	return ((float)att7022_ReadReg(ATT7022_REG_RmsADC7) / 8192 * fat);
}

//------------------------------------------------------------------------
//名	称: att7022_IronlossCalc ()
//设	计: 
//输	入: -
//输	出: -
//返	回: 铁损值
//功	能: 铁损值计算
//------------------------------------------------------------------------
float att7022_IronlossCalc(void) {
	float iron_loss, vin[3];
	uint32 i;
	//读出三相电压值
	for (i = 0; i < 3; i++) {
		vin[i] = att7022_GetVoltage(PHASE_A + i) / 10;
	}
	iron_loss = vin[0] *vin[0] + vin[1] *vin[1] + vin[2] *vin[2];
	return iron_loss;
}

//------------------------------------------------------------------------
//名	称: att7022_CopperlossCalc ()
//设	计: 
//输	入: -
//输	出: -
//返	回: 铜损值
//功	能: 铜损值计算
//------------------------------------------------------------------------
float att7022_CopperlossCalc(void) {
	float copper_loss, iin[3];
	uint32 i;
	//读出三相电流值
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

