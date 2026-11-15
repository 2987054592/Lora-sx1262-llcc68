//
// Created by 114514 on 2026/11/12.
//

#include "usartHandler.h"

#include "portmacro.h"
#include "projdefs.h"
#include "usart.h"
#define FRAME_MAX_LEN  300
#include "../../common/LoraStruct.h"
#include "../../App/Flash/bspFlash.h"
#include "FreeRTOS.h"
#include "task.h"
static uint8_t  rxBuf[STREAM_BUF_SIZE];
static uint16_t rxIndex = 0;
static uint16_t packetLen = 0;

#define HEADER_LEN  4   // AA BB LEN TYPE，payload 从 [4] 开始

StreamBufferHandle_t uart6_stream;
SemaphoreHandle_t    uart6_sem;
static uint8_t buffer[STREAM_BUF_SIZE];
void UsartCallBack(UART_HandleTypeDef *huart,uint16_t Size) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xStreamBufferSendFromISR(uart6_stream,
        buffer,
        Size,
        &xHigherPriorityTaskWoken
        );
    xSemaphoreGiveFromISR(uart6_sem, &xHigherPriorityTaskWoken);   /* 信号量唤醒任务 */
    HAL_UARTEx_ReceiveToIdle_DMA(&huart6, buffer, STREAM_BUF_SIZE);
    __HAL_DMA_DISABLE_IT(&hdma_usart6_rx, DMA_IT_HT);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
void ByteRead(uint8_t byte) {
    if (rxIndex >= STREAM_BUF_SIZE) {
        rxIndex = 0;
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
        return;
    }
    if (rxIndex >= HEADER_LEN + packetLen) {
        processData(rxBuf[3], &rxBuf[4], packetLen);
        rxIndex = 0;
        packetLen = 0;
    }
}

void UsartDataProcess(void) {
    uint8_t tmpBuffer[STREAM_BUF_SIZE];

    for (;;) {
        /* 阻塞等信号量: ISR 来数据才唤醒, 非轮询 */
        xSemaphoreTake(uart6_sem, portMAX_DELAY);
        /* 取出当前可读数据(非阻塞), 取空为止之后再等信号量 */
        size_t size = xStreamBufferReceive(uart6_stream, tmpBuffer, STREAM_BUF_SIZE, 0);
        if (size <= 0) {
            continue;   /* 信号量对上了但没数据, 回到等待 */
        }
        for (size_t i=0; i<size; i++) {
            ByteRead(tmpBuffer[i]);
        }
    }
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

void InitUsart(void) {
    uart6_stream = xStreamBufferCreate(STREAM_BUF_SIZE,0);
    if (uart6_stream == NULL) {
        printf("USART STREAM CREATE FAILED\r\n");
        return;
    }
    uart6_sem = xSemaphoreCreateCounting(8, 0);
    if (uart6_sem == NULL) {
        printf("USART SEM CREATE FAILED\r\n");
        return;
    }
    HAL_UARTEx_ReceiveToIdle_DMA(&huart6, buffer, STREAM_BUF_SIZE);
    __HAL_DMA_DISABLE_IT(&hdma_usart6_rx, DMA_IT_HT);
}

void processData(uint8_t type, uint8_t *data, uint8_t len) {
    if (type == 0xDA) {
        //OTA_HEADER
    }else if (type == 0xDB) {
        //OTA_PACKET
    }else if (type == 0xDD) {
        //重启mcu
        printf("RESTART MCU\r\n");
        HAL_GPIO_WritePin(GPIOI, GPIO_PIN_1, GPIO_PIN_RESET);   /* 拉低复位 */
        { volatile uint32_t d; for(d=0; d<0x20000; d++); }      /* 短延时 */
        HAL_GPIO_WritePin(GPIOI, GPIO_PIN_1, GPIO_PIN_SET);     /* 拉高释放 */
        { volatile uint32_t d; for(d=0; d<0x20000; d++); }
        vTaskDelay(10);
        __set_FAULTMASK(1);
        NVIC_SystemReset();
    }else if (type == 0xDE) {
        //ETH_HEADER
        LoraEthHeader ethHeader;
        memcpy(&ethHeader, data, sizeof(ethHeader));
        if (ethHeader.crc != (uint8_t)crc32((uint8_t *)&ethHeader, sizeof(ethHeader) - 1)) {
            printf("ETH_HEADER CRC ERROR\r\n");
            return;
        }
        // 写入eth数据
        uint8_t flash_write_eth_data = FlashWriteEthData(&ethHeader);
        if (!flash_write_eth_data) {
            printf("ETH_HEADER WRITE FAILED\r\n");
            return;
        }
        printf("ETH_HEADER WRITE SUCCESS\r\n");
        printf("RESTART MCU\r\n");
        HAL_GPIO_WritePin(GPIOI, GPIO_PIN_1, GPIO_PIN_RESET);   /* 拉低复位 */
        { volatile uint32_t d; for(d=0; d<0x20000; d++); }      /* 短延时 */
        HAL_GPIO_WritePin(GPIOI, GPIO_PIN_1, GPIO_PIN_SET);     /* 拉高释放 */
        { volatile uint32_t d; for(d=0; d<0x20000; d++); }
        vTaskDelay(10);
        __set_FAULTMASK(1);
        NVIC_SystemReset();

    }else if (type == 0xDC) {
        printf("%.*s\r\n", len, (char *)data);
    }
}