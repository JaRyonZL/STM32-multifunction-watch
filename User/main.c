#include "stm32f10x.h"
#include "Drv_delay.h"
#include "Drv_tim.h"
#include "Inf_oled.h"
#include "Inf_oled_gfx.h"
#include "Inf_w25q.h"
#include "Inf_key.h"
#include "App_menu.h"

/* 测试变量与占位函数 */
uint8_t g_test_switch = 0;
uint8_t g_test_number = 50;

void Test_action(void)
{
}

int main(void)
{
	Drv_delay_init();
	Inf_oled_init();
	Inf_w25q_init();
	Drv_tim2_init();
	Inf_key_init();

	/* 测试选项表：覆盖4种模式 */
	App_menu_option2_t option_list[] = {
		{"返回"  , APP_MENU_MODE_FUNCTION, NULL, 0, 0},
		{"开关项", APP_MENU_MODE_ON_OFF,   Test_action, &g_test_switch, 0},
		{"数值项", APP_MENU_MODE_NUMBER,   Test_action, &g_test_number, 0},
		{"展示项", APP_MENU_MODE_DISPLAY,         NULL,              0, 0},
		{".."}
	};

	while (1)
	{
		App_menu_run_list(option_list);
	}
}
