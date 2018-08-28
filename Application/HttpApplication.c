#include "main.h" 

/* 包含头文件 ----------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "lwip/debug.h"
#include "http/httpd.h"
#include "lwip/tcp.h"
#include "http/fs.h"
#include <string.h>
#include <stdlib.h>

#include "includes.h"
#include "string.h"

#include "HttpApplication.h"
#include "LwipApplication.h"

#include "Json/cJson.h"
#include "malloc/malloc.h"

#include "Gpio/Gpio.h"

#define NUM_CONFIG_CGI_URIS	(sizeof(ppcURLs) / sizeof(tCGI))
#define NUM_CONFIG_SSI_TAGS	(sizeof(ppcTAGs) / sizeof(char *))

/* CGI handler for LED control */ 
const char * LEDS_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);

static TaskHandle_t xHandleTaskHttp = NULL;

static const char *ppcTAGs[]=  //SSI的Tag
{
	"t", //ADC值
	"w", //温度值
	"h", //时间
	"y"  //日期
};

static const tCGI ppcURLs[]= //cgi程序
{
	{"/leds.cgi",LEDS_CGI_Handler},
};

//当web客户端请求浏览器的时候,使用此函数被CGI handler调用
static int FindCGIParameter(const char *pcToFind,char *pcParam[],int iNumParams)
{
	int iLoop;
	for(iLoop = 0;iLoop < iNumParams;iLoop ++ )
	{
		if(strcmp(pcToFind, pcParam[iLoop]) == 0)
		{
			return (iLoop); //返回iLOOP
		}
	}
	return (-1);
}

u16_t SSIHandler(int iIndex, char *pcInsert, int iInsertLen)
{

	return 0;
}

const char * LEDS_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{

	uint8_t i=0;
	//注意根据自己的GET的参数的多少来选择i值范围

	iIndex = FindCGIParameter("LED1",pcParam,iNumParams);
	//找到led的索引号

	if (iIndex != -1)
	{
		
		//关闭所有的LED1灯
		for (i=0; i<iNumParams; i++)
		// 检查CGI参数: example GET /leds.cgi?led=2&led=4 */
		{
			if (strcmp(pcParam[i] , "LED1")==0)
			//检查参数"led"
			{
				//改变LED1状态
				if(strcmp(pcValue[i], "LED1ON") ==0)
					GpioPinSet(LED2);
				//打开LED1
				else if(strcmp(pcValue[i],"LED1OFF") == 0)
					GpioPinRes(LED2);
					//关闭LED1
			}
		}
	}
	return "/LEDControl.shtml";  
}

void httpd_ssi_init(void)
{  
	http_set_ssi_handler(SSIHandler, ppcTAGs, NUM_CONFIG_SSI_TAGS);
}

void httpd_cgi_init(void)
{
	http_set_cgi_handlers(ppcURLs, NUM_CONFIG_CGI_URIS);
}

static void vTaskHttp(void *pvParameters)
{
	xEventGroupWaitBits((EventGroupHandle_t	)EthNetStatusHandler,
						(EventBits_t		)NetConnect,
						(BaseType_t			)pdFALSE,
						(BaseType_t			)pdFALSE,
						(TickType_t			)portMAX_DELAY);

	httpd_init();

	vTaskDelete(xHandleTaskHttp);
}


void HttpApplicationInit (void)
{
    xTaskCreate( vTaskHttp,
				 /* 任务函数  */
                 "vTaskHttp",
				 /* 任务名    */
                 128,
				 /* 任务栈大小，单位word，也就是4字节 */
                 NULL,
				 /* 任务参数  */
                 3,
				 /* 任务优先级*/
                 &xHandleTaskHttp );
				 /* 任务句柄  */
}
