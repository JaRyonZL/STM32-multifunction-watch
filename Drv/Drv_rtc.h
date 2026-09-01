/*
 * @Author: JaRyon
 * @Date: 2026-08-31 19:36:19
 * @Email: jaryonzl@163.com
 */
#ifndef __DRV_RTC_H
#define __DRV_RTC_H

/******************************************************************************
 * 文件名称：Drv_rtc.h（驱动层）
 * 说    明：RTC驱动（由旧工程MyRTC.c拆分）：时钟源配置、预分频、
 *           BKP首次初始化标记、计数器读写；日历换算由上层（Inf_rtc）负责
 ******************************************************************************/

#include "Com_def.h"

/******************************************************************************
 * RTC时钟源选择（预编译宏切换）
 * 1 = 使用LSE（外接32.768kHz晶振）
 * 0 = 使用LSI（内部约40kHz）
 * 说明：使用LSE时走时准确，且可由VBAT后备电池供电保持计时；
 *       但若LSE无法起振，程序会卡在等待LSERDY处，此时请改为0使用LSI；
 *       LSI无法由备用电源供电，掉电后RTC的计时会停
 ******************************************************************************/
#define DRV_RTC_USE_LSE    0

uint8_t  Drv_rtc_init(void);                    /* RTC初始化，返回1=首次初始化，0=非首次 */
void     Drv_rtc_set_counter(uint32_t Counter); /* 写计数器 */
uint32_t Drv_rtc_get_counter(void);             /* 读计数器 */
void     Drv_rtc_set_prescaler(uint32_t Prescaler);
uint32_t Drv_rtc_get_prescaler(void);

#endif
