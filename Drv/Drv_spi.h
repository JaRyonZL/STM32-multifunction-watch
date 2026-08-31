#ifndef __DRV_SPI_H
#define __DRV_SPI_H

/******************************************************************************
 * 文件名称：Drv_spi.h（驱动层）
 * 说    明：SPI1硬件驱动（由旧工程W25Q128_SPI1.c的SPI配置部分抽出）
 *           模式0，2分频（36MHz），软件NSS；片选SS由上层管理
 ******************************************************************************/

#include "Com_def.h"
#include "Com_board.h"

void Drv_spi1_init(void);                        /* SPI1初始化 */
uint8_t Drv_spi1_swap_byte(uint8_t ByteSend);    /* 交换传输一个字节，返回接收字节 */

#endif
