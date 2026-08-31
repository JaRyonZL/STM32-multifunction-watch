/*
 * @Author: JaRyon
 * @Date: 2026-08-31 16:26:22
 * @Email: jaryonzl@163.com
 */
#include "Drv_gpio.h"

/******************************************************************************
 * 文件名称：Drv_gpio.c（驱动层）
 * 说    明：GPIO通用初始化实现（由旧工程sys.c迁移）
 ******************************************************************************/

/**
  * 函    数：GPIO通用初始化
  * 参    数：GPIOx 	端口（GPIOA~GPIOG）
  * 参    数：GPIO_Pin 	引脚号（可多引脚或运算）
  * 参    数：mode 		模式（见GPIO_Drv_TypeDef枚举前半部分）
  * 参    数：up_down 	上下拉选择（仅输入模式有效）
  * 参    数：speed 	输出速度（见GPIO_Drv_TypeDef枚举后半部分）
  * 返 回 值：无
  * 注意事项：自动使能端口时钟；
  *           由旧工程sys.c迁移，修复GPIO_25MHz误映射为10MHz的笔误，
  *           并补齐GPIO_10MHz分支；调用方（软件I2C）只使用50MHz档，修复不影响行为
  */
void Drv_GPIO_Init(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin,
                  GPIO_Drv_TypeDef mode, GPIO_Drv_TypeDef up_down, GPIO_Drv_TypeDef speed)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* 根据端口地址偏移计算APB2时钟使能位并自动使能时钟 */
	RCC_APB2PeriphClockCmd(Com_pow(2, ((uint32_t)GPIOx - (uint32_t)GPIOA) / 0x0400 + 2), ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin;

	if (mode == GPIO_AD_IN)
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	if (mode == GPIO_FK_IN && up_down == GPIO_P_NO)
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	if (mode == GPIO_FK_IN && up_down == GPIO_P_DOWN)
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	if (mode == GPIO_FK_IN && up_down == GPIO_P_UP)
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	if (mode == GPIO_KL_OUT)
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	if (mode == GPIO_TW_OUT)
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	if (mode == GPIO_KL_AF_OUT)
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
	if (mode == GPIO_TW_AF_OUT)
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;

	if (speed == GPIO_2MHz)
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	if (speed == GPIO_10MHz)
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	if (speed == GPIO_25MHz)
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz; // 实际没有25MHz档位，故映射为10MHz
	if (speed == GPIO_50MHz)
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

	GPIO_Init(GPIOx, &GPIO_InitStructure);
}
