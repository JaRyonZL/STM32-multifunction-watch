/*
 * @Author: JaRyon
 * @Date: 2026-09-01 15:59:50
 * @Email: jaryonzl@163.com
 */
#include "Inf_rtc.h"
#include "Drv_rtc.h"
#include "Drv_tim.h"
#include "Com_config.h"
#include <time.h>

/******************************************************************************
 * 文件名称：Inf_rtc.c（接口层）
 * 说    明：RTC日历换算，由旧工程MyRTC.c日历部分迁移（硬件部分在Drv_rtc）
 ******************************************************************************/

uint16_t Inf_rtc_time[] = {2026, 9, 1, 16, 2, 0, 0};  /* 首次初始化的默认时间 */

static void Inf_rtc_calibrate_lsi(void);

/* LSI校准状态：只测量不写RTC，显示时软件补偿漂移 */
static uint8_t  s_calib_done = 0;                 /* 校准完成标志 */
static uint32_t s_calib_ref_cnt = 0;              /* 校准时RTC计数器基准值 */
static uint32_t s_calib_tick_us = 1000000UL;      /* 实测一个RTC秒的真实时长（微秒），未校准按1s */
/**
  * 函    数：Inf_rtc_init
  * 功    能：初始化RTC，首次初始化时写入默认时间
  */
void Inf_rtc_init(void)
{
	if (Drv_rtc_init())  /* 返回1=首次初始化 */
	{
		Inf_rtc_set_time();
	}

#if !DRV_RTC_USE_LSE
	Inf_rtc_calibrate_lsi();    /* LSI走时校准（依赖Drv_tim2已初始化） */
#endif
}

/**
  * 函    数：Inf_rtc_set_time
  * 功    能：将Inf_rtc_time写回RTC硬件
  * 说    明：mktime按本地时间解析，减去时区偏移换算为UTC计数
  */
void Inf_rtc_set_time(void)
{
	time_t time_cnt;
	struct tm time_date;
	// 将设置数组写入到time_date结构体中
	time_date.tm_year = Inf_rtc_time[0] - 1900;
	time_date.tm_mon  = Inf_rtc_time[1] - 1;
	time_date.tm_mday = Inf_rtc_time[2];
	time_date.tm_hour = Inf_rtc_time[3];
	time_date.tm_min  = Inf_rtc_time[4];
	time_date.tm_sec  = Inf_rtc_time[5];
	// 将结构体数据转换为时间戳
	time_cnt = mktime(&time_date) - COM_RTC_UTC_OFFSET_HOURS * 60 * 60;
	// 将时间戳写入RTC硬件
	Drv_rtc_set_counter(time_cnt);

	/* 校时基准同步到新计数器，避免补偿系数作用于人工时间跳变 */
	if (s_calib_done)
	{
		s_calib_ref_cnt = Drv_rtc_get_counter();
	}
}

/**
  * 函    数：Inf_rtc_read_time
  * 功    能：从RTC硬件读取时间并刷新Inf_rtc_time
  * 说    明：UTC计数加上时区偏移换算为本地时间，星期做补偿转换
  */
void Inf_rtc_read_time(void)
{
	time_t time_cnt;
	struct tm time_date;
	// 从RTC硬件读取时间戳并加上时区偏移换算为本地时间
	time_cnt = Drv_rtc_get_counter();
	if (s_calib_done)   // 鎸夊疄娴嬫瘡绉掔湡瀹炴椂闀胯ˉ鍋縇SI婕傜Щ
	{
		time_cnt = s_calib_ref_cnt + (uint32_t)((uint64_t)(time_cnt - s_calib_ref_cnt) * s_calib_tick_us / 1000000ULL);
	}
	time_cnt += COM_RTC_UTC_OFFSET_HOURS * 60 * 60;
	// 将时间戳转换为结构体数据
	time_date = *localtime(&time_cnt);
	// 将结构体数据写入到设置数组中
	Inf_rtc_time[0] = time_date.tm_year + 1900;
	Inf_rtc_time[1] = time_date.tm_mon + 1;
	Inf_rtc_time[2] = time_date.tm_mday;
	Inf_rtc_time[3] = time_date.tm_hour;
	Inf_rtc_time[4] = time_date.tm_min;
	Inf_rtc_time[5] = time_date.tm_sec;
	Inf_rtc_time[6] = time_date.tm_wday;

	/* 星期补偿：tm_wday周日=0，转换为0=周一~6=周日 */
	if (Inf_rtc_time[6] == 0)
	{
		Inf_rtc_time[6] = 6;
	}
	else
	{
		Inf_rtc_time[6]--;
	}
}


/**
  * 函    数：Inf_rtc_calibrate_lsi
  * 功    能：LSI走时校准：用TIM2心跳（HSE派生）实测一个RTC秒的真实时长，
  *           记录补偿系数供Inf_rtc_read_time软件修正，不写任何RTC寄存器
  * 说    明：仅LSI模式调用；依赖Drv_tim2已初始化；超时放弃则不补偿
  */
static void Inf_rtc_calibrate_lsi(void)
{
	uint32_t us_start, us_end;
	uint32_t us_wait;
	uint32_t counter;

	// 等首次秒跳变作为测量起点（2s超时放弃）
	counter = Drv_rtc_get_counter();
	us_wait = Drv_tim2_get_us();
	while (Drv_rtc_get_counter() == counter)
	{
		if (Drv_tim2_get_us() - us_wait > 2000000UL) { return; }
	}
	us_start = Drv_tim2_get_us();

	// 再等一个秒跳变，实测一个RTC秒的真实时长（2s超时放弃）
	counter = Drv_rtc_get_counter();
	us_wait = Drv_tim2_get_us();
	while (Drv_rtc_get_counter() == counter)
	{
		if (Drv_tim2_get_us() - us_wait > 2000000UL) { return; }
	}
	us_end = Drv_tim2_get_us();

	if (us_end == us_start) { return; }   // 防除零

	// 记录补偿系数：基准计数器值+实测每秒真实时长
	s_calib_ref_cnt = Drv_rtc_get_counter();
	s_calib_tick_us = us_end - us_start;
	s_calib_done = 1;
}
