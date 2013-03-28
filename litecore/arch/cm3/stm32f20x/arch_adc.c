

void arch_AdcInit()
{

 	/* Enable ADC1 clock                                                        */
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
}


uint_t arch_AdcData(uint_t nPort, uint_t nPin)
{
#if 0
	uint_t i, nSum;

	ADC_InitTypeDef ADC_InitStructure_ADC1;

	ADC_InitStructure_ADC1.ADC_Resolution = ADC_Resolution_12b;
	ADC_InitStructure_ADC1.ADC_ScanConvMode = ENABLE;								// 配置ADC1 模数转换工作在扫描模式(多通道模式)
	ADC_InitStructure_ADC1.ADC_ContinuousConvMode = ENABLE; 						// 配置ADC1 模数转换工作在连续模式
	ADC_InitStructure_ADC1.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;// 配置ADC1 模数转换有软件方式启动而非中断方式
	ADC_InitStructure_ADC1.ADC_DataAlign = ADC_DataAlign_Right; 					// 配置ADC1 模数转换数据对齐方式为右对齐
	ADC_InitStructure_ADC1.ADC_NbrOfConversion = 1;									// 配置ADC1 模数转换的通道数目 为 1个通道
	ADC_Init(ADC1, &ADC_InitStructure_ADC1);										// 配置ADC1 初始化

	ADC_RegularChannelConfig(ADC1, ADC_Channel_16, 1, ADC_SampleTime_480Cycles);

	/* Enable ADC1 ------------------------------------------------------*/
	ADC_Cmd(ADC1, ENABLE);
	/* Enable ADC1 reset calibration register */
	ADC_ResetCalibration(ADC1);
	/* Check the end of ADC1 reset calibration register */
	while (ADC_GetResetCalibrationStatus(ADC1));
	/* Start ADC1 calibration */
	ADC_StartCalibration(ADC1);
	/* Check the end of ADC1 calibration */
	while (ADC_GetCalibrationStatus(ADC1));
 	/* Start ADC1 Software Conversion */ 
 	ADC_SoftwareStartConvCmd(ADC1, ENABLE);

	nSum = 0;
	for (i = 0; i < 5; i++) {
		os_thd_Sleep(10);
		nSum += ADC1->DR;
	}
	nSum /= 5;	
	return nSum;
#endif
}


