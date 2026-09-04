/*
 * @Author: JaRyon
 * @Date: 2026-09-04 18:15:31
 * @Email: jaryonzl@163.com
 */
#include "App_calcu.h"
#include "App_menu.h"
#include "Inf_oled.h"
#include "Inf_oled_gfx.h"
#include "Inf_key.h"
#include "Drv_tim.h"

/******************************************************************************
 * 文件名：App_calcu.c（应用层）
 * 说  明：计算工具应用
 *           App_calcu_menu 计算子菜单：选择计算工具进入（后续扩展在此加项）
 *           上下键 ±0.01，短按确定在 0.01/0.001步进间切换，长按退出
 ******************************************************************************/

/**
  * 函  数：App_calcu_menu
  * 功  能：计算子菜单：选择计算工具进入（后续扩展在此加项）
  */
void App_calcu_menu(void)
{
	App_menu_option2_t option_list[] = {
		{"- 退出"      , APP_MENU_MODE_FUNCTION, APP_MENU_RETURN,   0, 0},
		{"- cos(x)-1"  , APP_MENU_MODE_FUNCTION, App_calc_cos,      0, 0},
		{"- 进制转换"  , APP_MENU_MODE_FUNCTION, App_calcu_convert,   0, 0},
		{".."}
	};

	App_menu_run_list(option_list);
}

/**
  * 函  数：App_calc_cos
  * 功  能：反正余弦计算器：输入cos值，用asin/acos换算角度显示。
  *          上下键 ±0.01，短按确定在0.01步进间切换，长按退出
  */
void App_calc_cos(void)
{
	int8_t key, i = 0;
	float cos1 = 0, tpp, tpp1;

	Inf_oled_fade_flag = 1;

	Inf_oled_clear();
	Inf_oled_show_string(22, 0, "反正余弦计算器", OLED_6X8);
	Inf_oled_show_ascii(0, 20, "a =", OLED_6X8);
	Inf_oled_show_ascii(5, 40, "asin =", OLED_6X8);
	Inf_oled_show_ascii(5, 55, "acos =", OLED_6X8);
	Inf_oled_update();
	while (1)
	{
		tpp = asin(cos1);
		tpp1 = acos(cos1);

		Inf_oled_show_float_num(24, 20, cos1, 1, 2, OLED_6X8);
		Inf_oled_show_float_num(50, 40, tpp / 3.141592 * 180, 2, 9, OLED_6X8);
		Inf_oled_show_float_num(50, 55, tpp1 / 3.141592 * 180, 2, 9, OLED_6X8);
		Inf_oled_update();

		Inf_oled_gradient(1);

		key = Inf_key_scan();
		if (key == 1)
		{
			if (i) cos1 += 0.01;
			else cos1 += 0.01;
		}
		else if (key == 2)
		{
			if (i) cos1 -= 0.01;
			else cos1 -= 0.01;
		}

		if (key == 3) { i = !i; }
		else if (key == 4)
		{
			Inf_oled_fade_flag = 1;
			Inf_oled_gradient(0);
			return;
		}
	}
}


/* 连发加速步进封顶 */
#define APP_CALCU_STEP_MAX      200

/**
  * 函  数：App_calcu_format_bin
  * 功  能：value格式化为16位二进制字符串（两组8位，中间空格，共17字符）
  */
static void App_calcu_format_bin(uint16_t value, char* buf)
{
	uint8_t i;

	for (i = 0; i < 8; i++)
	{
		buf[i] = ((value >> (15 - i)) & 1) ? '1' : '0';
	}
	buf[8] = ' ';
	for (i = 0; i < 8; i++)
	{
		buf[9 + i] = ((value >> (7 - i)) & 1) ? '1' : '0';
	}
	buf[17] = '\0';
}

/**
  * 函  数：App_calcu_format_base
  * 功  能：value格式化为定宽进制字符串（除基取余，前导零填充；base=8/10/16）
  */
