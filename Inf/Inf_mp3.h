#ifndef __INF_MP3_H
#define __INF_MP3_H

/******************************************************************************
 * 文件名称：Inf_mp3.h（接口层）
 * 说    明：MP3模块（USART2，9600），由旧工程MP3_tf.c协议部分与menu_Data.c
 *           扬声器/音量部分迁移
 *           协议帧格式：7E FF 06 CMD FB DH DL EF（0x06音量 0x0D播 0x0E停 0x12指定曲目）
 *           回传包：0xFF包头+9字节，CMD=0x3D为播放结束
 ******************************************************************************/

#include "Com_def.h"

/******************************************************************************
 * MP3状态结构体
 ******************************************************************************/
typedef struct
{
	uint8_t playing;         /* 播放标志：1=播放中 */
	uint8_t loop_play;       /* 循环播放标志 */
	uint8_t play_once;       /* 播完即停标志 */
	uint8_t volume;          /* 音量0~30 */
	uint8_t chapter;         /* 当前曲目 */
	uint8_t chapter_max;     /* 曲目总数 */
	uint8_t update_pending;  /* 曲目变更待发送标志 */
	uint8_t loudspeaker_on;  /* 扬声器开关状态：1=开 */
} Inf_mp3_state_t;

extern Inf_mp3_state_t Inf_mp3;       /* MP3状态 */

void Inf_mp3_detect_init(void);          /* 电源检测初始化（仅PA11输入，不驱动PA12） */
void Inf_mp3_init(void);                 /* 初始化：功放使能默认关+电源检测输入+串口 */
void Inf_mp3_power_off(void);            /* 断电：关功放+串口断电 */
void Inf_mp3_send_cmd(uint8_t CMD, uint8_t feedback, uint16_t data);  /* 发送协议帧 */
void Inf_mp3_start(uint8_t on);          /* 开始/停止播放 */
void Inf_mp3_switch_chapter(uint8_t next);  /* 曲目切换：1=下一曲 0=上一曲 */
void Inf_mp3_send_volume(void);          /* 发送音量命令 */
void Inf_mp3_loudspeaker(void);          /* 按Inf_mp3.loudspeaker_on刷新功放 */
uint8_t Inf_mp3_is_powered(void);        /* 电源检测：1=模块已上电（PA11=1） */
void Inf_mp3_poll(void);                 /* 回传处理：主循环周期调用（处理0x3D播放结束） */

#endif
