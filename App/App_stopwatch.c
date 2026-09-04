#include "App_stopwatch.h"
#include "App_watchface.h"
#include "App_menu.h"
#include "Inf_oled.h"
#include "Inf_oled_gfx.h"
#include "Inf_key.h"
#include "Drv_tim.h"

/******************************************************************************
 * 文件名：App_stopwatch.c（应用层）
 * 说   明：秒表功能：大数字显示时分秒（00:00:00），底部三选项
 *          （记录/启停/重置）左右键选、短按确定执行、长按确定进记录页
 *          计时基于 Drv_tim2 1ms 心跳（tick 差计算，界面重绘不影响走时）
 *          选项高亮复用 App_menu_draw_cursor，与菜单光标样式统一
 ******************************************************************************/

/* 秒表状态 */
typedef struct
{
	uint8_t  running;          /* 运行标志：1=计时中 */
	uint32_t start_tick;       /* 本段计时起点(ms) */
	uint32_t elapsed_paused;   /* 暂停累计(ms) */
	uint32_t records[15];      /* 记录(ms)，上限 15 条 */
	uint8_t  record_count;     /* 已记录条数 */
	uint8_t  option_index;     /* 底部光标：0=记录 1=启停 2=重置 */
	uint32_t tip_tick;         /* 提示消失时刻(ms) */
} App_stopwatch_t;

static App_stopwatch_t App_stopwatch;

/* 底部选项区布局 */
#define APP_STOPWATCH_OPTION_Y  48   /* 选项行Y坐标 */
#define APP_STOPWATCH_OPTION_W  24   /* 每项宽 */

/* 选项名称表（显示用，中文依赖 W25Q 字库） */
static char* App_stopwatch_options[3] = { "记录", "启停", "重置" };

/**
  * 函   数：App_stopwatch_elapsed
  * 功   能：计算当前秒表时间(ms)。基于 tick 差：运行=暂停累计+(now-起点)，
  *          暂停=暂停累计。同轮单快照（调用方每循环只读一次 now）
  */
static uint32_t App_stopwatch_elapsed(uint32_t now)
{
	if (App_stopwatch.running)
	{
		return App_stopwatch.elapsed_paused + (now - App_stopwatch.start_tick);
	}
	return App_stopwatch.elapsed_paused;
}

/**
  * 函   数：App_stopwatch_show_time
  * 功   能：大数字显示时分秒（00:00:00，16x16 图标，冒号 2x4 小矩形）
  */
static void App_stopwatch_show_time(uint32_t elapsed)
{
	uint8_t h = (uint8_t)((elapsed / 3600000UL) % 100);
	uint8_t m = (uint8_t)((elapsed / 60000UL) % 60);
	uint8_t s = (uint8_t)((elapsed / 1000UL) % 60);

	App_watchface_draw_digit(8, 16, h / 10, 1);
	App_watchface_draw_digit(24, 16, h % 10, 1);

	Inf_oled_draw_rectangle(40, 22, 2, 4, OLED_UNFILLED);   /* 冒号上点 */
	Inf_oled_draw_rectangle(40, 37, 2, 4, OLED_UNFILLED);   /* 冒号下点 */

	App_watchface_draw_digit(48, 16, m / 10, 1);
	App_watchface_draw_digit(64, 16, m % 10, 1);

	Inf_oled_draw_rectangle(80, 22, 2, 4, OLED_UNFILLED);
	Inf_oled_draw_rectangle(80, 37, 2, 4, OLED_UNFILLED);

	App_watchface_draw_digit(88, 16, s / 10, 1);
	App_watchface_draw_digit(104, 16, s % 10, 1);
}

/**
  * 函   数：App_stopwatch_show_options
  * 功   能：底部三选项显示 + 选中项光标高亮（复用 App_menu_draw_cursor，
  *          跟随系统光标样式设置）
  */
static void App_stopwatch_show_options(void)
{
	uint8_t i;
	uint8_t x0 = (128 - APP_STOPWATCH_OPTION_W * 3) / 4;  

	for (i = 0; i < 3; i++)
	{
		uint8_t x = x0 + i * (x0 + APP_STOPWATCH_OPTION_W);
		Inf_oled_show_string(x, APP_STOPWATCH_OPTION_Y + 2,
		                     App_stopwatch_options[i], OLED_6X8);
	}

	/* 选中项光标（与菜单光标样式统一） */
	App_menu_draw_cursor(x0 + App_stopwatch.option_index * (APP_STOPWATCH_OPTION_W + x0),
	                     APP_STOPWATCH_OPTION_Y, APP_STOPWATCH_OPTION_W, 16);
}

/**
  * 函   数：App_stopwatch_run
  * 功   能：秒表主循环：每循环同轮单快照 now，大数字+顶栏+三选项重绘；
  *          1/2 移光标，3 执行选中项（S1：启停/重置，记录 S2 补），
  *          4 长按进记录页（S2 补）
  */
