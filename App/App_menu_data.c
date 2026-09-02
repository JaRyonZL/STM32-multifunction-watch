#include "App_menu_data.h"
#include "App_menu.h"
#include "Inf_oled.h"
#include "Inf_oled_gfx.h"
#include "Inf_battery.h"
#include "Inf_key.h"
#include "Drv_adc.h"
#include "Drv_delay.h"
#include "Inf_w25q.h"
#include "Com_oled_res.h"

/******************************************************************************
 * 文件名：App_menu_data.c（应用层）
 * 说   明：设置/信息类应用（迁移自旧工程 menu_Data.c 的设置部分）
 *          设置重启后不保存（RAM 变量，旧固件行为）
 ******************************************************************************/

/* 设置状态 */
static uint8_t App_settings_inverse = 0;       /* 反色显示开关 */
static uint8_t App_settings_precharge = 0xF1;  /* 预充电周期 0xD9 */
static uint8_t App_settings_divide = 0xFE;     /* 显示时钟分频 0xD5 */

/* 静态函数声明 */
static void App_settings_cursor_run(void);
static void App_settings_cursor_arrow(void);
static void App_settings_cursor_block(void);
static void App_settings_cursor_pointer(void);
static void App_settings_brightness_apply(void);
static void App_settings_inverse_apply(void);
static void App_settings_adjust_run(uint8_t* value, uint8_t cmd);
static void App_settings_precharge_run(void);
static void App_settings_divide_run(void);
static void App_settings_information(void);
static void App_settings_voltage_run(void);

/**
  * 函   数：App_settings_run
  * 功   能：设置菜单（旧 System_settings 六项）
  */
void App_settings_run(void)
{
	App_menu_option2_t option_list[] = {
		{"- 返回"          , APP_MENU_MODE_FUNCTION, APP_MENU_RETURN,             0, 0},
		{"- 屏幕亮度"      , APP_MENU_MODE_NUMBER,   App_settings_brightness_apply, &Inf_oled_brightness, 0},
		{"- 光标样式"      , APP_MENU_MODE_FUNCTION, App_settings_cursor_run,     0, 0},
		{"- 反色显示"      , APP_MENU_MODE_ON_OFF,   App_settings_inverse_apply,   &App_settings_inverse, 0},
		{"- 信息"          , APP_MENU_MODE_FUNCTION, App_settings_information,     0, 0},
		{"- 预充电周期"    , APP_MENU_MODE_FUNCTION, App_settings_precharge_run,  0, 0},
		{"- 显示时钟分频比", APP_MENU_MODE_FUNCTION, App_settings_divide_run,     0, 0},
		{".."}
	};

	App_menu_run_list(option_list);
}

/**
  * 函   数：App_settings_cursor_run
  * 功   能：光标样式子菜单（Cursor 为死设置）
  */
static void App_settings_cursor_run(void)
{
	App_menu_option2_t option_list[] = {
		{"- 返回", APP_MENU_MODE_FUNCTION, APP_MENU_RETURN,           0, 0},
		{"- 反色", APP_MENU_MODE_FUNCTION, App_settings_cursor_arrow,  0, 0},
		{"- 矩形", APP_MENU_MODE_FUNCTION, App_settings_cursor_block,  0, 0},
		{"- 指针", APP_MENU_MODE_FUNCTION, App_settings_cursor_pointer, 0, 0},
		{".."}
	};

	App_menu_run_list(option_list);
}

static void App_settings_cursor_arrow(void) { App_menu_cursor = 0; }
static void App_settings_cursor_block(void) { App_menu_cursor = 1; }
static void App_settings_cursor_pointer(void) { App_menu_cursor = 2; }

/**
  * 函   数：App_settings_brightness_apply
  * 功   能：亮度立即写入 OLED
  */
static void App_settings_brightness_apply(void)
{
	Inf_oled_write_command(0x81);	/* 设置对比度 */
	Inf_oled_write_command(Inf_oled_brightness);	/* 0x00~0xFF */
}

/**
  * 函   数：App_settings_inverse_apply
  * 功   能：反色显示切换
  */
static void App_settings_inverse_apply(void)
{
	if (App_settings_inverse)
	{
		Inf_oled_write_command(0xA7);   /* 反色显示 */
	}
	else
	{
		Inf_oled_write_command(0xA6);   /* 正常显示 */
	}
}

/**
  * 函   数：App_settings_adjust_run
  * 功   能：十六进制调节循环：
  *          上下键±1并立即写入 OLED 命令，确定键退出
  */
