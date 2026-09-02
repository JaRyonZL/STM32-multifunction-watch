#include "App_mp3.h"
#include "App_watchface.h"
#include "Inf_oled.h"
#include "Inf_oled_gfx.h"
#include "Inf_key.h"
#include "Inf_mp3.h"
#include "Drv_delay.h"
#include "Com_oled_res.h"
#include "App_menu.h"

/******************************************************************************
 * 文件名称：App_mp3.c（应用层）
 * 说    明：MP3播放器应用，由旧工程MP3_tf.c界面部分迁移
 ******************************************************************************/

/**
  * 函    数：App_mp3_player
  * 功    能：MP3播放界面（原MP3os）：播放/停止图标、曲目号、播放模式文字，
  *           上下键切曲（熄屏时唤醒），短按播放/停止，长按唤醒/退出，
  *           无操作400循环后熄屏省电
  */
void App_mp3_player(void)
{
	uint8_t key;
	uint16_t idle_cnt = 2;   /* 屏幕熄灭等待计数 */

	Inf_oled_fade_flag = 1;

	while (1)
	{
		Inf_oled_clear();

		App_watchface_show_battery(90, 0);
		App_watchface_time_small(0, 0);

		if (Inf_mp3.playing)
		{
			Inf_oled_show_image(40, 27, 16, 16, mp3play);
		}
		else
		{
			Inf_oled_show_image(40, 27, 16, 16, mp3stop);
		}

		Inf_oled_show_image(18, 27, 16, 16, mp3right);
		Inf_oled_show_image(62, 27, 16, 16, mp3left);

		Inf_oled_show_num(0, 56, Inf_mp3.chapter, 3, OLED_6X8);
		Inf_oled_show_num(20, 56, Inf_mp3.chapter_max, 3, OLED_6X8);

		if (Inf_mp3.play_once)
		{
			Inf_oled_show_string(104, 52, "播停", OLED_6X8);
		}
		else if (Inf_mp3.loop_play)
		{
			Inf_oled_show_string(104, 52, "循环", OLED_6X8);
		}
		else
		{
			Inf_oled_show_string(104, 52, "顺序", OLED_6X8);
		}

		Inf_oled_update();

		Inf_oled_gradient(1);

		/* 熄屏/唤醒控制 */
		if (idle_cnt > 400)
		{
			Inf_oled_fade_flag = 1;
			Inf_oled_gradient(0);
			Inf_oled_clear();
			Inf_oled_update();

			Inf_oled_write_command(0xAE);   /* 显示关 */
			Inf_oled_write_command(0xAD);   /* 设置充电泵 */
			Inf_oled_write_command(0x8A);   /* 关闭充电泵 */

			idle_cnt = 0;
		}
		else if (idle_cnt == 1)
		{
			Inf_oled_write_command(0xAD);   /* 设置充电泵 */
			Inf_oled_write_command(0x8B);   /* 开启充电泵 */
			Inf_oled_write_command(0xAF);   /* 显示开 */

			idle_cnt++;

			Inf_oled_fade_flag = 1;
		}
		else if (idle_cnt > 1)
		{
			idle_cnt++;
		}

		key = Inf_key_scan();
		if (key == 1)               /* 上键：下一曲（熄屏时唤醒） */
		{
			if (idle_cnt)
			{
				Inf_mp3_switch_chapter(1);
				idle_cnt = 2;
			}
			else
			{
				idle_cnt++;
			}
		}
		else if (key == 2)          /* 下键：上一曲（熄屏时唤醒） */
		{
			if (idle_cnt)
			{
				Inf_mp3_switch_chapter(0);
				idle_cnt = 2;
			}
			else
			{
				idle_cnt++;
			}
		}
		else if (idle_cnt > 25 | idle_cnt == 0)
		{
			if (Inf_mp3.update_pending)     /* 曲目变更待发送 */
			{
				Inf_mp3_send_volume();
				Drv_delay_ms(50);
				Inf_mp3_send_cmd(0x12, 0x00, Inf_mp3.chapter);
				Inf_mp3.update_pending = 0;
			}
		}

		if (key == 3)               /* 短按：播放/停止（熄屏时先唤醒） */
		{
			if (idle_cnt)
			{
				idle_cnt = 2;
				Inf_mp3_start(Inf_mp3.playing ? 0 : 1);
			}
			else
			{
				idle_cnt++;
			}
		}
		else if (key == 4)          /* 长按：熄屏时唤醒，亮屏时退出 */
		{
			idle_cnt++;
			if (idle_cnt == 1)
			{
				Inf_oled_write_command(0xAF);
				Inf_oled_write_command(0xAD);
				Inf_oled_write_command(0x8B);
			}
			else
			{
				Inf_mp3_send_cmd(0x19, 0x00, 0x00);  
				Inf_oled_fade_flag = 1;
				Inf_oled_gradient(0);
				return;
			}
		}
	}
}

/**
  * 函    数：App_mp3_run
  * 功    能：MP3应用入口（原mp3up）：未上电时提示等待开启电源，初始化后进入
  *           播放界面，返回后显示设置菜单，退出时断电
  */
void App_mp3_run(void)
{
	Inf_mp3_detect_init();   /* 先初始化电源检测输入，再检查上电状态 */
	/* 未上电：提示等待开启MP3电源 */
	if (!Inf_mp3_is_powered())
	{
		Inf_oled_fade_flag = 1;
		Inf_oled_clear();
		Inf_oled_show_string(0, 13, "请开启MP3电源", OLED_6X8);
		Inf_oled_show_string(0, 26, "<--", OLED_6X8);

		while (!Inf_mp3_is_powered())
		{
			Inf_oled_gradient(1);
			Inf_oled_update();
		}

		Inf_oled_gradient(0);
	}

	Inf_mp3_init();
	Inf_mp3_loudspeaker();
	Inf_mp3_loudspeaker();   

	App_mp3_player();

	App_menu_option2_t option_list[] = {
		{"退出"      , APP_MENU_MODE_FUNCTION, APP_MENU_RETURN,    0, 0},
		{"返回界面"  , APP_MENU_MODE_FUNCTION, App_mp3_player,     0, 0},
		{"扬声器"    , APP_MENU_MODE_ON_OFF,   Inf_mp3_loudspeaker, &Inf_mp3.loudspeaker_on, 0},
		{"音量"      , APP_MENU_MODE_NUMBER,   Inf_mp3_send_volume, &Inf_mp3.volume, 0},
		{"单曲循环"  , APP_MENU_MODE_ON_OFF,   APP_MENU_RETURN,     &Inf_mp3.loop_play, 0},
		{"播完即停"  , APP_MENU_MODE_ON_OFF,   APP_MENU_RETURN,     &Inf_mp3.play_once, 0},
		{"曲目总数"  , APP_MENU_MODE_NUMBER,   APP_MENU_RETURN,     &Inf_mp3.chapter_max, 0},
		{".."}
	};

	App_menu_run_list(option_list);

	Inf_mp3_power_off();
}
