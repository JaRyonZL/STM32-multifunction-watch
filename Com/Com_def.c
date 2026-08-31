/*
 * @Author: JaRyon
 * @Date: 2026-08-31 15:38:49
 * @Email: jaryonzl@163.com
 */
#include "Com_def.h"

/******************************************************************************
 * 文件名称：Com_def.c
 * 说    明：通用汇编函数与通用算法实现
 *           （由旧工程 System/sys.c迁移，power/log_2缺陷修复并重命名）
 ******************************************************************************/

/**
  * 函    数：执行WFI指令（等待中断）
  * 参    数：无
  * 返 回 值：无
  * 注意事项：THUMB指令不支持基本汇编，故用内联汇编方式实现
  */
void WFI_SET(void)
{
	__ASM volatile("wfi");
}

/**
  * 函    数：关闭所有中断
  * 参    数：无
  * 返 回 值：无
  */
void INTX_DISABLE(void)
{
	__ASM volatile("cpsid i");
}

/**
  * 函    数：开启所有中断
  * 参    数：无
  * 返 回 值：无
  */
void INTX_ENABLE(void)
{
	__ASM volatile("cpsie i");
}

/**
  * 函    数：设置栈顶地址
  * 参    数：addr 栈顶地址
  * 返 回 值：无
  */
__asm void MSR_MSP(u32 addr)
{
    MSR MSP, r0			//set Main Stack value
    BX r14
}

/**
  * 函    数：x的y次方
  * 参    数：x底数
  * 参    数：y指数
  * 返 回 值：计算结果
  * 注意事项：由原sys.c的power()迁移，供Drv_gpio时钟使能计算等使用
  */
u32 Com_pow(u32 x, u32 y)
{
	u32 mul = 1;
	u32 i;

	for (i = 0; i < y; i++)
	{
		mul = mul * x;
	}

	return mul;
}

/**
  * 函    数：求以2为底的对数
  * 参    数：x输入值，须为2的整数次幂
  * 返 回 值：对数值（0~31）
  * 注意事项：原sys.c的log_2()在x不是2的整数次幂时会陷入死循环，
  *           此版本增加32次迭代上界，失败时返回0xFF
  */
u32 Com_log2(u32 x)
{
	u32 i = 0;

	while (i < 32)
	{
		if (Com_pow(2, i) == x)
		{
			return i;
		}
		i++;
	}

	return 0xFF;
}
