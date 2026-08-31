/*
 * @Author: JaRyon
 * @Date: 2026-08-31 15:30:56
 * @Email: jaryonzl@163.com
 */
#ifndef __COM_DEF_H
#define __COM_DEF_H

/******************************************************************************
 * 文件名称：Com_def.h（公共层）
 * 说    明：通用类型定义、位带操作宏、标准库传递包含
 *           （由旧工程 System/sys.h 保真迁移，仅做命名调整）
 * 依赖规则：本层不依赖 App/Inf/Drv，仅依赖标准外设库
 ******************************************************************************/

#include "stm32f10x.h"
#include "stm32f10x_conf.h"
#include "stm32f10x_it.h"

#include "stdbool.h"
#include "stdio.h"
#include <stdarg.h>
#include "string.h"
#include "math.h"
#include "stdlib.h"

#ifndef TRUE
#define TRUE    1
#endif
#ifndef FALSE
#define FALSE   0
#endif
#ifndef NULL
#define NULL    0
#endif

/* u8/u16/u32类型定义来自stm32f10x.h，此处仅补充u64 */
typedef unsigned long long  u64;

/******************************************************************************
 * 位带操作，实现类似51的GPIO位控制功能
 * 实现思想参考<<CM3权威指南>>第87页~92页
 ******************************************************************************/
#define BITBAND(addr, bitnum) ((addr & 0xF0000000)+0x2000000+((addr &0xFFFFF)<<5)+(bitnum<<2))
#define MEM_ADDR(addr)  *((volatile unsigned long  *)(addr))
#define BIT_ADDR(addr, bitnum)   MEM_ADDR(BITBAND(addr, bitnum))

/* IO 口地址映射 */
#define GPIOA_ODR_Addr    (GPIOA_BASE+12) /* 0x4001080C */
#define GPIOB_ODR_Addr    (GPIOB_BASE+12) /* 0x40010C0C */
#define GPIOC_ODR_Addr    (GPIOC_BASE+12) /* 0x4001100C */
#define GPIOD_ODR_Addr    (GPIOD_BASE+12) /* 0x4001140C */
#define GPIOE_ODR_Addr    (GPIOE_BASE+12) /* 0x4001180C */
#define GPIOF_ODR_Addr    (GPIOF_BASE+12) /* 0x40011A0C */
#define GPIOG_ODR_Addr    (GPIOG_BASE+12) /* 0x40011E0C */

#define GPIOA_IDR_Addr    (GPIOA_BASE+8)  /* 0x40010808 */
#define GPIOB_IDR_Addr    (GPIOB_BASE+8)  /* 0x40010C08 */
#define GPIOC_IDR_Addr    (GPIOC_BASE+8)  /* 0x40011008 */
#define GPIOD_IDR_Addr    (GPIOD_BASE+8)  /* 0x40011408 */
#define GPIOE_IDR_Addr    (GPIOE_BASE+8)  /* 0x40011808 */
#define GPIOF_IDR_Addr    (GPIOF_BASE+8)  /* 0x40011A08 */
#define GPIOG_IDR_Addr    (GPIOG_BASE+8)  /* 0x40011E08 */

/* IO口位操作宏，只对单个IO口有效，n的值必须小于16 */
#define PAout(n)   BIT_ADDR(GPIOA_ODR_Addr,n)  /* 输出 */
#define PAin(n)    BIT_ADDR(GPIOA_IDR_Addr,n)  /* 输入 */

#define PBout(n)   BIT_ADDR(GPIOB_ODR_Addr,n)  /* 输出 */
#define PBin(n)    BIT_ADDR(GPIOB_IDR_Addr,n)  /* 输入 */

#define PCout(n)   BIT_ADDR(GPIOC_ODR_Addr,n)  /* 输出 */
#define PCin(n)    BIT_ADDR(GPIOC_IDR_Addr,n)  /* 输入 */

#define PDout(n)   BIT_ADDR(GPIOD_ODR_Addr,n)  /* 输出 */
#define PDin(n)    BIT_ADDR(GPIOD_IDR_Addr,n)  /* 输入 */

#define PEout(n)   BIT_ADDR(GPIOE_ODR_Addr,n)  /* 输出 */
#define PEin(n)    BIT_ADDR(GPIOE_IDR_Addr,n)  /* 输入 */

#define PFout(n)   BIT_ADDR(GPIOF_ODR_Addr,n)  /* 输出 */
#define PFin(n)    BIT_ADDR(GPIOF_IDR_Addr,n)  /* 输入 */

#define PGout(n)   BIT_ADDR(GPIOG_ODR_Addr,n)  /* 输出 */
#define PGin(n)    BIT_ADDR(GPIOG_IDR_Addr,n)  /* 输入 */

/******************************************************************************
 * 汇编内联函数
 ******************************************************************************/
void WFI_SET(void);         /* 执行WFI指令 */
void INTX_DISABLE(void);    /* 关闭所有中断 */
void INTX_ENABLE(void);     /* 开启所有中断 */
void MSR_MSP(u32 addr);     /* 设置栈顶地址 */

/******************************************************************************
 * 通用算法（供Drv_gpio等使用）
 ******************************************************************************/
u32 Com_pow(u32 x, u32 y);      /* x的y次方 */
u32 Com_log2(u32 x);            /* 求以2为底的对数（x须为2的整数次幂） */

#endif
