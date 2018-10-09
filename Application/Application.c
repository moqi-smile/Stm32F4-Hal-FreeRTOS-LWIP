#include "main.h" 

/* Eval */
#include "Gpio/Gpio.h"
#include "Uart/Uart.h"

/* Peripheral */
#include "lcd/bsp_lcd.h"

/* Lib */
#include "malloc/malloc.h"
#include "Json/cJson.h"

#include "includes.h"

#include "LwipApplication.h"
#include "MqttApplication.h"
#include "HttpApplication.h"

/* 私有类型定义 --------------------------------------------------------------*/
/* 私有宏定义 ----------------------------------------------------------------*/
/* 私有变量 ------------------------------------------------------------------*/
static TaskHandle_t xHandleTaskLED1 = NULL;

/* 扩展变量 ------------------------------------------------------------------*/
/* 私有函数原形 --------------------------------------------------------------*/
static void vTaskLED1(void *pvParameters);

/**
  * 函数功能: LED1任务
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明: 无
  */
static void vTaskLED1(void *pvParameters)
{
	while(1)
	{
		GpioPinToggle(LED1);
		vTaskDelay(1000);
	}
}

/**
  * 函数功能: 创建任务应用
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明: 无
  */
void Application_Main (void)
{
	xTaskCreate( vTaskLED1,     		/* 任务函数  */
                 "vTaskLED1",   		/* 任务名    */
                 512,             		/* 任务栈大小，单位word，也就是4字节 */
                 NULL,           		/* 任务参数  */
                 3,               		/* 任务优先级*/
                 &xHandleTaskLED1 );  /* 任务句柄  */

	LwipApplicationInit();
	MqttApplicationInit();

	/* 启动调度，开始执行任务 */
	vTaskStartScheduler();
}
