#include "App_game.h"
#include "App_menu.h"
#include "Inf_oled.h"
#include "Inf_oled_gfx.h"
#include "Inf_key.h"
#include "Com_oled_res.h"
#include "Drv_tim.h"

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
		{"- 打砖块"  , APP_MENU_MODE_FUNCTION, App_game_block_run, 0, 0},
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





/* 打砖块游戏：布局参数 */
#define APP_GAME_BLOCK_COLS      17      /* 砖块列数 */
#define APP_GAME_BLOCK_ROWS      5       /* 砖块行数 */
#define APP_GAME_BLOCK_X0        5       /* 砖块区距左边界 */
#define APP_GAME_BLOCK_Y0        2       /* 砖块区距顶 */
#define APP_GAME_BLOCK_W         6       /* 砖块宽 */
#define APP_GAME_BLOCK_H         3       /* 砖块高 */
#define APP_GAME_BLOCK_GAPX      1       /* 砖块横向间隔 */
#define APP_GAME_BLOCK_GAPY      1       /* 砖块纵向间隔 */

#define APP_GAME_BAFFLE_W        20      /* 挡板宽 */
#define APP_GAME_BAFFLE_H        3       /* 挡板高 */
#define APP_GAME_BAFFLE_Y        58      /* 挡板顶部 Y */

#define APP_GAME_BALL_R          2       /* 小球半径 */
#define APP_GAME_BALL_SPEED      0.5f    /* 小球速度：每16ms 1步 */
#define APP_GAME_BLOCK_DT_MAX    32      /* 单循环最大时间步进(ms)，卡顿防跳变 */

/* 打砖块游戏：状态枚举 */
typedef enum
{
	APP_GAME_BLOCK_READY = 0,    /* 初始：球贴挡板等待发射 */
	APP_GAME_BLOCK_PLAYING,      /* 游戏中：球运动，碰撞检测生效 */
	APP_GAME_BLOCK_PAUSED,       /* 暂停 */
	APP_GAME_BLOCK_GAMEOVER,     /* 失败：球掉出底部 */
	APP_GAME_BLOCK_WIN           /* 胜利：砖块全部消除 */
} App_game_block_state_t;

/* 打砖块游戏：状态结构体 */
typedef struct
{
	uint32_t blocks[APP_GAME_BLOCK_ROWS];   /* 砖块位图：每行17位，1=存在 */
	uint8_t state;                          /* 游戏状态 */
	float ball_x, ball_y;                   /* 球心坐标 */
	float ball_vx, ball_vy;                 /* 球速度方向分量 */
	int16_t board_x;                        /* 挡板左X */
	float acc;                              /* 固定步长累积 */
	uint16_t score;                         /* 得分 */
} App_game_block_t;

static App_game_block_t App_game_block;     /* 打砖块游戏状态 */

static void App_game_block_init(void);
static void App_game_block_draw(void);
static void App_game_block_launch(void);
static void App_game_block_step(void);

/**
  * 函  数：App_game_block_init
  * 功  能：打砖块复位：砖块全部重建、挡板居中、球贴挡板、分数清零、状态ready
  */
static void App_game_block_init(void)
{
	uint8_t i;

	for (i = 0; i < APP_GAME_BLOCK_ROWS; i++)
	{
		App_game_block.blocks[i] = (1UL << APP_GAME_BLOCK_COLS) - 1;   /* 17位全1 */
	}
	App_game_block.state = APP_GAME_BLOCK_READY;
	App_game_block.board_x = (128 - APP_GAME_BAFFLE_W) / 2;            /* 居中 */
	App_game_block.ball_x = App_game_block.board_x + APP_GAME_BAFFLE_W / 2;   /* 球贴挡板中心 */
	App_game_block.ball_y = APP_GAME_BAFFLE_Y - APP_GAME_BALL_R;       /* 球底贴挡板顶 */
	App_game_block.ball_vx = 0;
	App_game_block.ball_vy = 0;
	App_game_block.acc = 0;
	App_game_block.score = 0;
}

