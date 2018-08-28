#include "main.h"
#include "Uart/Uart.h"

/* 私有类型定义 --------------------------------------------------------------*/
/* 私有宏定义 ----------------------------------------------------------------*/
/* 私有变量 ------------------------------------------------------------------*/
UART_HandleTypeDef husart[Uartnum];

/* 扩展变量 ------------------------------------------------------------------*/
/* 私有函数原形 --------------------------------------------------------------*/
void HAL_UART_MspInit(UART_HandleTypeDef* huart);
void HAL_UART_MspDeInit(UART_HandleTypeDef* huart);
static void MX_NVIC_USARTx_Init(void);
/* 函数体 --------------------------------------------------------------------*/

int fputc(int ch,FILE *f)
{
	HAL_UART_Transmit(&husart[User_DEBUGUart], (uint8_t*)&ch, 1,1000);
	return ch;
}

void Uart_Init(void)
{
	/* 串口外设时钟使能 */
	__HAL_RCC_USART1_CLK_ENABLE();

	husart[Uart1].Instance = USART1;
	husart[Uart1].Init.Mode = UART_MODE_TX_RX;
	husart[Uart1].Init.Parity = UART_PARITY_NONE;
	husart[Uart1].Init.StopBits = UART_STOPBITS_1;
	husart[Uart1].Init.BaudRate = 115200;
	husart[Uart1].Init.WordLength = UART_WORDLENGTH_8B;
	husart[Uart1].Init.HwFlowCtl = UART_HWCONTROL_NONE;
	husart[Uart1].Init.OverSampling = UART_OVERSAMPLING_16;
	HAL_UART_Init(&husart[Uart1]);

	/* 配置串口中断并使能，需要放在HAL_UART_Init函数后执行修改才有效 */
	MX_NVIC_USARTx_Init();
}

/**
  * 函数功能: 串口硬件初始化配置
  * 输入参数: huart：串口句柄类型指针
  * 返 回 值: 无
  * 说    明: 该函数被HAL库内部调用
  */
void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	if(huart->Instance==USART1)
	{
		/* 使能串口功能引脚GPIO时钟 */
		__HAL_RCC_GPIOA_CLK_ENABLE();

		/* 串口外设功能GPIO配置 */
		GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_10;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	}
}

/**
  * 函数功能: 串口硬件反初始化配置
  * 输入参数: huart：串口句柄类型指针
  * 返 回 值: 无
  * 说    明: 该函数被HAL库内部调用
  */
void HAL_UART_MspDeInit(UART_HandleTypeDef* huart)
{
	if(huart->Instance==USART1)
	{
		/* 串口外设时钟禁用 */
		__HAL_RCC_USART1_CLK_DISABLE();

		/* 串口外设功能GPIO配置 */
		HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9);
		HAL_GPIO_DeInit(GPIOA, GPIO_PIN_10);

		/* 串口中断禁用 */
		HAL_NVIC_DisableIRQ(USART1_IRQn);
	}
}

/**
  * 函数功能: NVIC配置
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明: 无
  */
static void MX_NVIC_USARTx_Init(void)
{
  /* USART1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(USART1_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(USART1_IRQn);
}

/**
  * 函数功能: 串口参数配置.
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明：无
  */

/******************* (C) COPYRIGHT 2015-2020 硬石嵌入式开发团队 *****END OF FILE****/
