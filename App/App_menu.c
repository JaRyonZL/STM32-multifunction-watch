#include "App_menu.h"
#include "Inf_oled.h"
#include "Inf_oled_gfx.h"
#include "Inf_key.h"
#include "Com_oled_res.h"
#include "App_watchface.h"
#include "App_voltage.h"
#include "App_mp3.h"
#include "App_video.h"
#include "App_game.h"
#include "App_menu_data.h"

/******************************************************************************
 * 文件名称：App_menu.c（应用层）
 * 说    明：菜单引擎，由旧工程menu.c迁移
 ******************************************************************************/

uint8_t App_menu_cursor = 0;    /* 光标样式：0=反色 1=方框 2=箭头 */

/**
  * 函    数：App_menu_get_name_len
  * 功    能：计算名称显示宽度（列数）：GBK汉字=2列，ASCII=1列
  * 说    明：strlen不能准确得到显示宽度，故单独计算并存储
  */
uint8_t App_menu_get_name_len(char* String)
{
	uint8_t i = 0, len = 0;
	while (String[i] != '\0')        /* 遍历字符串每个字符 */
	{
		if (String[i] > '~') { len += 2; i += 2; }   /* 中文：GB2312双字节，宽度2 */
		else { len += 1; i += 1; }                   /* ASCII：宽度1 */
	}
	return len;
}

/**
  * 函    数：App_menu_reverse_area_frame
  * 功    能：绘制空心反色框（光标反色样式用，由旧工程OLED_ReverseArea2迁移）
  */
void App_menu_reverse_area_frame(uint8_t X, uint8_t Y, uint8_t Width, uint8_t Height)
{
	Inf_oled_reverse_area(X + 1, Y, Width - 2, 1);
	Inf_oled_reverse_area(X, Y + 1, Width, Height - 2);
	Inf_oled_reverse_area(X + 1, Y + Height - 1, Width - 2, 1);
}

/**
  * 函    数：App_menu_run_list
  * 功    能：列表菜单，将选项列表显示出来，并根据按键事件执行对应动作
  * 说    明：由旧工程run_menu2迁移，选项数组以{".."}结尾
  *           按键事件由Inf_key_scan提供：1=上 2=下 3=确定短按 4=确定长按
  */
