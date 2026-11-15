//
// Created by 114514 on 2026/11/15.
//

#include "TcpDataProcess.h"
#include "../../common/LoraStruct.h"
#include "../../App/Flash/bspFlash.h"
#include <lwip/sockets.h>
#include <lwip/netif.h>
#include <lwip/ip_addr.h>
#include <string.h>

#include "queue.h"
#include "semphr.h"

static int g_tcp_sock = -1;   // 全局共享 socket, Connect 建立, Receive/Send 使用

/* ===== FreeRTOS 现成组件: 任务间通信 (不手写 buffer) ===== */
StreamBufferHandle_t tcp_rx_stream;  /* 接收原始字节: Receive -> Process */
QueueHandle_t        tcp_tx_queue;   /* 待发送完整帧: Process -> Send    */
SemaphoreHandle_t    tcp_rx_sem;     /* 计数信号量: 通知 Process 有数据 */
SemaphoreHandle_t    tcp_tx_sem;     /* 计数信号量: 通知 Send 有帧要发  */

/* 初始化任务间通信组件, 必须在创建任务前调用一次 */
void EthTcpComInit(void) {
    if (tcp_rx_stream == NULL) {
        tcp_rx_stream = xStreamBufferCreate(TCP_RX_STREAM_SIZE, 1);
    }
    if (tcp_tx_queue == NULL) {
        tcp_tx_queue = xQueueCreate(TCP_TX_QUEUE_LEN, TCP_TX_ITEM_SIZE);
    }
    if (tcp_rx_sem == NULL) {
        tcp_rx_sem = xSemaphoreCreateCounting(16, 0);
    }
    if (tcp_tx_sem == NULL) {
        tcp_tx_sem = xSemaphoreCreateCounting(TCP_TX_QUEUE_LEN, 0);
    }
}

uint8_t EthTcpConnect(void) {
    const uint8_t *ip;
    uint8_t        port;

    // 关键:确保配置已从 Flash 读入且 CRC 校验通过
    if (!EthCfgInit()) {
        printf("ETH CFG NOT VALID\r\n");
        return 1;   // 配置无效(空扇区/读取失败/CRC失败),中止连接
    }
    ip   = g_ethCfg.serverIp;
    port = g_ethCfg.serverPort;

    // 已有连接则先关旧 socket
    if (g_tcp_sock >= 0) {
        closesocket(g_tcp_sock);
        g_tcp_sock = -1;
    }

    // 组装服务器地址
    struct sockaddr_in server;
    server.sin_family      = AF_INET;
    server.sin_port        = htons(port);
    server.sin_addr.s_addr = htonl(((uint32_t)ip[0] << 24) | ((uint32_t)ip[1] << 16) |
                                   ((uint32_t)ip[2] << 8)  |  (uint32_t)ip[3]);

    // 建 TCP socket
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0) {
        return 1; // socket 创建失败
    }

    // 建立连接(阻塞,等对端 ACK)
    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) != 0) {
        closesocket(sock);
        return 1; // 连接失败
    }

    g_tcp_sock = sock;
    return 0; // 连接成功
}

// 供 Receive/Send 取当前 socket
int EthTcpGetSock(void) {
    return g_tcp_sock;
}

// 供 Receive 任务在检测到断开时调用, 把 socket 置为未连接
void EthTcpSockReset(void) {
    if (g_tcp_sock >= 0) {
        closesocket(g_tcp_sock);
        g_tcp_sock = -1;
    }
}

/* ===================== AP_HEADER 协议解析 ===================== */
/* 协议帧 = AP_HEADER(8字节) + payload
 * 字节序: Magic/Length 为小端, PacketType 取低字节(上位机沿用 struct, PC 端 int=4字节)
 * 偏移: 0 MagicNumber(2) | 2 Length(2) | 4 PacketType(4, 实际只用低1字节) */
static uint16_t ApGetMagic(const uint8_t *b) { return (uint16_t)(b[0] | (b[1] << 8)); }
static uint16_t ApGetLength(const uint8_t *b) { return (uint16_t)(b[2] | (b[3] << 8)); }
static uint16_t ApGetType(const uint8_t *b)    { return (uint16_t)b[4]; }

/* 上位机↙下位机: Magic=0x1234 ; 下位机↗上位机: Magic=0x5678 */
#define AP_MAGIC_HOST  0x1234
#define AP_MAGIC_MCU   0x5678
#define AP_HEADER_SIZE 8

