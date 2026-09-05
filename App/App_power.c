/*
 * @Author: JaRyon
 * @Date: 2026-09-02 20:22:41
 * @Email: jaryonzl@163.com
 */
#include "App_power.h"
#include "Inf_oled.h"
#include "Drv_pwr.h"
#include "Com_board.h"
#include "Inf_key.h"
#include "Drv_tim.h"

/******************************************************************************
 * 文件名：App_power.c（应用层）
 * 说   明：关机/待机序列（迁移自旧工程 menu_Data.c 的 Power_OFF 与
 *          main.c 主循环的长按待机分支）
 ******************************************************************************/

/**
  * 函   数：App_power_off
  * 功   能：关机序列：渐变灭→断测量电源→OLED 关→
  *          进 STOP 休眠→PA0 EXTI0 唤醒→SystemInit→OLED 开→恢复测量电源
  */
void App_power_off(void)
{
	Inf_oled_fade_flag = 1;
	Inf_oled_gradient(0);		/* 渐变灭 */

	GPIO_SetBits(MEAS_PWR_PORT, MEAS_PWR_PIN);	/* 断开测量电源（1=断开） */

	Inf_oled_clear();			/* 清空显存等 */
	Inf_oled_update();			/* 刷新显示等 */

	Inf_oled_write_command(0xAE);	/* 0xAF为开显示，0xAE为关显示 */

	Inf_oled_write_command(0xAD);	/* 设置充电泵 */
	Inf_oled_write_command(0x8A);	/* 0x8B为开启充电泵，0x8A为关闭充电泵 */

	Drv_pwr_enter_stop();		/* 进入停止模式，等待 PA0 EXTI0 唤醒 */
	SystemInit();

	Inf_oled_write_command(0xAD);	/* 设置充电泵 */
	Inf_oled_write_command(0x8B);	/* 开启充电泵 */

	Inf_oled_write_command(0xAF);	/* 开显示 */

	GPIO_ResetBits(MEAS_PWR_PORT, MEAS_PWR_PIN);

	/* 等待按键松开：避免唤醒按键误触发菜单 */
	{
		uint32_t last_key_tick = Drv_tim2_get_tick();
		while (1)
		{
			if (Inf_key_scan() != 0) { last_key_tick = Drv_tim2_get_tick(); }
			if ((Drv_tim2_get_tick() - last_key_tick) > 150) { break; }   /* 150ms无事件视为松开（大于连发间隔100ms） */
		}
	}
}

/**
  * 函   数：App_power_standby
  * 功   能：待机序列（长按待机分支）：渐变灭→
  *          断测量电源→OLED 关→WKUP 使能→进 STANDBY（PA0 WKUP 复位重启）
  */
void App_power_standby(void)
{
	Inf_oled_fade_flag = 1;
	Inf_oled_gradient(0);		/* 渐变灭 */

	GPIO_SetBits(MEAS_PWR_PORT, MEAS_PWR_PIN);

	Inf_oled_clear();			/* 清空显存等 */
	Inf_oled_update();			/* 刷新显示等 */

	Inf_oled_write_command(0x8A);	/* 关闭充电泵 */

	Drv_pwr_enter_standby();	/* 使能 WKUP（PA0），进入待机，复位重启 */
}
