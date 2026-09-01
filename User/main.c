#include "stm32f10x.h"
#include "Drv_delay.h"
#include "Inf_oled.h"
#include "Inf_oled_gfx.h"
#include "Inf_rtc.h"
#include "App_watchface.h"

int main(void)
{
	Drv_delay_init();
	Inf_oled_init();
	Inf_rtc_init();

	while (1)
	{
		Inf_oled_clear();
		App_watchface_show_time(5, 18);
		Inf_oled_update();
		Drv_delay_ms(200);
	}
}
