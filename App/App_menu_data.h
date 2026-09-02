#ifndef __APP_MENU_DATA_H
#define __APP_MENU_DATA_H

/******************************************************************************
 * 文件名：App_menu_data.h（应用层）
 * 说   明：设置/信息类应用（迁移自旧工程 menu_Data.c 的设置部分）
 ******************************************************************************/

#include "Com_def.h"

void App_settings_run(void);   /* 设置菜单入口（旧 System_settings） */

void App_error(void);             /* 占位界面：待开发 */
void App_flashlight(void);        /* 手电筒 */
void App_qrcode_wechat(void);     /* 微信收款码（占位图） */
void App_qrcode_zfb(void);        /* 支付宝收款码（占位图） */
void App_calc_cos(void);          /* cos 计算器 */
void App_font_browser(void);      /* 字库浏览 */

#endif
