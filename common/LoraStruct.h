//
// Created by 114514 on 2026/11/10.
//

#ifndef LORACOPY_LORASTRUCT_H
#define LORACOPY_LORASTRUCT_H
#include "stm32f4xx_hal.h"

typedef struct __SPI_HandleTypeDef SPI_HandleTypeDef;
typedef struct {
    GPIO_TypeDef *Gpio; //GPIO端口
    uint16_t pin;  //Pin引脚
}GpioPin;

typedef struct {
    SPI_HandleTypeDef* spi; //spi句柄
    GpioPin cs;
    GpioPin rest;
    GpioPin busy;
}LoraConfig;
typedef struct {
    uint8_t serverIp[4];
    uint8_t serverPort;
    uint8_t ip[4];
    uint8_t gateway[4];
    uint8_t netMask[4];
    uint8_t mac[6];
    uint8_t crc;
}LoraEthHeader;
typedef enum
{
    /* ---------------- 设备级指令 / 返回 ---------------- */

            AP_PACKET_DEV_INFO_REQ = 0,     /**< 查询设备信息（上位机 → 下位机） */
            AP_PACKET_DEV_INFO_RSP,     /**< 设备信息返回（下位机 → 上位机） */

            /* ---------------- 通道状态 ---------------- */

            AP_PACKET_CH_STATUS_REQ,    /**< 查询通道状态（上位机 → 下位机） */
            AP_PACKET_CH_STATUS_RSP,    /**< 通道状态返回（下位机 → 上位机） */

            /* ---------------- 通道控制指令 ---------------- */

            AP_PACKET_CAD_REQ,         /**< 扫描指令 */
            AP_PACKET_MONITOR_REQ,      /**< 重点监测指令 */
            AP_PACKET_GUARD_REQ,        /**< 控守指令 */
            AP_PACKET_STOP_REQ,         /**< 停止指令 */

            /* ---------------- 通道结果返回 ---------------- */

            AP_PACKET_CAD_RSP,      /**< 扫描结果返回 */
            AP_PACKET_MONITOR_RSP,   /**< 重点监测结果返回 */
            AP_PACKET_GUARD_RSP,     /**< 控守结果返回 */
            AP_PACKET_STOP_RSP,      /**< 停止结果返回（成功/失败） */

            /* ---------------- ETH ---------------- */
            AP_PACKET_SET_ETH_REQ,       /**< 请求设置ETH配置 */
            AP_PACKET_SET_ETH_RSP,       /**< ETH配置设置结果返回 */

            AP_PACKET_ETH_INFO_REQ,      /**< 请求获取ETH配置 */
            AP_PACKET_ETH_INFO_RSP,      /**< ETH配置返回信息 */

            /* ---------------- OTA ---------------- */
            AP_PACKET_OTA_HEADER_REQ,    /**< 请求OTA升级头信息 */
            AP_PACKET_OTA_HEADER_RSP,    /**< OTA升级头信息返回 */

            AP_PACKET_OTA_PACKET_REQ,    /**< 请求OTA升级数据包 */
            AP_PACKET_OTA_PACKET_RSP,    /**< OTA升级数据包返回 */

            /* ---------------- Reset ---------------- */
            AP_PACKET_REQ_RESET_REQ,          /**< 重启单片机 */
            AP_PACKET_REQ_RESET_RSP,          /**< 重启单片机 */

            /* ---------------- 其他 ---------------- */
            AP_PACKET_REQ_ACK,          /**< ACK / TASK返回 */
            AP_PACKET_LOG,              /**< 日志 / 调试信息数据包 */
            AP_PACKET_TASK_FINISHED,    /**< 任务完成 */
            AP_PACKET_UNKNOWN = 0xFF,          /**< 未知或非法数据包类型 */

} ap_lora_packet_type_t;
typedef struct
{
    uint16_t         MagicNumber;      /**< 数据来源标识（魔术字），用于 区分数据来源<单片机 0x5678 or上位机0x1234>*/
    uint16_t         Length;           /**< 整个packet数据长度 */
    ap_lora_packet_type_t PacketType;       /**< 数据包类型：send rev ota log/ 其他类型 */
} AP_HEADER;
uint32_t crc32(const uint8_t *data, uint32_t len);
#endif //LORACOPY_LORASTRUCT_H