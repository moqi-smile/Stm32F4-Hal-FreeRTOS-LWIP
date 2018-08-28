#include "main.h" 

/* Peripheral */
#include "ethernet/ethernetif.h"
#include "netif/etharp.h"
#include "lwip/dhcp.h"
#include "lwip/mem.h"
#include "lwip/memp.h"
#include "lwip/init.h"
#include "lwip/timers.h"
#include "lwip/tcp_impl.h"
#include "lwip/ip_frag.h"
#include "lwip/tcpip.h" 

#include "includes.h"

#include "LwipApplication.h"

#define emacBLOCK_TIME_WAITING_FOR_INPUT	( ( portTickType ) 100 )

#define LWIP_MAX_DHCP_TRIES		4
//DHCP服务器最大重试次数

struct netif lwip_netif;
//定义一个全局的网络接口

xSemaphoreHandle s_xSemaphore;
xSemaphoreHandle ETH_link_xSemaphore;

static TaskHandle_t xHandleTaskLWIP = NULL;
static TaskHandle_t xHandleTaskEthIrq = NULL;

EventGroupHandle_t EthNetStatusHandler;

const uint8_t Static_IP[4] = {192, 168, 1, 30};
const uint8_t Static_netmask[4] = {255, 255, 255, 0};
const uint8_t Static_gateway[4] = {192, 168, 1, 1};

static void vTaskLWIP(void *pvParameters)
{
	struct ip_addr ipaddr;
	//ip地址
	struct ip_addr netmask;
	//子网掩码
	struct ip_addr gw;
	//默认网关
	EventBits_t StatusValue;

	tcpip_init( NULL, NULL );	
	//初始化LWIP内核

    while(1)
    {
		StatusValue=xEventGroupWaitBits((EventGroupHandle_t	)EthNetStatusHandler,		
									    (EventBits_t		)EVENTBIT_ALL,
									    (BaseType_t			)pdFALSE,				
									    (BaseType_t			)pdFALSE,
									    (TickType_t			)250);	
		switch (StatusValue)
		{
			case StartNET: 
				ipaddr.addr = 0;
				netmask.addr = 0;
				gw.addr = 0;

				netif_add(&lwip_netif,&ipaddr,&netmask,&gw,NULL, &ethernetif_init, &tcpip_input);
				//向网卡列表中添加一个网口

				lwip_netif.flags |= NETIF_FLAG_LINK_UP;

				netif_set_default(&lwip_netif);
				//设置netif为默认网口
				netif_set_up(&lwip_netif);
				//打开netif网口

				xEventGroupClearBits(EthNetStatusHandler,StartNET);
				xEventGroupSetBits(EthNetStatusHandler,StartDHCP);
				printf("初始化网络...........\r\n");  
				break;
			case StartDHCP:
				//开启DHCP
				dhcp_start(&lwip_netif);
				xEventGroupClearBits(EthNetStatusHandler,StartDHCP);
				xEventGroupSetBits(EthNetStatusHandler,WaitDHCP);
				//等待通过DHCP获取到的地址
				printf("正在查找DHCP服务器,请稍等...........\r\n");  
				break;
			case WaitDHCP:
				//等待获取到IP地址
			{
				if(lwip_netif.ip_addr.addr!=0)
					//正确获取到IP地址的时候
				{
					xEventGroupClearBits(EthNetStatusHandler,WaitDHCP);
					xEventGroupSetBits(EthNetStatusHandler,DhcpSuccess);
					//DHCP成功
					printf("网卡en的MAC地址为:................%.2X%.2X%.2X%.2X%.2X%.2X\r\n",
						lwip_netif.hwaddr[0],lwip_netif.hwaddr[1],lwip_netif.hwaddr[2],
						lwip_netif.hwaddr[3],lwip_netif.hwaddr[4],lwip_netif.hwaddr[5]);
					//解析出通过DHCP获取到的IP地址
					printf("通过DHCP获取到IP地址..............%d.%d.%d.%d\r\n",
						(uint8_t)(lwip_netif.ip_addr.addr),
							(uint8_t)(lwip_netif.ip_addr.addr>>8),
								(uint8_t)(lwip_netif.ip_addr.addr>>16),
									(uint8_t)(lwip_netif.ip_addr.addr>>24));
				}
				else if(lwip_netif.dhcp->tries>LWIP_MAX_DHCP_TRIES)
					//通过DHCP服务获取IP地址失败,且超过最大尝试次数
				{
					xEventGroupClearBits(EthNetStatusHandler,WaitDHCP);
					xEventGroupSetBits(EthNetStatusHandler,DhcpFail);
					//DHCP超时失败.
					//使用静态IP地址
					IP4_ADDR(&(lwip_netif.ip_addr),
						Static_IP[0], Static_IP[1], Static_IP[2],Static_IP[3]);
					IP4_ADDR(&(lwip_netif.netmask), Static_netmask[0], Static_netmask[1],Static_netmask[2],Static_netmask[3]);
					IP4_ADDR(&(lwip_netif.gw), Static_gateway[0], Static_gateway[1],Static_gateway[2],Static_gateway[3]);
					printf("网卡en的MAC地址为:................%d.%d.%d.%d.%d.%d\r\n",
						lwip_netif.hwaddr[0],lwip_netif.hwaddr[1],lwip_netif.hwaddr[2],
						lwip_netif.hwaddr[3],lwip_netif.hwaddr[4],lwip_netif.hwaddr[5]);
					printf("静态IP地址........................%d.%d.%d.%d\r\n",
						Static_IP[0],Static_IP[1],Static_IP[2],Static_IP[3]);
					printf("子网掩码..........................%d.%d.%d.%d\r\n",
						Static_netmask[0],Static_netmask[1],Static_netmask[2],Static_netmask[3]);
					printf("默认网关..........................%d.%d.%d.%d\r\n",
						Static_gateway[0],Static_gateway[1],Static_gateway[2],Static_gateway[3]);
				}
			}
			break;
			case DhcpSuccess:
				//等待获取到IP地址
			{
				printf("网络连接成功\r\n");
				xEventGroupClearBits(EthNetStatusHandler,DhcpSuccess);
				xEventGroupSetBits(EthNetStatusHandler,NetConnect);
				vTaskSuspend(xHandleTaskLWIP);
			}
			break;
			case NetConnect:
				printf("NetConnect\r\n");
			break;
			default :
				break;
		}
    }
}