void App_menu_run_list(App_menu_option2_t* option)
{
	int8_t sel_index = 1;        /* 选中项下标 */
	int8_t cursor_row = 0;       /* 光标行 */
	int8_t show_start = 0;       /* 显示起始下标 */
	int8_t item_max = 0;         /* 选项数量-1 */
	uint8_t bar_step;            /* 进度条每项高度 */
	float bar_target = 0, bar_current = 0;   /* 进度条目标/当前高度 */

	while (option[++item_max].Name[0] != '.');   /* 以".."结尾，取选项数量 */
	item_max--;

	for (int8_t i = 0; i <= item_max; i++)       /* 计算所有选项名称宽度 */
	{
		option[i].NameLen = App_menu_get_name_len(option[i].Name);
	}

	/* 光标位置和长度的动画端点（静态变量：保留上次进入的动画状态） */
	static float cursor_pos_cur = 0, cursor_pos_target = 0, cursor_len_cur = 0, cursor_len_target = 0;

	int8_t scroll_offset = 0, show_start_prev = item_max;   /* 显示滚动偏移 */

	bar_step = 64 / item_max;   /* 屏幕高度/选项数量 */

	Inf_oled_fade_flag = 1;

	while (1)
	{
		uint8_t key = Inf_key_scan();   /* 获取按键事件 */

		Inf_oled_clear();

		if (key == 1 || key == 2)       /* 如果有上下键事件 */
		{
			int8_t roll = (key == 1) ? 1 : -1;
			cursor_row += roll;         /* 光标行变化 */
			sel_index += roll;          /* 选中项下标变化 */

			if (sel_index < 0) { sel_index = 0; }               /* 限制选中项下标 */
			if (sel_index > item_max) { sel_index = item_max; }

			if (cursor_row < 0) { cursor_row = 0; }             /* 限制光标行位置 */
			if (cursor_row > 3) { cursor_row = 3; }
			if (cursor_row > item_max) { cursor_row = item_max; }
		}

		/* 显示部分 */
		show_start = sel_index - cursor_row;    /* 计算显示起始下标 */

		if (show_start - show_start_prev)       /* 若下标有偏移 */
		{
			scroll_offset = (show_start - show_start_prev) * APP_MENU_ROW_H;
			show_start_prev = show_start;
		}
		if (scroll_offset) { scroll_offset /= APP_MENU_SCROLL_SPEED; }   /* 滚动变化速度 */

		for (int8_t i = 0; i < 5; i++)          /* 最多显示5行选项和对应模式 */
		{
			if (show_start + i > item_max) { break; }
			Inf_oled_show_string(2, (i * APP_MENU_ROW_H) + scroll_offset + 2, option[show_start + i].Name, OLED_6X8);

			if (option[show_start + i].mode == APP_MENU_MODE_ON_OFF)    /* 如果是开关项 */
			{
				Inf_oled_draw_rectangle(96, (i * APP_MENU_ROW_H) + scroll_offset + 2, 12, 12, OLED_UNFILLED);
				if (*(option[show_start + i].Num))                      /* 如果开关状态为开 */
				{
					Inf_oled_draw_rectangle(99, (i * APP_MENU_ROW_H) + scroll_offset + 5, 6, 6, OLED_FILLED);
				}
			}
			else if (option[show_start + i].mode == APP_MENU_MODE_NUMBER)   /* 如果是数值项 */
			{
				/* 如果百位不为零 */
				if (*(option[show_start + i].Num) / Inf_oled_pow(10, 3 - 0 - 1) % 10)
					Inf_oled_show_num(92, (i * APP_MENU_ROW_H) + scroll_offset + 6, *(option[show_start + i].Num), 3, OLED_6X8);
				/* 如果十位不为零 */
				else if (*(option[show_start + i].Num) / Inf_oled_pow(10, 3 - 1 - 1) % 10)
					Inf_oled_show_num(95, (i * APP_MENU_ROW_H) + scroll_offset + 6, *(option[show_start + i].Num), 2, OLED_6X8);
				/* 如果个位不为零 */
				else if (*(option[show_start + i].Num) / Inf_oled_pow(10, 3 - 2 - 1) % 10)
					Inf_oled_show_num(98, (i * APP_MENU_ROW_H) + scroll_offset + 6, *(option[show_start + i].Num), 1, OLED_6X8);
				else
					Inf_oled_show_num(98, (i * APP_MENU_ROW_H) + scroll_offset + 6, 0, 1, OLED_6X8);
			}
		}

		/* 光标 */
		cursor_pos_target = cursor_row * APP_MENU_ROW_H;                         /* 查询光标目标位置 */
		cursor_len_target = option[sel_index].NameLen * APP_MENU_WORD_W + 4;     /* 查询光标目标长度 */

		/* 如果本次循环光标位置与目标位置不同，当前位置向目标位置移动 */
		if ((cursor_pos_target - cursor_pos_cur) > 1) { cursor_pos_cur += (cursor_pos_target - cursor_pos_cur) / APP_MENU_CURSOR_SPEED + 1; }
		else if ((cursor_pos_target - cursor_pos_cur) < -1) { cursor_pos_cur += (cursor_pos_target - cursor_pos_cur) / APP_MENU_CURSOR_SPEED - 1; }
		else { cursor_pos_cur = cursor_pos_target; }

		/* 如果本次循环光标长度与目标长度不同，当前长度向目标长度移动 */
		if ((cursor_len_target - cursor_len_cur) > 1) { cursor_len_cur += (cursor_len_target - cursor_len_cur) / APP_MENU_CURSOR_SPEED + 1; }
		else if ((cursor_len_target - cursor_len_cur) < -1) { cursor_len_cur += (cursor_len_target - cursor_len_cur) / APP_MENU_CURSOR_SPEED - 1; }
		else { cursor_len_cur = cursor_len_target; }

		/* 显示光标 */
		if (App_menu_cursor == 0)
			App_menu_reverse_area_frame(0, cursor_pos_cur, cursor_len_cur, 16);   /* 在光标位置取反 */
		else if (App_menu_cursor == 1)
			Inf_oled_draw_rectangle(0, cursor_pos_cur, cursor_len_cur, 16, OLED_UNFILLED);
		else if (App_menu_cursor == 2)
			Inf_oled_show_string(cursor_len_cur, cursor_pos_cur + 2, "<-", OLED_6X8);   /* 尾部光标 */

		/* 进度条 */
		if (sel_index == item_max) { bar_target = 64; }
		else { bar_target = bar_step * sel_index; }

		if (bar_target - bar_current > 1) { bar_current += (bar_target - bar_current) / APP_MENU_BAR_SPEED + 1; }
		else if (bar_current - bar_target > 1) { bar_current -= (bar_current - bar_target) / APP_MENU_BAR_SPEED + 1; }
		else { bar_current = bar_target; }

		Inf_oled_draw_line(123, 0, 127, 0);
		Inf_oled_draw_line(125, 0, 125, 63);
		Inf_oled_draw_line(123, 63, 127, 63);

		Inf_oled_draw_rectangle(123, 0, 5, bar_current, OLED_FILLED);

		/* 刷新屏幕 */
		Inf_oled_update();

		Inf_oled_gradient(1);

		/* 获取按键 */
		if (key == 3 || key == 4)       /* 短按与长按在此处等效 */
		{
			if (option[sel_index].mode == APP_MENU_MODE_FUNCTION)    /* 如果是可进入函数 */
			{
				/* 如果功能不为空执行功能，否则返回 */
				if (option[sel_index].func)
				{
					Inf_oled_fade_flag = 1;
					Inf_oled_gradient(0);

					option[sel_index].func();

					Inf_oled_fade_flag = 1;
				}
				else
				{
					Inf_oled_fade_flag = 1;
					Inf_oled_gradient(0);

					return;
				}
			}
			else if (option[sel_index].mode == APP_MENU_MODE_ON_OFF)  /* 如果是开关 */
			{
				*(option[sel_index].Num) = !*(option[sel_index].Num);   /* 当前开关取反 */

				if (option[sel_index].func) { option[sel_index].func(); }   /* 执行功能一次 */
			}
			else if (option[sel_index].mode == APP_MENU_MODE_NUMBER)    /* 如果是数值 */
			{
				Inf_oled_fade_flag = 1;
				Inf_oled_gradient(0);

				/* 内部的数值调节界面 */
				float box_size = 0, box_size_target = 96;   /* 方框当前大小/目标大小 */
				float bar_width = 0, bar_width_target;      /* 进度条当前宽度/目标宽度 */

				Inf_oled_fade_flag = 1;

				while (1)
				{
					uint8_t key_in = Inf_key_scan();

					if (box_size < box_size_target)     /* 如果方框大小还没到 */
					{
						if (box_size_target - box_size > 25) box_size += 5;
						else if (box_size_target - box_size > 5) box_size += 2.5;
						else if (box_size_target - box_size > 0) box_size += 0.5;

						Inf_oled_draw_rectangle(16, 8, box_size, box_size / 2, OLED_UNFILLED);
						Inf_oled_clear_area(17, 9, box_size - 2, box_size / 2 - 2);

						Inf_oled_update();
					}
					else
					{
						bar_width_target = *(option[sel_index].Num) * 0.31;
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

						Inf_oled_clear_area(24, 43, 80, 3);
						Inf_oled_draw_rectangle(24, 43, bar_width, 3, OLED_FILLED);

						Inf_oled_clear_area(55, 20, 18, 8);
						/* 如果百位不为零 */
						if (*(option[sel_index].Num) / Inf_oled_pow(10, 3 - 0 - 1) % 10)
							Inf_oled_show_num(55, 20, *(option[sel_index].Num), 3, OLED_6X8);
						/* 如果十位不为零 */
						else if (*(option[sel_index].Num) / Inf_oled_pow(10, 3 - 1 - 1) % 10)
							Inf_oled_show_num(58, 20, *(option[sel_index].Num), 2, OLED_6X8);
						/* 如果个位不为零 */
						else if (*(option[sel_index].Num) / Inf_oled_pow(10, 3 - 2 - 1) % 10)
							Inf_oled_show_num(61, 20, *(option[sel_index].Num), 1, OLED_6X8);
						else
							Inf_oled_show_num(61, 20, 0, 1, OLED_6X8);

						Inf_oled_update();
					}

					Inf_oled_gradient(1);

					if (key_in == 1)    /* 数值调节：每次变化执行功能一次 */
					{
						*(option[sel_index].Num) += 1;
						if (option[sel_index].func) { option[sel_index].func(); }
					}
					else if (key_in == 2)
					{
						*(option[sel_index].Num) -= 1;
						if (option[sel_index].func) { option[sel_index].func(); }
					}

					if (key_in == 3 || key_in == 4)     /* 确定退出（短按/长按等效） */
					{
						Inf_oled_fade_flag = 1;
						Inf_oled_gradient(0);
						Inf_oled_clear();
						Inf_oled_update();

						Inf_oled_fade_flag = 1;
						break;
					}
				}
			}
			/* 既不是函数也不是开关和数值的模式只做展示，不做处理 */
		}
		/* 旧工程menu_Back_event恒0，对应分支已删除 */
	}
}

