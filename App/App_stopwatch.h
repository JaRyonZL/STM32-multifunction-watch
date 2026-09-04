#ifndef __APP_STOPWATCH_H
#define __APP_STOPWATCH_H

/******************************************************************************
 * 文件名：App_stopwatch.h（应用层）
 * 说   明：秒表功能（计时基于 Drv_tim2 1ms 心跳）
 ******************************************************************************/

#include "Com_def.h"

void App_stopwatch_run(void);   /* 秒表功能入口 */

#endif