static void App_settings_adjust_run(uint8_t* value, uint8_t cmd)
{
	int8_t key;

	Inf_oled_fade_flag = 1;

	while (1)
	{
		Inf_oled_show_hex_num(100, 20, *value, 4, OLED_6X8);
		Inf_oled_update();

		Inf_oled_gradient(1);

		key = Inf_key_scan();
		if (key == 1)
		{
			(*value)++;
			Inf_oled_write_command(cmd);
			Inf_oled_write_command(*value);
		}
		else if (key == 2)
		{
			(*value)--;
			Inf_oled_write_command(cmd);
			Inf_oled_write_command(*value);
		}

		if (key == 3 || key == 4)
		{
			Inf_oled_fade_flag = 1;
			Inf_oled_gradient(0);
			return;
		}
	}
}

/**
  * 函   数：App_settings_precharge_run
  * 功   能：预充电周期调节
  */
static void App_settings_precharge_run(void)
{
	App_settings_adjust_run(&App_settings_precharge, 0xD9);
}

/**
  * 函   数：App_settings_divide_run
  * 功   能：显示时钟分频调节
  */
static void App_settings_divide_run(void)
{
	App_settings_adjust_run(&App_settings_divide, 0xD5);
}

/**
  * 函   数：App_settings_information
  * 功   能：信息页（旧 System_information，固件版本号更新为 V2.0）
  */
static void App_settings_information(void)
{
	App_menu_option2_t option_list[] = {
		{"- 返回"              , APP_MENU_MODE_FUNCTION, APP_MENU_RETURN,        0, 0},
		{"-----STM32C8T6-----" , APP_MENU_MODE_DISPLAY,  0, 0, 0},
		{"- RAM; 20K"          , APP_MENU_MODE_DISPLAY,  0, 0, 0},
		{"- FLASH: 64K"        , APP_MENU_MODE_DISPLAY,  0, 0, 0},
		{"------W25Q128------"  , APP_MENU_MODE_DISPLAY,  0, 0, 0},
		{"- 储存:16MB"          , APP_MENU_MODE_DISPLAY,  0, 0, 0},
		{"-------------------" , APP_MENU_MODE_DISPLAY,  0, 0, 0},
		{"- 固件版本V2.0 改版", APP_MENU_MODE_DISPLAY,  0, 0, 0},
		{"- BiliBili 时光久泽" , APP_MENU_MODE_DISPLAY,  0, 0, 0},
		{"- 系统电压"          , APP_MENU_MODE_FUNCTION, App_settings_voltage_run, 0, 0},
		{".."}
	};

	App_menu_run_list(option_list);
}

/**
  * 函   数：App_settings_voltage_run
  * 功   能：系统电压校准页：显示 VIN 基准、
  *          CH8/CH17 采样与换算电压，上下键微调 VIN(±0.01)，确定键退出
  */
static void App_settings_voltage_run(void)
{
	int8_t key;
	uint16_t ADValue, ADrefint;
	float voltage;

	Inf_oled_fade_flag = 1;

	while (1)
	{
		ADValue = Drv_adc1_get_value(ADC_CH_BATTERY);
		ADrefint = Drv_adc1_get_value(ADC_CH_VREFINT);

		Inf_oled_clear();

		Inf_oled_show_float_num(5, 2, VIN, 2, 3, OLED_6X8);
		Inf_oled_show_num(5, 15, ADrefint, 4, OLED_6X8);
		Inf_oled_show_num(5, 25, ADValue, 4, OLED_6X8);

		voltage = ((float)ADValue * VIN) / 4095 * 2;
		Inf_oled_show_float_num(64, 25, voltage, 2, 2, OLED_6X8);

		voltage = ((float)ADrefint * VIN) / 4095;   /* 得到电压 */
		Inf_oled_show_float_num(64, 35, voltage, 2, 2, OLED_6X8);

		Inf_oled_update();
		Inf_oled_gradient(1);

		key = Inf_key_scan();
		if (key == 1)
		{
			VIN = VIN + 0.01;
		}
		else if (key == 2)
		{
			VIN = VIN - 0.01;
		}

		if (key == 3 || key == 4)
		{
			Inf_oled_fade_flag = 1;
			Inf_oled_gradient(0);
			return;
		}
	}
}


/**
  * 函   数：App_error
  * 功   能：占位界面：提示待开发，任意键退出
  */
void App_error(void)
{
	int8_t key;

	Inf_oled_fade_flag = 1;

	Inf_oled_clear();
	Inf_oled_show_string(39, 16, "待开发中", OLED_6X8);
	Inf_oled_show_string(0, 52, "当然你也可以自己来写", OLED_6X8);
	Inf_oled_update();

	while (1)
	{
		Inf_oled_gradient(1);

		key = Inf_key_scan();
		if (key != 0)   /* 任意键退出 */
		{
			Inf_oled_fade_flag = 1;
			Inf_oled_gradient(0);
			return;
		}
	}
}

/**
  * 函   数：App_flashlight
  * 功   能：手电筒：全屏白 + 渐亮完成，任意键退出
  */
