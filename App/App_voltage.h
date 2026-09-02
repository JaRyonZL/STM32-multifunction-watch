#ifndef __APP_VOLTAGE_H
#define __APP_VOLTAGE_H

/******************************************************************************
 * 文件名称：App_voltage.h（应用层）
 * 说    明：电压检测应用
 *           电压表界面（分压电阻可调，分压比由R1/R2计算）、波形显示（预留）
 ******************************************************************************/

#include "Com_def.h"

extern float App_voltage_r1;      /* 分压上臂电阻（kΩ，默认43.0） */
extern float App_voltage_r2;      /* 分压下臂电阻（kΩ，默认10.0） */

void App_voltage_run(void);
void App_voltage_waveform(void);

#endif