/* 单个完整 AP 包解析 + 分发 (由接收逻辑构造缓冲调用) */
static void ApDispatch(ap_lora_packet_type_t type, const uint8_t *payload, uint16_t plen) {
    switch (type) {
        case AP_PACKET_DEV_INFO_REQ:      /* TODO */ break;
        case AP_PACKET_CH_STATUS_REQ:     /* TODO */ break;
        case AP_PACKET_CAD_REQ:           /* TODO */ break;
        case AP_PACKET_MONITOR_REQ:       /* TODO */ break;
        case AP_PACKET_GUARD_REQ:         /* TODO */ break;
        case AP_PACKET_STOP_REQ:          /* TODO */ break;

        case AP_PACKET_SET_ETH_REQ: {
            /* 参考串口 0xDE: 校验 CRC -> 写 Flash -> 回复 RSP -> 重启 */
            LoraEthHeader ethHeader;
            uint8_t result = 1;   /* 0=成功 */

            if (plen < sizeof(LoraEthHeader)) {
                printf("SET_ETH LEN ERROR\r\n");
            } else {
                memset(&ethHeader, 0, sizeof(ethHeader));
                memcpy(&ethHeader, payload, sizeof(LoraEthHeader));
                if (ethHeader.crc != (uint8_t)crc32((uint8_t *)&ethHeader, sizeof(ethHeader) - 1)) {
                    printf("SET_ETH CRC ERROR\r\n");
                } else if (!FlashWriteEthData(&ethHeader)) {
                    printf("SET_ETH WRITE FAILED\r\n");
                } else {
                    printf("SET_ETH WRITE SUCCESS\r\n");
                    result = 0;
                }
            }
            /* 回 RSP, payload 1 字节: 0=成功 1=失败 */
            EthTcpSendPacket(AP_PACKET_SET_ETH_RSP, &result, 1);
            if (result == 0) {
                vTaskDelay(pdMS_TO_TICKS(50));   /* 让 RSP 先发出去 */
                printf("RESTART MCU\r\n");
                HAL_GPIO_WritePin(GPIOI, GPIO_PIN_1, GPIO_PIN_RESET);   /* 拉低复位 */
                { volatile uint32_t d; for(d=0; d<0x20000; d++); }      /* 短延时 */
                HAL_GPIO_WritePin(GPIOI, GPIO_PIN_1, GPIO_PIN_SET);     /* 拉高释放 */
                { volatile uint32_t d; for(d=0; d<0x20000; d++); }
                vTaskDelay(10);
                __set_FAULTMASK(1);
                NVIC_SystemReset();
            }
            break;
        }
        case AP_PACKET_ETH_INFO_REQ:      /* TODO */ break;

        case AP_PACKET_OTA_HEADER_REQ:    /* TODO */ break;
        case AP_PACKET_OTA_PACKET_REQ:    /* TODO */ break;

        case AP_PACKET_REQ_RESET_REQ: {
            /* 先通知上位机已收到重启命令, 再复位 MCU */
            EthTcpSendPacket(AP_PACKET_REQ_RESET_RSP, NULL, 0);
            vTaskDelay(pdMS_TO_TICKS(50));   /* 给发送任务时间把 RSP 发出去 */
            printf("RESTART MCU\r\n");
            HAL_GPIO_WritePin(GPIOI, GPIO_PIN_1, GPIO_PIN_RESET);   /* 拉低复位 */
            { volatile uint32_t d; for(d=0; d<0x20000; d++); }      /* 短延时 */
            HAL_GPIO_WritePin(GPIOI, GPIO_PIN_1, GPIO_PIN_SET);     /* 拉高释放 */
            { volatile uint32_t d; for(d=0; d<0x20000; d++); }
            vTaskDelay(10);
            __set_FAULTMASK(1);
            NVIC_SystemReset();
            break;
        }

        case AP_PACKET_DEV_INFO_RSP:
        case AP_PACKET_CH_STATUS_RSP:
        case AP_PACKET_CAD_RSP:
        case AP_PACKET_MONITOR_RSP:
        case AP_PACKET_GUARD_RSP:
        case AP_PACKET_STOP_RSP:
        case AP_PACKET_SET_ETH_RSP:
        case AP_PACKET_ETH_INFO_RSP:
        case AP_PACKET_OTA_HEADER_RSP:
        case AP_PACKET_OTA_PACKET_RSP:
        case AP_PACKET_REQ_RESET_RSP:
        case AP_PACKET_REQ_ACK:
        case AP_PACKET_LOG:
        case AP_PACKET_TASK_FINISHED:
            break;  /* 这些是回复/日志, 设备作为客户端一般不主动收 */

        default:
            /* AP_PACKET_UNKNOWN 或未定义类型 */
            break;
    }
    (void)payload; (void)plen;
}

/* ================= 解包状态机(Process 任务内部缓冲) ================= */
/* TCP 字节流 -> 拼入缓冲 -> 按 AP_HEADER Length 切出完整帧 -> ApDispatch */
static uint8_t  g_rxBuf[TCP_RX_STREAM_SIZE];
static uint16_t g_rxLen = 0;

/* 喂入原始字节, 内部处理粘包/半包, 解出完整帧后调用 ApDispatch
 * raw: socket 或 stream 取出的原始字节流  len: 字节数 */
