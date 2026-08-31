#include "Drv_tim.h"

/******************************************************************************
 * 文件名称：Drv_tim.c（驱动层）
 * 说    明：TIM2定时中断驱动实现
 ******************************************************************************/

/**
  * 函    数：TIM2初始化
  * 参    数：无
  * 返 回 值：无
  * 说    明：内部时钟，PSC=72-1，ARR=1000-1，即1ms一次更新中断
  */
void Drv_tim2_init(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	/* 开时钟并选择内部时钟 */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	TIM_InternalClockConfig(TIM2);

	/* 时基单元初始化 */
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_Period = 1000 - 1;
	TIM_TimeBaseInitStructure.TIM_Prescaler = 72 - 1;
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);

	/* 清除更新标志并使能更新中断（初始化末尾会触发一次更新事件） */
	TIM_ClearFlag(TIM2, TIM_FLAG_Update);
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

	/* NVIC配置 */
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&NVIC_InitStructure);

	/* TIM使能 */
	TIM_Cmd(TIM2, ENABLE);
}

/**
  * 函    数：TIM2中断服务函数
  * 参    数：无
  * 返 回 值：无
  */
void TIM2_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	}
}
