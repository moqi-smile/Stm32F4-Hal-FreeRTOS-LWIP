/**
  ******************************************************************************
  * �ļ�����: main.c 
  * ��    ��: ӲʯǶ��ʽ�����Ŷ�
  * ��    ��: V1.0
  * ��д����: 2017-3-30
  * ��    ��: GPIO-��ˮ��
  ******************************************************************************
  * ˵����
  * ����������Ӳʯstm32������YS-F4Proʹ�á�
  * 
  * �Ա���
  * ��̳��http://www.ing10bbs.com
  * ��Ȩ��ӲʯǶ��ʽ�����Ŷ����У��������á�
  ******************************************************************************
  */
/* ����ͷ�ļ� ----------------------------------------------------------------*/
#include "main.h" 

/* Eval */
#include "Gpio/Gpio.h"
#include "Uart/Uart.h"

/* Peripheral */
#include "lcd/bsp_lcd.h"
#include "sram/sram.h"

/* Lib */
#include "malloc/malloc.h" 

/* ˽�����Ͷ��� --------------------------------------------------------------*/
/* ˽�к궨�� ----------------------------------------------------------------*/
/* ˽�б��� ------------------------------------------------------------------*/
/* ��չ���� ------------------------------------------------------------------*/
/* ˽�к���ԭ�� --------------------------------------------------------------*/
void Application_Main (void);

/**
  * ��������: ϵͳʱ������
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ��: ��
  */
void SystemClock_Config(void)
{
	RCC_OscInitTypeDef RCC_OscInitStruct;
	RCC_ClkInitTypeDef RCC_ClkInitStruct;

	__HAL_RCC_PWR_CLK_ENABLE();
	//ʹ��PWRʱ��

	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
	//���õ�ѹ�������ѹ����1

	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	// �ⲿ����8MHz
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	//��HSE 
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	//��PLL
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	//PLLʱ��Դѡ��HSE
	RCC_OscInitStruct.PLL.PLLM = 8;
	//8��ƵMHz
	RCC_OscInitStruct.PLL.PLLN = 336;
	//336��Ƶ
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	//2��Ƶ���õ�168MHz��ʱ��
	RCC_OscInitStruct.PLL.PLLQ = 7;
	//USB/SDIO/������������ȵ���PLL��Ƶϵ��
	HAL_RCC_OscConfig(&RCC_OscInitStruct);

	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
							  |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	// ϵͳʱ�ӣ�168MHz
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	// AHBʱ�ӣ� 168MHz
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	// APB1ʱ�ӣ�42MHz
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
	// APB2ʱ�ӣ�84MHz
	HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);

	HAL_RCC_EnableCSS();
	// ʹ��CSS���ܣ�����ʹ���ⲿ�����ڲ�ʱ��ԴΪ����

	// HAL_RCC_GetHCLKFreq()/1000    1ms�ж�һ��
	// HAL_RCC_GetHCLKFreq()/100000	 10us�ж�һ��
	// HAL_RCC_GetHCLKFreq()/1000000 1us�ж�һ��
	HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);                // ���ò�����ϵͳ�δ�ʱ��
	/* ϵͳ�δ�ʱ��ʱ��Դ */
	HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

	/* ϵͳ�δ�ʱ���ж����ȼ����� */
	HAL_NVIC_SetPriority(SysTick_IRQn, 5, 0);
}

/**
  * ��������: ������.
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ��: ��
  */
int main(void)
{
	/* ��λ�������裬��ʼ��Flash�ӿں�ϵͳ�δ�ʱ�� */
	HAL_Init();
	/* ����ϵͳʱ�� */
	SystemClock_Config();

	__HAL_RCC_GPIOH_CLK_ENABLE();
	__HAL_RCC_GPIOG_CLK_ENABLE();
	__HAL_RCC_GPIOE_CLK_ENABLE();

	Uart_Init();
	Gpio_Init();
	SRAM_Init();
	//��ʼ���ⲿSRAM  
	
	my_mem_init(SRAMIN);
	//��ʼ���ڲ��ڴ��
	my_mem_init(SRAMEX);
	//��ʼ���ⲿ�ڴ��
	my_mem_init(SRAMCCM);
	//��ʼ��CCM�ڴ��
	

	/* �������� */
	Application_Main();
}

__weak void Application_Main (void)
{
	
}
