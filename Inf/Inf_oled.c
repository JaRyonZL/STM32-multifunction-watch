#include "Inf_oled.h"
#include "Drv_gpio.h"
#include "Drv_delay.h"
#include "Com_config.h"

/******************************************************************************
 * 文件名称：Inf_oled.c（接口层）
 * 说    明：OLED模块硬件时序：5引脚位带写、软件SPI、SSD1306命令与初始化
 ******************************************************************************/

/**
  * 函    数：写D0（CLK）引脚电平
  * 参    数：BitValue 电平值，范围0/1
  * 返 回 值：无
  * 说    明：位带操作，比GPIO_WriteBit快，适合软件SPI时序
  */
static void Inf_oled_w_d0(uint8_t BitValue)
{
	PBout(OLED_D0_BIT) = BitValue;
}

/**
  * 函    数：写D1（MOSI）引脚电平
  * 参    数：BitValue 电平值，范围0/1
  * 返 回 值：无
  */
static void Inf_oled_w_d1(uint8_t BitValue)
{
	PBout(OLED_D1_BIT) = BitValue;
}

/**
  * 函    数：写RES引脚电平
  * 参    数：BitValue 电平值，范围0/1
  * 返 回 值：无
  */
static void Inf_oled_w_res(uint8_t BitValue)
{
	PAout(OLED_RES_BIT) = BitValue;
}

/**
  * 函    数：写DC引脚电平
  * 参    数：BitValue 电平值，范围0/1
  * 返 回 值：无
  */
static void Inf_oled_w_dc(uint8_t BitValue)
{
	PBout(OLED_DC_BIT) = BitValue;
}

/**
  * 函    数：写CS引脚电平
  * 参    数：BitValue 电平值，范围0/1
  * 返 回 值：无
  */
static void Inf_oled_w_cs(uint8_t BitValue)
{
	PBout(OLED_CS_BIT) = BitValue;
}

/**
  * 函    数：软件SPI发送一个字节
  * 参    数：Byte 要发送的字节
  * 返 回 值：无
  * 说    明：MSB先行；D1先置位，D0拉高产生上升沿（上升沿OLED采样D1），再拉低
  */
static void Inf_oled_spi_send_byte(uint8_t Byte)
{
	uint8_t i;

	for (i = 0; i < 8; i++)
	{
		Inf_oled_w_d1(!!(Byte & (0x80 >> i)));
		Inf_oled_w_d0(1);
		Inf_oled_w_d0(0);
	}
}

/**
  * 函    数：OLED引脚初始化（内部调用）
  * 参    数：无
  * 返 回 值：无
  * 说    明：推挽输出50MHz，配置后置默认电平
  */
static void Inf_oled_gpio_init(void)
{
	/* 上电延时，等待OLED内部稳定 */
	Drv_delay_ms(10);

	/* D0/D1/DC/CS初始化为推挽输出 */
	Drv_GPIO_Init(OLED_CS_PORT, OLED_CS_PIN | OLED_DC_PIN | OLED_D0_PIN | OLED_D1_PIN,
	              GPIO_TW_OUT, GPIO_P_NO, GPIO_50MHz);

	/* RES初始化为推挽输出 */
	Drv_GPIO_Init(OLED_RES_PORT, OLED_RES_PIN, GPIO_TW_OUT, GPIO_P_NO, GPIO_50MHz);

	/* 默认电平 */
	Inf_oled_w_d0(0);
	Inf_oled_w_d1(1);
	Inf_oled_w_res(1);
	Inf_oled_w_dc(1);
	Inf_oled_w_cs(1);
}

/**
  * 函    数：OLED写命令
  * 参    数：Command 命令值
  * 返 回 值：无
  */
void Inf_oled_write_command(uint8_t Command)
{
	Inf_oled_w_cs(0);
	Inf_oled_w_dc(0);
	Inf_oled_spi_send_byte(Command);
	Inf_oled_w_cs(1);
}

/**
  * 函    数：OLED写数据
  * 参    数：Data 数据首地址
  * 参    数：Count 数据个数
  * 返 回 值：无
  */
void Inf_oled_write_data(uint8_t* Data, uint8_t Count)
{
	uint8_t i;

	Inf_oled_w_cs(0);
	Inf_oled_w_dc(1);
	for (i = 0; i < Count; i++)
	{
		Inf_oled_spi_send_byte(Data[i]);
	}
	Inf_oled_w_cs(1);
}

