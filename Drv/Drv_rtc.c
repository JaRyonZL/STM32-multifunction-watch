/*
 * @Author: JaRyon
 * @Date: 2026-08-31 19:36:19
 * @Email: jaryonzl@163.com
 */
#include "Drv_rtc.h"

/******************************************************************************
 * 文件名称：Drv_rtc.c（驱动层）
 * 说    明：RTC硬件驱动
 ******************************************************************************/

/**
  * 函    数：使能RTC时钟源（内部调用）
  * 参    数：无
  * 返 回 值：无
  * 说    明：按DRV_RTC_USE_LSE宏选择LSE或LSI，等待就绪并选择RTCCLK
  */
static void Drv_rtc_clock_enable(void)
{
#if DRV_RTC_USE_LSE
	/* LSE：外接32.768kHz晶振，起振失败会卡在等待LSERDY */
	RCC_LSEConfig(RCC_LSE_ON);
	while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) != SET);

	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
#else
	/* LSI：内部约40kHz */
	RCC_LSICmd(ENABLE);
	while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) != SET);

	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
#endif
	RCC_RTCCLKCmd(ENABLE);

	RTC_WaitForSynchro();
	RTC_WaitForLastTask();
}

/**
  * 函    数：RTC初始化
  * 参    数：无
  * 返 回 值：1=首次初始化（备份域被清），0=非首次
  * 说    明：BKP_DR1写入0xA5A5作首次初始化标记；
  *           LSE模式掉电后由VBAT后备电池保持计时，
  *           LSI模式掉电后RTC计时会停，重新上电需上层判断首次标志重写时间
  */
uint8_t Drv_rtc_init(void)
{
	uint8_t first = 0;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_BKP, ENABLE);

	PWR_BackupAccessCmd(ENABLE);

	/* 使能并选择时钟源 */
	Drv_rtc_clock_enable();

	if (BKP_ReadBackupRegister(BKP_DR1) != 0xA5A5)
	{
		/* 首次初始化：配预分频并写标记 */
		first = 1;

#if DRV_RTC_USE_LSE
		RTC_SetPrescaler(32768 - 1);
#else
		RTC_SetPrescaler(42340 - 1);
#endif
		RTC_WaitForLastTask();

		BKP_WriteBackupRegister(BKP_DR1, 0xA5A5);
	}

	return first;
}

/**
  * 函    数：写RTC计数器
  * 参    数：Counter 计数器值（秒）
  * 返 回 值：无
  */
void Drv_rtc_set_counter(uint32_t Counter)
{
	RTC_SetCounter(Counter);
	RTC_WaitForLastTask();
}

/**
  * 函    数：读RTC计数器
  * 参    数：无
  * 返 回 值：计数器值（秒）
  */
uint32_t Drv_rtc_get_counter(void)
{
	uint32_t counter, confirm;
	uint8_t retry = 0;

	do
	{
		counter = RTC_GetCounter();
		confirm = RTC_GetCounter();
		retry++;
	} while (confirm != counter && retry < 3);

	return confirm;
}

/**
  * 函    数：Drv_rtc_set_prescaler
  * 功    能：设置RTC分频（自动进出配置模式，计数器值保持不变）
  */
void Drv_rtc_set_prescaler(uint32_t Prescaler)
{
	RTC_WaitForLastTask();
	RTC_EnterConfigMode();
	RTC_SetPrescaler(Prescaler);
	RTC_ExitConfigMode();
	RTC_WaitForLastTask();
}

/**
  * 函    数：Drv_rtc_get_prescaler
  * 功    能：读取RTC分频值
  */
uint32_t Drv_rtc_get_prescaler(void)
{
	return (((uint32_t)RTC->PRLH << 16) | RTC->PRLL);
}
