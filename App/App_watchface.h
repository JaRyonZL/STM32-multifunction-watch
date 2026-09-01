#ifndef __APP_WATCHFACE_H
#define __APP_WATCHFACE_H

/******************************************************************************
 * 文件名称：App_watchface.h（应用层）
 * 说    明：表盘应用，由旧工程biaopan.c迁移
 *           大数字时间显示、日期/星期、APP状态图标、电量显示、时间调整
 ******************************************************************************/

#include "Com_def.h"

void App_watchface_draw_digit(uint8_t X, uint8_t Y, uint8_t Num, uint8_t Big);  /* 单个数字图标（原bittt）：Big=1用13x30大数字，0用9x21小数字 */
void App_watchface_show_time(uint8_t X, uint8_t Y);    /* 大数字时间：AM/PM+时:分:秒（原biaopan_Timer），内部读取RTC */
void App_watchface_date(uint8_t X, uint8_t Y);         /* 日期：年/月/日（原biaopan_date，复用show_time读取的RTC值） */
void App_watchface_week(uint8_t X, uint8_t Y);         /* 星期："week:"+数字（原biaopan_week，周日显示R） */
void App_watchface_show_app_status(uint8_t X, uint8_t Y);  /* APP状态图标（原biaopan_APP）：MP3上电/体感 */
void App_watchface_run(void);                          /* 表盘刷新入口（原biaopan，C4补电量显示后与旧完全一致） */

#endif
