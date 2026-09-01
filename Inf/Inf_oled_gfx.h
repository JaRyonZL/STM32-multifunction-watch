#ifndef __INF_OLED_GFX_H
#define __INF_OLED_GFX_H

/******************************************************************************
 * 文件名称：Inf_oled_gfx.h（接口层）
 * 说    明：OLED绘图库（由旧工程OLED.c的图形函数部分拆分迁移）
 ******************************************************************************/

#include "Com_def.h"

/* IsFilled参数取值 */
#define OLED_UNFILLED			0
#define OLED_FILLED				1

void Inf_oled_draw_point(int8_t X, int8_t Y);                                   /* 画点 */
uint8_t Inf_oled_get_point(uint8_t X, uint8_t Y);                               /* 取点 */
void Inf_oled_draw_line(uint8_t X0, uint8_t Y0, uint8_t X1, uint8_t Y1);        /* 画线 */
void Inf_oled_draw_rectangle(int8_t X, int8_t Y, uint8_t Width, uint8_t Height, uint8_t IsFilled);   /* 画矩形 */
void Inf_oled_draw_triangle(uint8_t X0, uint8_t Y0, uint8_t X1, uint8_t Y1,
                            uint8_t X2, uint8_t Y2, uint8_t IsFilled);          /* 画三角形 */
void Inf_oled_draw_circle(uint8_t X, uint8_t Y, uint8_t Radius, uint8_t IsFilled);  /* 画圆 */
void Inf_oled_draw_ellipse(uint8_t X, uint8_t Y, uint8_t A, uint8_t B, uint8_t IsFilled);    /* 画椭圆 */
void Inf_oled_draw_arc(uint8_t X, uint8_t Y, uint8_t Radius, int16_t StartAngle,
                       int16_t EndAngle, uint8_t IsFilled);                     /* 画圆弧 */


/* FontSize参数取值 */
#define OLED_8X16				8
#define OLED_6X8				6

uint32_t Inf_oled_pow(uint32_t X, uint32_t Y);                                  /* x的y次方 */
void Inf_oled_show_char(int8_t X, int8_t Y, char Char, uint8_t FontSize);       /* 显示字符 */
void Inf_oled_show_ascii(int8_t X, int8_t Y, char* String, uint8_t FontSize);   /* 显示ASCII字符串 */
void Inf_oled_show_num(int8_t X, int8_t Y, uint32_t Number, uint8_t Length, uint8_t FontSize);          /* 十进制无符号数 */
void Inf_oled_show_signed_num(uint8_t X, uint8_t Y, int32_t Number, uint8_t Length, uint8_t FontSize);  /* 十进制有符号数 */
void Inf_oled_show_hex_num(uint8_t X, uint8_t Y, uint32_t Number, uint8_t Length, uint8_t FontSize);    /* 十六进制数 */
void Inf_oled_show_bin_num(uint8_t X, uint8_t Y, uint32_t Number, uint8_t Length, uint8_t FontSize);    /* 二进制数 */
void Inf_oled_show_float_num(uint8_t X, uint8_t Y, double Number, uint8_t IntLength, uint8_t FraLength, uint8_t FontSize); /* 浮点数 */
void Inf_oled_show_chinese(uint8_t X, uint8_t Y, char* Chinese);                 /* 中文（内置字模查表） */
void Inf_oled_show_image(int8_t X, int8_t Y, uint8_t Width, uint8_t Height, const uint8_t* Image);      /* 图片 */
#endif
