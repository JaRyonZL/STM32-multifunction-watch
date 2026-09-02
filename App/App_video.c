#include "App_video.h"
#include "App_menu.h"
#include "Inf_oled.h"
#include "Inf_oled_gfx.h"
#include "Inf_w25q.h"
#include "Inf_video_in.h"
#include "Inf_mp3.h"
#include "Inf_key.h"
#include "Drv_delay.h"

/******************************************************************************
 * 文件名：App_video.c（应用层）
 * 说   明：视频播放应用（迁移自旧工程 video_app.c 的播放部分）
 *          视频区格式：3字节头（0xAA+帧数）+ 每帧1024字节
 *          播放时 MP3 模块伴音：0x0F 指定文件夹播放
 ******************************************************************************/

/* 视频区起始地址 0x28000（与 W25Q128 分区约定一致） */
#define APP_VIDEO_BASE_ADDRESS 163840

/**
  * 函   数：App_video_stop_mp3
  * 功   能：停止 MP3 伴音并断电（旧工程退出路径行为）
  */
static void App_video_stop_mp3(void)
{
	if (Inf_mp3_is_powered())
	{
		Inf_mp3_start(0);        /* 停止播放 */
		Inf_mp3_power_off();     /* 关功放+串口断电 */
	}
}

/**
  * 函   数：App_video_next_site
  * 功   能：计算下一个视频的地址（旧工程 VIDEO_SITE）
  */
static uint32_t App_video_next_site(uint32_t address)
{
	uint8_t temp[3];      /* 文件头(3字节) */
	uint32_t mydata = 0;  /* 数据量(不含文件头) */

	Inf_w25q_read_data(address, temp, 3);

	mydata = (temp[2] << 8) | temp[1];  /* 获取总帧数 */
	mydata *= 1024;
	address += (mydata + 3);

	return address;
}

/**
  * 函   数：App_video_play
  * 功   能：视频播放（旧工程 SHOW_VIDEO）
  * 说   明：上下键微调帧速(temm)，短按确定暂停/恢复，长按退出，
  *          播放完成自动退出
  */
static void App_video_play(uint32_t address, uint16_t mpg)
{
	int8_t key;
	uint8_t temp[3];       /* 文件头(3字节) */
	uint32_t mydata = 0;   /* 数据量(不含文件头) */
	uint32_t datasite = 3; /* 帧数据偏移地址 */
	uint16_t temm = 15;    /* 帧间隔时间(ms) */

	Inf_oled_clear();
	Inf_oled_update();
	Inf_oled_write_command(0x81);	/* 设置对比度 */
	Inf_oled_write_command(0xFF);	/* 0x00~0xFF */

	Inf_w25q_read_data(address, temp, 3);

	if (temp[0] != 0xAA) /* 检查文件头是否正确 */
	{
		Inf_oled_show_string(0, 20, "未检测到文件", OLED_6X8);
		Inf_oled_update();

		Drv_delay_ms(500);
		Inf_oled_fade_flag = 1;
		Inf_oled_gradient(0);
		return;
	}

	mydata = (temp[2] << 8) | temp[1];  /* 获取总帧数 */

	Inf_oled_show_string(0, 0, "帧数:30FPS", OLED_6X8);
	Inf_oled_show_string(0, 13, "帧数:", OLED_6X8);
	Inf_oled_show_num(38, 15, mydata, 4, OLED_6X8);

	mydata *= 1024; 				 /* 换算总字节数 */

	Inf_oled_show_string(0, 26, "数据量(字节):", OLED_6X8);
	Inf_oled_show_num(0, 40, mydata, 7, OLED_6X8);

	if (Inf_mp3_is_powered())
	{
		Inf_oled_show_string(0, 52, "MP3已打开", OLED_6X8);

		Inf_mp3_init();

		Inf_mp3_send_cmd(0x06, 0x00, Inf_mp3.volume);   /* 设置音量 */
		Inf_mp3_loudspeaker();
		Inf_mp3_loudspeaker();
		Drv_delay_ms(50);
		Inf_mp3_send_cmd(0x0F, 0x00, mpg);              /* 指定文件夹播放 */
	}
	else Inf_oled_show_string(0, 52, "MP3未打开", OLED_6X8);

	Inf_oled_show_string(92, 52, "读取中", OLED_6X8);	/* 提示长按退出 */

	Inf_oled_update();

	while (1)
	{
		Inf_w25q_read_data(address + datasite, Inf_oled_display_buf[0], 1024); /* 读取一帧到显存 */
		datasite += 1024;       /* 地址偏移到下一帧 */

		Drv_delay_ms(temm);		/* 帧间隔时间 */

		Inf_oled_update();
		Inf_oled_gradient(1);

		key = Inf_key_scan();   /* 上下键微调帧速 */
		if (key == 1)
		{
			temm++;
		}
		else if (key == 2)
		{
			if (temm > 1) temm--;   /* 下限保护，防下溢卡死 */
		}

		if (key == 3)           /* 短按确定：暂停 */
		{
			do
			{
				Inf_oled_update();
				key = Inf_key_scan();

				if (key == 4)   /* 暂停中长按：退出 */
				{
					App_video_stop_mp3();

					Inf_oled_fade_flag = 1;
					Inf_oled_gradient(0);
					return;
				}
			}
			while (key != 3);   /* 再短按：恢复播放 */
		}
		else if (key == 4)      /* 长按：退出 */
		{
			App_video_stop_mp3();

			Inf_oled_fade_flag = 1;
			Inf_oled_gradient(0);
			return;
		}

		if (datasite > mydata) /* 是否播放完成 */
		{
			App_video_stop_mp3();

			Inf_oled_clear();
			Inf_oled_show_string(0, 20, "文件播放完毕", OLED_6X8);
			Inf_oled_update();
			Drv_delay_ms(500);
			Inf_oled_fade_flag = 1;
			Inf_oled_gradient(0);
			return;
		}
	}
}

