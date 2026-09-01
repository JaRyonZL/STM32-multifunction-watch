/*
 * @Author: JaRyon
 * @Date: 2026-09-01 09:52:26
 * @Email: jaryonzl@163.com
 */
#ifndef __INF_OLED_H
#define __INF_OLED_H

/******************************************************************************
 * 文件名称：Inf_oled.h（接口层）
 * 说    明：OLED模块基础配置（0.96寸SSD1306，软件SPI，7针接口）
 *           由旧工程OLED.c按层拆分：本文件为硬件时序/命令/显存部分
 ******************************************************************************/

#include "Com_def.h"
#include "Com_board.h"

extern uint8_t Inf_oled_display_buf[8][128];            /* 显存（页式，1bit/像素） */
extern uint8_t Inf_oled_brightness;                     /* 目标亮度（对比度0x00~0xFF） */
extern uint8_t Inf_oled_brightness_up;                  /* 当前过渡亮度 */
extern uint8_t Inf_oled_fade_flag;                      /* 过渡标志 */

void Inf_oled_init(void);                               /* OLED初始化 */
void Inf_oled_write_command(uint8_t Command);           /* 写命令 */
void Inf_oled_write_data(uint8_t* Data, uint8_t Count); /* 写数据 */
void Inf_oled_set_cursor(uint8_t Page, uint8_t X);      /* 设置显示位置 */


void Inf_oled_update(void);                             /* 整屏刷新 */
void Inf_oled_update_area(uint8_t X, uint8_t Y, uint8_t Width, uint8_t Height); /* 局部刷新 */
void Inf_oled_clear(void);                              /* 清屏 */
void Inf_oled_clear_area(uint8_t X, uint8_t Y, uint8_t Width, uint8_t Height);   /* 局部清屏 */
void Inf_oled_reverse(void);                            /* 整屏取反 */
void Inf_oled_reverse_area(uint8_t X, uint8_t Y, uint8_t Width, uint8_t Height); /* 局部取反 */

void Inf_oled_gradient(uint8_t dir);                    /* 亮度过渡：1渐亮，0渐灭 */

#endif
