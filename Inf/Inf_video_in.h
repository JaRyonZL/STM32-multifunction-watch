/*
 * @Author: JaRyon
 * @Date: 2026-09-02 16:29:21
 * @Email: jaryonzl@163.com
 */
#ifndef __INF_VIDEO_IN_H
#define __INF_VIDEO_IN_H

/******************************************************************************
 * 文件名：Inf_video_in.h（接口层）
 * 说   明：USART3视频流输入封装（上位机921600bps下发1024字节/帧）
 ******************************************************************************/

#include "Com_def.h"

void Inf_video_in_init(void);           /* 初始化并重置帧位置 */
void Inf_video_in_power_off(void);      /* 断电省电 */
void Inf_video_in_reset_frame(void);    /* 重置帧位置 */
uint8_t Inf_video_in_fill_frame(void);  /* 收字节填显存，满一帧(1024B)返回1 */

#endif
