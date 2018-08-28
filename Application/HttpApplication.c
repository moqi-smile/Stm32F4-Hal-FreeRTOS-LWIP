#include "main.h" 

/* ����ͷ�ļ� ----------------------------------------------------------------*/
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

static const char *ppcTAGs[]=  //SSI��Tag
{
	"t", //ADCֵ
	"w", //�¶�ֵ
	"h", //ʱ��
	"y"  //����
};

static const tCGI ppcURLs[]= //cgi����
{
	{"/leds.cgi",LEDS_CGI_Handler},
};

//��web�ͻ��������������ʱ��,ʹ�ô˺�����CGI handler����
static int FindCGIParameter(const char *pcToFind,char *pcParam[],int iNumParams)
{
	int iLoop;
	for(iLoop = 0;iLoop < iNumParams;iLoop ++ )
	{
		if(strcmp(pcToFind, pcParam[iLoop]) == 0)
		{
			return (iLoop); //����iLOOP
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
	//ע������Լ���GET�Ĳ����Ķ�����ѡ��iֵ��Χ

	iIndex = FindCGIParameter("LED1",pcParam,iNumParams);
	//�ҵ�led��������

	if (iIndex != -1)
	{
		
		//�ر����е�LED1��
		for (i=0; i<iNumParams; i++)
		// ���CGI����: example GET /leds.cgi?led=2&led=4 */
		{
			if (strcmp(pcParam[i] , "LED1")==0)
			//������"led"
			{
				//�ı�LED1״̬
				if(strcmp(pcValue[i], "LED1ON") ==0)
					GpioPinSet(LED2);
				//��LED1
				else if(strcmp(pcValue[i],"LED1OFF") == 0)
					GpioPinRes(LED2);
					//�ر�LED1
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
				 /* ������  */
                 "vTaskHttp",
				 /* ������    */
                 128,
				 /* ����ջ��С����λword��Ҳ����4�ֽ� */
                 NULL,
				 /* �������  */
                 3,
				 /* �������ȼ�*/
                 &xHandleTaskHttp );
				 /* ������  */
}