/**
  * 函    数：App_menu_run_wheel
  * 功    能：32x32图标轮盘菜单，根据按键事件切换并执行对应功能
  * 说    明：由旧工程run_menu1迁移，选项数组以{".."}结尾
  *           按键事件由Inf_key_scan提供：1=上 2=下 3=确定短按 4=确定长按
  */
void App_menu_run_wheel(App_menu_option1_t* option)
{
	int8_t sel_index = 1;        /* 选中项下标 */
	int8_t item_max = 0;         /* 选项数量-1 */
	int8_t scroll_offset = 0;    /* 图标滚动偏移 */
	int8_t show_center_prev;     /* 上次显示中心下标 */
	int8_t icon_rise = 40;       /* 图标上移量（进入时从上方落下） */
	float name_width_target = 0, name_width_current = 0;   /* 名称下划线目标/当前宽度 */

	while (option[++item_max].Name[0] != '.');   /* 以".."结尾，取选项数量 */
	item_max--;

	for (int8_t i = 0; i <= item_max; i++)       /* 计算所有选项名称宽度 */
	{
		option[i].NameLen = App_menu_get_name_len(option[i].Name);
	}

	show_center_prev = item_max;

	Inf_oled_fade_flag = 1;

	while (1)
	{
		uint8_t key = Inf_key_scan();   /* 获取按键事件 */

		Inf_oled_clear();

		if (key == 1 || key == 2)       /* 如果有上下键事件 */
		{
			sel_index += (key == 1) ? 1 : -1;

			if (sel_index < 0) { sel_index = 0; }               /* 限制选中项下标 */
			if (sel_index > item_max) { sel_index = item_max; }
		}

		/* 显示部分 */
		if (sel_index - show_center_prev)       /* 若中心下标有偏移 */
		{
			scroll_offset = (sel_index - show_center_prev) * 40;
			show_center_prev = sel_index;

			name_width_target = option[sel_index].NameLen * APP_MENU_WORD_W;
		}
		if (scroll_offset) { scroll_offset /= 1.15; }   /* 滚动变化速度 */
		if (icon_rise) { icon_rise /= 1.3; }            /* 图标落下变化速度 */

		if (name_width_current < name_width_target)     /* 下划线宽度比例逼近 */
		{
			if (name_width_target - name_width_current > 10) name_width_current += 5;
			else if (name_width_target - name_width_current > 5) name_width_current += 2.5;
			else if (name_width_target - name_width_current > 0) name_width_current += 0.5;
		}
		if (name_width_current > name_width_target)
		{
			if (name_width_current - name_width_target > 10) name_width_current -= 5;
			else if (name_width_current - name_width_target > 5) name_width_current -= 2.5;
			else if (name_width_current - name_width_target > 0) name_width_current -= 0.5;
		}

		for (int8_t i = -2; i < 3; i++)     /* 显示中心前后各2个图标 */
		{
			int8_t icon_x = i * 40 + scroll_offset + 48;

			if (sel_index + i < 0) { continue; }
			if (sel_index + i > item_max) { break; }

			Inf_oled_show_image(icon_x, 8 - icon_rise, 32, 32, option[sel_index + i].Image);
		}

		/* 名称与下划线（旧工程在图标循环内重复绘制5次，效果等价，提至循环外） */
		uint8_t name_shift = abs(scroll_offset / 3);

		Inf_oled_show_string(64 - name_width_target / 2, 50 + name_shift, option[sel_index].Name, OLED_6X8);

		Inf_oled_draw_rectangle(64 - (name_width_current / 2) - 2, 63, name_width_current + 4, 1, OLED_UNFILLED);

		/* 刷新屏幕 */
		Inf_oled_update();

		Inf_oled_gradient(1);

		/* 获取按键（短按/长按等效，与旧工程一致） */
		if (key == 3 || key == 4)
		{
			/* 如果功能不为空执行功能，否则返回 */
			if (option[sel_index].func)
			{
				Inf_oled_fade_flag = 1;
				Inf_oled_gradient(0);

				option[sel_index].func();

				Inf_oled_fade_flag = 1;
				icon_rise = 40;
				scroll_offset = -40;
			}
			else
			{
				Inf_oled_fade_flag = 1;
				Inf_oled_gradient(0);

				return;
			}
		}
		/* 旧工程menu_Back_event恒0，对应分支已删除 */
	}
}

