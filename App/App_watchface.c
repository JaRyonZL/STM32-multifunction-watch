#include "App_watchface.h"
#include "Inf_oled.h"
#include "Inf_oled_gfx.h"
#include "Inf_rtc.h"
#include "Inf_mp3.h"
#include "Inf_battery.h"
#include "Com_oled_res.h"

/******************************************************************************
 * 文件名称：App_watchface.c（应用层）
 * 说    明：表盘应用，由旧工程biaopan.c迁移
 ******************************************************************************/

/**
  * 函    数：App_watchface_draw_digit
  * 功    能：在指定位置显示单个数字图标（原bittt）
  * 参    数：Big=1 显示13x30大数字，0 显示9x21小数字
  */
void App_watchface_draw_digit(uint8_t X, uint8_t Y, uint8_t Num, uint8_t Big)
{
	if (Big)
	{
		switch (Num)
		{
			case 1: Inf_oled_show_image(X, Y, 13, 30, t1); break;
			case 2: Inf_oled_show_image(X, Y, 13, 30, t2); break;
			case 3: Inf_oled_show_image(X, Y, 13, 30, t3); break;
			case 4: Inf_oled_show_image(X, Y, 13, 30, t4); break;
			case 5: Inf_oled_show_image(X, Y, 13, 30, t5); break;
			case 6: Inf_oled_show_image(X, Y, 13, 30, t6); break;
			case 7: Inf_oled_show_image(X, Y, 13, 30, t7); break;
			case 8: Inf_oled_show_image(X, Y, 13, 30, t8); break;
			case 9: Inf_oled_show_image(X, Y, 13, 30, t9); break;
			case 0: Inf_oled_show_image(X, Y, 13, 30, t0); break;
		}
	}
	else
	{
		switch (Num)
		{
			case 1: Inf_oled_show_image(X, Y, 9, 21, i1); break;
			case 2: Inf_oled_show_image(X, Y, 9, 21, i2); break;
			case 3: Inf_oled_show_image(X, Y, 9, 21, i3); break;
			case 4: Inf_oled_show_image(X, Y, 9, 21, i4); break;
			case 5: Inf_oled_show_image(X, Y, 9, 21, i5); break;
			case 6: Inf_oled_show_image(X, Y, 9, 21, i6); break;
			case 7: Inf_oled_show_image(X, Y, 9, 21, i7); break;
			case 8: Inf_oled_show_image(X, Y, 9, 21, i8); break;
			case 9: Inf_oled_show_image(X, Y, 9, 21, i9); break;
			case 0: Inf_oled_show_image(X, Y, 9, 21, i0); break;
		}
	}
}

/**
  * 函    数：App_watchface_show_time
  * 功    能：读取RTC并显示大数字时间（AM/PM+时:分:秒）（原biaopan_Timer）
  */
void App_watchface_show_time(uint8_t X, uint8_t Y)
{
	uint16_t hour;
	uint16_t value;
	uint16_t ones;

	Inf_rtc_read_time();

	/* 时：12小时制，显示AM/PM */
	hour = Inf_rtc_time[3];
	if (hour > 12)
	{
		hour = hour - 12;
		Inf_oled_show_ascii(X + 109, Y + 21, "PM", OLED_6X8);
	}
	else
	{
		Inf_oled_show_ascii(X + 109, Y + 21, "AM", OLED_6X8);
	}

	ones = hour % 10;
	App_watchface_draw_digit(X + 19, Y, ones, 1);
	App_watchface_draw_digit(X, Y, (hour - ones) / 10, 1);

	/* 冒号 */
	Inf_oled_draw_rectangle(X + 37, Y + 6, 2, 4, OLED_UNFILLED);
	Inf_oled_draw_rectangle(X + 37, Y + 19, 2, 4, OLED_UNFILLED);

	/* 分 */
	value = Inf_rtc_time[4];
	ones = value % 10;
	App_watchface_draw_digit(X + 63, Y, ones, 1);
	App_watchface_draw_digit(X + 44, Y, (value - ones) / 10, 1);

	/* 秒（小数字） */
	value = Inf_rtc_time[5];
	ones = value % 10;
	App_watchface_draw_digit(X + 95, Y + 8, ones, 0);
	App_watchface_draw_digit(X + 81, Y + 8, (value - ones) / 10, 0);
}

extern uint8_t TLYPW;   /* 体感开关状态（预留：待Inf_mpu6050迁入后归其定义，暂由main.c定义） */

/**
  * 函    数：App_watchface_date
  * 功    能：显示日期：年/月/日
  * 说    明：复用App_watchface_show_time读取的Inf_rtc_time，调用顺序勿乱
  */