/**
  * 函   数：App_video_play_first
  * 功   能：播放第一段视频（旧工程 VIDEO_miku，顺播TF卡01文件夹01曲）
  */
void App_video_play_first(void)
{
	App_video_play(APP_VIDEO_BASE_ADDRESS, 0x0101);
}

/**
  * 函   数：App_video_play_second
  * 功   能：播放第二段视频（旧工程 VIDEO_CXUK）
  */
void App_video_play_second(void)
{
	App_video_play(App_video_next_site(APP_VIDEO_BASE_ADDRESS), 0x0102);
}

/**
  * 函   数：App_video_play_third
  * 功   能：播放第三段视频（旧工程 VIDEO_custom，地址再留4096空隙）
  */
void App_video_play_third(void)
{
	App_video_play(App_video_next_site(App_video_next_site(APP_VIDEO_BASE_ADDRESS)) + 4096, 0x0103);
}


/**
  * 函   数：App_video_write
  * 功   能：视频录制（旧工程 WRITE_VIDEO）：USART3 收帧烧录 Flash，
  *          确定键保存文件头并退出
  */
static void App_video_write(uint32_t address)
{
	int8_t key;
	uint8_t temp[3] = { 0xAA, 0, 0 };   /* 文件头(3字节) */
	uint32_t datasite = 3;              /* 帧数据偏移地址 */

	Inf_video_in_init();

	Inf_oled_write_command(0x81);	/* 设置对比度 */
	Inf_oled_write_command(0xFF);	/* 0x00~0xFF */

	Inf_oled_clear();
	Inf_oled_show_string(0, 20, "开始烧录中", OLED_6X8);
	Inf_oled_update();

	while (1)
	{
		if (Inf_video_in_fill_frame())  /* 收满一帧 */
		{
			Inf_w25q_program(address + datasite, Inf_oled_display_buf[0], 1024); /* 烧录一帧 */
			datasite += 1024;
		}

		key = Inf_key_scan();
		if (key == 3 || key == 4)   /* 确定键：保存并退出 */
		{
			if ((datasite - 3) != 0) /* 已烧录帧 */
			{
				temp[1] = ((datasite - 3) / 1024) & 0xFF;
				temp[2] = (((datasite - 3) / 1024) >> 8) & 0xFF;  /* 写入总帧数 */

				Inf_w25q_program(address, temp, 3);	/* 写入文件头帧数 */

				Inf_oled_clear();
				Inf_oled_show_string(0, 20, "文件烧录完", OLED_6X8);
				Inf_oled_update();
				Drv_delay_ms(500);
				Inf_oled_fade_flag = 1;
				Inf_oled_gradient(0);
				Inf_video_in_power_off();
				return;
			}
			else
			{
				Inf_oled_clear();
				Inf_oled_show_string(0, 20, "未检测到烧录数据流", OLED_6X8);
				Inf_oled_update();
				Drv_delay_ms(500);
				Inf_oled_fade_flag = 1;
				Inf_oled_gradient(0);
				Inf_video_in_power_off();
				return;
			}
		}
	}
}

/**
  * 函   数：App_video_write_first
  * 功   能：录制第一段（旧工程 VIDEO_write_miku，先解锁写保护）
  */
void App_video_write_first(void)
{
	Inf_w25q_write_status();     /* 取消写保护 */
	App_video_write(APP_VIDEO_BASE_ADDRESS);
}

/**
  * 函   数：App_video_write_second
  * 功   能：录制第二段（旧工程 VIDEO_write_CXUK）
  */
void App_video_write_second(void)
{
	App_video_write(App_video_next_site(APP_VIDEO_BASE_ADDRESS));
}

/**
  * 函   数：App_video_write_third
  * 功   能：录制第三段（旧工程 VIDEO_write_custom，先擦除旧数据）
  */
