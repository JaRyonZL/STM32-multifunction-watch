#include "App_game.h"
#include "App_menu.h"
#include "Inf_oled.h"
#include "Inf_oled_gfx.h"
#include "Inf_key.h"

/******************************************************************************
 * 文件名：App_game.c（应用层）
 * 说   明：游戏应用
 * 			生命游戏：（迁移自旧工程 menu_Data.c 的 Game_Of_Life 全家）
 *          玩法：光标移动画线作为图案，按生命游戏规则自动演化；
 *          短按确定切换光标移动轴向（横/纵），长按退出
 ******************************************************************************/

static uint8_t App_game_life_cache[8][128];   /* 生命计算缓冲 */



/* 生命游戏静态函数声明 */

static void App_game_life_update_display(void);
static void App_game_life_point_live(uint8_t X, uint8_t Y);
static void App_game_life_point_death(uint8_t X, uint8_t Y);
static uint8_t App_game_life_count_round(uint8_t X, uint8_t Y);
static void App_game_life_turn(void);
static void App_game_life_seed(int16_t seed);



/**
  * 函   数：App_game_menu
  * 功   能：游戏子菜单：选择游戏进入（后续扩展游戏在此加项）
  */
void App_game_menu(void)
{
	App_menu_option2_t option_list[] = {
		{"- 退出"    , APP_MENU_MODE_FUNCTION, APP_MENU_RETURN,   0, 0},
		{"- 生命游戏", APP_MENU_MODE_FUNCTION, App_game_life_run, 0, 0},
		{".."}
	};

	App_menu_run_list(option_list);
}

/**
  * 函   数：App_game_life_run
  * 功   能：生命游戏（旧 Game_Of_Life_Play）
  * 说   明：光标移动画线作为图案种子，生命规则自动演化；
  *          短按确定切换移动轴向，长按退出
  */
void App_game_life_run(void)
{
	uint8_t x1 = 8, x2 = 16, y1 = 32, y2 = 32;
	uint8_t i, j;
	int8_t shift = -1;
	int8_t key, roll;

	Inf_oled_fade_flag = 1;

	Inf_oled_clear();
	for (j = 0; j < 8; j++)				/* 遍历8页 */
	{
		for (i = 0; i < 128; i++)		/* 遍历128列 */
		{
			App_game_life_cache[j][i] = 0x00;	/* 清空计算缓冲区，全图无活点 */
		}
	}

	while (1)
	{
		App_game_life_turn();

		key = Inf_key_scan();
		if (key == 1) roll = 1;
		else if (key == 2) roll = -1;
		else roll = 0;

		if (shift > 0) { y2 += roll * 8; }
		else { x2 += roll * 8; }
		x2 %= 128;
		y2 %= 64;
		Inf_oled_draw_line(x1, y1, x2, y2);
		if ((x2 - x1) > 1) { x1 += (x2 - x1) / 8 + 1; }
		else if ((x2 - x1) < -1) { x1 += (x2 - x1) / 8 - 1; }
		else { x1 = x2; }
		if ((y2 - y1) > 1) { y1 += (y2 - y1) / 2 + 1; }
		else if ((y2 - y1) < -1) { y1 += (y2 - y1) / 2 - 1; }
		else { y1 = y2; }

		Inf_oled_update();
		Inf_oled_gradient(1);

		if (key == 3) { shift = -shift; }
		else if (key == 4)
		{
			Inf_oled_fade_flag = 1;
			Inf_oled_gradient(0);
			return;
		}
	}
}


/**
  * 函   数：App_game_life_update_display
  * 功   能：计算缓冲整体刷入显存
  */
static void App_game_life_update_display(void)
{
	memcpy(Inf_oled_display_buf, App_game_life_cache, 1024);
}

/**
  * 函   数：App_game_life_point_live
  * 功   能：写活点
  */
static void App_game_life_point_live(uint8_t X, uint8_t Y)
{
	App_game_life_cache[(Y / 8)][X] |= (0x01 << (Y % 8));
}

/**
  * 函   数：App_game_life_point_death
  * 功   能：写死点
  */
static void App_game_life_point_death(uint8_t X, uint8_t Y)
{
	App_game_life_cache[(Y / 8)][X] &= ~(0x01 << (Y % 8));
}

/**
  * 函   数：App_game_life_count_round
  * 功   能：统计某点周围存活细胞数
  */
static uint8_t App_game_life_count_round(uint8_t X, uint8_t Y)
{
	return (
	Inf_oled_get_point(X - 1, Y - 1) + 	Inf_oled_get_point(X, Y - 1) + 	Inf_oled_get_point(X + 1, Y - 1) +
	Inf_oled_get_point(X - 1, Y) + 							Inf_oled_get_point(X + 1, Y) +
	Inf_oled_get_point(X - 1, Y + 1) + 	Inf_oled_get_point(X, Y + 1) + 	Inf_oled_get_point(X + 1, Y + 1)
	);
}

/**
  * 函   数：App_game_life_turn
  * 功   能：推演一轮
  */
static void App_game_life_turn(void)
{
	uint8_t x, y;
	uint8_t Count;

	for (y = 0; y < 64; y++)
	{
		for (x = 0; x < 128; x++)
		{
			Count = App_game_life_count_round(x, y);
			if (Inf_oled_get_point(x, y))
			{
				if (Count < 2 || Count > 3)
				{
					App_game_life_point_death(x, y);
				}
			}
			else
			{
				if (Count == 3)
				{
					App_game_life_point_live(x, y);
				}
			}
		}
	}
	App_game_life_update_display();
}

/**
  * 函   数：App_game_life_seed
  * 功   能：随机开局（未调用，保留）
  */
static void App_game_life_seed(int16_t seed)
{
	uint8_t i, j;

	srand(seed);
	for (i = 0; i < 8; i++)
	{
		for (j = 0; j < 128; j++)
		{
			App_game_life_cache[i][j] = rand();
		}
	}
	App_game_life_update_display();
}

