#include "Inf_w25q.h"
#include "Com_board.h"
#include "Drv_spi.h"
#include "Drv_gpio.h"

/******************************************************************************
 * 文件名称：Inf_w25q.c（接口层）
 * 说    明：W25Q128外部Flash驱动实现
 ******************************************************************************/

/* 指令定义（由旧工程W25Q128_SPI1_Ins.h迁移） */
#define W25Q_WRITE_ENABLE               0x06
#define W25Q_WRITE_DISABLE              0x04
#define W25Q_READ_STATUS_REGISTER_1     0x05
#define W25Q_WRITE_STATUS_REGISTER      0x01
#define W25Q_PAGE_PROGRAM               0x02
#define W25Q_SECTOR_ERASE_4KB           0x20
#define W25Q_READ_DATA                  0x03
#define W25Q_JEDEC_ID                   0x9F
#define W25Q_DUMMY_BYTE                 0xFF

/**
  * 函    数：写SS片选电平
  * 参    数：BitValue 电平值，范围0/1
  * 返 回 值：无
  */
static void Inf_w25q_w_ss(uint8_t BitValue)
{
	GPIO_WriteBit(W25Q_SS_PORT, W25Q_SS_PIN, (BitAction)BitValue);
}

/**
  * 函    数：SPI起始（拉低SS）
  * 参    数：无
  * 返 回 值：无
  */
static void Inf_w25q_start(void)
{
	Inf_w25q_w_ss(0);
}

/**
  * 函    数：SPI终止（拉高SS）
  * 参    数：无
  * 返 回 值：无
  */
static void Inf_w25q_stop(void)
{
	Inf_w25q_w_ss(1);
}

/**
  * 函    数：写使能
  * 参    数：无
  * 返 回 值：无
  */
static void Inf_w25q_write_enable(void)
{
	Inf_w25q_start();
	Drv_spi1_swap_byte(W25Q_WRITE_ENABLE);
	Inf_w25q_stop();
}

/**
  * 函    数：等待忙（超时计数保真迁移自旧工程）
  * 参    数：无
  * 返 回 值：无
  */
static void Inf_w25q_wait_busy(void)
{
	uint32_t Timeout;

	Inf_w25q_start();
	Drv_spi1_swap_byte(W25Q_READ_STATUS_REGISTER_1);
	Timeout = 114514;
	while ((Drv_spi1_swap_byte(W25Q_DUMMY_BYTE) & 0x01) == 0x01)
	{
		Timeout--;
		if (Timeout <= 1919)
		{
			/* 超时的错误处理代码，可以添加到此处 */
			break;
		}
	}
	Inf_w25q_stop();
}

/**
  * 函    数：W25Q128初始化
  * 参    数：无
  * 返 回 值：无
  * 说    明：SS引脚推挽输出置高，SPI1由Drv_spi1初始化（模式0，2分频）
  */
void Inf_w25q_init(void)
{
	/* SS引脚初始化为推挽输出 */
	Drv_GPIO_Init(W25Q_SS_PORT, W25Q_SS_PIN, GPIO_TW_OUT, GPIO_P_NO, GPIO_50MHz);

	/* SPI1初始化 */
	Drv_spi1_init();

	/* SS默认高电平 */
	Inf_w25q_w_ss(1);
}

/**
  * 函    数：读取ID号
  * 参    数：MID 工厂ID，输出参数
  * 参    数：DID 设备ID，输出参数
  * 返 回 值：无
  */
void Inf_w25q_read_id(uint8_t* MID, uint16_t* DID)
{
	Inf_w25q_start();
	Drv_spi1_swap_byte(W25Q_JEDEC_ID);
	*MID = Drv_spi1_swap_byte(W25Q_DUMMY_BYTE);
	*DID = Drv_spi1_swap_byte(W25Q_DUMMY_BYTE);
	*DID <<= 8;
	*DID |= Drv_spi1_swap_byte(W25Q_DUMMY_BYTE);
	Inf_w25q_stop();
}