static void vTaskEthIrq(void *pvParameters)
{
    while(1)
    {
        if (xSemaphoreTake( s_xSemaphore, emacBLOCK_TIME_WAITING_FOR_INPUT)==pdTRUE)
        {
			/* 接收数据并发送至LwIP */
			ethernetif_input(&lwip_netif);
        }
    }
}

void LwipApplicationInit (void)
{
	vSemaphoreCreateBinary(s_xSemaphore);
	xSemaphoreTake( s_xSemaphore, 0);

	EthNetStatusHandler=xEventGroupCreate();
	//创建事件标志组
	xEventGroupSetBits(EthNetStatusHandler,StartNET);
	
    xTaskCreate( vTaskLWIP,   	/* 任务函数  */
                 "vTaskLWIP",     	/* 任务名    */
                 512,               	/* 任务栈大小，单位word，也就是4字节 */
                 NULL,              	/* 任务参数  */
                 1,                 	/* 任务优先级*/
                 &xHandleTaskLWIP );  /* 任务句柄  */
    xTaskCreate( vTaskEthIrq,   	/* 任务函数  */
                 "vTaskEthIrq",     	/* 任务名    */
                 512,               	/* 任务栈大小，单位word，也就是4字节 */
                 NULL,              	/* 任务参数  */
                 1,                 	/* 任务优先级*/
                 &xHandleTaskEthIrq );  /* 任务句柄  */
	
}

void HAL_ETH_RxCpltCallback(ETH_HandleTypeDef *heth)
{
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	/* Give the semaphore to wakeup LwIP task */
	xSemaphoreGiveFromISR( s_xSemaphore, &xHigherPriorityTaskWoken );   
	/* Switch tasks if necessary. */

	if( xHigherPriorityTaskWoken != pdFALSE )
	{
		portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
	}
}
