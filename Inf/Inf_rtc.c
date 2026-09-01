/*
 * @Author: JaRyon
 * @Date: 2026-09-01 15:59:50
 * @Email: jaryonzl@163.com
 */
#include "Inf_rtc.h"
#include "Drv_rtc.h"
#include "Com_config.h"
#include <time.h>

/******************************************************************************
 * 文件名称：Inf_rtc.c（接口层）
 * 说    明：RTC日历换算，由旧工程MyRTC.c日历部分迁移（硬件部分在Drv_rtc）
 ******************************************************************************/

uint16_t Inf_rtc_time[] = {2026, 9, 1, 16, 2, 0, 0};  /* 首次初始化的默认时间 */

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
	time_cnt = Drv_rtc_get_counter() + COM_RTC_UTC_OFFSET_HOURS * 60 * 60;
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
