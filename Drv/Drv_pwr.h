/*
 * @Author: JaRyon
 * @Date: 2026-08-31 20:39:42
 * @Email: jaryonzl@163.com
 */
#ifndef __DRV_PWR_H
#define __DRV_PWR_H

/******************************************************************************
 * 文件名称：Drv_pwr.h（驱动层）
 * 说    明：低功耗驱动（由旧工程main.c的低功耗部分抽出）：
 *           PA0（EXTI0）唤醒配置、STOP/STANDBY进入
 ******************************************************************************/

#include "Com_def.h"

void Drv_pwr_wakeup_init(void);     /* PA0外部中断唤醒初始化（EXTI0上升沿） */
void Drv_pwr_enter_stop(void);      /* 进入停止模式STOP */
void Drv_pwr_enter_standby(void);   /* 进入待机模式STANDBY */

#endif
