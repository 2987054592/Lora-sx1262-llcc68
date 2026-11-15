//
// Created by 114514 on 2026/11/15.
//

#ifndef LORACOPY_TCPDATAPROCESS_H
#define LORACOPY_TCPDATAPROCESS_H
#include "main.h"
#include "../../common/LoraStruct.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "stream_buffer.h"
#include "cmsis_os.h"

/* StreamBuffer: 网口收到原始字节(接收 → 处理) */
#define TCP_RX_STREAM_SIZE  1024
/* Queue: 待发送的完整帧(处理 → 发送), 元素 512 字节 */
#define TCP_TX_ITEM_SIZE    512
#define TCP_TX_QUEUE_LEN    4

extern StreamBufferHandle_t tcp_rx_stream;
extern QueueHandle_t        tcp_tx_queue;
extern SemaphoreHandle_t    tcp_rx_sem;
extern SemaphoreHandle_t    tcp_tx_sem;

uint8_t EthTcpConnect(void);
int EthTcpGetSock(void);
void EthTcpSockReset(void);
int EthTcpRecvPacket(const uint8_t *raw, uint16_t len);
int EthTcpSendPacket(ap_lora_packet_type_t type, const uint8_t *payload, uint16_t plen);
void EthTcpComInit(void);

/* 任务入口(在 TcpDataProcess.c 实现, FreeRtosHandler 只创建) */
void EthTcpReceiveTaskLoop(void);
void EthTcpProcessTaskLoop(void);
void EthTcpSendTaskLoop(void);
#endif //LORACOPY_TCPDATAPROCESS_H