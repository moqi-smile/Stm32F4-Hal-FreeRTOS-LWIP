#ifndef _SDRAM_H
#define _SDRAM_H


extern SRAM_HandleTypeDef SRAM_Handler;    //SRAM句柄

//使用NOR/SRAM的 Bank1.sector3,地址位HADDR[27,26]=10 
//对IS61LV25616/IS62WV25616,地址线范围为A0~A17 
//对IS61LV51216/IS62WV51216,地址线范围为A0~A18
#define Bank1_SRAM3_ADDR    ((uint32_t)(0x68000000))	


void SRAM_Init(void);
void FSMC_SRAM_WriteBuffer(uint8_t *pBuffer,uint32_t WriteAddr,uint32_t n);
void FSMC_SRAM_ReadBuffer(uint8_t *pBuffer,uint32_t ReadAddr,uint32_t n);
#endif
