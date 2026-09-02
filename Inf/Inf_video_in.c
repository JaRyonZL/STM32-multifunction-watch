#include "Inf_video_in.h"
#include "Inf_oled.h"
#include "Drv_uart.h"

/******************************************************************************
 * 文件名：Inf_video_in.c（接口层）
 * 说   明：USART3视频流输入封装（921600bps，上位机下发1024字节/帧）
 *          收字节直填OLED显存，满一帧返回标志；帧组装由上层轮询驱动
 ******************************************************************************/

static uint8_t s_page = 0;   /* 帧位置：页(0~7) */
static uint8_t s_col = 0;    /* 帧位置：列(0~127) */

/**
  * 函   数：Inf_video_in_init
  * 功   能：USART3 初始化并重置帧位置
  */
void Inf_video_in_init(void)
{
	Drv_uart_init(&Drv_uart3);
	Inf_video_in_reset_frame();
}

/**
  * 函   数：Inf_video_in_power_off
  * 功   能：USART3 断电省电
  */
void Inf_video_in_power_off(void)
{
	Drv_uart_power_off(&Drv_uart3);
}

/**
  * 函   数：Inf_video_in_reset_frame
  * 功   能：重置帧位置（旧工程 p0=0,p1=0）
  */
void Inf_video_in_reset_frame(void)
{
	s_page = 0;
	s_col = 0;
}

/**
  * 函   数：Inf_video_in_fill_frame
  * 功   能：把接收缓冲中的字节填入显存，满一帧(1024字节)返回1
  * 说   明：烧录 Flash 期间字节积压在环形缓冲(1536B)中，不丢失
  */
uint8_t Inf_video_in_fill_frame(void)
{
	uint8_t frame_full = 0;

	while (Drv_uart_rx_available(&Drv_uart3))
	{
		Inf_oled_display_buf[s_page][s_col] = Drv_uart_read_byte(&Drv_uart3);
		s_col++;
		if (s_col >= 128)
		{
			s_col = 0;
			s_page++;
			if (s_page >= 8)
			{
				s_page = 0;
				frame_full = 1;   /* 满一帧(1024字节) */
			}
		}
	}

	return frame_full;
}
