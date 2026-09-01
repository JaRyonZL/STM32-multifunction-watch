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

#endif
