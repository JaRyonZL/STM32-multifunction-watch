#include "App_game.h"
#include "App_menu.h"

/******************************************************************************
 * 文件名：App_game.c（应用层）
 * 说   明：游戏应用
 ******************************************************************************/

/**
  * 函   数：App_game_menu
  * 功   能：游戏子菜单：选择游戏进入（后续扩展游戏在此加项）
  */
void App_game_menu(void)
{
	App_menu_option2_t option_list[] = {
		{"- 退出"    , APP_MENU_MODE_FUNCTION, APP_MENU_RETURN, 0, 0},
		{"- 生命游戏", APP_MENU_MODE_FUNCTION, App_game_life_run, 0, 0},
		{".."}
	};

	App_menu_run_list(option_list);
}

/**
  * 函   数：App_game_life_run
  * 功   能：生命游戏（原 Game_Of_Life_Play），C11 实现
  */
void App_game_life_run(void)
{
	/* 占位：C11 实现 */
}