/**
  * 函  数：App_game_block_launch
  * 功  能：发射小球：水平分量随机±0.35（防纯垂直死局）、垂直向上，归一化后进入playing
  */
static void App_game_block_launch(void)
{
	float len;

	srand(Drv_tim2_get_tick());                                  /* 随机种子 */
	App_game_block.ball_vx = (rand() & 1) ? 0.35f : -0.35f;
	App_game_block.ball_vy = -1.0f;

	len = sqrtf(App_game_block.ball_vx * App_game_block.ball_vx
	          + App_game_block.ball_vy * App_game_block.ball_vy);   /* 方向归一化 */
	if (len > 0.001f)
	{
		App_game_block.ball_vx /= len;
		App_game_block.ball_vy /= len;
	}
	App_game_block.acc = 0;
	App_game_block.state = APP_GAME_BLOCK_PLAYING;
}

/**
  * 函  数：App_game_block_step
  * 功  能：球推进1px（方向分量已归一化）：墙/顶/挡板反弹、砖块消除计分、失败与胜利判定
  */
static void App_game_block_step(void)
{
	int16_t bx, by;
	int16_t r0, r1, c0, c1, r, c;
	int16_t brick_x, brick_y;
	uint8_t hit = 0;

	/* 移动 */
	App_game_block.ball_x += App_game_block.ball_vx;
	App_game_block.ball_y += App_game_block.ball_vy;
	bx = (int16_t)App_game_block.ball_x;
	by = (int16_t)App_game_block.ball_y;

	/* 左右墙反弹（位置校正防卡墙） */
	if (bx - APP_GAME_BALL_R <= 0 && App_game_block.ball_vx < 0)
	{
		App_game_block.ball_vx = -App_game_block.ball_vx;
		App_game_block.ball_x = APP_GAME_BALL_R;
	}
	if (bx + APP_GAME_BALL_R >= 127 && App_game_block.ball_vx > 0)
	{
		App_game_block.ball_vx = -App_game_block.ball_vx;
		App_game_block.ball_x = 127 - APP_GAME_BALL_R;
	}

	/* 顶部反弹 */
	if (by - APP_GAME_BALL_R <= 0 && App_game_block.ball_vy < 0)
	{
		App_game_block.ball_vy = -App_game_block.ball_vy;
		App_game_block.ball_y = APP_GAME_BALL_R;
	}

	/* 挡板反弹：球向下且包围盒与挡板相交——按命中点重分配水平分量，速率归一 */
	if (App_game_block.ball_vy > 0
		&& by + APP_GAME_BALL_R >= APP_GAME_BAFFLE_Y
		&& by + APP_GAME_BALL_R <= APP_GAME_BAFFLE_Y + APP_GAME_BAFFLE_H
		&& bx + APP_GAME_BALL_R > App_game_block.board_x
		&& bx - APP_GAME_BALL_R < App_game_block.board_x + APP_GAME_BAFFLE_W)
	{
		float hit = (bx - (App_game_block.board_x + APP_GAME_BAFFLE_W / 2)) / (float)(APP_GAME_BAFFLE_W / 2);

		App_game_block.ball_vx = hit * 0.8f;                     /* -1~1 → ±0.8 */
		if (App_game_block.ball_vx > 0.8f) { App_game_block.ball_vx = 0.8f; }
		if (App_game_block.ball_vx < -0.8f) { App_game_block.ball_vx = -0.8f; }
		App_game_block.ball_vy = -sqrtf(1.0f - App_game_block.ball_vx * App_game_block.ball_vx);
		App_game_block.ball_y = APP_GAME_BAFFLE_Y - APP_GAME_BALL_R - 1;   /* 位置校正防重复触发 */
	}

	/* 砖块碰撞：球包围盒覆盖范围内逐块 AABB 相交检测，命中全部消除计分 */
	c0 = (bx - APP_GAME_BALL_R - APP_GAME_BLOCK_X0) / (APP_GAME_BLOCK_W + APP_GAME_BLOCK_GAPX);
	c1 = (bx + APP_GAME_BALL_R - APP_GAME_BLOCK_X0) / (APP_GAME_BLOCK_W + APP_GAME_BLOCK_GAPX);
	r0 = (by - APP_GAME_BALL_R - APP_GAME_BLOCK_Y0) / (APP_GAME_BLOCK_H + APP_GAME_BLOCK_GAPY);
	r1 = (by + APP_GAME_BALL_R - APP_GAME_BLOCK_Y0) / (APP_GAME_BLOCK_H + APP_GAME_BLOCK_GAPY);
	if (c0 < 0) { c0 = 0; }
	if (c1 > APP_GAME_BLOCK_COLS - 1) { c1 = APP_GAME_BLOCK_COLS - 1; }
	if (r0 < 0) { r0 = 0; }
	if (r1 > APP_GAME_BLOCK_ROWS - 1) { r1 = APP_GAME_BLOCK_ROWS - 1; }

	for (r = r0; r <= r1; r++)
	{
		for (c = c0; c <= c1; c++)
		{
			if (!(App_game_block.blocks[r] & (1UL << c))) { continue; }

			brick_x = APP_GAME_BLOCK_X0 + c * (APP_GAME_BLOCK_W + APP_GAME_BLOCK_GAPX);
			brick_y = APP_GAME_BLOCK_Y0 + r * (APP_GAME_BLOCK_H + APP_GAME_BLOCK_GAPY);
			if (bx - APP_GAME_BALL_R < brick_x + APP_GAME_BLOCK_W
				&& bx + APP_GAME_BALL_R > brick_x
				&& by - APP_GAME_BALL_R < brick_y + APP_GAME_BLOCK_H
				&& by + APP_GAME_BALL_R > brick_y)
			{
				App_game_block.blocks[r] &= ~(1UL << c);
				App_game_block.score++;
				hit = 1;
			}
		}
	}

	if (hit)
	{
		App_game_block.ball_vy = -App_game_block.ball_vy;       /* 砖块反弹（上下方向反转） */

		if (App_game_block.score >= APP_GAME_BLOCK_COLS * APP_GAME_BLOCK_ROWS)   /* 全部消除 */
		{
			App_game_block.state = APP_GAME_BLOCK_WIN;
			return;
		}
	}

	/* 失败判定：球顶低于挡板下边界 */
	if (by - APP_GAME_BALL_R > APP_GAME_BAFFLE_Y + APP_GAME_BAFFLE_H - 1)
	{
		App_game_block.state = APP_GAME_BLOCK_GAMEOVER;
	}
}

