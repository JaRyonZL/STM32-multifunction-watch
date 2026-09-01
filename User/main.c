#include "stm32f10x.h"
#include "Drv_delay.h"
#include "Drv_adc.h"
#include "Drv_tim.h"
#include "Drv_rtc.h"
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
	Inf_mp3_init();
	Inf_key_init();

	/* Calibration diagnostic: prescaler and measured RTC tick length */
	Inf_oled_clear();
	{
		uint32_t c0 = Drv_rtc_get_counter();
		while (Drv_rtc_get_counter() == c0) {}
		uint32_t u0 = Drv_tim2_get_us();
		c0 = Drv_rtc_get_counter();
		while (Drv_rtc_get_counter() == c0) {}
		uint32_t u1 = Drv_tim2_get_us();
		Inf_oled_printf(0, 16, OLED_6X8, "tick=%dus", (int)(u1 - u0));
	}
	{
		uint32_t d0 = Drv_tim2_get_us();
		Drv_delay_ms(1000);
		uint32_t d1 = Drv_tim2_get_us();
		Inf_oled_printf(0, 32, OLED_6X8, "dly=%dus", (int)(d1 - d0));
	}
	Inf_oled_update();
	Drv_delay_ms(5000);

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
