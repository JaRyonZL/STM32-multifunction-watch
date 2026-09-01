#ifndef __DRV_TIM_H
#define __DRV_TIM_H

/******************************************************************************
 * 文件名称：Drv_tim.h（驱动层）
 * 说    明：TIM2定时中断与1ms计数
 ******************************************************************************/

#include "Com_def.h"

void     Drv_tim2_init(void);     
uint32_t Drv_tim2_get_tick(void); 
uint32_t Drv_tim2_get_us(void);

#endif
