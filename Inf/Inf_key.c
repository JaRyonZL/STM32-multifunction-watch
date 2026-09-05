#include "Inf_key.h"
#include "Com_board.h"
#include "Drv_gpio.h"
#include "Drv_tim.h"

/******************************************************************************
 * 文件名称：Inf_key.c（接口层）
 * 说    明：按键输入与去抖事件
 *           KEY1=PC13(下)  KEY2=PA0(上)  KEY3=PA1(确定)
 ******************************************************************************/

#define KEY_DEBOUNCE 20			// 消抖阈值(ms)
#define KEY_LONG_THRESHOLD 1000 // 长按阈值(ms)

#define KEY_FIRST_CONTINUE 200   /* 首次连发间隔(ms) */
#define KEY_CONTINUE_STEP  100   /* 连发加速步进(ms) */

static uint8_t up_act = 0;
static uint16_t up_deb_cnt; // 上键单击标志与消抖计数
static uint16_t up_hold_cnt;
static uint8_t up_hold_first; // 上键连发计数与首次连发标志

static uint8_t down_act = 0;
static uint16_t down_deb_cnt; // 下键单击标志与消抖计数
static uint16_t down_hold_cnt;
static uint8_t down_hold_first; // 下键连发计数与首次连发标志

static uint8_t enter_act = 0;
static uint16_t enter_deb_cnt; // 确认单击标志与消抖计数
static uint8_t enter_long_flag = 0;
static uint16_t enter_long_cnt; // 确认长按标志与长按计数

/**
 * 函    数：Inf_key_init
 * 功    能：按键GPIO初始化（下拉输入）
 */
void Inf_key_init(void)
{
	Drv_GPIO_Init(KEY2_PORT, KEY2_PIN | KEY3_PIN, GPIO_FK_IN, GPIO_P_DOWN, GPIO_50MHz);
	Drv_GPIO_Init(KEY1_PORT, KEY1_PIN, GPIO_FK_IN, GPIO_P_DOWN, GPIO_50MHz);
}

/**
 * 函    数：Inf_key_get_state
 * 功    能：获取按键状态（1=上 2=下 3=确定）
 * 说    明：各个按键同时按下后，只执行最先检测到的按键
 */
static uint8_t Inf_key_get_state(void)
{
	uint8_t key_state = 0;

	if (GPIO_ReadInputDataBit(KEY2_PORT, KEY2_PIN) == 1)
	{
		key_state = 1;
	}
	else if (GPIO_ReadInputDataBit(KEY1_PORT, KEY1_PIN) == 1)
	{
		key_state = 2;
	}
	else if (GPIO_ReadInputDataBit(KEY3_PORT, KEY3_PIN) == 1)
	{
		key_state = 3;
	}

	return key_state;
}

/**
 * 函    数：Inf_key_up_event
 * 功    能：上键事件，单击或连发
 * 说    明：1ms周期执行
 */
int8_t Inf_key_up_event(uint8_t key_state)
{
	if (key_state == 1) /* 上键 */
	{
		// 检测到按键
		if (up_act == 0) // 实际没有触发单击
		{
			// 按键消抖
			if (++up_deb_cnt < KEY_DEBOUNCE)
			{
				return 0;
			}

			up_act = 1;
			up_deb_cnt = 0;
			up_hold_cnt = 0;
			up_hold_first = 1;
			return 1;
		}
		else
		{
			// 触发了单击时仍检测到按下 => 触发连击
			if (++up_hold_cnt >= (up_hold_first ? KEY_FIRST_CONTINUE : KEY_CONTINUE_STEP))
			{
				// 首次连发需要经过150ms，其他只需10ms
				up_hold_cnt = 0;
				up_hold_first = 0;
				return 1;
			}
		}
	}
	else
	{
		// 没检测到按键时，清除状态/计数器
		up_act = 0;
		up_deb_cnt = 0;
		up_hold_cnt = 0;
		up_hold_first = 0;
	}

	return 0;
}

/**
 * 函    数：Inf_key_up_event
 * 功    能：下键事件，单击或连发
 * 说    明：1ms周期执行
 */
