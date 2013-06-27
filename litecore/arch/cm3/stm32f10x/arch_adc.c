#if OS_TYPE

void arch_AdcInit()
{

 	/* Enable ADC3 clock                                                        */
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC3, ENABLE);
}


uint_t arch_AdcData(uint_t nPort, uint_t nPin)
{
	uint_t i, nSum;

	ADC_InitTypeDef ADC_InitStructure_ADC;

	ADC_InitStructure_ADC.ADC_Mode = ADC_Mode_Independent; 					// 配置ADC1 工作在独立模式
	ADC_InitStructure_ADC.ADC_ScanConvMode = ENABLE;						// 配置ADC1 模数转换工作在扫描模式(多通道模式)
	ADC_InitStructure_ADC.ADC_ContinuousConvMode = ENABLE; 					// 配置ADC1 模数转换工作在连续模式
	ADC_InitStructure_ADC.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	// 配置ADC1 模数转换有软件方式启动而非中断方式
	ADC_InitStructure_ADC.ADC_DataAlign = ADC_DataAlign_Right; 				// 配置ADC1 模数转换数据对齐方式为右对齐
	ADC_InitStructure_ADC.ADC_NbrOfChannel = 1;								// 配置ADC1 模数转换的通道数目 为 1个通道
	ADC_Init(ADC3, &ADC_InitStructure_ADC);									// 配置ADC1 初始化

	ADC_RegularChannelConfig(ADC3, ADC_Channel_4, 1, ADC_SampleTime_239Cycles5);//配置ADC1 的规则通道，转化顺序和采样时间

	/* Enable ADC1 ------------------------------------------------------*/
	ADC_Cmd(ADC3, ENABLE);
	/* Enable ADC1 reset calibration register */
	ADC_ResetCalibration(ADC3);
	/* Check the end of ADC1 reset calibration register */
	while (ADC_GetResetCalibrationStatus(ADC3));
	/* Start ADC1 calibration */
	ADC_StartCalibration(ADC3);
	/* Check the end of ADC1 calibration */
	while (ADC_GetCalibrationStatus(ADC3));
 	/* Start ADC1 Software Conversion */ 
// 	ADC_SoftwareStartConvCmd(ADC1, ENABLE);

	nSum = 0;
	for (i = 0; i < 5; i++) {
		os_thd_Sleep(10);
		nSum += (ADC3->DR & 0x0FFF);
	}
	nSum /= 5;	
	return nSum;
}

#endif

