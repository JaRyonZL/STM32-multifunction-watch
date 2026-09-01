#include "App_watchface.h"
#include "Inf_oled.h"
#include "Inf_oled_gfx.h"
#include "Inf_rtc.h"
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
