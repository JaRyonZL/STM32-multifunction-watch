#ifndef __APP_VIDEO_H
#define __APP_VIDEO_H

/******************************************************************************
 * 文件名：App_video.h（应用层）
 * 说   明：视频播放应用（迁移自旧工程 video_app.c）
 ******************************************************************************/

#include "Com_def.h"

void App_video_play_first(void);   /* 播放第一段视频（TF卡01文件夹01曲） */
void App_video_play_second(void);  /* 播放第二段视频（TF卡01文件夹02曲） */
void App_video_play_third(void);   /* 播放第三段视频（TF卡01文件夹03曲） */

#endif
