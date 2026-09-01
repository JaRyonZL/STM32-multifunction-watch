/*
 * @Author: JaRyon
 * @Date: 2026-09-01 15:28:13
 * @Email: jaryonzl@163.com
 */
#include "Inf_battery.h"
#include "Com_board.h"
#include "Drv_gpio.h"
#include "Drv_adc.h"

/******************************************************************************
 * 文件名称：Inf_battery.c（接口层）
 * 说    明：电池电量检测，由旧工程biaopan.c电量部分与AD.c电源控制部分迁移
 ******************************************************************************/

/* 电池电压-电量对应表，线性插值用（充电上限区为255） */
static const float battery_voltage[31] = { 6.7, 4.16, 4.15, 4.14, 4.12, 4.10, 4.08, 4.05, 4.03, 3.97, 3.93, 3.90, 3.87, 3.84, 3.81, 3.79, 3.77, 3.76, 3.74, 3.73, 3.72, 3.71, 3.68, 3.66, 3.64, 3.62, 3.60, 3.55, 3.50, 3.45, 3.40 };
static const uint8_t battery_level[31] = { 255, 255, 255, 255, 255, 100, 95, 90, 85, 80, 75, 70, 65, 60, 55, 50, 48, 47, 46, 45, 44, 43, 42, 41, 40, 30, 24, 18, 12, 6, 0 };

float VIN = 3.30;                       /* ADC电压基准校准值 */
static float s_display_level = 4.2;     /* 显示电量：最小保持，仅允许上跳超过5 */

/**
  * 函    数：Inf_battery_init
  * 功    能：PB3充电检测输入（上拉）、PB5测量电源输出（默认接通）
  */
void Inf_battery_init(void)
{
	/* 禁用JTAG，释放PB3/PB4/PA15为普通IO（保留SWD） */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

	Drv_GPIO_Init(CHG_DET_PORT, CHG_DET_PIN, GPIO_FK_IN, GPIO_P_UP, GPIO_50MHz);
	Drv_GPIO_Init(MEAS_PWR_PORT, MEAS_PWR_PIN, GPIO_TW_OUT, GPIO_P_NO, GPIO_50MHz);
	Inf_battery_meas_pwr_on();
}

/**
  * 函    数：Inf_battery_meas_pwr_on
  * 功    能：接通测量电压电路电源（PB5=0），接地形成回路
  */
void Inf_battery_meas_pwr_on(void)
{
	GPIO_ResetBits(MEAS_PWR_PORT, MEAS_PWR_PIN);
}

/**
  * 函    数：Inf_battery_meas_pwr_off
  * 功    能：断开测量电压电路电源（PB5=1），切断回路
  */
void Inf_battery_meas_pwr_off(void)
{
	GPIO_SetBits(MEAS_PWR_PORT, MEAS_PWR_PIN);
}

/**
  * 函    数：Inf_battery_is_charging
  * 功    能：读取充电状态
  * 返回值：1=充电中（PB3=0） 0=未充电
  */
uint8_t Inf_battery_is_charging(void)
{
	return (GPIO_ReadInputDataBit(CHG_DET_PORT, CHG_DET_PIN) == 0) ? 1 : 0;
}

/**
  * 函    数：Inf_battery_read_voltage
  * 功    能：读取电池电压（ADC CH8采样，硬件1/2分压按系数2折算）
  */
float Inf_battery_read_voltage(void)
{
	uint16_t ADValue = Drv_adc1_get_value(ADC_CH_BATTERY);

	return ((float)ADValue * VIN) / 4095 * 2.0;
}

/**
  * 函    数：Inf_battery_get_level
  * 功    能：读取电量百分比（电压-电量查表线性插值，显示值最小保持）
  * 说    明：与旧工程一致，查表边界i=30处保留原行为
  */
uint8_t Inf_battery_get_level(void)
{
	float level = Inf_battery_read_voltage();

	/* 查表线性插值得到电量 */
	for (uint8_t i = 0; i < 31; i++)
	{
		if ((level < battery_voltage[i]) & (level >= battery_voltage[i + 1]))
		{
			level = battery_level[i + 1] + ((level - battery_voltage[i + 1]) * ((battery_level[i] - battery_level[i + 1]) / (battery_voltage[i] - battery_voltage[i + 1])));
			break;
		}
	}

	/* 显示值最小保持，仅允许上跳超过5 */
	if (level < s_display_level)
	{
		s_display_level = level;
	}
	else if (level - s_display_level > 5)
	{
		s_display_level = level;
	}

	return (uint8_t)s_display_level;
}