int8_t Inf_key_down_event(uint8_t key_state)
{
	if (key_state == 2) /* 下键 */
	{
		// 检测到按键
		if (down_act == 0) // 实际没有触发单击
		{
			// 按键消抖
			if (++down_deb_cnt < KEY_DEBOUNCE)
			{
				return 0;
			}

			down_act = 1;
			down_deb_cnt = 0;
			down_hold_cnt = 0;
			down_hold_first = 1;
			return 1;
		}
		else
		{
			// 触发了单击时仍检测到按下 => 触发连击
			if (++down_hold_cnt >= (down_hold_first ? KEY_FIRST_CONTINUE : KEY_CONTINUE_STEP))
			{
				// 首次连发需要经过150ms，其他只需10ms
				down_hold_cnt = 0;
				down_hold_first = 0;
				return 1;
			}
		}
	}
	else
	{
		// 没检测到按键时，清除状态/计数器
		down_act = 0;
		down_deb_cnt = 0;
		down_hold_cnt = 0;
		down_hold_first = 0;
	}
	return 0;
}

/**
 * 函    数：Inf_key_enter_event
 * 功    能：确定键事件，短按=1 长按=2（计数去抖）
 * 说    明：1ms周期执行，长按返回2后须松开才能再次响应
 */
int8_t Inf_key_enter_event(uint8_t key_state)
{
	if (key_state == 3) /* 按下 */
	{
		// 检测到按键
		if (enter_act == 0) // 实际没有触发单击
		{
			// 按键消抖
			if (++enter_deb_cnt < KEY_DEBOUNCE)
			{
				return 0;
			}

			enter_act = 1;
			enter_deb_cnt = 0;
			enter_long_cnt = 0;
			enter_long_flag = 0;
		}
		else if (enter_long_flag == 0) // 若触发了单击，未触发长按
		{
			// 触发了单击时仍检测到按下 => 判定长按
			if (++enter_long_cnt >= KEY_LONG_THRESHOLD)
			{
				enter_long_flag = 1;
				return 2; // 触发长按
			}
		}
	}
	else
	{
		if (enter_act == 1 && enter_long_flag == 0)
		{
			enter_act = 0;
			enter_long_cnt = 0;
			enter_deb_cnt = 0;
			return 1; // 触发短按
		}
		enter_act = 0;
		enter_long_cnt = 0;
		enter_deb_cnt = 0;
	}
	return 0;
}

/**
  * 函    数：Inf_key_scan
  * 功    能：按键扫描，统一1ms周期调用
  * 说    明：由TIM2的1ms计数计算距上次调用的毫秒差，按差值补齐执行次数
  *           （封顶100ms），保证慢界面下按键计时依然准确；
  *           返回批量中首个事件：1=上 2=下 3=确定短按 4=确定长按
  */
int8_t Inf_key_scan(void)
{
	static uint32_t last_tick = 0;          /* 上次扫描时刻 */
	uint32_t now = Drv_tim2_get_tick();
	uint32_t elapsed = now - last_tick;     /* 距上次的毫秒数（无符号回绕安全） */
	last_tick = now;

	if (elapsed == 0) { elapsed = 1; }      /* 调用快于1ms时按1次处理 */
	if (elapsed > 100) { elapsed = 100; }   /* 封顶，防止长时间未扫描后补跑过多 */

	int8_t first_event = 0;                 /* 批量中首个事件 */

	while (elapsed--)
	{
		uint8_t state = Inf_key_get_state();

		if (Inf_key_up_event(state))
		{
			if (first_event == 0) first_event = 1;
			continue;
		}
		if (Inf_key_down_event(state))
		{
			if (first_event == 0) first_event = 2;
			continue;
		}

		uint8_t enter_state = Inf_key_enter_event(state);
		if (enter_state == 1)
		{
			if (first_event == 0) first_event = 3;
		}
		else if (enter_state == 2)
		{
			if (first_event == 0) first_event = 4;
		}
	}

	return first_event;
}
