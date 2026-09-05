#include "stm32f10x.h"
#include "Drv_delay.h"
#include "Drv_tim.h"
#include "Drv_adc.h"
#include "Drv_pwr.h"
#include "Inf_oled.h"
#include "Inf_w25q.h"
#include "Inf_battery.h"
#include "Inf_rtc.h"
#include "Inf_mp3.h"
#include "Inf_key.h"
#include "App_menu.h"
#include "App_watchface.h"
#include "App_power.h"
#include "App_settings_store.h"

uint8_t TLYPW = 0;             /* 体感开关状态标志位（MPU6050暂缓，预留） */

int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);   /* NVIC 分组2（先于所有中断初始化） */

	Drv_delay_init();   /* 延时函数初始化 */
	Inf_oled_init();    /* OLED 显示初始化 */

	Inf_oled_write_command(0x81);	/* 设置对比度 */
	Inf_oled_write_command(0x00);	/* 0x00~0xFF */

	Inf_w25q_init();    /* W25Q 字库（中文显示依赖，内部含SPI1） */
	App_settings_store_load();   /* 上电恢复用户设置 */

	Drv_tim2_init();    /* 1ms心跳（须先于RTC，进行LSI校准） */
	Inf_rtc_init();     /* RTC初始化 */
    
    Drv_adc1_init();
	Inf_battery_init(); /* 电池检测（内部含JTAG重映射PB3/PB4/PA15） */
	Inf_key_init();     /* 按键初始化 */

	/* 等待按键松开：避免唤醒按键误触发菜单 */
	{
		uint32_t last_key_tick = Drv_tim2_get_tick();
		while (1)
		{
			if (Inf_key_scan() != 0) { last_key_tick = Drv_tim2_get_tick(); }
			if ((Drv_tim2_get_tick() - last_key_tick) > 150) { break; }   /* 150ms无事件视为松开（大于连发间隔100ms） */
		}
	}
	Inf_mp3_detect_init();   /* MP3电源检测输入（暂不驱动PA12功放使能，防漏电拉高PA11） */
	Drv_pwr_wakeup_init();   /* PA0 EXTI0唤醒 */

	int8_t key;                      /* 存储按键事件值 */
	uint32_t idle_tick = Drv_tim2_get_tick();   /* 上次按键时刻(ms)，10秒无操作关机 */

	/* 开机动画 */
	Inf_oled_fade_flag = 1;
	while (Inf_oled_fade_flag)
	{
		App_watchface_run();
		Drv_delay_ms(4);
		Inf_oled_gradient(1);
	}

	while (1)
	{
		App_watchface_run();   /* 刷新表盘 */
		Inf_mp3_poll();        /* 处理MP3模块回传（0x3D 切歌等） */
		key = Inf_key_scan();  /* 获取按键事件 */

		if ((uint32_t)(Drv_tim2_get_tick() - idle_tick) > 10000)   /* 10秒无操作关机 */
		{
			key = 2;
			idle_tick = Drv_tim2_get_tick();
		}

		if (key == 1)          /* 上键：进列表菜单 */
		{
			idle_tick = Drv_tim2_get_tick();
			Inf_oled_fade_flag = 1;
			Inf_oled_gradient(0);

			App_menu_main_list();
			idle_tick = Drv_tim2_get_tick();   /* 返回表盘重新计时 */

			Inf_oled_fade_flag = 1;
		}
		else if (key == 2)     /* 下键：关机（STOP 休眠，PA0 唤醒恢复） */
		{
			idle_tick = Drv_tim2_get_tick();
			Inf_oled_fade_flag = 1;
			Inf_oled_gradient(0);

			App_power_off();
			idle_tick = Drv_tim2_get_tick();   /* 唤醒返回重新计时 */

			Inf_oled_fade_flag = 1;
		}

		/* 渐亮循环（防止关机后立刻误触发按键） */
		while (Inf_oled_fade_flag)
		{
			App_watchface_run();
			Drv_delay_ms(4);
			Inf_oled_gradient(1);
		}

		if (key == 3)          /* 确认短按：进轮盘菜单 */
		{
			idle_tick = Drv_tim2_get_tick();
			Inf_oled_fade_flag = 1;
			Inf_oled_gradient(0);

			App_menu_main_wheel();
			idle_tick = Drv_tim2_get_tick();   /* 返回表盘重新计时 */

			Inf_oled_fade_flag = 1;
		}
		else if (key == 4)     /* 长按确定：待机（STANDBY，PA0 WKUP复位重启） */
		{
			App_power_standby();
		}
	}
}
