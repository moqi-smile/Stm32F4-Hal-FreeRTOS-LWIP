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

/* ˽�����Ͷ��� --------------------------------------------------------------*/
/* ˽�к궨�� ----------------------------------------------------------------*/
/* ˽�б��� ------------------------------------------------------------------*/
static TaskHandle_t xHandleTaskLED1 = NULL;

/* ��չ���� ------------------------------------------------------------------*/
/* ˽�к���ԭ�� --------------------------------------------------------------*/
static void vTaskLED1(void *pvParameters);

/**
  * ��������: LED1����
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ��: ��
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
  * ��������: ��������Ӧ��
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ��: ��
  */
void Application_Main (void)
{
	xTaskCreate( vTaskLED1,     		/* ������  */
                 "vTaskLED1",   		/* ������    */
                 512,             		/* ����ջ��С����λword��Ҳ����4�ֽ� */
                 NULL,           		/* �������  */
                 3,               		/* �������ȼ�*/
                 &xHandleTaskLED1 );  /* ������  */

	LwipApplicationInit();
	MqttApplicationInit();

	/* �������ȣ���ʼִ������ */
	vTaskStartScheduler();
}
