/*
 * @Author: JaRyon
 * @Date: 2026-08-31 16:26:22
 * @Email: jaryonzl@163.com
 */
#ifndef __DRV_GPIO_H
#define __DRV_GPIO_H

/******************************************************************************
 * 文件名称：Drv_gpio.h（驱动层）
 * 说    明：GPIO通用初始化封装（由旧工程sys.c/sys.h迁移）
 ******************************************************************************/

#include "stm32f10x.h"
#include "Com_def.h"

/******************************************************************************
 * GPIO模式枚举（Drv_GPIO_Init使用）
 ******************************************************************************/
typedef enum
{
	GPIO_FK_IN = 0,         /* 浮空输入 */
	GPIO_AD_IN = 1,         /* 模拟输入 */

	GPIO_KL_OUT = 2,        /* 开漏输出 */
	GPIO_KL_AF_OUT = 3,     /* 复用开漏输出 */
	GPIO_TW_OUT = 4,        /* 推挽输出 */
	GPIO_TW_AF_OUT = 5,     /* 复用推挽输出 */

	GPIO_P_NO = 6,          /* 无上下拉 */
	GPIO_P_UP = 7,          /* 上拉 */
	GPIO_P_DOWN = 8,        /* 下拉 */

	GPIO_2MHz = 9,
	GPIO_10MHz = 10,
	GPIO_25MHz = 11,
	GPIO_50MHz = 12,
	GPIO_100MHz = 13        /* F103无此速度档，保留占位 */
} GPIO_Drv_TypeDef;

/******************************************************************************
 * 函数声明
 ******************************************************************************/
void Drv_GPIO_Init(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin,
                  GPIO_Drv_TypeDef mode, GPIO_Drv_TypeDef up_down, GPIO_Drv_TypeDef speed);

#endif
