/*
 * @Author: JaRyon
 * @Date: 2026-09-01 17:30:53
 * @Email: jaryonzl@163.com
 */
#ifndef __APP_MENU_H
#define __APP_MENU_H

/******************************************************************************
 * 文件名称：App_menu.h（应用层）
 * 说    明：菜单引擎，由旧工程menu.c迁移
 *           App_menu_run_list：列表菜单（4种选项模式）
 *           App_menu_run_wheel：32x32图标轮盘菜单
 *           选项数组均以{".."}结尾
 ******************************************************************************/

#include "Com_def.h"

/******************************************************************************
 * 选项模式
 ******************************************************************************/
#define APP_MENU_MODE_DISPLAY   0   /* 仅展示：选中不执行任何动作 */
#define APP_MENU_MODE_FUNCTION  1   /* 可进入函数：选中执行一次函数 */
#define APP_MENU_MODE_ON_OFF    2   /* 开关：选中取反变量后执行函数一次 */
#define APP_MENU_MODE_NUMBER    3   /* 数值：进入内部编辑，每次变化执行函数一次 */

#define APP_MENU_RETURN         0   /* 占位函数：不执行函数直接返回上层 */

/******************************************************************************
 * 引擎参数
 ******************************************************************************/
#define APP_MENU_ROW_H          16      /* 列表菜单行高（原MEHE） */
#define APP_MENU_WORD_W         6       /* 单字显示宽度（原WORD_H） */
#define APP_MENU_CURSOR_SPEED   12      /* 光标移动速度系数（原Speed_Factor） */
#define APP_MENU_SCROLL_SPEED   1.3f    /* 列表滚动速度系数（原Roll_Speed） */
#define APP_MENU_BAR_SPEED      10       /* 边栏进度条逼近速度系数 */

extern uint8_t App_menu_cursor;         /* 光标样式：0=反色 1=方框 2=箭头（设置项） */

/******************************************************************************
 * 选项结构体
 ******************************************************************************/
typedef struct
{
	char* Name;              /* 选项名称 */
	void (*func)(void);      /* 选中执行函数 */
	const uint8_t* Image;    /* 需要显示的32x32图标 */
	uint8_t NameLen;         /* 名称显示宽度（列数，GBK一字=2列，由引擎计算填充） */
} App_menu_option1_t;

typedef struct
{
	char* Name;              /* 选项名称 */
	uint8_t mode;            /* 选项模式，见APP_MENU_MODE_* */
	void (*func)(void);      /* 选中执行函数 */
	uint8_t* Num;            /* 绑定的变量地址（ON_OFF/NUMBER模式用） */
	uint8_t NameLen;         /* 名称显示宽度（列数，由引擎计算填充） */
} App_menu_option2_t;

/******************************************************************************
 * 引擎函数
 ******************************************************************************/
uint8_t App_menu_get_name_len(char* String);    /* 计算名称显示宽度：GBK一字=2列，ASCII=1列 */
void App_menu_reverse_area_frame(uint8_t X, uint8_t Y, uint8_t Width, uint8_t Height); /* 空心反色框 */
void App_menu_run_list(App_menu_option2_t* option);  /* 列表菜单 */
void App_menu_run_wheel(App_menu_option1_t* option); /* 图标轮盘菜单 */

#endif
