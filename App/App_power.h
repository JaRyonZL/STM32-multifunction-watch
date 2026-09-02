#ifndef __APP_POWER_H
#define __APP_POWER_H

/******************************************************************************
 * 文件名：App_power.h（应用层）
 * 说   明：关机/待机序列（迁移自旧工程 Power_OFF 与 main.c 长按待机分支）
 ******************************************************************************/

#include "Com_def.h"

void App_power_off(void);       
void App_power_standby(void);  

#endif
