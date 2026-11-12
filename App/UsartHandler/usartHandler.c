//
// Created by 114514 on 2026/11/12.
//

#include "usartHandler.h"

#include "portmacro.h"
#include "projdefs.h"
#include "usart.h"
StreamBufferHandle_t uart6_stream;
static uint8_t  buffer[STREAM_BUF_SIZE];
void UsartCallBack(UART_HandleTypeDef *huart) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xStreamBufferSendFromISR(uart6_stream,
        buffer,
        1,
        &xHigherPriorityTaskWoken
        );
    HAL_UART_Receive_IT(&huart6, buffer, STREAM_BUF_SIZE);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void UsartDataProcess(uint16_t Timeout) {

}

void InitUsart(void) {
    uart6_stream = xStreamBufferCreate(STREAM_BUF_SIZE,0);
    if (uart6_stream == NULL) {
        printf("USART STREAM CREATE FAILED\r\n");
    }
    HAL_UART_Receive_IT(&huart6, buffer, 1);
}
