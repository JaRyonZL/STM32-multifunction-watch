#include "Inf_mp3.h"
#include "Com_board.h"
#include "Com_config.h"
#include "Drv_gpio.h"
#include "Drv_delay.h"
#include "Drv_uart.h"

/******************************************************************************
 * 文件名称：Inf_mp3.c（接口层）
 * 说    明：MP3模块（USART2，9600），由旧工程MP3_tf.c协议部分与menu_Data.c
 *           扬声器/音量部分迁移
 ******************************************************************************/

/* 状态初始值按结构体字段顺序：playing/loop_play/play_once/volume/chapter/chapter_max/update_pending/loudspeaker_on */
Inf_mp3_state_t Inf_mp3 = { 0, 0, 0, COM_MP3_DEFAULT_VOLUME, 0, COM_MP3_DEFAULT_CHAPTER_MAX, 0, 1 };

/* 回传包解析状态（由旧工程Serial2.c中断状态机迁移，改为轮询处理） */
static uint8_t s_rx_state = 0;    /* 当前状态：0=找包头 1~9=收数据 */
static uint8_t s_rx_index = 0;    /* 包内数据写入位置 */
static uint8_t s_rx_packet[9];    /* 包头0xFF后的9字节数据 */
static uint8_t s_rx_flag = 0;     /* 收包完成标志 */

/**
  * 函    数：Inf_mp3_init
  * 功    能：功放使能脚默认关闭、电源检测输入、串口初始化
  */
void Inf_mp3_init(void)
{
	Drv_GPIO_Init(MP3_AMP_EN_PORT, MP3_AMP_EN_PIN, GPIO_TW_OUT, GPIO_P_NO, GPIO_50MHz);
	GPIO_SetBits(MP3_AMP_EN_PORT, MP3_AMP_EN_PIN);  /* 默认关闭功放 */

	Drv_GPIO_Init(MP3_PWR_DET_PORT, MP3_PWR_DET_PIN, GPIO_FK_IN, GPIO_P_DOWN, GPIO_50MHz);

	Drv_uart_init(&Drv_uart2);
}

/**
  * 函    数：Inf_mp3_power_off
  * 功    能：关闭功放并断电串口
  */
void Inf_mp3_power_off(void)
{
	GPIO_SetBits(MP3_AMP_EN_PORT, MP3_AMP_EN_PIN);
	Drv_uart_power_off(&Drv_uart2);
}

/**
  * 函    数：Inf_mp3_send_cmd
  * 功    能：发送协议帧：7E FF 06 CMD FB DH DL EF
  */
void Inf_mp3_send_cmd(uint8_t CMD, uint8_t feedback, uint16_t data)
{
	uint8_t buf[8];

	buf[0] = 0x7E;                  /* 帧头 */
	buf[1] = 0xFF;                  /* 版本 */
	buf[2] = 0x06;                  /* 长度 */
	buf[3] = CMD;                   /* 命令 */
	buf[4] = feedback;              /* 是否需要回传 */
	buf[5] = (uint8_t)(data >> 8);  /* 数据高字节 */
	buf[6] = (uint8_t)data;         /* 数据低字节 */
	buf[7] = 0xEF;                  /* 帧尾 */

	Drv_uart_send_array(&Drv_uart2, buf, 8);
}

/**
  * 函    数：Inf_mp3_start
  * 功    能：开始/停止播放（与旧工程MP3_starts一致，含等待延时）
  */
void Inf_mp3_start(uint8_t on)
{
	if (on)
	{
		Inf_mp3_send_cmd(0x06, 0x00, Inf_mp3.volume);  /* 设置音量 */
		Drv_delay_ms(10);
		Inf_mp3_send_cmd(0x0D, 0x00, 0x00);            /* 开始播放 */
		Drv_delay_ms(200);
		Inf_mp3.playing = 1;
	}
	else
	{
		Inf_mp3_send_cmd(0x0E, 0x00, 0x00);            /* 停止播放 */
		Drv_delay_ms(200);
		Inf_mp3.playing = 0;
	}
}

