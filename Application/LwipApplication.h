#ifndef __LWIPAPPLICATION_H
#define __LWIPAPPLICATION_H

#include "lwip/netif.h"

#define StartNET				(1<<0)
#define StartDHCP               (1<<1)
#define WaitDHCP                (1<<2)
#define DhcpSuccess             (1<<3)
#define DhcpFail                (1<<4)
#define NetConnect              (1<<5)
#define EVENTBIT_ALL			(StartNET|StartDHCP|WaitDHCP|DhcpSuccess|DhcpFail|NetConnect)

extern EventGroupHandle_t EthNetStatusHandler;

void LwipApplicationInit(void);

#endif
/* __LWIPAPPLICATION_H */


/******************* (C) COPYRIGHT 2015-2020 硬石嵌入式开发团队 *****END OF FILE****/

