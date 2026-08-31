#include "Drv_pwr.h"

/******************************************************************************
 * 文件名称：Drv_pwr.c（驱动层）
 * 说    明：低功耗驱动实现
 ******************************************************************************/

/**
  * 函    数：PA0外部中断唤醒初始化
  * 参    数：无
  * 返 回 值：无
  * 说    明：EXTI0上升沿触发，NVIC(2,2)，用于STOP模式唤醒
  */
void Drv_pwr_wakeup_init(void)
{
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	/* 开时钟（外部中断必须开AFIO时钟） */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

	/* PA0映射到EXTI0 */
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource0);

	/* EXTI初始化 */
	EXTI_InitStructure.EXTI_Line = EXTI_Line0;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_Init(&EXTI_InitStructure);

	/* NVIC配置 */
	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_Init(&NVIC_InitStructure);
}

/**
  * 函    数：进入停止模式（STOP）
  * 参    数：无
  * 返 回 值：无
  * 说    明：调节器保持开启，WFI进入；由EXTI0（PA0上升沿）唤醒后继续运行
  */
void Drv_pwr_enter_stop(void)
{
	PWR_EnterSTOPMode(PWR_Regulator_ON, PWR_STOPEntry_WFI);
}

/**
  * 函    数：进入待机模式（STANDBY）
  * 参    数：无
  * 返 回 值：无
  * 说    明：使能WKUP引脚（PA0）后进入，唤醒即复位重启
  */
void Drv_pwr_enter_standby(void)
{
	PWR_WakeUpPinCmd(ENABLE);
	PWR_EnterSTANDBYMode();
}

/**
  * 函    数：EXTI0中断服务函数
  * 参    数：无
  * 返 回 值：无
  * 说    明：STOP模式唤醒入口，仅清标志
  */
void EXTI0_IRQHandler(void)
{
	if (EXTI_GetITStatus(EXTI_Line0) == SET)
	{
		EXTI_ClearITPendingBit(EXTI_Line0);
	}
}
