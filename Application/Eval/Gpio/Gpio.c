#include "main.h"
#include "Gpio/Gpio.h"

void Gpio_OutConfig(Gpio_t Gpiox, GpioMode_t Mode, GpioSpeed_t Speed)
{
	uint16_t Pinx = 1;
	GPIO_TypeDef *PORT;
	GPIO_InitTypeDef GPIO_InitStruct;

	PORT = ((GPIO_TypeDef *) (GPIOA_BASE + ((Gpiox&0xF0)>>4)*0x0400U ));
	Pinx <<=  Gpiox&0x0F;

	GPIO_InitStruct.Pin = Pinx;
	GPIO_InitStruct.Mode = Mode;
	GPIO_InitStruct.Speed = Speed;
	HAL_GPIO_Init(PORT, &GPIO_InitStruct);
}

void GpioPinSet(Gpio_t Gpiox)
{
	uint16_t Pinx = 1;
	GPIO_TypeDef *PORT;

	PORT = ((GPIO_TypeDef *) (GPIOA_BASE + ((Gpiox&0xF0)>>4)*0x0400U ));
	Pinx <<=  Gpiox&0x0F;

	HAL_GPIO_WritePin(PORT, Pinx, GPIO_PIN_SET);
}

void GpioPinRes(Gpio_t Gpiox)
{
	uint16_t Pinx = 1;
	GPIO_TypeDef *PORT;

	PORT = ((GPIO_TypeDef *) (GPIOA_BASE + ((Gpiox&0xF0)>>4)*0x0400U ));
	Pinx <<=  Gpiox&0x0F;

	HAL_GPIO_WritePin(PORT, Pinx, GPIO_PIN_RESET);
}

void GpioPinToggle(Gpio_t Gpiox)
{
	uint16_t Pinx = 1;
	GPIO_TypeDef *PORT;

	PORT = ((GPIO_TypeDef *) (GPIOA_BASE + ((Gpiox&0xF0)>>4)*0x0400U ));
	Pinx <<=  Gpiox&0x0F;

	HAL_GPIO_TogglePin(PORT, Pinx);
}

void Gpio_Init(void)
{
	Gpio_OutConfig(LED1, Gpio_ModeOutputPP, Gpio_SpeedHigh);
	Gpio_OutConfig(LED2, Gpio_ModeOutputPP, Gpio_SpeedHigh);
	Gpio_OutConfig(LED3, Gpio_ModeOutputPP, Gpio_SpeedHigh);
}
