/*
 * @Author: JaRyon
 * @Date: 2026-08-31 16:48:30
 * @Email: jaryonzl@163.com
 */
#include "Drv_delay.h"

/******************************************************************************
 * 文件名称：Drv_delay.c（驱动层）
 * 说    明：SysTick延时实现（由旧工程System/Delay迁移）
 *           本工程不使用OS，仅保留无OS版本，已移除UCOS条件编译块
 ******************************************************************************/

static u8  fac_us = 0;                          /* us延时倍乘数 */
static u16 fac_ms = 0;                          /* ms延时倍乘数 */

/**
  * 函    数：延时初始化
  * 参    数：无
  * 返 回 值：无
  * 注意事项：SYSTICK的时钟固定为HCLK时钟的1/8
  *           SYSCLK为系统时钟
  */
void Drv_delay_init(void)
{
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);	/* 选择外部时钟HCLK/8 */
	fac_us = SystemCoreClock / 8000000;				/* 为系统时钟的1/8 */

	fac_ms = (u16)fac_us * 1000;					/* 无OS下，代表每个ms需要的systick时钟数 */
}

/**
  * 函    数：延时nus
  * 参    数：nus要延时的us数
  * 返 回 值：无
  */
void Drv_delay_us(u32 nus)
{
	if (nus)
	{
		u32 temp;

		SysTick->LOAD = nus * fac_us;				/* 时间加载 */
		SysTick->VAL = 0x00;						/* 清空计数器 */
		SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;	/* 开始倒数 */
		do
		{
			temp = SysTick->CTRL;
		} while ((temp & 0x01) && !(temp & (1 << 16)));	/* 等待时间到达 */
		SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;	/* 关闭计数器 */
		SysTick->VAL = 0X00;						/* 清空计数器 */
	}
}

/**
  * 函    数：延时nms
  * 参    数：nms要延时的ms数
  * 返 回 值：无
  * 注意事项：SysTick->LOAD为24位寄存器，所以最大延时为:
  *           nms<=0xffffff*8*1000/SYSCLK
  *           SYSCLK单位为Hz，nms单位为ms
  *           对72M条件下，nms<=1864
  */
void Drv_delay_ms(u16 nms)
{
	if (nms)
	{
		u32 temp;

		SysTick->LOAD = (u32)nms * fac_ms;			/* 时间加载（SysTick->LOAD为24bit） */
		SysTick->VAL = 0x00;						/* 清空计数器 */
		SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;	/* 开始倒数 */
		do
		{
			temp = SysTick->CTRL;
		} while ((temp & 0x01) && !(temp & (1 << 16)));	/* 等待时间到达 */
		SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;	/* 关闭计数器 */
		SysTick->VAL = 0X00;						/* 清空计数器 */
	}
}