/**
  * 函    数：OLED初始化（SSD1306命令序列）
  * 参    数：无
  * 返 回 值：无
  * 说    明：清屏与刷新在B1c段（Inf_oled_clear/Inf_oled_update）加入后调用
  */
void Inf_oled_init(void)
{
	Inf_oled_gpio_init();

	Inf_oled_write_command(0xAE);	/* 显示关 */
	Inf_oled_write_command(0xD5);	/* 设置显示时钟分频比/振荡器频率 */
	Inf_oled_write_command(0x80);
	Inf_oled_write_command(0xA8);	/* 设置多路复用率 */
	Inf_oled_write_command(0x3F);
	Inf_oled_write_command(0xD3);	/* 设置显示偏移 */
	Inf_oled_write_command(0x00);
	Inf_oled_write_command(0x40);	/* 设置显示开始行 */
	Inf_oled_write_command(0xA1);	/* 设置左右方向（0xA1左到右） */
	Inf_oled_write_command(0xC8);	/* 设置上下方向（0xC8上到下） */
	Inf_oled_write_command(0xDA);	/* 设置COM引脚硬件配置 */
	Inf_oled_write_command(0x12);
	Inf_oled_write_command(0x81);	/* 设置对比度 */
	Inf_oled_write_command(0xFF);
	Inf_oled_write_command(0xD9);	/* 设置预充电周期 */
	Inf_oled_write_command(0xF2);
	Inf_oled_write_command(0xDB);	/* 设置VCOMH取消选择级别 */
	Inf_oled_write_command(0x30);
	Inf_oled_write_command(0xA4);	/* 设置整个显示打开/关闭 */
	Inf_oled_write_command(0xA6);	/* 设置正常/倒转显示 */
	Inf_oled_write_command(0xAD);	/* 设置电荷泵 */
	Inf_oled_write_command(0x8B);	/* 0x8B开启电荷泵 */
	Inf_oled_write_command(0xAF);	/* 显示开 */

	Inf_oled_clear();	/* 清显存 */
	Inf_oled_update();	/* 刷新，防止初始花屏 */
}

/**
  * 函    数：OLED设置显示位置
  * 参    数：Page 页号，范围0~7
  * 参    数：X 列号，范围0~127
  * 返 回 值：无
  * 说    明：兼容1.3寸SH1106（132列），起始列偏移2
  */
void Inf_oled_set_cursor(uint8_t Page, uint8_t X)
{
	X += 2;

	Inf_oled_write_command(0xB0 | Page);
	Inf_oled_write_command(0x10 | ((X & 0xF0) >> 4));
	Inf_oled_write_command(0x00 | (X & 0x0F));
}


/******************************************************************************
 * 显存定义（页式布局，1bit/像素；所有绘制只改显存，update才刷屏）
 ******************************************************************************/
uint8_t Inf_oled_display_buf[8][128];

/**
  * 函    数：整屏刷新（显存→硬件）
  * 参    数：无
  * 返 回 值：无
  */
void Inf_oled_update(void)
{
	uint8_t j;

	for (j = 0; j < 8; j++)
	{
		Inf_oled_set_cursor(j, 0);
		Inf_oled_write_data(Inf_oled_display_buf[j], 128);
	}
}

/**
  * 函    数：局部刷新（指定区域显存→硬件）
  * 参    数：X,Y 区域左上角坐标
  * 参    数：Width,Height 区域宽高
  * 返 回 值：无
  * 说    明：超出屏幕部分自动裁剪；Y方向按页整页刷新
  */
void Inf_oled_update_area(uint8_t X, uint8_t Y, uint8_t Width, uint8_t Height)
{
	uint8_t j;

	if (X > 127) {return;}
	if (Y > 63) {return;}
	if (X + Width > 128) {Width = 128 - X;}
	if (Y + Height > 64) {Height = 64 - Y;}

	for (j = Y / 8; j < (Y + Height - 1) / 8 + 1; j++)
	{
		Inf_oled_set_cursor(j, X);
		Inf_oled_write_data(&Inf_oled_display_buf[j][X], Width);
	}
}

/**
  * 函    数：清屏（显存全置0）
  * 参    数：无
  * 返 回 值：无
  */
void Inf_oled_clear(void)
{
	uint8_t i, j;

	for (j = 0; j < 8; j++)
	{
		for (i = 0; i < 128; i++)
		{
			Inf_oled_display_buf[j][i] = 0x00;
		}
	}
}