/**
  * 函    数：页编程
  * 参    数：Address 起始地址（0x000000~0xFFFFFF）
  * 参    数：DataArray 数据数组
  * 参    数：Count 数据个数（0~256，跨页请用Inf_w25q_program）
  * 返 回 值：无
  */
void Inf_w25q_page_program(uint32_t Address, uint8_t* DataArray, uint32_t Count)
{
	uint16_t i;

	Inf_w25q_wait_busy();
	Inf_w25q_write_enable();

	Inf_w25q_start();
	Drv_spi1_swap_byte(W25Q_PAGE_PROGRAM);
	Drv_spi1_swap_byte(Address >> 16);
	Drv_spi1_swap_byte(Address >> 8);
	Drv_spi1_swap_byte(Address);
	for (i = 0; i < Count; i++)
	{
		Drv_spi1_swap_byte(DataArray[i]);
	}
	Inf_w25q_stop();
}

/**
  * 函    数：无检验连续写（自动换页）
  * 参    数：Address 起始地址
  * 参    数：DataArray 数据数组
  * 参    数：Count 数据个数（0~65535）
  * 返 回 值：无
  */
void Inf_w25q_program(uint32_t Address, uint8_t* DataArray, uint16_t Count)
{
	uint16_t Pageremain;

	Pageremain = 256 - Address % 256;           /* 单页剩余字节数 */
	if (Count <= Pageremain) Pageremain = Count;    /* 不大于256个字节 */

	while (1)
	{
		Inf_w25q_page_program(Address, DataArray, Pageremain);
		if (Count == Pageremain) break;             /* 写入结束 */
		else
		{
			DataArray += Pageremain;
			Address += Pageremain;

			Count -= Pageremain;

			if (Count > 256) Pageremain = 256;      /* 一次最多写入256字节 */
			else Pageremain = Count;
		}
	}
}

/**
  * 函    数：扇区擦除（4KB）
  * 参    数：Address 扇区地址（0x000000~0xFFFFFF）
  * 返 回 值：无
  */
void Inf_w25q_sector_erase(uint32_t Address)
{
	Inf_w25q_wait_busy();
	Inf_w25q_write_enable();

	Inf_w25q_start();
	Drv_spi1_swap_byte(W25Q_SECTOR_ERASE_4KB);
	Drv_spi1_swap_byte(Address >> 16);
	Drv_spi1_swap_byte(Address >> 8);
	Drv_spi1_swap_byte(Address);
	Inf_w25q_stop();
}

/**
  * 函    数：连续读数据
  * 参    数：Address 起始地址（0x000000~0xFFFFFF）
  * 参    数：DataArray 接收数组
  * 参    数：Count 读取个数
  * 返 回 值：无
  */
void Inf_w25q_read_data(uint32_t Address, uint8_t* DataArray, uint32_t Count)
{
	uint32_t i;

	Inf_w25q_wait_busy();

	Inf_w25q_start();
	Drv_spi1_swap_byte(W25Q_READ_DATA);
	Drv_spi1_swap_byte(Address >> 16);
	Drv_spi1_swap_byte(Address >> 8);
	Drv_spi1_swap_byte(Address);
	for (i = 0; i < Count; i++)
	{
		DataArray[i] = Drv_spi1_swap_byte(W25Q_DUMMY_BYTE);
	}
	Inf_w25q_stop();
}

/**
  * 函    数：清除写保护（状态寄存器1/2写0）
  * 参    数：无
  * 返 回 值：无
  * 说    明：录制视频前必调，否则写入无效
  */
void Inf_w25q_write_status(void)
{
	Inf_w25q_write_enable();

	Inf_w25q_start();
	Drv_spi1_swap_byte(W25Q_WRITE_STATUS_REGISTER);
	Drv_spi1_swap_byte(0x00);       /* 状态寄存器1 */
	Drv_spi1_swap_byte(0x00);       /* 状态寄存器2 */
	Inf_w25q_stop();

	Inf_w25q_wait_busy();
}
