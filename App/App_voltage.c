#include "App_voltage.h"
#include "App_settings_store.h"
#include "App_watchface.h"
#include "Inf_oled.h"
#include "Inf_oled_gfx.h"
#include "Inf_battery.h"
#include "Inf_key.h"
#include "Drv_adc.h"
#include "Com_board.h"

/******************************************************************************
 * 文件名称：App_voltage.c（应用层）
 * 说    明：电压检测应用
 ******************************************************************************/

float App_voltage_r1 = 43.0;   /* 分压上臂电阻（kΩ） */
float App_voltage_r2 = 10.0;    /* 分压下臂电阻（kΩ） */


/**
  * 函    数：App_voltage_show_resistor
  * 功    能：底行分压电阻显示：整数+小数点+两位小数（kΩ）
  * 说    明：自绘显示，避免浮点显示函数的前导符号占宽
  */
static void App_voltage_show_resistor(uint8_t X, uint8_t Y, float Value, uint8_t IntLength)
{
	uint32_t hundred = (uint32_t)(Value * 100 + 0.5f);   /* 四舍五入到0.01k */

	Inf_oled_show_num(X, Y, hundred / 100, IntLength, OLED_6X8);
	Inf_oled_show_char(X + IntLength * OLED_6X8, Y, '.', OLED_6X8);
	Inf_oled_show_num(X + (IntLength + 1) * OLED_6X8, Y, hundred % 100, 2, OLED_6X8);
}

/**
  * 函    数：App_voltage_run
  * 功    能：电压表界面：显示外部电压（CH9）、临界百分比与分压电阻，
  *           上下键微调当前编辑电阻（±0.01kΩ），短按切换R1/R2，长按退出
  * 说    明：分压比 = (R1+R2)/R2，由两个电阻值计算
  */
void App_voltage_run(void)
{
	uint8_t key;
	uint8_t edit_r2 = 0;             /* 当前编辑对象：0=R1 1=R2 */
	int8_t i = 0;                    /* 采样计数 */
	float voltage;
	float critical_percent;          /* 临界百分比 */
	uint16_t ADValue;
	float bar_width = 0, bar_width_target;   /* 进度条当前宽度/目标宽度 */

	Inf_oled_fade_flag = 1;

	while (1)
	{
		Inf_oled_clear();

		App_watchface_show_battery(90, 0);
		App_watchface_time_small(0, 0);

		/* 每约26帧采样一次 */
		if (i > 25)
		{
			ADValue = Drv_adc1_get_value(ADC_CH_VOLTAGE);
			critical_percent = (float)ADValue / 4095 * 100;
			voltage = ((float)ADValue * VIN) / 4095 * ((App_voltage_r1 + App_voltage_r2) / App_voltage_r2);
			i = 0;
		}
		else
		{
			i++;
		}

		Inf_oled_show_float_num(33, 16, voltage, 2, 3, OLED_8X16);
		Inf_oled_show_ascii(89, 24, "V", OLED_6X8);

		Inf_oled_show_num(0, 16, critical_percent, 3, OLED_6X8);
		Inf_oled_show_ascii(20, 16, "%", OLED_6X8);

		/* 底行：两个分压电阻（当前编辑项前加"-"） */
		Inf_oled_show_ascii(0, 56, edit_r2 ? " " : "-", OLED_6X8);
		Inf_oled_show_ascii(6, 56, "R1=", OLED_6X8);
		App_voltage_show_resistor(24, 56, App_voltage_r1, 3);

		Inf_oled_show_ascii(60, 56, edit_r2 ? "-" : " ", OLED_6X8);
		Inf_oled_show_ascii(66, 56, "R2=", OLED_6X8);
		App_voltage_show_resistor(84, 56, App_voltage_r2, 2);

		/* 进度条逼近 */
		bar_width_target = voltage * 4;
		if (bar_width < bar_width_target)
		{
			if (bar_width_target - bar_width > 25) bar_width += 5;
			else if (bar_width_target - bar_width > 7) bar_width += 2.4;
			else if (bar_width_target - bar_width > 0) bar_width += 0.5;
		}
		if (bar_width > bar_width_target)
		{
			if (bar_width - bar_width_target > 25) bar_width -= 5;
			else if (bar_width - bar_width_target > 7) bar_width -= 2.4;
			else if (bar_width - bar_width_target > 0) bar_width -= 0.5;
		}
		if (bar_width > 80) { bar_width = 80; }

		Inf_oled_clear_area(24, 42, 80, 3);
		Inf_oled_draw_rectangle(24, 42, bar_width, 3, OLED_FILLED);

		Inf_oled_draw_line(24, 49, 104, 49);
		Inf_oled_draw_line(24, 47, 24, 49);
		Inf_oled_draw_line(44, 47, 44, 49);
		Inf_oled_draw_line(64, 47, 64, 49);
		Inf_oled_draw_line(84, 47, 84, 49);
		Inf_oled_draw_line(104, 47, 104, 49);

		Inf_oled_update();

		Inf_oled_gradient(1);

		key = Inf_key_scan();
		if (key == 1 || key == 2)   /* 上下键：当前编辑电阻±0.01kΩ */
		{
			float step = (key == 1) ? 0.01f : -0.01f;

			if (edit_r2)
			{
				App_voltage_r2 += step;
				if (App_voltage_r2 < 0.01f) { App_voltage_r2 = 0.01f; }
				if (App_voltage_r2 > 99.99f) { App_voltage_r2 = 99.99f; }
			}
			else
			{
				App_voltage_r1 += step;
				if (App_voltage_r1 < 0.0f) { App_voltage_r1 = 0.0f; }
				if (App_voltage_r1 > 999.9f) { App_voltage_r1 = 999.9f; }
			}
		}

		if (key == 3)               /* 短按：切换编辑R1/R2 */
		{
			edit_r2 = !edit_r2;
		}
		else if (key == 4)          /* 长按：退出 */
		{
			Inf_oled_fade_flag = 1;
			Inf_oled_gradient(0);
			App_settings_store_save();   /* 保存设置 */
			return;
		}
	}
}

/**
  * 函    数：App_voltage_waveform
  * 功    能：波形显示（预留）
  */
void App_voltage_waveform(void)
{
}
