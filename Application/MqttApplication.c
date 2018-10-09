#include "main.h" 

/* Peripheral */
#include "ethernet/ethernetif.h"

#include "netif/etharp.h"
#include "lwip/dhcp.h"
#include "lwip/mem.h"
#include "lwip/memp.h"
#include "lwip/init.h"
#include "lwip/inet.h"
#include "lwip/timers.h"
#include "lwip/tcp_impl.h"
#include "lwip/ip_frag.h"
#include "lwip/tcpip.h" 
#include "lwip/sockets.h" 

#include "includes.h"
#include "string.h"

#include "MqttApplication.h"
#include "LwipApplication.h"

#include "Json/cJson.h"
#include "malloc/malloc.h"

#include "MqttClient.h"

#define MQTT_BUFFERLENTH        512

static TaskHandle_t xHandleTaskMqtt = NULL;

static void vTaskMqtt(void *pvParameters)
{
	int mysock = 0;
	uint32_t curtick,pubtick;
	uint8_t no_mqtt_msg_exchange = 1;
	int sessionPresent = 0;
	int rc = 0;

	uint8_t *buf;
	int buflen = MQTT_BUFFERLENTH,type,ret;

	fd_set readfd;
	struct timeval tv;

	xEventGroupWaitBits((EventGroupHandle_t	)EthNetStatusHandler,
						(EventBits_t		)NetConnect,
						(BaseType_t			)pdFALSE,
						(BaseType_t			)pdFALSE,
						(TickType_t			)portMAX_DELAY);

	tv.tv_sec = 5;
	tv.tv_usec = 0;

	printf ("HOST_NAME = %s\r\n", HOST_NAME);
	printf ("ClientID = %s\r\n", ClientID);
	printf ("UserName = %s\r\n", UserName);
	printf ("UserPassword = %s\r\n", UserPassword);

	buf = mymalloc(SRAMEX, MQTT_BUFFERLENTH);
MQTT_START: 
	//������������
	while(1)
	{
		//���ӷ�����
		mysock = transport_open((char*)HOST_NAME,HOST_PORT);
		//������ӷ������ɹ�
		if(mysock >= 0)
		{
			break;
		}
		//��Ϣ3��
		vTaskDelay(3000/portTICK_RATE_MS);
	}

	//��¼������

	rc = MQTTClientInit(mysock, buf, MQTT_BUFFERLENTH);
	if (rc < 0)
	{
		goto MQTT_reconnect;
	}

	//������Ϣ
	if(sessionPresent != 1)
	{
		//������Ϣ
		rc = MQTTSubscribe(mysock, SubscribeMsg, QOS2, buf, MQTT_BUFFERLENTH);
		if(rc < 0)
		{
			//����������
			goto MQTT_reconnect;	 
		}	
	}

	//��ȡ��ǰ�δ���Ϊ��������ʼʱ��
	curtick = xTaskGetTickCount();

	//����ѭ��
	printf("��ʼѭ��������Ϣ...\r\n");

	while(1)
	{
		//���������ݽ���
		no_mqtt_msg_exchange = 1;

		//������Ϣ
		FD_ZERO(&readfd);
		FD_SET(mysock,&readfd);						  

		//�ȴ��ɶ��¼�
		select(mysock+1, &readfd, NULL, NULL, &tv);

		//�ж�MQTT�������Ƿ�������
		if(FD_ISSET(mysock,&readfd) != 0)
		{
			//��ȡ���ݰ�--ע���������Ϊ0��������
			type = ReadPacketTimeout(mysock,buf,buflen,0);
			if(type != -1)
			{
				mqtt_pktype_ctl(type,buf,buflen);
				//���������ݽ���
				no_mqtt_msg_exchange = 0;
				//��ȡ��ǰ�δ���Ϊ��������ʼʱ��
				curtick = xTaskGetTickCount();
			}
		}

		//ÿ��5�������һ����Ϣ
		if((xTaskGetTickCount() - pubtick) >(5000))
		{
			pubtick = xTaskGetTickCount();

			//������Ϣ
			ret = MQTTMsgPublish(mysock, ReleaseMsg, 0, 0,(uint8_t *)"This is my first MQTT programme!",strlen("This is my first MQTT programme!"));
			if(ret >= 0)
			{
				//���������ݽ���
				no_mqtt_msg_exchange = 0;
				//��ȡ��ǰ�δ���Ϊ��������ʼʱ��
				curtick = xTaskGetTickCount();				
			}
		}

		//������ҪĿ���Ƕ�ʱ�����������PING��������
		if((xTaskGetTickCount() - curtick) >(KEEPLIVE_TIME/2*1000))
		{
			curtick = xTaskGetTickCount();
			//�ж��Ƿ������ݽ���
			if(no_mqtt_msg_exchange == 0)
			{
				//��������ݽ�������ξͲ���Ҫ����PING��Ϣ
				continue;
			}

			if(my_mqtt_send_pingreq(mysock) < 0)
			{
				goto MQTT_reconnect;
			}

			//���������ݽ���
			no_mqtt_msg_exchange = 0;
		}
	}

MQTT_reconnect:
	 //�ر�����
	 transport_close(mysock);
	 //�������ӷ�����
	 goto MQTT_START;
}


void MqttApplicationInit (void)
{
    xTaskCreate( vTaskMqtt,
				 /* ������  */
                 "vTaskMqtt",
				 /* ������    */
                 512,
				 /* ����ջ��С����λword��Ҳ����4�ֽ� */
                 NULL,
				 /* �������  */
                 configMAX_PRIORITIES - 1,
				 /* �������ȼ�*/
                 &xHandleTaskMqtt );
				 /* ������  */
}
