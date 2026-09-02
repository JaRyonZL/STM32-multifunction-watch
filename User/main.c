#include "stm32f10x.h"
#include "Drv_delay.h"
#include "Drv_adc.h"
#include "Drv_tim.h"
#include "Inf_oled.h"
#include "Inf_oled_gfx.h"
#include "Inf_w25q.h"
#include "Inf_battery.h"
#include "Inf_rtc.h"
#include "Inf_mp3.h"
#include "Inf_key.h"
#include "App_menu.h"
#include "App_watchface.h"

uint8_t TLYPW = 0;   /* 体感开关状态（预留） */

int main(void)
{
	Drv_delay_init();
	Inf_oled_init();
	Inf_w25q_init();
	Drv_adc1_init();
	Inf_battery_init();
	Drv_tim2_init();
	Inf_rtc_init();
	Inf_mp3_detect_init();
	Inf_key_init();

	while (1)
	{
		/* 表盘显示 */
		Inf_oled_fade_flag = 1;
		App_watchface_run();
		Drv_delay_ms(4);
		Inf_oled_gradient(1);

		int8_t key = Inf_key_scan();

		if (key == 1)       /* 上键：进列表菜单，返回后进轮盘菜单 */
		{
			Inf_oled_fade_flag = 1;
			Inf_oled_gradient(0);
			App_menu_main_list();
			Inf_oled_fade_flag = 1;

			Inf_oled_fade_flag = 1;
			Inf_oled_gradient(0);
			App_menu_main_wheel();
			Inf_oled_fade_flag = 1;
		}
		/* 下键：关机 */
	}
}