int EthTcpRecvPacket(const uint8_t *raw, uint16_t len) {
    uint16_t i;
    for (i = 0; i < len; i++) {
        g_rxBuf[g_rxLen++] = raw[i];
        if (g_rxLen >= TCP_RX_STREAM_SIZE) {
            g_rxLen = 0;    /* 溢出保护 */
        }
    }

    /* 尝试从缓冲中解析出所有完整帧 */
    uint16_t off = 0;
    while (g_rxLen - off >= AP_HEADER_SIZE) {
        const uint8_t *hdr = g_rxBuf + off;
        if (ApGetMagic(hdr) != AP_MAGIC_HOST && ApGetMagic(hdr) != AP_MAGIC_MCU) {
            off++;              /* 找帧同步 */
            continue;
        }
        uint16_t flen = ApGetLength(hdr);
        if (flen < AP_HEADER_SIZE || flen > TCP_RX_STREAM_SIZE) {
            off++;              /* 非法长度, 丢一字节 */
            continue;
        }
        if (g_rxLen - off < flen) {
            break;              /* 半包, 等下次数据 */
        }
        /* 完整一帧 -> 分发 */
        ap_lora_packet_type_t type = (ap_lora_packet_type_t)ApGetType(hdr);
        ApDispatch(type, g_rxBuf + off + AP_HEADER_SIZE, flen - AP_HEADER_SIZE);
        off += flen;
    }

    /* 把未消费的数据移到缓冲头部 */
    if (off > 0) {
        uint16_t remain = g_rxLen - off;
        if (remain > 0) memmove(g_rxBuf, g_rxBuf + off, remain);
        g_rxLen = remain;
    }
    return 0;
}

/* 进程任务向 Send 任务投递一帧(必须 ≤ 队列元素大小), 阻塞等待队列空位 */
static int ApTxEnqueue(ap_lora_packet_type_t type, const uint8_t *payload, uint16_t plen) {
    if (plen > (TCP_TX_ITEM_SIZE - AP_HEADER_SIZE)) {
        return -1;
    }
    uint8_t frame[TCP_TX_ITEM_SIZE];
    frame[0] = AP_MAGIC_MCU & 0xFF;           frame[1] = (AP_MAGIC_MCU >> 8) & 0xFF;
    frame[2] = (AP_HEADER_SIZE + plen) & 0xFF; frame[3] = ((AP_HEADER_SIZE + plen) >> 8) & 0xFF;
    frame[4] = (uint8_t)type;                 frame[5] = 0; frame[6] = 0; frame[7] = 0;
    if (plen > 0) {
        memcpy(frame + AP_HEADER_SIZE, payload, plen);
    }
    if (xQueueSend(tcp_tx_queue, frame, 0) != pdPASS) {
        return -1;   /* 队列满(非阻塞) */
    }
    xSemaphoreGive(tcp_tx_sem);   /* 唤醒 Send 任务 */
    return 0;
}

/* ================= 任务入口 (FreeRtosHandler 只创建) ================= */

/* 接收任务: 只管 recv 原始字节 -> 放进 tcp_rx_stream (生产) */
void EthTcpReceiveTaskLoop(void) {
    uint8_t tmp[512];
    for (;;) {
        int sock = EthTcpGetSock();
        if (sock < 0) {
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }
        int n = recv(sock, tmp, sizeof(tmp), 0);
        if (n > 0) {
            xStreamBufferSend(tcp_rx_stream, tmp, (size_t)n, 0);
            xSemaphoreGive(tcp_rx_sem);   /* 信号量唤醒 Process */
        } else if (n <= 0) {
            /* 断开: 交给上层处理(发送方重置) */
            printf("ETH TCP DISCONNECTED\r\n");
            EthTcpSockReset();
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

/* 处理任务: 阻塞等信号量(不是轮询), Data来 -> 唤醒 -> 非阻塞读空 stream -> 解包/分派 */
void EthTcpProcessTaskLoop(void) {
    uint8_t tmp[512];
    for (;;) {
        /* 让出CPU, 直到 rx 信号量被 Release 才醒来 */
        xSemaphoreTake(tcp_rx_sem, portMAX_DELAY);
        /* 取出当前所有可读数据(非阻塞) */
        for (;;) {
            size_t n = xStreamBufferReceive(tcp_rx_stream, tmp, sizeof(tmp), 0);
            if (n == 0) break;          /* 取空 */
            EthTcpRecvPacket(tmp, (uint16_t)n);
        }
    }
}

/* 发送任务: 阻塞等 tx 信号量, 有帧 -> 取出 -> send (消费) */
void EthTcpSendTaskLoop(void) {
    uint8_t frame[TCP_TX_ITEM_SIZE];
    for (;;) {
        /* 让出CPU, 直到 tx 信号量被 Release 才醒来 */
        xSemaphoreTake(tcp_tx_sem, portMAX_DELAY);
        if (xQueueReceive(tcp_tx_queue, frame, 0) == pdPASS) {
            int sock = EthTcpGetSock();
            uint16_t total = ApGetLength(frame);
            if (sock >= 0 && total > 0 && send(sock, frame, total, 0) != total) {
                printf("ETH TCP SEND FAILED\r\n");
            }
        }
    }
}

/* 供任意外部代码把数据发到网络: 拼帧入发送队列(Send 任务会送出) */
int EthTcpSendPacket(ap_lora_packet_type_t type, const uint8_t *payload, uint16_t plen) {
    return ApTxEnqueue(type, payload, plen);
}