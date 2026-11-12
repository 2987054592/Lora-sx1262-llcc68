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
void UsartDataProcess(uint16_t Timeout);
void UsartCallBack(UART_HandleTypeDef *huart);
void InitUsart(void);
#endif //LORACOPY_USARTHANDLER_H