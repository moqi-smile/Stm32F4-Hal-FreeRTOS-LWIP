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
	//创建网络连接
	while(1)
	{
		//连接服务器
		mysock = transport_open((char*)HOST_NAME,HOST_PORT);
		//如果连接服务器成功
		if(mysock >= 0)
		{
			break;
		}
		//休息3秒
		vTaskDelay(3000/portTICK_RATE_MS);
	}

	//登录服务器

	rc = MQTTClientInit(mysock, buf, MQTT_BUFFERLENTH);
	if (rc < 0)
	{
		goto MQTT_reconnect;
	}

	//订阅消息
	if(sessionPresent != 1)
	{
		//订阅消息
		rc = MQTTSubscribe(mysock, SubscribeMsg, QOS2, buf, MQTT_BUFFERLENTH);
		if(rc < 0)
		{
			//重连服务器
			goto MQTT_reconnect;	 
		}	
	}

	//获取当前滴答，作为心跳包起始时间
	curtick = xTaskGetTickCount();

	//无线循环
	printf("开始循环接收消息...\r\n");

	while(1)
	{
		//表明无数据交换
		no_mqtt_msg_exchange = 1;

		//推送消息
		FD_ZERO(&readfd);
		FD_SET(mysock,&readfd);						  

		//等待可读事件
		select(mysock+1, &readfd, NULL, NULL, &tv);

		//判断MQTT服务器是否有数据
		if(FD_ISSET(mysock,&readfd) != 0)
		{
			//读取数据包--注意这里参数为0，不阻塞
			type = ReadPacketTimeout(mysock,buf,buflen,0);
			if(type != -1)
			{
				mqtt_pktype_ctl(type,buf,buflen);
				//表明有数据交换
				no_mqtt_msg_exchange = 0;
				//获取当前滴答，作为心跳包起始时间
				curtick = xTaskGetTickCount();
			}
		}

		//每隔5秒就推送一次消息
		if((xTaskGetTickCount() - pubtick) >(5000))
		{
			pubtick = xTaskGetTickCount();

			//发布消息
			ret = MQTTMsgPublish(mysock, ReleaseMsg, 0, 0,(uint8_t *)"This is my first MQTT programme!",strlen("This is my first MQTT programme!"));
			if(ret >= 0)
			{
				//表明有数据交换
				no_mqtt_msg_exchange = 0;
				//获取当前滴答，作为心跳包起始时间
				curtick = xTaskGetTickCount();				
			}
		}

		//这里主要目的是定时向服务器发送PING保活命令
		if((xTaskGetTickCount() - curtick) >(KEEPLIVE_TIME/2*1000))
		{
			curtick = xTaskGetTickCount();
			//判断是否有数据交换
			if(no_mqtt_msg_exchange == 0)
			{
				//如果有数据交换，这次就不需要发送PING消息
				continue;
			}

			if(my_mqtt_send_pingreq(mysock) < 0)
			{
				goto MQTT_reconnect;
			}

			//表明有数据交换
			no_mqtt_msg_exchange = 0;
		}
	}

MQTT_reconnect:
	 //关闭链接
	 transport_close(mysock);
	 //重新链接服务器
	 goto MQTT_START;
}


void MqttApplicationInit (void)
{
    xTaskCreate( vTaskMqtt,
				 /* 任务函数  */
                 "vTaskMqtt",
				 /* 任务名    */
                 512,
				 /* 任务栈大小，单位word，也就是4字节 */
                 NULL,
				 /* 任务参数  */
                 configMAX_PRIORITIES - 1,
				 /* 任务优先级*/
                 &xHandleTaskMqtt );
				 /* 任务句柄  */
}