void App_watchface_date(uint8_t X, uint8_t Y)
{
	Inf_oled_show_num(X, Y, Inf_rtc_time[0], 4, OLED_6X8);
	Inf_oled_show_num(X + 26, Y, Inf_rtc_time[1], 2, OLED_6X8);
	Inf_oled_show_num(X + 40, Y, Inf_rtc_time[2], 2, OLED_6X8);
}

/**
  * 函    数：App_watchface_week
  * 功    能：显示星期："week:"+数字（原biaopan_week）
  * 说    明：Inf_rtc_time[6]：0=周一~5=周六，6=周日显示R
  */
void App_watchface_week(uint8_t X, uint8_t Y)
{
	uint16_t week = Inf_rtc_time[6];

	Inf_oled_show_ascii(X, Y, "week:", OLED_6X8);
	switch (week)
	{
		case 1: Inf_oled_show_ascii(X + 30, Y, "2", OLED_6X8); break;
		case 2: Inf_oled_show_ascii(X + 30, Y, "3", OLED_6X8); break;
		case 3: Inf_oled_show_ascii(X + 30, Y, "4", OLED_6X8); break;
		case 4: Inf_oled_show_ascii(X + 30, Y, "5", OLED_6X8); break;
		case 5: Inf_oled_show_ascii(X + 30, Y, "6", OLED_6X8); break;
		case 6: Inf_oled_show_ascii(X + 30, Y, "R", OLED_6X8); break;
		case 0: Inf_oled_show_ascii(X + 30, Y, "1", OLED_6X8); break;
	}
}

/**
  * 函    数：App_watchface_show_app_status
  * 功    能：显示小模块开关状态图标：MP3上电/体感
  */
void App_watchface_show_app_status(uint8_t X, uint8_t Y)
{
	uint8_t offset = 0;

	if (Inf_mp3_is_powered())       /* MP3模块已上电 */
	{
		Inf_oled_show_image(X + offset, Y, 8, 8, APPYYBFsmall);
		offset += 10;
	}
	if (TLYPW)                      /* 体感功能开启 */
	{
		Inf_oled_show_image(X + offset, Y, 8, 8, APPTLYsmall);
		offset += 10;
	}

	if (offset == 0)
	{
		Inf_oled_show_ascii(0, 0, "-", OLED_6X8);   
	}
}

/**
  * 函    数：App_watchface_run
  * 功    能：表盘刷新入口
  * 说    明：show_time内部读取RTC，日期/星期复用其结果
  */
void App_watchface_run(void)
{
	Inf_oled_clear();

	App_watchface_show_app_status(0, 0);
	App_watchface_show_battery(89, 0);
	App_watchface_show_time(5, 18);
	App_watchface_date(0, 56);
	App_watchface_week(90, 57);

	Inf_oled_update();
}

/**
  * 函    数：App_watchface_show_battery
  * 功    能：显示电池电量图标与百分比（数据由Inf_battery提供）
  */
void App_watchface_show_battery(uint8_t X, uint8_t Y)
{
	uint8_t level = Inf_battery_get_level();   /* 查表插值+最小保持显示值 */

	if (Inf_battery_is_charging())             /* 充电中显示"--" */
	{
		Inf_oled_show_ascii(X + 25, Y + 1, "--", OLED_6X8);
	}

	Inf_oled_draw_rectangle(X + 25, Y + 1, 12, 7, OLED_UNFILLED);   /* 电池图标 */
	Inf_oled_draw_line(X + 37, Y + 3, X + 37, Y + 5);               /* 电池图标头部 */

	/* 根据电量百分比填充格数 */
	if (level > 9)  Inf_oled_draw_line(X + 26, Y + 2, X + 26, Y + 6);
	if (level > 19) Inf_oled_draw_line(X + 27, Y + 2, X + 27, Y + 6);
	if (level > 29) Inf_oled_draw_line(X + 28, Y + 2, X + 28, Y + 6);
	if (level > 39) Inf_oled_draw_line(X + 29, Y + 2, X + 29, Y + 6);
	if (level > 49) Inf_oled_draw_line(X + 30, Y + 2, X + 30, Y + 6);
	if (level > 59) Inf_oled_draw_line(X + 31, Y + 2, X + 31, Y + 6);
	if (level > 69) Inf_oled_draw_line(X + 32, Y + 2, X + 32, Y + 6);
	if (level > 79) Inf_oled_draw_line(X + 33, Y + 2, X + 33, Y + 6);
	if (level > 89) Inf_oled_draw_line(X + 34, Y + 2, X + 34, Y + 6);

	if (level > 99)
	{
		Inf_oled_draw_line(X + 35, Y + 2, X + 35, Y + 6);
		Inf_oled_show_num(X + 5, Y + 1, 100, 3, OLED_6X8);
	}
	else
	{
		Inf_oled_show_num(X + 11, Y + 1, level, 2, OLED_6X8);
	}
}
