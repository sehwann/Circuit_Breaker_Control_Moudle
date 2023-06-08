#ifndef _PETA_H_
#define _PETA_H_

#include "at32f4xx.h"
USART_InitType USART_InitStructure;


void SendData(USART_Type* USARTx, uint8_t TxMsg[], int32_t len);
void RCC_Configuration(void);
void GPIO_Configuration(void);
void UART_Configuration(void);
void ADC_Configuration(void);
void NVIC_Configuration(void);

#endif
