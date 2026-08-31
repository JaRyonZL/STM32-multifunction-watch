/*
 * @Author: JaRyon
 * @Date: 2026-08-31 16:48:29
 * @Email: jaryonzl@163.com
 */
#ifndef __DRV_DELAY_H
#define __DRV_DELAY_H

/******************************************************************************
 * 文件名称：Drv_delay.h（驱动层）
 * 说    明：SysTick延时函数（由旧工程System/Delay迁移）
 *           本工程不使用OS，仅保留无OS版本
 ******************************************************************************/

#include "Com_def.h"

void Drv_delay_init(void);
void Drv_delay_ms(u16 nms);
void Drv_delay_us(u32 nus);

#endif
