#ifndef __INF_KEY_H
#define __INF_KEY_H

/******************************************************************************
 * 文件名称：Inf_key.h（接口层）
 * 说    明：按键输入与去抖事件
 *           KEY1=PC13(下)  KEY2=PA0(上)  KEY3=PA1(确定)
 *           事件函数为计数去抖：按下响应，按住后连续触发
 ******************************************************************************/

#include "Com_def.h"

void  Inf_key_init(void);           /* 按键GPIO初始化（下拉输入） */
int8_t Inf_key_up_event(uint8_t key_state);
int8_t Inf_key_down_event(uint8_t key_state);
int8_t Inf_key_enter_event(uint8_t key_state);
int8_t Inf_key_scan(void);

#endif
