/*
 * @Author: JaRyon
 * @Date: 2026-08-31 17:07:33
 * @Email: jaryonzl@163.com
 */
#ifndef __DRV_UART_H
#define __DRV_UART_H

/******************************************************************************
 * 文件名称：Drv_uart.h（驱动层）
 * 说    明：USART1/2/3通用驱动，句柄化设计（由旧工程Serial1/2/3三合一迁移，
 *           消除三份重复的收发代码）
 *           驱动层只负责片上外设配置与字节收发，接收中断将字节写入环形缓冲；
 *           协议解析、帧填充等业务由上层基于本驱动的读取接口自行实现
 ******************************************************************************/

#include "Com_def.h"
#include "Com_board.h"

/******************************************************************************
 * 各实例接收缓冲大小（字节）
 * UART1/UART2波特率9600，小缓冲足够；
 * UART3视频流921600bps，主循环烧写Flash期间（约3ms）数据量约280字节，
 * 取1536字节留足余量
 ******************************************************************************/
#define DRV_UART1_RX_BUF_SIZE   64
#define DRV_UART2_RX_BUF_SIZE   64
#define DRV_UART3_RX_BUF_SIZE   1536

/******************************************************************************
 * 串口句柄结构体
 ******************************************************************************/
typedef struct
{
	/* 外设配置 */
	USART_TypeDef* USARTx;                          /* USART外设地址 */
	uint32_t Baud;                                  /* 波特率 */
	GPIO_TypeDef* GPIOx;                            /* 引脚所在端口 */
	uint16_t TxPin;                                 /* TX引脚 */
	uint16_t RxPin;                                 /* RX引脚 */
	uint32_t RCC_USARTx;                            /* USART外设时钟 */
	uint32_t RCC_GPIOx;                             /* 端口时钟 */
	IRQn_Type IRQn;                                 /* 中断号 */
	uint8_t PrePri;                                 /* 抢占优先级 */
	uint8_t SubPri;                                 /* 子优先级 */
	uint8_t APBx;                                   /* USART所在总线：1=APB1，2=APB2 */

	/* 接收环形缓冲区（中断写入，主循环读取） */
	uint8_t* RxBuf;                                 /* 缓冲首地址，由实例配置指定 */
	uint16_t RxBufSize;                             /* 缓冲大小 */
	volatile uint16_t RxHead;                       /* 写指针 */
	volatile uint16_t RxCount;                      /* 缓冲内字节数 */
	volatile uint8_t  RxOverflow;                   /* 溢出标志：缓冲满丢数据时置1 */
} Drv_uart_inst_t;

/******************************************************************************
 * 三路串口实例
 ******************************************************************************/
extern Drv_uart_inst_t Drv_uart1;                   /* USART1备用：PA9/PA10，9600 */
extern Drv_uart_inst_t Drv_uart2;                   /* USART2 MP3模块：PA2/PA3，9600 */
extern Drv_uart_inst_t Drv_uart3;                   /* USART3视频流：PB10/PB11，921600 */

/******************************************************************************
 * 函数声明
 ******************************************************************************/
void Drv_uart_init(Drv_uart_inst_t* inst);                  /* 串口初始化 */
void Drv_uart_send_byte(Drv_uart_inst_t* inst, uint8_t Byte);
void Drv_uart_send_array(Drv_uart_inst_t* inst, uint8_t* Array, uint16_t Length);
void Drv_uart_send_string(Drv_uart_inst_t* inst, char* String);
uint16_t Drv_uart_rx_available(Drv_uart_inst_t* inst);      /* 缓冲中可读字节数 */
uint8_t  Drv_uart_read_byte(Drv_uart_inst_t* inst);         /* 读取一个字节（无数据返回0） */
void Drv_uart_power_off(Drv_uart_inst_t* inst);             /* 断电省电：引脚浮空+关闭USART */
void Drv_uart_irq_handler(Drv_uart_inst_t* inst);           /* 中断分发入口，由IRQ函数调用 */

#endif
