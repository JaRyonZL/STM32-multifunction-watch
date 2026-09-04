#ifndef __APP_SETTINGS_STORE_H
#define __APP_SETTINGS_STORE_H

/******************************************************************************
 * 文件名：App_settings_store.h（应用层）
 * 说   明：用户设置持久化（W25Q 设置区 0x28000，单扇区）
 ******************************************************************************/

#include "Com_def.h"

void App_settings_store_load(void);   /* 上电读取设置区恢复变量（校验失败静默回默认） */
void App_settings_store_save(void);   /* 保存设置（先比较后写：相同则不擦写） */

#endif