/******************************************************************************
 * 应用入口占位函数（C3~C13迁移对应模块后删除，改用各App模块真实实现）
 ******************************************************************************/
void App_qrcode_wechat(void) {}
void App_qrcode_zfb(void) {}
void App_calc_cos(void) {}
void App_font_browser(void) {}
void App_flashlight(void) {}
void App_error(void) {}

/**
  * 函    数：App_menu_main_wheel
  * 功    能：表盘轮盘菜单入口（旧工程main_menu1迁移）
  * 说    明：选项格式{名称, 选中执行函数, 32x32图标}
  */
void App_menu_main_wheel(void)
{
	App_menu_option1_t option_list[] = {
		{"返回表盘"        , APP_MENU_RETURN,   APPBIPAN},
		{"电压测量"        , App_voltage_run,   APPADDV},
		{"音乐"            , App_mp3_run,       APPYYBF},
		{"收款微信"        , App_qrcode_wechat, APPWXZF},
		{"收款支付宝"      , App_qrcode_zfb,    APPZFBZF},
		{"视频"            , App_video_menu,    APPVIDEO},
		{"游戏"            , App_game_menu,     APPGAME},
		{"cos(x)-1"        , App_calc_cos,      bug},
		{"W25Q128字库浏览" , App_font_browser,  bug},
		{"设置"            , App_settings_run,  APPsetting},
		{".."}                                  /* 结尾标志，不可删除 */
	};

	App_menu_run_wheel(option_list);
}

/**
  * 函    数：App_menu_main_list
  * 功    能：列表菜单入口（旧工程main_menu2迁移）
  * 说    明：选项格式{名称, 模式, 选中执行函数, 绑定变量, 名称宽度}
  */
void App_menu_main_list(void)
{
	App_menu_option2_t option_list[] = {
		{"- 返回"    , APP_MENU_MODE_FUNCTION, APP_MENU_RETURN,         0, 0},
		{"- 手电筒"  , APP_MENU_MODE_FUNCTION, App_flashlight,          0, 0},
		{"- 便签"    , APP_MENU_MODE_FUNCTION, App_error,               0, 0},
		{"- 时间调整", APP_MENU_MODE_FUNCTION, App_watchface_time_adjust, 0, 0},
		{".."}                                  /* 结尾标志，不可删除 */
	};

	App_menu_run_list(option_list);
}