void App_video_write_third(void)
{
	uint8_t temp[3];      /* 文件头(3字节) */
	uint32_t data, pdata = 0;
	uint32_t mydata = 0;

	data = (App_video_next_site(App_video_next_site(APP_VIDEO_BASE_ADDRESS)) + 4096);

	Inf_w25q_read_data(data, temp, 3);

	if (temp[0] == 0xAA) /* 已有文件 */
	{
		mydata = (temp[2] << 8) | temp[1];  /* 获取总帧数 */
		mydata = (mydata + 3) / 4 + 1;      /* 需要擦除的扇区数(向上取整再+1) */

		Inf_oled_clear();
		Inf_oled_show_string(0, 20, "擦除中", OLED_6X8);
		Inf_oled_update();

		Inf_oled_write_command(0x81);	/* 设置对比度 */
		Inf_oled_write_command(0xFF);	/* 0x00~0xFF */

		while (pdata < mydata)
		{
			Inf_w25q_sector_erase(data + (pdata * 4096));
			pdata++;
		}
	}

	App_video_write(data);
}

/**
  * 函   数：App_video_erase
  * 功   能：擦除视频区（旧工程 VIDEO_erase），按扇区连续擦除，
  *          确定键停止，擦到 0xFFFFFF 自动停止
  */
void App_video_erase(void)
{
	int8_t key;
	uint32_t data = 163840;   /* 视频区起始地址 0x28000 */

	Inf_oled_fade_flag = 1;

	Inf_oled_clear();
	Inf_oled_show_string(0, 20, "擦除中", OLED_6X8);
	Inf_oled_update();

	Inf_oled_write_command(0x81);	/* 设置对比度 */
	Inf_oled_write_command(0xFF);	/* 0x00~0xFF */

	while (1)
	{
		Inf_w25q_sector_erase(data);

		Inf_oled_show_num(0, 40, data, 8, OLED_6X8);

		Inf_oled_update();

		data += 4096;

		key = Inf_key_scan();
		if (data > 0xFFFFFF) key = 1;   /* 擦完整片自动停止 */

		if (key == 3 || key == 4)       /* 确定键：停止 */
		{
			Inf_oled_show_string(0, 20, "擦除停止", OLED_6X8);
			Inf_oled_update();
			Drv_delay_ms(1000);
			Inf_oled_fade_flag = 1;
			Inf_oled_gradient(0);
			return;
		}
	}
}

/**
  * 函   数：App_video_streaming
  * 功   能：串流显示（旧工程 VIDEO_streaming）：USART3 帧数据直写显存
  *          实时显示，确定键退出
  */
void App_video_streaming(void)
{
	int8_t key;

	Inf_oled_fade_flag = 1;

	Inf_video_in_init();

	Inf_oled_clear();

	while (1)
	{
		Inf_video_in_fill_frame();  /* 帧数据直填显存 */

		Inf_oled_update();

		Inf_oled_gradient(1);

		key = Inf_key_scan();
		if (key == 3 || key == 4)   /* 确定键：退出 */
		{
			Inf_video_in_power_off();
			Inf_oled_fade_flag = 1;
			Inf_oled_gradient(0);
			return;
		}
	}
}


/**
  * 函   数：App_video_write_menu
  * 功   能：烧录子菜单（旧工程 menu_Data.c 的 VIDEO_write）
  */
static void App_video_write_menu(void)
{
	App_menu_option2_t option_list[] = {
		{"- 返回"          , APP_MENU_MODE_FUNCTION, APP_MENU_RETURN,       0, 0},
		{"- 烧录miku"      , APP_MENU_MODE_FUNCTION, App_video_write_first,  0, 0},
		{"- 烧录rabbit hole", APP_MENU_MODE_FUNCTION, App_video_write_second, 0, 0},
		{"- 烧录第三视频"  , APP_MENU_MODE_FUNCTION, App_video_write_third,  0, 0},
		{"- 擦除所有视频"  , APP_MENU_MODE_FUNCTION, App_video_erase,        0, 0},
		{".."}
	};

	App_menu_run_list(option_list);
}

/**
  * 函   数：App_video_menu
  * 功   能：视频菜单（旧工程 menu_Data.c 的 VIDEO）：三个播放入口+
  *          烧录子菜单+串流
  */
void App_video_menu(void)
{
	App_menu_option2_t option_list[] = {
		{"- 退出"        , APP_MENU_MODE_FUNCTION, APP_MENU_RETURN,      0, 0},
		{"- miku"        , APP_MENU_MODE_FUNCTION, App_video_play_first,  0, 0},
		{"- rabbit hole" , APP_MENU_MODE_FUNCTION, App_video_play_second, 0, 0},
		{"- bad apple"   , APP_MENU_MODE_FUNCTION, App_video_play_third,  0, 0},
		{"- 烧录"        , APP_MENU_MODE_FUNCTION, App_video_write_menu,  0, 0},
		{"- 串流"        , APP_MENU_MODE_FUNCTION, App_video_streaming,   0, 0},
		{".."}
	};

	App_menu_run_list(option_list);
}
