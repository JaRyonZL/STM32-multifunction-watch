/*
 * @Author: JaRyon
 * @Date: 2026-08-31 19:10:22
 * @Email: jaryonzl@163.com
 */
#ifndef __DRV_ADC_H
#define __DRV_ADC_H

/******************************************************************************
 * 文件名称：Drv_adc.h（驱动层）
 * 说    明：ADC1采样原语（由旧工程AD.c迁移，PB5测量电源控制移出到Inf层）
 ******************************************************************************/

#include "Com_def.h"
#include "Com_board.h"

void Drv_adc1_init(void);                               /* ADC1初始化 */
uint16_t Drv_adc1_get_value(uint8_t ADC_Channel);       /* 单次采样指定通道，返回0~4095 */

#endif