/**
  * 函  数：App_game_block_draw
  * 功  能：打砖块全量绘制：砖块区 + 挡板 + 小球（每帧重绘，简单零残留）
  */
static void App_game_block_draw(void)
{
	uint8_t row, col;
	int16_t x, y;

	Inf_oled_clear();

	/* 砖块区 */
	for (row = 0; row < APP_GAME_BLOCK_ROWS; row++)
	{
		for (col = 0; col < APP_GAME_BLOCK_COLS; col++)
		{
			if (App_game_block.blocks[row] & (1UL << col))
			{
				x = APP_GAME_BLOCK_X0 + col * (APP_GAME_BLOCK_W + APP_GAME_BLOCK_GAPX);
				y = APP_GAME_BLOCK_Y0 + row * (APP_GAME_BLOCK_H + APP_GAME_BLOCK_GAPY);
				Inf_oled_show_image(x, y, APP_GAME_BLOCK_W, APP_GAME_BLOCK_H, block);
			}
		}
	}

	/* 挡板 */
	Inf_oled_show_image(App_game_block.board_x, APP_GAME_BAFFLE_Y, APP_GAME_BAFFLE_W, APP_GAME_BAFFLE_H, baffle);

	/* 小球（结束态不绘制，提示文字不与球混叠） */
	if (App_game_block.state != APP_GAME_BLOCK_GAMEOVER && App_game_block.state != APP_GAME_BLOCK_WIN)
	{
		Inf_oled_draw_circle((uint8_t)App_game_block.ball_x, (uint8_t)App_game_block.ball_y, APP_GAME_BALL_R, OLED_FILLED);
	}

	/* 暂停提示 */
	if (App_game_block.state == APP_GAME_BLOCK_PAUSED)
	{
		Inf_oled_show_ascii(49, 28, "PAUSE", OLED_6X8);
	}

	/* 结束画面：提示 + 分数 */
	if (App_game_block.state == APP_GAME_BLOCK_GAMEOVER || App_game_block.state == APP_GAME_BLOCK_WIN)
	{
		if (App_game_block.state == APP_GAME_BLOCK_GAMEOVER)
		{
			Inf_oled_show_ascii(40, 28, "GameOver", OLED_6X8);
		}
		else
		{
			Inf_oled_show_ascii(31, 28, "GameVictory", OLED_6X8);
		}

		Inf_oled_show_ascii(40, 40, "Score:", OLED_6X8);
		Inf_oled_show_num(76, 40, App_game_block.score, 2, OLED_6X8);
		Inf_oled_show_string(16, 52, "短按重开 长按退出", OLED_6X8);
	}

	Inf_oled_update();
}

