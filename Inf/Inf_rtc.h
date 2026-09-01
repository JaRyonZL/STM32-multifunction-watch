#ifndef __INF_RTC_H
#define __INF_RTC_H

/******************************************************************************
 * 文件名称：Inf_rtc.h（接口层）
 * 说    明：RTC日历换算，由旧工程MyRTC.c日历部分迁移（硬件部分在Drv_rtc）
 *           时区偏移统一取Com_config.h的COM_RTC_UTC_OFFSET_HOURS
 ******************************************************************************/

#include "Com_def.h"

extern uint16_t Inf_rtc_time[7];  /* 年/月/日/时/分/秒/星期（星期：0=周一~6=周日） */

void Inf_rtc_init(void);      /* 初始化：调用Drv_rtc_init，首次写入默认时间；LSI模式含走时校准（依赖Drv_tim2已初始化） */
void Inf_rtc_set_time(void);  /* 将Inf_rtc_time写回RTC硬件 */
void Inf_rtc_read_time(void); /* 从RTC硬件读取时间并刷新Inf_rtc_time */

#endif