/**
  * 函    数：Inf_mp3_switch_chapter
  * 功    能：曲目切换：1=下一曲 0=上一曲（下溢回绕到最大，与旧工程一致）
  */
void Inf_mp3_switch_chapter(uint8_t next)
{
	if (next)
	{
		Inf_mp3.chapter++;
		if (Inf_mp3.chapter > Inf_mp3.chapter_max) Inf_mp3.chapter = 0;
		Inf_mp3.playing = 1;
		Inf_mp3.update_pending = 1;
	}
	else
	{
		Inf_mp3.chapter--;
		if (Inf_mp3.chapter > Inf_mp3.chapter_max) Inf_mp3.chapter = Inf_mp3.chapter_max;
		Inf_mp3.playing = 1;
		Inf_mp3.update_pending = 1;
	}
}

/**
  * 函    数：Inf_mp3_send_volume
  * 功    能：发送音量命令（旧工程MP3ping/mp3_volume合并，二者命令相同）
  */
void Inf_mp3_send_volume(void)
{
	Inf_mp3_send_cmd(0x06, 0x00, Inf_mp3.volume);
}

/**
  * 函    数：Inf_mp3_loudspeaker
  * 功    能：按Inf_mp3.loudspeaker_on刷新功放（PA12：0=开 1=关）
  */
void Inf_mp3_loudspeaker(void)
{
	if (Inf_mp3.loudspeaker_on)
	{
		GPIO_ResetBits(MP3_AMP_EN_PORT, MP3_AMP_EN_PIN);
	}
	else
	{
		GPIO_SetBits(MP3_AMP_EN_PORT, MP3_AMP_EN_PIN);
	}
}

/**
  * 函    数：Inf_mp3_is_powered
  * 功    能：模块电源检测，1=已上电（PA11=1）
  */
uint8_t Inf_mp3_is_powered(void)
{
	return (GPIO_ReadInputDataBit(MP3_PWR_DET_PORT, MP3_PWR_DET_PIN) == 1) ? 1 : 0;
}

/**
  * 函    数：Inf_mp3_poll
  * 功    能：回传包解析与处理，主循环周期调用
  * 说    明：状态机由旧工程Serial2.c中断处理迁移（0xFF包头+9字节），
  *           CMD=0x3D为播放结束：播完即停->停止，循环->重播，顺序->切下一曲
  */
void Inf_mp3_poll(void)
{
	while (Drv_uart_rx_available(&Drv_uart2))
	{
		uint8_t RxData = Drv_uart_read_byte(&Drv_uart2);

		if (s_rx_state == 0)                /* 找包头0xFF（上一包已处理完） */
		{
			if (RxData == 0xFF && s_rx_flag == 0)
			{
				s_rx_state = 1;
				s_rx_index = 0;
			}
		}
		else if (s_rx_state <= 9)           /* 收9字节数据 */
		{
			s_rx_packet[s_rx_index] = RxData;
			s_rx_index++;

			if (s_rx_state == 9)
			{
				s_rx_flag = 1;              /* 收包完成 */
				s_rx_state = 0;
			}
			else
			{
				s_rx_state++;
			}
		}

		if (s_rx_flag == 1)                 /* 处理完整回传包 */
		{
			if (s_rx_packet[1] == 0x3D)     /* 播放结束 */
			{
				if (Inf_mp3.play_once)
				{
					Inf_mp3.playing = 0;
				}
				else if (Inf_mp3.loop_play)
				{
					Inf_mp3.update_pending = 1;    /* 重播当前曲 */
				}
				else
				{
					Inf_mp3_switch_chapter(1);     /* 顺序播放切下一曲 */
				}
			}
			s_rx_flag = 0;
		}
	}
}
