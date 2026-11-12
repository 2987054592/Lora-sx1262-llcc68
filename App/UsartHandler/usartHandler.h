//
// Created by 114514 on 2026/11/12.
//

#ifndef LORACOPY_USARTHANDLER_H
#define LORACOPY_USARTHANDLER_H
#define STREAM_BUF_SIZE 256
#include "main.h"
#include "FreeRTOS.h"
#include "stream_buffer.h"
extern StreamBufferHandle_t uart6_stream;
extern DMA_HandleTypeDef hdma_usart6_rx;
void UsartDataProcess(uint16_t Timeout);
void UsartCallBack(UART_HandleTypeDef *huart,uint16_t Size);
void InitUsart(void);
void processData(uint8_t type, uint8_t *data, uint8_t len);
#endif //LORACOPY_USARTHANDLER_H