/**
  * 函   数：App_stopwatch_records_page
  * 功   能：记录查看页（自绘）：顶栏+标题+4 行记录滚动显示（01 00:00:00），
  *          上下键滚动、短按确定返回秒表页、长按确定退出秒表功能
  * 返回值：0=返回秒表页 1=退出秒表功能
  */
static uint8_t App_stopwatch_records_page(void)
{
	int8_t key;
	uint8_t scroll = 0;   /* 滚动偏移（首条显示序号） */
	uint8_t i;

	while (1)
	{
		uint8_t idx;
		char buf[16];
		uint32_t rec;

		key = Inf_key_scan();

		Inf_oled_clear();
		App_watchface_time_small(0, 0);
		App_watchface_show_battery(90, 0);

		Inf_oled_show_string(0, 14, "记录", OLED_6X8);   /* 标题 */

		if (App_stopwatch.record_count == 0)
		{
			Inf_oled_show_string(0, 28, "暂无记录", OLED_6X8);
		}
		else
		{
			for (i = 0; i < 3; i++)   /* 最多显示 3 行 */
			{
				idx = scroll + i;
				if (idx >= App_stopwatch.record_count) break;

				rec = App_stopwatch.records[idx];
				sprintf(buf, "%02d %02d:%02d:%02d", idx + 1,
				        (uint8_t)(rec / 3600000UL % 100),
				        (uint8_t)(rec / 60000UL % 60),
				        (uint8_t)(rec / 1000UL % 60));
				Inf_oled_show_ascii(0, 28 + i * 8, buf, OLED_6X8);
				Inf_oled_reverse_area(0, 28 + i * 8, 128, 8);   /* 反色背景 */
			}
		}

		Inf_oled_show_string(0, 52, "短按返回 长按退出", OLED_6X8);   /* 底部提示 */

		Inf_oled_update();
		Inf_oled_gradient(1);

		if (key == 2 && scroll > 0)
		{
			scroll--;
		}
		else if (key == 1 && scroll + 3 < App_stopwatch.record_count)
		{
			scroll++;
		}
		else if (key == 3)   /* 短按：返回秒表页 */
		{
			Inf_oled_fade_flag = 1;
			return 0;
		}
		else if (key == 4)   /* 长按：退出秒表功能 */
		{
			Inf_oled_fade_flag = 1;
			Inf_oled_gradient(0);
			return 1;
		}
	}
}

void App_stopwatch_run(void)
{
	int8_t key;
	uint32_t now;
	uint32_t elapsed;

	Inf_oled_fade_flag = 1;

	while (1)
	{
		now = Drv_tim2_get_tick();   /* 同轮单快照：显示与按键处理共用 */
		key = Inf_key_scan();

		elapsed = App_stopwatch_elapsed(now);

		Inf_oled_clear();
		App_watchface_time_small(0, 0);
		App_watchface_show_battery(90, 0);
		App_stopwatch_show_time(elapsed);
		App_stopwatch_show_options();

		if (now < App_stopwatch.tip_tick)   /* 记录已满提示 2 秒 */
		{
			Inf_oled_show_string(52, 40, "记录已满", OLED_6X8);
		}

		Inf_oled_update();
		Inf_oled_gradient(1);

		if (key == 1)   /* 上键：光标左移（环绕） */
		{
			if (App_stopwatch.option_index == 0) { App_stopwatch.option_index = 2; }
			else { App_stopwatch.option_index--; }
		}
		else if (key == 2)   /* 下键：光标右移（环绕） */
		{
			if (App_stopwatch.option_index == 2) { App_stopwatch.option_index = 0; }
			else { App_stopwatch.option_index++; }
		}
		else if (key == 3)   /* 短按确定：执行选中项 */
		{
			if (App_stopwatch.option_index == 0)
			{
				if (App_stopwatch.record_count >= 15)   /* 记录已满：提示不写入 */
				{
					App_stopwatch.tip_tick = now + 2000;
				}
				else
				{
					App_stopwatch.records[App_stopwatch.record_count] = elapsed;
					App_stopwatch.record_count++;
				}
			}
			else if (App_stopwatch.option_index == 1)
			{
				if (App_stopwatch.running)   /* 停：快照计入暂停累计 */
				{
					App_stopwatch.elapsed_paused = elapsed;
					App_stopwatch.running = 0;
				}
				else   /* 启：记录本段起点 */
				{
					App_stopwatch.start_tick = now;
					App_stopwatch.running = 1;
				}
			}
			else   /* 重置：清零并清空运行状态与记录 */
			{
				App_stopwatch.elapsed_paused = 0;
				App_stopwatch.running = 0;
				App_stopwatch.record_count = 0;
			}
		}
		else if (key == 4)   /* 长按：进记录页；页内长按=退出功能 */
		{
			if (App_stopwatch_records_page())
			{
				Inf_oled_fade_flag = 1;
				Inf_oled_gradient(0);
				return;
			}
			Inf_oled_fade_flag = 1;   /* 返回秒表页：重新渐亮 */
		}
	}
}