/**
  * 函  数：App_game_block_run
  * 功  能：打砖块游戏：ready 态球贴挡板，上下键横移挡板（上=左 下=右），首次移动发射；
  *          playing态固定步长推进球并做碰撞；结束态（B3提示画面）长按退出
  */
void App_game_block_run(void)
{
	uint8_t key;
	uint32_t now = 0, prev_tick = 0;
	uint16_t dt;

	App_game_block_init();

	Inf_oled_fade_flag = 1;

	while (1)
	{
		key = Inf_key_scan();

		now = Drv_tim2_get_tick();
		dt = (uint16_t)(now - prev_tick);
		prev_tick = now;
		if (dt > APP_GAME_BLOCK_DT_MAX) { dt = APP_GAME_BLOCK_DT_MAX; }   /* 卡顿防跳变 */

		if ((key == 1 || key == 2) && (App_game_block.state == APP_GAME_BLOCK_READY || App_game_block.state == APP_GAME_BLOCK_PLAYING))   /* 挡板移动：上=左 下=右（ready/playing 可动） */
		{
			if (key == 1) { App_game_block.board_x -= 4; }
			else          { App_game_block.board_x += 4; }
			if (App_game_block.board_x < 0) { App_game_block.board_x = 0; }
			if (App_game_block.board_x > 128 - APP_GAME_BAFFLE_W) { App_game_block.board_x = 128 - APP_GAME_BAFFLE_W; }

			if (App_game_block.state == APP_GAME_BLOCK_READY)   /* ready: 球跟随挡板，首次移动即发射 */
			{
				App_game_block.ball_x = App_game_block.board_x + APP_GAME_BAFFLE_W / 2;
				App_game_block_launch();
			}
		}

		if (key == 3)                           /* 短按：暂停切换/结束态重开 */
		{
			if (App_game_block.state == APP_GAME_BLOCK_PLAYING)
			{
				App_game_block.state = APP_GAME_BLOCK_PAUSED;
			}
			else if (App_game_block.state == APP_GAME_BLOCK_PAUSED)
			{
				App_game_block.state = APP_GAME_BLOCK_PLAYING;
				App_game_block.acc = 0;           /* 恢复时步进重新累计，防跳变 */
			}
			else if (App_game_block.state == APP_GAME_BLOCK_GAMEOVER || App_game_block.state == APP_GAME_BLOCK_WIN)
			{
				App_game_block_init();            /* 结束态短按：重开回ready */
			}
		}

		if (App_game_block.state == APP_GAME_BLOCK_PLAYING)     /* 球物理：固定步长（速度恒定） */
		{
			App_game_block.acc += dt * (APP_GAME_BALL_SPEED / 16.0f);  
			while (App_game_block.acc >= 1.0f && App_game_block.state == APP_GAME_BLOCK_PLAYING)
			{
				App_game_block.acc -= 1.0f;
				App_game_block_step();
			}
		}

		App_game_block_draw();

		Inf_oled_gradient(1);

		if (key == 4)                           /* 长按：退出 */
		{
			Inf_oled_fade_flag = 1;
			Inf_oled_gradient(0);
			return;
		}
	}
}
