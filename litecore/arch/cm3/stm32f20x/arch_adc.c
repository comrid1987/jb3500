

void arch_AdcInit()
{
// 	ADC_InitTypeDef xAdc;

// 	/* Enable ADC1 clock                                                        */
// 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

// 	/* ADC1 configuration ------------------------------------------------------*/
// 	ADC_DeInit();
// 	xAdc.ADC_Mode = ADC_Mode_Independent;
// 	xAdc.ADC_ScanConvMode = DISABLE;
// 	xAdc.ADC_ContinuousConvMode = ENABLE;	//连续还是单次选择
// 	xAdc.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
// 	xAdc.ADC_DataAlign = ADC_DataAlign_Right;
// 	xAdc.ADC_NbrOfChannel = 2;
// 	ADC_Init(ADC1, &xAdc);
// 	/* ADC1 Regular Channel1 Configuration                                      */
// 	ADC_RegularChannelConfig(ADC1, ADC_Channel_14, 1, ADC_SampleTime_239Cycles5);
// 	/* Enable ADC1 */
// 	ADC_Cmd(ADC1, ENABLE);             

// 	/* Enable ADC1 reset calibaration register */	
// 	ADC_ResetCalibration(ADC1);
// 	/* Check the end of ADC1 reset calibration register */
// 	while(ADC_GetResetCalibrationStatus(ADC1));
// 	/* Start ADC1 calibaration */
// 	ADC_StartCalibration(ADC1);
// 	/* Check the end of ADC1 calibration */
// 	while(ADC_GetCalibrationStatus(ADC1));
// 	/* Start ADC1 Software Conversion */ 
// 	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}


float arch_AdcData()
{
	float fV;

	fV = ADC1->DR * (3.3f / 4096.0f * 0.686f) + 0.24f;
	return fV;
}


