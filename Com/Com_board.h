#ifndef __COM_BOARD_H
#define __COM_BOARD_H

/******************************************************************************
 * 文件名称：Com_board.h（公共层）
 * 说    明：板级引脚分配总表（STM32手表V2.0）
 *           所有外设GPIO引脚宏集中在此处定义，各层统一引用；
 *           改线时只需修改本文件，无需改动各驱动源码。
 * 依赖规则：仅依赖标准外设库（stm32f10x.h的GPIO定义）
 ******************************************************************************/

#include "stm32f10x.h"

/******************************************************************************
 * OLED显示屏（0.96寸SSD1306，软件SPI，7针接口）
 ******************************************************************************/
#define OLED_CS_PORT    GPIOB
#define OLED_CS_PIN     GPIO_Pin_12     /* 片选 CS      -> PB12 */
#define OLED_DC_PORT    GPIOB
#define OLED_DC_PIN     GPIO_Pin_14     /* 数据/命令 DC -> PB14 */
#define OLED_D0_PORT    GPIOB
#define OLED_D0_PIN     GPIO_Pin_13     /* 时钟 D0/CLK  -> PB13 */
#define OLED_D1_PORT    GPIOB
#define OLED_D1_PIN     GPIO_Pin_15     /* 数据 D1/MOSI -> PB15 */
#define OLED_RES_PORT   GPIOA
#define OLED_RES_PIN    GPIO_Pin_8      /* 复位 RES     -> PA8 */

/******************************************************************************
 * W25Q128外部Flash（SPI1硬件外设，片选由软件控制）
 ******************************************************************************/
#define W25Q_SS_PORT    GPIOA
#define W25Q_SS_PIN     GPIO_Pin_4      /* 片选 SS -> PA4 */
#define W25Q_SCK_PORT   GPIOA
#define W25Q_SCK_PIN    GPIO_Pin_5      /* SPI1_SCK -> PA5 */
#define W25Q_MISO_PORT  GPIOA
#define W25Q_MISO_PIN   GPIO_Pin_6      /* SPI1_MISO -> PA6 */
#define W25Q_MOSI_PORT  GPIOA
#define W25Q_MOSI_PIN   GPIO_Pin_7      /* SPI1_MOSI -> PA7 */

/******************************************************************************
 * MPU6050 姿态传感器（软件I2C）
 ******************************************************************************/
#define MPU6050_IIC_GPIO        GPIOB
#define MPU6050_IIC_SCL_Pin     GPIO_Pin_9      /* SCL -> PB9 */
#define MPU6050_IIC_SDA_Pin     GPIO_Pin_8      /* SDA -> PB8 */

/******************************************************************************
 * 按键（下拉输入）
 ******************************************************************************/
#define KEY1_PORT   GPIOC
#define KEY1_PIN    GPIO_Pin_13    /* KEY1 下 -> PC13 */
#define KEY2_PORT   GPIOA
#define KEY2_PIN    GPIO_Pin_0     /* KEY2 上 -> PA0（兼WKUP唤醒/EXTI0） */
#define KEY3_PORT   GPIOA
#define KEY3_PIN    GPIO_Pin_1     /* KEY3 确定 -> PA1 */

/******************************************************************************
 * ADC1 采样通道
 ******************************************************************************/
#define ADC_BATTERY_PORT    GPIOB
#define ADC_BATTERY_PIN     GPIO_Pin_0          /* CH8：电池电压 -> PB0 */
#define ADC_CH_BATTERY      ADC_Channel_8       /* PB0：电池电压（硬件1/2分压） */

#define ADC_VOLTAGE_PORT    GPIOB
#define ADC_VOLTAGE_PIN     GPIO_Pin_1          /* CH9：外部电压 -> PB1 */
#define ADC_CH_VOLTAGE      ADC_Channel_9       /* PB1：外部电压检测 */

#define ADC_CH_VREFINT      ADC_Channel_17      /* 内部参考电压 */

/******************************************************************************
 * 电池与电源
 ******************************************************************************/
#define MEAS_PWR_PORT   GPIOB
#define MEAS_PWR_PIN    GPIO_Pin_5         /* 测量分压电路电源，0=接通 1=断开 -> PB5 */
#define CHG_DET_PORT    GPIOB
#define CHG_DET_PIN     GPIO_Pin_3         /* 充电状态检测，0=正在充电 -> PB3 */

/******************************************************************************
 * MP3模块（USART2串口协议模块）
 ******************************************************************************/
#define MP3_PWR_DET_PORT    GPIOA
#define MP3_PWR_DET_PIN     GPIO_Pin_11     /* 电源检测，1=模块已供电 -> PA11 */
#define MP3_AMP_EN_PORT     GPIOA
#define MP3_AMP_EN_PIN      GPIO_Pin_12     /* 功放使能，0=开启 1=关闭 -> PA12 */

/******************************************************************************
 * 串口引脚分配
 * UART1：备用（PA9/PA10，9600）
 * UART2：MP3模块（PA2/PA3，9600）
 * UART3：上位机视频帧收发（PB10/PB11，921600）
 ******************************************************************************/
#define UART1_TX_PORT   GPIOA
#define UART1_TX_PIN    GPIO_Pin_9
#define UART1_RX_PORT   GPIOA
#define UART1_RX_PIN    GPIO_Pin_10

#define UART2_TX_PORT   GPIOA
#define UART2_TX_PIN    GPIO_Pin_2
#define UART2_RX_PORT   GPIOA
#define UART2_RX_PIN    GPIO_Pin_3

#define UART3_TX_PORT   GPIOB
#define UART3_TX_PIN    GPIO_Pin_10
#define UART3_RX_PORT   GPIOB
#define UART3_RX_PIN    GPIO_Pin_11

#endif
