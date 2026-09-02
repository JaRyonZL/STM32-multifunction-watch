#include "Drv_uart.h"

/******************************************************************************
 * 文件名称：Drv_uart.c（驱动层）
 * 说    明：USART1/2/3通用驱动实现（由旧工程Serial1/2/3三合一迁移）
 *           仅负责外设配置与字节收发，接收中断将字节写入环形缓冲
 ******************************************************************************/

/**
  * 函    数：串口初始化
  * 参    数：inst 串口句柄
  * 返 回 值：无
  */
void Drv_uart_init(Drv_uart_inst_t* inst)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	/* 开时钟 */
	if (inst->APBx == 1)
	{
		RCC_APB1PeriphClockCmd(inst->RCC_USARTx, ENABLE);
	}
	else
	{
		RCC_APB2PeriphClockCmd(inst->RCC_USARTx, ENABLE);
	}
	RCC_APB2PeriphClockCmd(inst->RCC_GPIOx, ENABLE);

	/* GPIO初始化 */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = inst->TxPin;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(inst->GPIOx, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = inst->RxPin;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(inst->GPIOx, &GPIO_InitStructure);

	/* USART初始化 */
	USART_InitStructure.USART_BaudRate = inst->Baud;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_Init(inst->USARTx, &USART_InitStructure);

	/* 接收中断 */
	USART_ITConfig(inst->USARTx, USART_IT_RXNE, ENABLE);

	/* NVIC配置 */
	NVIC_InitStructure.NVIC_IRQChannel = inst->IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = inst->PrePri;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = inst->SubPri;
	NVIC_Init(&NVIC_InitStructure);

	/* USART使能 */
	USART_Cmd(inst->USARTx, ENABLE);

	/* 复位接收状态 */
	inst->RxHead = 0;
	inst->RxCount = 0;
	inst->RxOverflow = 0;
}

/**
  * 函    数：串口发送一个字节
  * 参    数：inst 串口句柄
  * 参    数：Byte 要发送的一个字节
  * 返 回 值：无
  */
void Drv_uart_send_byte(Drv_uart_inst_t* inst, uint8_t Byte)
{
	USART_SendData(inst->USARTx, Byte);
	while (USART_GetFlagStatus(inst->USARTx, USART_FLAG_TXE) == RESET);
}

/**
  * 函    数：串口发送一个数组
  * 参    数：inst 串口句柄
  * 参    数：Array 数组首地址
  * 参    数：Length 数组长度
  * 返 回 值：无
  */
void Drv_uart_send_array(Drv_uart_inst_t* inst, uint8_t* Array, uint16_t Length)
{
	uint16_t i;
	for (i = 0; i < Length; i++)
	{
		Drv_uart_send_byte(inst, Array[i]);
	}
}

/**
  * 函    数：串口发送一个字符串
  * 参    数：inst 串口句柄
  * 参    数：String 字符串首地址
  * 返 回 值：无
  */
void Drv_uart_send_string(Drv_uart_inst_t* inst, char* String)
{
	uint8_t i;
	for (i = 0; String[i] != '\0'; i++)
	{
		Drv_uart_send_byte(inst, String[i]);
	}
}

/**
  * 函    数：获取接收缓冲中可读字节数
  * 参    数：inst 串口句柄
  * 返 回 值：可读字节数
  */
uint16_t Drv_uart_rx_available(Drv_uart_inst_t* inst)
{
	return inst->RxCount;
}

/**
  * 函    数：从接收缓冲读取一个字节
  * 参    数：inst 串口句柄
  * 返 回 值：读取的字节（无数据返回0）
  */
uint8_t Drv_uart_read_byte(Drv_uart_inst_t* inst)
{
	uint8_t data = 0;
	uint16_t tail;

	if (inst->RxCount)
	{
		/* 临界区：防IRQ在RxCount读-改-写期间抢占丢计数 */
		__disable_irq();
		tail = (inst->RxHead + inst->RxBufSize - inst->RxCount) % inst->RxBufSize;
		data = inst->RxBuf[tail];
		inst->RxCount--;
		__enable_irq();
	}

	return data;
}

/**
  * 函    数：串口断电（省电：引脚浮空+关闭USART）
  * 参    数：inst 串口句柄
  * 返 回 值：无
  */
void Drv_uart_power_off(Drv_uart_inst_t* inst)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = inst->TxPin | inst->RxPin;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(inst->GPIOx, &GPIO_InitStructure);

	USART_Cmd(inst->USARTx, DISABLE);
}

/**
  * 函    数：串口中断分发入口（由USARTx_IRQHandler调用）
  * 参    数：inst 串口句柄
  * 返 回 值：无
  * 说    明：接收中断将字节写入环形缓冲，缓冲满时丢数据并置溢出标志
  */
void Drv_uart_irq_handler(Drv_uart_inst_t* inst)
{
	if (USART_GetITStatus(inst->USARTx, USART_IT_RXNE) == SET)
	{
		uint8_t data = USART_ReceiveData(inst->USARTx);

		if (inst->RxCount < inst->RxBufSize)
		{
			inst->RxBuf[inst->RxHead] = data;
			inst->RxHead = (inst->RxHead + 1) % inst->RxBufSize;
			inst->RxCount++;
		}
		else
		{
			inst->RxOverflow = 1;
		}

		USART_ClearITPendingBit(inst->USARTx, USART_IT_RXNE);
	}
}


/******************************************************************************
 * 三路串口实例定义
 ******************************************************************************/

/* 各实例接收缓冲 */
static uint8_t Drv_uart1_rxbuf[DRV_UART1_RX_BUF_SIZE];
static uint8_t Drv_uart2_rxbuf[DRV_UART2_RX_BUF_SIZE];
static uint8_t Drv_uart3_rxbuf[DRV_UART3_RX_BUF_SIZE];

Drv_uart_inst_t Drv_uart1 =      /* USART1备用：PA9/PA10，9600 */
{
	USART1, 9600, GPIOA, UART1_TX_PIN, UART1_RX_PIN,
	RCC_APB2Periph_USART1, RCC_APB2Periph_GPIOA,
	USART1_IRQn, 1, 2, 2,
	Drv_uart1_rxbuf, DRV_UART1_RX_BUF_SIZE,
	0, 0, 0
};

Drv_uart_inst_t Drv_uart2 =      /* USART2 MP3模块：PA2/PA3，9600 */
{
	USART2, 9600, GPIOA, UART2_TX_PIN, UART2_RX_PIN,
	RCC_APB1Periph_USART2, RCC_APB2Periph_GPIOA,
	USART2_IRQn, 1, 1, 1,
	Drv_uart2_rxbuf, DRV_UART2_RX_BUF_SIZE,
	0, 0, 0
};

Drv_uart_inst_t Drv_uart3 =      /* USART3视频流：PB10/PB11，921600 */
{
	USART3, 921600, GPIOB, UART3_TX_PIN, UART3_RX_PIN,
	RCC_APB1Periph_USART3, RCC_APB2Periph_GPIOB,
	USART3_IRQn, 1, 2, 1,
	Drv_uart3_rxbuf, DRV_UART3_RX_BUF_SIZE,
	0, 0, 0
};

/******************************************************************************
 * 中断服务函数
 ******************************************************************************/

void USART1_IRQHandler(void)
{
	Drv_uart_irq_handler(&Drv_uart1);
}

void USART2_IRQHandler(void)
{
	Drv_uart_irq_handler(&Drv_uart2);
}

void USART3_IRQHandler(void)
{
	Drv_uart_irq_handler(&Drv_uart3);
}
