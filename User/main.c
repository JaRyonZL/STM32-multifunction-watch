#include "stm32f10x.h"
#include "Drv_delay.h"
#include "Drv_tim.h"
#include "Inf_oled.h"
#include "Inf_oled_gfx.h"
#include "Inf_w25q.h"
#include "Inf_key.h"
#include "App_menu.h"

int main(void)
{
	Drv_delay_init();
	Inf_oled_init();
	Inf_w25q_init();
	Drv_tim2_init();
	Inf_key_init();

	while (1)
	{
		App_menu_main_wheel();
	}
}
