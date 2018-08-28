/**
  ******************************************************************************
  * 文件名程: main.c 
  * 作    者: 硬石嵌入式开发团队
  * 版    本: V1.0
  * 编写日期: 2017-3-30
  * 功    能: GPIO-流水灯
  ******************************************************************************
  * 说明：
  * 本例程配套硬石stm32开发板YS-F4Pro使用。
  * 
  * 淘宝：
  * 论坛：http://www.ing10bbs.com
  * 版权归硬石嵌入式开发团队所有，请勿商用。
  ******************************************************************************
  */
/* 包含头文件 ----------------------------------------------------------------*/
#include "main.h" 

/* Eval */
#include "Gpio/Gpio.h"
#include "Uart/Uart.h"

/* Peripheral */
#include "lcd/bsp_lcd.h"
#include "sram/sram.h"

/* Lib */
#include "malloc/malloc.h" 

/* 私有类型定义 --------------------------------------------------------------*/
/* 私有宏定义 ----------------------------------------------------------------*/
/* 私有变量 ------------------------------------------------------------------*/
/* 扩展变量 ------------------------------------------------------------------*/
/* 私有函数原形 --------------------------------------------------------------*/
void Application_Main (void);

/**
  * 函数功能: 系统时钟配置
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明: 无
  */
void SystemClock_Config(void)
{
	RCC_OscInitTypeDef RCC_OscInitStruct;
	RCC_ClkInitTypeDef RCC_ClkInitStruct;

	__HAL_RCC_PWR_CLK_ENABLE();
	//使能PWR时钟

	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
	//设置调压器输出电压级别1

	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	// 外部晶振，8MHz
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	//打开HSE 
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	//打开PLL
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	//PLL时钟源选择HSE
	RCC_OscInitStruct.PLL.PLLM = 8;
	//8分频MHz
	RCC_OscInitStruct.PLL.PLLN = 336;
	//336倍频
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	//2分频，得到168MHz主时钟
	RCC_OscInitStruct.PLL.PLLQ = 7;
	//USB/SDIO/随机数产生器等的主PLL分频系数
	HAL_RCC_OscConfig(&RCC_OscInitStruct);

	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
							  |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	// 系统时钟：168MHz
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	// AHB时钟： 168MHz
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	// APB1时钟：42MHz
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
	// APB2时钟：84MHz
	HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);

	HAL_RCC_EnableCSS();
	// 使能CSS功能，优先使用外部晶振，内部时钟源为备用

	// HAL_RCC_GetHCLKFreq()/1000    1ms中断一次
	// HAL_RCC_GetHCLKFreq()/100000	 10us中断一次
	// HAL_RCC_GetHCLKFreq()/1000000 1us中断一次
	HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);                // 配置并启动系统滴答定时器
	/* 系统滴答定时器时钟源 */
	HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

	/* 系统滴答定时器中断优先级配置 */
	HAL_NVIC_SetPriority(SysTick_IRQn, 5, 0);
}

/**
  * 函数功能: 主函数.
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明: 无
  */
int main(void)
{
	/* 复位所有外设，初始化Flash接口和系统滴答定时器 */
	HAL_Init();
	/* 配置系统时钟 */
	SystemClock_Config();

	__HAL_RCC_GPIOH_CLK_ENABLE();
	__HAL_RCC_GPIOG_CLK_ENABLE();
	__HAL_RCC_GPIOE_CLK_ENABLE();

	Uart_Init();
	Gpio_Init();
	SRAM_Init();
	//初始化外部SRAM  
	
	my_mem_init(SRAMIN);
	//初始化内部内存池
	my_mem_init(SRAMEX);
	//初始化外部内存池
	my_mem_init(SRAMCCM);
	//初始化CCM内存池
	

	/* 创建任务 */
	Application_Main();
}

__weak void Application_Main (void)
{
	
}
