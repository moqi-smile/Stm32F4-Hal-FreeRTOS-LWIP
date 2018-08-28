#include "main.h"
#include "Uart/Uart.h"

/* ˽�����Ͷ��� --------------------------------------------------------------*/
/* ˽�к궨�� ----------------------------------------------------------------*/
/* ˽�б��� ------------------------------------------------------------------*/
UART_HandleTypeDef husart[Uartnum];

/* ��չ���� ------------------------------------------------------------------*/
/* ˽�к���ԭ�� --------------------------------------------------------------*/
void HAL_UART_MspInit(UART_HandleTypeDef* huart);
void HAL_UART_MspDeInit(UART_HandleTypeDef* huart);
static void MX_NVIC_USARTx_Init(void);
/* ������ --------------------------------------------------------------------*/

int fputc(int ch,FILE *f)
{
	HAL_UART_Transmit(&husart[User_DEBUGUart], (uint8_t*)&ch, 1,1000);
	return ch;
}

void Uart_Init(void)
{
	/* ��������ʱ��ʹ�� */
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

	/* ���ô����жϲ�ʹ�ܣ���Ҫ����HAL_UART_Init������ִ���޸Ĳ���Ч */
	MX_NVIC_USARTx_Init();
}

/**
  * ��������: ����Ӳ����ʼ������
  * �������: huart�����ھ������ָ��
  * �� �� ֵ: ��
  * ˵    ��: �ú�����HAL���ڲ�����
  */
void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	if(huart->Instance==USART1)
	{
		/* ʹ�ܴ��ڹ�������GPIOʱ�� */
		__HAL_RCC_GPIOA_CLK_ENABLE();

		/* �������蹦��GPIO���� */
		GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_10;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	}
}

/**
  * ��������: ����Ӳ������ʼ������
  * �������: huart�����ھ������ָ��
  * �� �� ֵ: ��
  * ˵    ��: �ú�����HAL���ڲ�����
  */
void HAL_UART_MspDeInit(UART_HandleTypeDef* huart)
{
	if(huart->Instance==USART1)
	{
		/* ��������ʱ�ӽ��� */
		__HAL_RCC_USART1_CLK_DISABLE();

		/* �������蹦��GPIO���� */
		HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9);
		HAL_GPIO_DeInit(GPIOA, GPIO_PIN_10);

		/* �����жϽ��� */
		HAL_NVIC_DisableIRQ(USART1_IRQn);
	}
}

/**
  * ��������: NVIC����
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ��: ��
  */
static void MX_NVIC_USARTx_Init(void)
{
  /* USART1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(USART1_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(USART1_IRQn);
}

/**
  * ��������: ���ڲ�������.
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ������
  */

/******************* (C) COPYRIGHT 2015-2020 ӲʯǶ��ʽ�����Ŷ� *****END OF FILE****/
