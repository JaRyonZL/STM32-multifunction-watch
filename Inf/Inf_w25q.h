/*
 * @Author: JaRyon
 * @Date: 2026-09-01 11:45:34
 * @Email: jaryonzl@163.com
 */
#ifndef __INF_W25Q_H
#define __INF_W25Q_H

/******************************************************************************
 * 文件名称：Inf_w25q.h（接口层）
 * 说    明：W25Q128外部Flash驱动（由旧工程W25Q128_SPI1迁移，
 *           基于Drv_spi1硬件SPI1，片选SS软件控制）
 ******************************************************************************/

#include "Com_def.h"

void Inf_w25q_init(void);                                       /* 初始化 */
void Inf_w25q_read_id(uint8_t* MID, uint16_t* DID);             /* 读厂商ID与设备ID */
void Inf_w25q_page_program(uint32_t Address, uint8_t* DataArray, uint32_t Count);  /* 页编程（<=256B） */
void Inf_w25q_program(uint32_t Address, uint8_t* DataArray, uint16_t Count);        /* 连续写（自动换页） */
void Inf_w25q_sector_erase(uint32_t Address);                   /* 扇区擦除（4KB） */
void Inf_w25q_read_data(uint32_t Address, uint8_t* DataArray, uint32_t Count);      /* 连续读 */
void Inf_w25q_write_status(void);                               /* 清写保护（状态寄存器1/2写0） */

#endif
