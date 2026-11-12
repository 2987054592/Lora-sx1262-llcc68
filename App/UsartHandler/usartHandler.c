//
// Created by 114514 on 2026/11/12.
//

#include "usartHandler.h"

#include "portmacro.h"
#include "projdefs.h"
#include "usart.h"
#define FRAME_MAX_LEN  300

static uint8_t  rxBuf[STREAM_BUF_SIZE];
static uint16_t rxIndex = 0;
static uint16_t packetLen = 0;
static uint8_t  receiving = 0;

StreamBufferHandle_t uart6_stream;
static uint8_t buffer[STREAM_BUF_SIZE];
void UsartCallBack(UART_HandleTypeDef *huart,uint16_t Size) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xStreamBufferSendFromISR(uart6_stream,
        buffer,
        Size,
        &xHigherPriorityTaskWoken
        );
    HAL_UARTEx_ReceiveToIdle_DMA(&huart6, buffer, STREAM_BUF_SIZE);
    __HAL_DMA_DISABLE_IT(&hdma_usart6_rx, DMA_IT_HT);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
void ByteRead(uint8_t byte) {
    if (rxIndex >= STREAM_BUF_SIZE) {
        rxIndex = 0;
        receiving = 0;
        packetLen = 0;
        return;
    }
    rxBuf[rxIndex++] = byte;
    if (rxIndex == 1 && byte != 0xaa) {
        rxIndex = 0;
        return;
    }
    if (rxIndex == 2 && byte != 0xbb) {
        rxIndex = 0;
        return;
    }
    if (rxIndex == 3) {
        packetLen = byte;
        if (packetLen == 0) {
            rxIndex = 0;
            return;
        }
        receiving = 1;
        return;
    }
    static const uint8_t HEADER_LEN = 4;   // AA BB len type
    if (receiving && rxIndex >= HEADER_LEN + packetLen) {
        // rxBuf[3] = type, rxBuf[4]起 = payload
        processData(rxBuf[3], &rxBuf[4], packetLen);
        // 复位，接收下一帧
        rxIndex = 0; receiving = 0; packetLen = 0;
    }
}

void UsartDataProcess(uint16_t Timeout) {
    uint8_t tmpBuffer[STREAM_BUF_SIZE];
    size_t size = xStreamBufferReceive(uart6_stream, tmpBuffer, STREAM_BUF_SIZE, Timeout);
    if (size<=0) {
        return;
    }
    for (size_t i=0; i<size; i++) {
        ByteRead(tmpBuffer[i]);
    }
       // 完整包长度为2（aa bb 头部） + 1（len长度）+1(类型) + packet_length（数据长度）
    /*
    prefix: 0xAA  0xBB
    length:  0x1F
    type : 0xDE
    magic: 0x55AA55AA
    server_ip: 192.168.1.100
    server_port: 8883
    ip: 192.168.1.10
    port: 5688
    gateway: 192.168.1.1
    netmask: 255.255.255.0
    mac (hex): 84:A9:3E:12:34:56
    crc:  0x25
    */



}

void InitUsart(void) {
    uart6_stream = xStreamBufferCreate(STREAM_BUF_SIZE,0);
    if (uart6_stream == NULL) {
        printf("USART STREAM CREATE FAILED\r\n");
        return;
    }
    HAL_UARTEx_ReceiveToIdle_DMA(&huart6, buffer, STREAM_BUF_SIZE);
    __HAL_DMA_DISABLE_IT(&hdma_usart6_rx, DMA_IT_HT);
}

void processData(uint8_t type, uint8_t *data, uint8_t len) {

}
