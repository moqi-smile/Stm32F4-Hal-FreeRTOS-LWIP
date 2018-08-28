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
//DHCP������������Դ���

struct netif lwip_netif;
//����һ��ȫ�ֵ�����ӿ�

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
	//ip��ַ
	struct ip_addr netmask;
	//��������
	struct ip_addr gw;
	//Ĭ������
	EventBits_t StatusValue;

	tcpip_init( NULL, NULL );	
	//��ʼ��LWIP�ں�

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
				//�������б������һ������

				lwip_netif.flags |= NETIF_FLAG_LINK_UP;

				netif_set_default(&lwip_netif);
				//����netifΪĬ������
				netif_set_up(&lwip_netif);
				//��netif����

				xEventGroupClearBits(EthNetStatusHandler,StartNET);
				xEventGroupSetBits(EthNetStatusHandler,StartDHCP);
				printf("��ʼ������...........\r\n");  
				break;
			case StartDHCP:
				//����DHCP
				dhcp_start(&lwip_netif);
				xEventGroupClearBits(EthNetStatusHandler,StartDHCP);
				xEventGroupSetBits(EthNetStatusHandler,WaitDHCP);
				//�ȴ�ͨ��DHCP��ȡ���ĵ�ַ
				printf("���ڲ���DHCP������,���Ե�...........\r\n");  
				break;
			case WaitDHCP:
				//�ȴ���ȡ��IP��ַ
			{
				if(lwip_netif.ip_addr.addr!=0)
					//��ȷ��ȡ��IP��ַ��ʱ��
				{
					xEventGroupClearBits(EthNetStatusHandler,WaitDHCP);
					xEventGroupSetBits(EthNetStatusHandler,DhcpSuccess);
					//DHCP�ɹ�
					printf("����en��MAC��ַΪ:................%.2X%.2X%.2X%.2X%.2X%.2X\r\n",
						lwip_netif.hwaddr[0],lwip_netif.hwaddr[1],lwip_netif.hwaddr[2],
						lwip_netif.hwaddr[3],lwip_netif.hwaddr[4],lwip_netif.hwaddr[5]);
					//������ͨ��DHCP��ȡ����IP��ַ
					printf("ͨ��DHCP��ȡ��IP��ַ..............%d.%d.%d.%d\r\n",
						(uint8_t)(lwip_netif.ip_addr.addr),
							(uint8_t)(lwip_netif.ip_addr.addr>>8),
								(uint8_t)(lwip_netif.ip_addr.addr>>16),
									(uint8_t)(lwip_netif.ip_addr.addr>>24));
				}
				else if(lwip_netif.dhcp->tries>LWIP_MAX_DHCP_TRIES)
					//ͨ��DHCP�����ȡIP��ַʧ��,�ҳ�������Դ���
				{
					xEventGroupClearBits(EthNetStatusHandler,WaitDHCP);
					xEventGroupSetBits(EthNetStatusHandler,DhcpFail);
					//DHCP��ʱʧ��.
					//ʹ�þ�̬IP��ַ
					IP4_ADDR(&(lwip_netif.ip_addr),
						Static_IP[0], Static_IP[1], Static_IP[2],Static_IP[3]);
					IP4_ADDR(&(lwip_netif.netmask), Static_netmask[0], Static_netmask[1],Static_netmask[2],Static_netmask[3]);
					IP4_ADDR(&(lwip_netif.gw), Static_gateway[0], Static_gateway[1],Static_gateway[2],Static_gateway[3]);
					printf("����en��MAC��ַΪ:................%d.%d.%d.%d.%d.%d\r\n",
						lwip_netif.hwaddr[0],lwip_netif.hwaddr[1],lwip_netif.hwaddr[2],
						lwip_netif.hwaddr[3],lwip_netif.hwaddr[4],lwip_netif.hwaddr[5]);
					printf("��̬IP��ַ........................%d.%d.%d.%d\r\n",
						Static_IP[0],Static_IP[1],Static_IP[2],Static_IP[3]);
					printf("��������..........................%d.%d.%d.%d\r\n",
						Static_netmask[0],Static_netmask[1],Static_netmask[2],Static_netmask[3]);
					printf("Ĭ������..........................%d.%d.%d.%d\r\n",
						Static_gateway[0],Static_gateway[1],Static_gateway[2],Static_gateway[3]);
				}
			}
			break;
			case DhcpSuccess:
				//�ȴ���ȡ��IP��ַ
			{
				printf("�������ӳɹ�\r\n");
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
			/* �������ݲ�������LwIP */
			ethernetif_input(&lwip_netif);
        }
    }
}

void LwipApplicationInit (void)
{
	vSemaphoreCreateBinary(s_xSemaphore);
	xSemaphoreTake( s_xSemaphore, 0);

	EthNetStatusHandler=xEventGroupCreate();
	//�����¼���־��
	xEventGroupSetBits(EthNetStatusHandler,StartNET);
	
    xTaskCreate( vTaskLWIP,   	/* ������  */
                 "vTaskLWIP",     	/* ������    */
                 512,               	/* ����ջ��С����λword��Ҳ����4�ֽ� */
                 NULL,              	/* �������  */
                 1,                 	/* �������ȼ�*/
                 &xHandleTaskLWIP );  /* ������  */
    xTaskCreate( vTaskEthIrq,   	/* ������  */
                 "vTaskEthIrq",     	/* ������    */
                 512,               	/* ����ջ��С����λword��Ҳ����4�ֽ� */
                 NULL,              	/* �������  */
                 1,                 	/* �������ȼ�*/
                 &xHandleTaskEthIrq );  /* ������  */
	
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