static void App_calcu_format_base(uint16_t value, uint8_t base, char* buf, uint8_t len)
{
	uint8_t pos = len;
	uint8_t d;

	buf[len] = '\0';
	while (pos > 0)
	{
		d = value % base;
		buf[--pos] = (d < 10) ? ('0' + d) : ('A' + d - 10);
		value /= base;
	}
}

/**
  * 函  数：App_calcu_convert
  * 功  能：进制转换页：16位（0~65535）二进制/十进制/八进制/十六进制互转。
  *          光标行 = 输入源，上下键改数值（连发加速：连续触发每10次步进 +10，
  *          松开或反向即重置），短按切换输入源行，长按退出
  * 说  明：内部只存单一uint16 value，四行均为它的格式化呈现（转换 = 显示格式化）
  */
void App_calcu_convert(void)
{
	char buf[18];
	uint16_t value = 0;
	uint8_t row = 2;            /* 光标行：0=BIN 1=OCT 2=DEC 3=HEX，初始十进制 */
	uint8_t key;
	uint16_t cnt = 0;           /* 连续同向触发计数（松开/反向清零） */
	uint16_t step;
	int8_t dir = 0;             /* 上次方向：0=无 1=上 -1=下 */
	uint32_t last_tick = 0;     /* 上次同向按键的1ms心跳 */

	Inf_oled_fade_flag = 1;

	Inf_oled_clear();

	Inf_oled_show_ascii(0, 0, "BIN ", OLED_6X8);
	Inf_oled_show_ascii(0, 12, "OCT ", OLED_6X8);
	Inf_oled_show_ascii(0, 24, "DEC ", OLED_6X8);
	Inf_oled_show_ascii(0, 36, "HEX ", OLED_6X8);
	Inf_oled_show_string(16, 48, "短按换行 长按退出", OLED_6X8);
	Inf_oled_update();
	while (1)
	{
		key = Inf_key_scan();

		/* 清数值与光标区 */
		Inf_oled_clear_area(24, 0, 104, 48);

		/* 行数值 */
		App_calcu_format_bin(value, buf);
		Inf_oled_show_ascii(24, 0, buf, OLED_6X8);

		App_calcu_format_base(value, 8, buf, 6);
		Inf_oled_show_ascii(24, 12, buf, OLED_6X8);

		App_calcu_format_base(value, 10, buf, 5);
		Inf_oled_show_ascii(24, 24, buf, OLED_6X8);

		Inf_oled_show_ascii(24, 36, "0x", OLED_6X8);
		App_calcu_format_base(value, 16, buf, 4);
		Inf_oled_show_ascii(36, 36, buf, OLED_6X8);

		/* 光标行高亮 */
		App_menu_draw_cursor(24, row * 12, 104, 8);

		Inf_oled_update();

		Inf_oled_gradient(1);

		if (key == 1 || key == 2)               /* 上下键：改数值（连发加速） */
		{
			uint32_t now = Drv_tim2_get_tick();
			int8_t d = (key == 1) ? 1 : -1;

			if (d != dir || (now - last_tick) > 500)
			{
				cnt = 0;                        /* 方向变化或间隔过长：重新连续计数 */
			}
			else
			{
				cnt++;
			}
			dir = d;
			last_tick = now;

			step = (uint16_t)(cnt / 10) * 10;   /* 每连续10次步进 +10：1→10→20→… */
			if (step == 0) step = 1;
			if (step > APP_CALCU_STEP_MAX) step = APP_CALCU_STEP_MAX;

			if (d > 0) { value = (uint16_t)(value + step); }   /* uint16 运算环绕 */
			else       { value = (uint16_t)(value - step); }
		}

		if (key == 3)                           /* 短按：输入源切到下一行 */
		{
			row++;
			if (row >= 4) row = 0;
		}
		else if (key == 4)                      /* 长按：退出 */
		{
			Inf_oled_fade_flag = 1;
			Inf_oled_gradient(0);
			return;
		}
	}
}