void App_flashlight(void)
{
	int8_t key;

	Inf_oled_fade_flag = 1;

	Inf_oled_clear();	/* 清屏 */
	Inf_oled_draw_rectangle(0, 0, 128, 64, OLED_FILLED);/* 全屏白 */
	Inf_oled_update();	/* 刷新屏幕 */

	while (1)
	{
		while (Inf_oled_fade_flag) { Inf_oled_update(); Inf_oled_gradient(1); }   /* 渐亮直到完成 */

		Inf_oled_write_command(0x81);	/* 设置对比度 */
		Inf_oled_write_command(0xFF);	/* 0x00~0xFF */

		key = Inf_key_scan();
		if (key != 0)
		{
			Inf_oled_fade_flag = 1;
			Inf_oled_gradient(0);
			return;
		}
	}
}


/**
  * 函   数：App_qrcode_show
  * 功   能：收款码占位显示，确定键退出
  * 参   数：image 收款码图片数据
  *         type 收款码类型：1-微信，2-支付宝
  */
static void App_qrcode_show(const uint8_t* image, uint8_t type)
{
	int8_t key;

	Inf_oled_fade_flag = 1;

	Inf_oled_clear();
	if(type == 1)
	{
		Inf_oled_show_image(41, 7, 45, 48, image);
	}
	else if(type == 2)
	{
		Inf_oled_show_image(43, 11, 41, 41, image);
	}

	Inf_oled_update();

	while (1)
	{
		Inf_oled_gradient(1);

		key = Inf_key_scan();
		if (key == 3 || key == 4)   /* 确定键：退出 */
		{
			Inf_oled_fade_flag = 1;
			Inf_oled_gradient(0);
			return;
		}
	}
}

/**
  * 函   数：App_qrcode_wechat
  * 功   能：微信收款码（占位图）
  */
void App_qrcode_wechat(void) { App_qrcode_show(qrcode_wechat_img, 1); }

/**
  * 函   数：App_qrcode_zfb
  * 功   能：支付宝收款码（占位图）
  */
void App_qrcode_zfb(void) { App_qrcode_show(qrcode_zfb_img, 2); }


/**
  * 函   数：App_calc_cos
  * 功   能：cos 计算器（旧 cosjiaod）：cos 值与 asin/acos 换算角度显示，
  *          上下键±0.01（短按确定切 ±0.01 步进），长按退出
  */
void App_calc_cos(void)
{
	int8_t key, i = 0;
	float cos1 = 0, tpp, tpp1;

	Inf_oled_fade_flag = 1;

	Inf_oled_clear();

	while (1)
	{
		tpp = asin(cos1);
		tpp1 = acos(cos1);

		Inf_oled_show_float_num(10, 10, cos1, 1, 3, OLED_6X8);
		Inf_oled_show_float_num(10, 30, tpp / 3.141592 * 180, 2, 9, OLED_6X8);
		Inf_oled_show_float_num(10, 40, tpp1 / 3.141592 * 180, 2, 9, OLED_6X8);
		Inf_oled_update();

		Inf_oled_gradient(1);

		key = Inf_key_scan();
		if (key == 1)
		{
			if (i) cos1 += 0.01;
			else cos1 += 0.01;
		}
		else if (key == 2)
		{
			if (i) cos1 -= 0.01;
			else cos1 -= 0.01;
		}

		if (key == 3) { i = !i; }
		else if (key == 4)
		{
			Inf_oled_fade_flag = 1;
			Inf_oled_gradient(0);
			return;
		}
	}
}



/**
  * 函   数：App_font_browser
  * 功   能：字库浏览：4×10 网格滚动显示 12x12 汉字，
  *          上下键调滚动速度(dld±10)，确定键退出
  */
void App_font_browser(void)
{
	int8_t key;
	uint8_t SChinese[24];
	uint32_t Addr_offset = 0;   /* 汉字的偏移地址 */
	uint16_t dld = 0;
	uint8_t i, o;

	Inf_oled_fade_flag = 1;

	Inf_oled_clear();

	while (1)
	{
		Inf_oled_clear();
		for (i = 0; i < 4; i++)
		{
			for (o = 0; o < 10; o++)
			{
				Inf_w25q_read_data(Addr_offset, SChinese, 24);
				Inf_oled_show_image(o * 12, i * 12, 12, 12, SChinese);
				Inf_oled_update_area(o * 12, i * 12, 12, 12);
				Addr_offset += 24;
				Inf_oled_show_hex_num(0, 48, Addr_offset, 6, OLED_6X8);
				Inf_oled_show_num(0, 56, Addr_offset, 8, OLED_6X8);
				Inf_oled_update_area(0, 48, 48, 16);
				Drv_delay_ms(dld);
			}
		}
		Inf_oled_gradient(1);

		key = Inf_key_scan();
		if (key == 1)
		{
			dld += 10;
		}
		else if (key == 2)
		{
			if (dld) dld -= 10;
		}

		if (key == 3 || key == 4)
		{
			Inf_oled_fade_flag = 1;
			Inf_oled_gradient(0);
			return;
		}
	}
}
