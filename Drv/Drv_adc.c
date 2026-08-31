/*
 * @Author: JaRyon
 * @Date: 2026-08-31 19:10:22
 * @Email: jaryonzl@163.com
 */
#include "Drv_adc.h"

/******************************************************************************
 * 文件名称：Drv_adc.c（驱动层）
 * 说    明：ADC1采样实现
 ******************************************************************************/

/**
  * 函    数：ADC1初始化
  * 参    数：无
  * 返 回 值：无
  * 说    明：PB0（CH8）/PB1（CH9）配置为模拟输入；时钟6分频；单次软件触发
  */
void Drv_adc1_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	ADC_InitTypeDef ADC_InitStructure;

	/* 开时钟 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	/* ADC时钟6分频 */
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);

	/* PB0/PB1模拟输入 */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_InitStructure.GPIO_Pin = ADC_BATTERY_PIN | ADC_VOLTAGE_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(ADC_BATTERY_PORT, &GPIO_InitStructure);

	/* ADC初始化 */
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	ADC_InitStructure.ADC_NbrOfChannel = 1;
	ADC_Init(ADC1, &ADC_InitStructure);

	/* ADC使能 */
	ADC_Cmd(ADC1, ENABLE);

	/* 校准 */
	ADC_ResetCalibration(ADC1);
	while (ADC_GetResetCalibrationStatus(ADC1) == SET);
	ADC_StartCalibration(ADC1);
	while (ADC_GetCalibrationStatus(ADC1) == SET);
}

/**
  * 函    数：ADC1单次采样指定通道
  * 参    数：ADC_Channel 通道号
  * 返 回 值：转换结果，范围0~4095
  * 说    明：每次转换前重配通道，采样周期239.5周期，软件触发后等待EOC
  */
uint16_t Drv_adc1_get_value(uint8_t ADC_Channel)
{
	ADC_RegularChannelConfig(ADC1, ADC_Channel, 1, ADC_SampleTime_239Cycles5);
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
	while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
	return ADC_GetConversionValue(ADC1);
}
