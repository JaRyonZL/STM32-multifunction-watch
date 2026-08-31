#include "Drv_spi.h"

/******************************************************************************
 * 文件名称：Drv_spi.c（驱动层）
 * 说    明：SPI1硬件驱动实现
 ******************************************************************************/

/**
  * 函    数：SPI1初始化
  * 参    数：无
  * 返 回 值：无
  * 说    明：主模式，8位，高位先行，模式0，2分频（36MHz），软件NSS
  *           片选SS由上层管理，本驱动不配置
  */
void Drv_spi1_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	SPI_InitTypeDef SPI_InitStructure;

	/* 开时钟 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

	/* GPIO：SCK/MOSI复用推挽输出 */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = W25Q_SCK_PIN | W25Q_MOSI_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(W25Q_SCK_PORT, &GPIO_InitStructure);

	/* GPIO：MISO上拉输入 */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = W25Q_MISO_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(W25Q_MISO_PORT, &GPIO_InitStructure);

	/* SPI初始化 */
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPI1, &SPI_InitStructure);

	/* SPI使能 */
	SPI_Cmd(SPI1, ENABLE);
}

/**
  * 函    数：SPI交换传输一个字节
  * 参    数：ByteSend 要发送的字节
  * 返 回 值：接收的字节
  */
uint8_t Drv_spi1_swap_byte(uint8_t ByteSend)
{
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) != SET);
	SPI_I2S_SendData(SPI1, ByteSend);
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) != SET);
	return SPI_I2S_ReceiveData(SPI1);
}
