#ifndef __INF_BATTERY_H
#define __INF_BATTERY_H

/******************************************************************************
 * 文件名称：Inf_battery.h（接口层）
 * 说    明：电池电量检测，由旧工程biaopan.c电量部分与AD.c电源控制部分迁移
 *           电压-电量查表插值，VIN为电压基准校准值（信息页/校准页可调）
 ******************************************************************************/

#include "Com_def.h"

extern float VIN;                     /* ADC电压基准校准值，信息页/校准页可调 */

void    Inf_battery_init(void);       /* PB3充电检测输入（上拉）、PB5测量电源输出（默认接通） */
void    Inf_battery_meas_pwr_on(void);   /* 接通测量电压电路电源（PB5=0） */
void    Inf_battery_meas_pwr_off(void);  /* 断开测量电压电路电源（PB5=1） */
uint8_t Inf_battery_is_charging(void);   /* 充电状态：1=充电中（PB3=0） */
float   Inf_battery_read_voltage(void);  /* 电池电压（ADC CH8，硬件1/2分压折算） */
uint8_t Inf_battery_get_level(void);     /* 电量百分比（查表插值+最小保持显示值） */

#endif
