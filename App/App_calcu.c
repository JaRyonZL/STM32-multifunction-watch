/*
 * @Author: JaRyon
 * @Date: 2026-09-04 18:15:31
 * @Email: jaryonzl@163.com
 */
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