/**
  * 函    数：局部清屏（区域显存置0）
  * 参    数：X,Y 区域左上角坐标
  * 参    数：Width,Height 区域宽高
  * 返 回 值：无
  */
void Inf_oled_clear_area(uint8_t X, uint8_t Y, uint8_t Width, uint8_t Height)
{
	uint8_t i, j;

	if (X > 127) {return;}
	if (Y > 63) {return;}
	if (X + Width > 128) {Width = 128 - X;}
	if (Y + Height > 64) {Height = 64 - Y;}

	for (j = Y; j < Y + Height; j++)
	{
		for (i = X; i < X + Width; i++)
		{
			Inf_oled_display_buf[j / 8][i] &= ~(0x01 << (j % 8));
		}
	}
}

/**
  * 函    数：整屏取反
  * 参    数：无
  * 返 回 值：无
  */
void Inf_oled_reverse(void)
{
	uint8_t i, j;

	for (j = 0; j < 8; j++)
	{
		for (i = 0; i < 128; i++)
		{
			Inf_oled_display_buf[j][i] ^= 0xFF;
		}
	}
}

/**
  * 函    数：局部取反（区域显存按位取反）
  * 参    数：X,Y 区域左上角坐标
  * 参    数：Width,Height 区域宽高
  * 返 回 值：无
  */
void Inf_oled_reverse_area(uint8_t X, uint8_t Y, uint8_t Width, uint8_t Height)
{
	uint8_t i, j;

	if (X > 127) {return;}
	if (Y > 63) {return;}
	if (X + Width > 128) {Width = 128 - X;}
	if (Y + Height > 64) {Height = 64 - Y;}

	for (j = Y; j < Y + Height; j++)
	{
		for (i = X; i < X + Width; i++)
		{
			Inf_oled_display_buf[j / 8][i] ^= 0x01 << (j % 8);
		}
	}
}


/******************************************************************************
 * 亮度过渡动画
 * 用法：置Inf_oled_fade_flag=1后，每循环调Inf_oled_transition(1)渐亮、
 * Inf_oled_transition(0)渐灭（阻塞），过渡完成后标志自动清零
 ******************************************************************************/
uint8_t Inf_oled_brightness = COM_OLED_DEFAULT_BRIGHTNESS;   /* 目标亮度 */
uint8_t Inf_oled_brightness_up = 0x00;                       /* 当前过渡亮度 */
uint8_t Inf_oled_fade_flag = 0;                              /* 过渡标志 */
uint8_t Inf_oled_fade_step = COM_OLED_FADE_STEP;             /* 每步增量 */

/**
  * 函    数：亮度过渡
  * 参    数：dir 1=渐亮（非阻塞，每循环调用一次），0=渐灭（阻塞，直到全灭）
  * 返 回 值：无
  * 说    明：使用前需先置Inf_oled_fade_flag=1，过渡完成后标志自动清零
  */
void Inf_oled_gradient(uint8_t dir)
{
	if (Inf_oled_fade_flag)
	{
		if (dir)
		{
			/* 渐亮：快到位时直接跳目标并清标志 */
			if (Inf_oled_brightness_up > Inf_oled_brightness - Inf_oled_fade_step)
			{
				Inf_oled_brightness_up = Inf_oled_brightness;
				Inf_oled_write_command(0x81);
				Inf_oled_write_command(Inf_oled_brightness_up);
				Inf_oled_fade_flag = 0;
				return;
			}

			Inf_oled_brightness_up = Inf_oled_brightness_up + Inf_oled_fade_step;
			Inf_oled_write_command(0x81);
			Inf_oled_write_command(Inf_oled_brightness_up);
		}
		else
		{
			/* 渐灭：阻塞步进直到全灭 */
			while (1)
			{
				if (Inf_oled_brightness_up < Inf_oled_fade_step)
				{
					Inf_oled_brightness_up = 0;
					Inf_oled_write_command(0x81);
					Inf_oled_write_command(Inf_oled_brightness_up);
					break;
				}

				Inf_oled_write_command(0x81);
				Inf_oled_write_command(Inf_oled_brightness_up);
				Drv_delay_ms(16);
				Inf_oled_brightness_up = Inf_oled_brightness_up - Inf_oled_fade_step;
			}

			Drv_delay_ms(16);
			Inf_oled_fade_flag = 0;
		}
	}
}
