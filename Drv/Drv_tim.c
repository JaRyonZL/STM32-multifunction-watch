#include "Drv_tim.h"

/******************************************************************************
 * 文件名称：Drv_tim.c（驱动层）
 * 说    明：TIM2定时中断与1ms计数实现
 ******************************************************************************/

static volatile uint32_t s_tick_ms = 0;   /* 1ms计数（中断累加，主循环读取） */

/**
  * 函    数：Drv_tim2_init
  * 功    能：TIM2初始化（内部时钟，PSC=72-1，ARR=1000-1，1ms一次更新中断）
  */
void Drv_tim2_init(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	/* 时钟不选择，使用内部时钟 */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	TIM_InternalClockConfig(TIM2);

	/* 时钟单元初始化 */
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_Period = 1000 - 1;
	TIM_TimeBaseInitStructure.TIM_Prescaler = 72 - 1;
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);

	/* 清除更新标志，使能更新中断（初始化末尾会触发一次更新事件） */
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
  * 函    数：Drv_tim2_get_tick
  * 功    能：读取1ms计数（自初始化以来累计毫秒）
  */
uint32_t Drv_tim2_get_tick(void)
{
	return s_tick_ms;
}

/**
  * 函    数：TIM2中断服务函数
  * 功    能：更新中断中累加1ms计数
  */
void TIM2_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);

		s_tick_ms++;    /* 累加1ms计数 */
	}
}

/**
  * 函    数：Drv_tim2_get_us
  * 功    能：读取微秒级时间（1ms心跳+计数器细分，用于精确计时）
  */
uint32_t Drv_tim2_get_us(void)
{
	uint32_t cnt_before, cnt_after;
	uint32_t tick;

	do
	{
		cnt_before = TIM_GetCounter(TIM2);
		tick = s_tick_ms;
		cnt_after = TIM_GetCounter(TIM2);
	} while (cnt_after < cnt_before);   /* 若读取期间跨毫秒回绕，重读保证一致 */

	return tick * 1000 + cnt_after;
}
