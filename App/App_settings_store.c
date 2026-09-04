#include "App_settings_store.h"
#include "Inf_w25q.h"
#include "Inf_mp3.h"
#include "Inf_oled.h"
#include "Inf_battery.h"
#include "App_menu.h"
#include "App_menu_data.h"
#include "App_voltage.h"
#include "Com_config.h"

/******************************************************************************
 * 文件名：App_settings_store.c（应用层）
 * 说   明：用户设置持久化（W25Q 设置区，单扇区）
 *          存储格式：[魔数0xA5][版本1B][校验和1B][设置数据 23B]
 *          策略（用户定稿）：load 校验失败静默回默认；save 先比较后写
 *          （memcmp 原始字节，相同不擦写——护磨损寿命，脏数据与默认
 *          比较必然不同而自动修复，附带免费出厂初始化）
 ******************************************************************************/

/* 设置数据布局（字段顺序保证结构体无填充，共 23 字节） */
typedef struct
{
	float    voltage_r1;          /* 电压表上臂电阻 kΩ */
	float    voltage_r2;          /* 电压表下臂电阻 kΩ */
	float    vin;                 /* ADC 基准电压 */
	uint16_t mp3_chapter_max;     /* 曲目总数 */
	uint8_t  mp3_volume;          /* 音量 */
	uint8_t  mp3_loop_play;       /* 单曲循环 */
	uint8_t  mp3_play_once;       /* 播完即停 */
	uint8_t  mp3_loudspeaker_on;  /* 扬声器 */
	uint8_t  oled_brightness;     /* 屏幕亮度 */
	uint8_t  menu_cursor;         /* 光标样式 */
	uint8_t  settings_inverse;    /* 反色显示 */
	uint8_t  settings_precharge;  /* 预充电周期 */
	uint8_t  settings_divide;     /* 显示时钟分频 */
} App_settings_data_t;

#define APP_SETTINGS_HEADER_SIZE 3   /* 魔数+版本+校验和 */

/**
  * 函   数：App_settings_store_checksum
  * 功   能：设置数据校验和（各字节求和取低 8 位）
  */
static uint8_t App_settings_store_checksum(uint8_t* data, uint8_t len)
{
	uint8_t sum = 0;
	uint8_t i;

	for (i = 0; i < len; i++)
	{
		sum += data[i];
	}
	return sum;
}

/**
  * 函   数：App_settings_store_load
  * 功   能：上电读取设置区并恢复各变量；校验失败静默保持默认值
  *          （脏数据/擦写窗口断电/首次上电全0xFF 全靠此兜底）
  */
void App_settings_store_load(void)
{
	uint8_t buf[APP_SETTINGS_HEADER_SIZE + sizeof(App_settings_data_t)];
	App_settings_data_t data;

	Inf_w25q_read_data(APP_SETTINGS_ADDR, buf, sizeof(buf));

	if (buf[0] != 0xA5) return;                                /* 魔数错误：未写过 */
	if (buf[1] != APP_SETTINGS_VERSION) return;                /* 版本不符 */
	if (buf[2] != App_settings_store_checksum(buf + APP_SETTINGS_HEADER_SIZE, sizeof(data))) return;

	memcpy(&data, buf + APP_SETTINGS_HEADER_SIZE, sizeof(data));

	/* 分发到各模块变量 */
	App_voltage_r1 = data.voltage_r1;
	App_voltage_r2 = data.voltage_r2;
	VIN = data.vin;
	Inf_mp3.chapter_max = data.mp3_chapter_max;
	Inf_mp3.volume = data.mp3_volume;
	Inf_mp3.loop_play = data.mp3_loop_play;
	Inf_mp3.play_once = data.mp3_play_once;
	Inf_mp3.loudspeaker_on = data.mp3_loudspeaker_on;
	Inf_oled_brightness = data.oled_brightness;
	App_menu_cursor = data.menu_cursor;
	App_settings_inverse = data.settings_inverse;
	App_settings_precharge = data.settings_precharge;
	App_settings_divide = data.settings_divide;
}

/**
  * 函   数：App_settings_store_save
  * 功   能：保存设置（先比较后写：数据区与Flash原数据相同则不擦写）
  */
void App_settings_store_save(void)
{
	uint8_t buf[APP_SETTINGS_HEADER_SIZE + sizeof(App_settings_data_t)];
	App_settings_data_t data;

	/* 收集各模块变量 */
	data.voltage_r1 = App_voltage_r1;
	data.voltage_r2 = App_voltage_r2;
	data.vin = VIN;
	data.mp3_chapter_max = Inf_mp3.chapter_max;
	data.mp3_volume = Inf_mp3.volume;
	data.mp3_loop_play = Inf_mp3.loop_play;
	data.mp3_play_once = Inf_mp3.play_once;
	data.mp3_loudspeaker_on = Inf_mp3.loudspeaker_on;
	data.oled_brightness = Inf_oled_brightness;
	data.menu_cursor = App_menu_cursor;
	data.settings_inverse = App_settings_inverse;
	data.settings_precharge = App_settings_precharge;
	data.settings_divide = App_settings_divide;

	/* 先比较后写 */
	Inf_w25q_read_data(APP_SETTINGS_ADDR + APP_SETTINGS_HEADER_SIZE, buf, sizeof(data));
	if (memcmp(buf, &data, sizeof(data)) == 0) return;

	/* 擦除扇区后整体写入 */
	buf[0] = 0xA5;
	buf[1] = APP_SETTINGS_VERSION;
	buf[2] = App_settings_store_checksum((uint8_t*)&data, sizeof(data));
	memcpy(buf + APP_SETTINGS_HEADER_SIZE, &data, sizeof(data));

	Inf_w25q_sector_erase(APP_SETTINGS_ADDR);
	Inf_w25q_program(APP_SETTINGS_ADDR, buf, sizeof(buf));
}
