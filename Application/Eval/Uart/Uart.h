#ifndef __UART_H__
#define __UART_H__

typedef enum 
{
	Uart1 = 0,
	Uart2,
	Uart3,
	Uart4,
	Uart5,
	Uartnum,

	User_DEBUGUart = Uart1,
	User_ClientUart = Uart3,
} Eval_UartTypeDef;

void Uart_Init(void);

#define UARTn Uartnum

#endif
