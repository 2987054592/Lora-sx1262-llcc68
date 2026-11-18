//
// Created by 114514 on 2026/11/10.
//

#ifndef LORACOPY_LORASTRUCT_H
#define LORACOPY_LORASTRUCT_H
#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

typedef struct __SPI_HandleTypeDef SPI_HandleTypeDef;
typedef struct {
    GPIO_TypeDef *Gpio; //GPIO端口
    uint16_t pin;  //Pin引脚
}GpioPin;

/* ============================ 芯片类型 ============================ */
typedef enum {
    LORA_CHIP_SX1262 = 0,   /**< Semtech SX1262 */
    LORA_CHIP_LLCC68        /**< Semtech LLCC68  */
} lora_chip_t;

/* ============================ 中性枚举（与具体库解耦） ============================ */
typedef enum {
    LORA_SF_5 = 5, LORA_SF_6, LORA_SF_7, LORA_SF_8,
    LORA_SF_9, LORA_SF_10, LORA_SF_11, LORA_SF_12
} lora_sf_t;

typedef enum {
    LORA_BW_125K = 0x04, LORA_BW_250K = 0x05, LORA_BW_500K = 0x06
} lora_bw_t;

typedef enum {
    LORA_CR_4_5 = 0x01, LORA_CR_4_6 = 0x02, LORA_CR_4_7 = 0x03, LORA_CR_4_8 = 0x04
} lora_cr_t;

typedef enum {
    LORA_CRC_DISABLE = 0, LORA_CRC_ENABLE
} lora_crc_t;

typedef enum {
    LORA_IQ_NORMAL = 0, LORA_IQ_INVERTED
} lora_iq_t;

typedef enum {
    LORA_HEADER_EXPLICIT = 0, LORA_HEADER_IMPLICIT
} lora_header_t;

typedef enum {
    LORA_LDRO_DISABLE = 0, LORA_LDRO_ENABLE
} lora_ldro_t;

typedef enum {
    LORA_CAD_SYMBOL_1 = 0, LORA_CAD_SYMBOL_2, LORA_CAD_SYMBOL_4,
    LORA_CAD_SYMBOL_8, LORA_CAD_SYMBOL_16
} lora_cad_sym_t;

typedef enum {
    LORA_CAD_EXIT_ONLY = 0, LORA_CAD_EXIT_RX
} lora_cad_exit_t;

/* ---- 芯片底层模式（与 DriverLib 状态机映射） ---- */
typedef enum {
    LORA_RADIO_IDLE = 0, LORA_RADIO_CAD, LORA_RADIO_RX, LORA_RADIO_TX
} lora_radio_t;

/* ---- 业务阶段 ---- */
typedef enum {
    LORA_PHASE_IDLE = 0, LORA_PHASE_SCAN, LORA_PHASE_MONITOR, LORA_PHASE_GUARD
} lora_phase_t;

/* ============================ LoRa 参数分组 ============================ */
typedef struct {
    lora_sf_t   sf;     /**< 扩频因子 */
    lora_bw_t   bw;     /**< 带宽     */
    lora_cr_t   cr;     /**< 编码率   */
    lora_ldro_t ldro;   /**< 低速优化 */
} LoraModParams;

typedef struct {
    uint16_t      preamble_len;   /**< 前导码长度 */
    lora_header_t header;         /**< 显式/隐式头 */
    uint8_t       payload_len;    /**< 固定负载长度 */
    lora_crc_t    crc;            /**< CRC 开关 */
    lora_iq_t     iq;             /**< IQ 极性 */
} LoraPktParams;

typedef struct {
    lora_cad_sym_t symbol_num;   /**< CAD 符号数 */
    uint8_t        det_peak;     /**< 检出峰值 */
    uint8_t        det_min;      /**< 最小检出值 */
    lora_cad_exit_t exit_mode;   /**< CAD 退出模式 */
    uint32_t       timeout;      /**< 超时 */
} LoraCadParams;

typedef struct {
    uint32_t freq_hz;        /**< 中心频率 Hz */
    int8_t   tx_dbm;         /**< 发射功率 dBm */
    uint16_t sync_word;      /**< LoRa 同步字 */
    uint8_t  rx_gain;        /**< 接收增益 0/1/2 */
    uint8_t  ocp;            /**< 过流保护加流值 */
    uint8_t  symb_num_timeout; /**< 接收符号超时 */
    uint8_t  reg_mode;       /**< 0=LDO 1=DC_DC 预留 */
} LoraRfParams;

/* ============================ 前向声明 ============================ */
typedef struct _LoraInstance LoraInstance;
typedef struct _ap_lora_runtime ap_lora_runtime_t;

/* ============================ 硬件（1:1 引脚绑定） ============================ */
typedef struct {
    SPI_HandleTypeDef* spi;  /**< SPI 句柄 */
    GpioPin cs;              /**< 片选 */
    GpioPin reset;           /**< 复位（原 rest） */
    GpioPin busy;            /**< BUSY 状态 */
    GpioPin dio1;            /**< DIO1 中断 */
}LoraConfig;
typedef LoraConfig LoraHw;

/* ============================ 扫描计划 / 结果 ============================ */
typedef struct {
    uint32_t       freq_hz;      /**< 频点 */
    lora_sf_t      sf;
    lora_bw_t      bw;
    lora_cr_t      cr;
    lora_crc_t     crc;
    lora_iq_t      iq;
    uint8_t        payload_len;
    lora_cad_sym_t cad_sym;
} lora_scan_item_t;

typedef struct {
    uint8_t  tried;          /**< 1=已扫 */
    uint8_t  found;          /**< 1=命中 */
    int16_t  rssi;
    int8_t   snr;
    uint8_t  frame_cnt;
    uint32_t last_seen_ms;
} lora_scan_result_t;

/* ===== 433M 盲扫总表（定义在 LoraStruct.c） ===== */
extern const lora_scan_item_t g_lora_sx1262_scan_table[];   /**< SX1262：SF5..12 × 3BW × 4CR，每频点 96 条 */
extern const uint16_t         g_lora_sx1262_scan_table_len;
extern const lora_scan_item_t g_lora_llcc68_scan_table[];   /**< LLCC68：SF5..11 × 3BW × 4CR，每频点 84 条 */
extern const uint16_t         g_lora_llcc68_scan_table_len;

/* ============================ 行为回调 ============================ */
typedef void    (*lora_dispatch_fn)(LoraInstance *inst, uint8_t type, uint8_t *buf, uint16_t len);
typedef void    (*lora_data_receive_fn)(LoraInstance *inst, uint8_t *buf, uint16_t len);
typedef uint8_t (*lora_start_rx_fn)(LoraInstance *inst, uint32_t timeout);
typedef uint8_t (*lora_start_cad_fn)(LoraInstance *inst);
typedef int16_t (*lora_get_rssi_fn)(LoraInstance *inst);
typedef void    (*lora_set_idle_fn)(LoraInstance *inst);

/* ============================ 运行时（与实例双向指针互指，1:1） ============================ */
struct _ap_lora_runtime {
    LoraInstance *inst;         /**< 反向指回芯片实例 */

    lora_radio_t radio;         /**< 芯片底层当前模式 */
    lora_phase_t phase;         /**< 业务阶段 */
    uint8_t      running;

    const lora_scan_item_t *scan_table;    /**< 扫描计划表 */
    lora_scan_result_t     *scan_result;   /**< 扫描结果表 */
    uint16_t               table_len;
    uint16_t               table_idx;      /**< 单游标：当前项 */

    uint16_t cal_freq_lo_mhz;
    uint16_t cal_freq_hi_mhz;

    lora_dispatch_fn      dispatch;
    lora_start_rx_fn      start_rx;
    lora_start_cad_fn     start_cad;
    lora_data_receive_fn  data_receive;
    lora_get_rssi_fn      get_rssi;
    lora_set_idle_fn      set_idle;
};

/* ============================ 芯片实例 ============================ */
struct _LoraInstance {
    lora_chip_t chip;           /**< 芯片类型 */
    LoraHw      hw;             /**< 硬件引脚 */

    LoraModParams mod;          /**< 调制参数 */
    LoraPktParams pkt;          /**< 帧参数 */
    LoraCadParams cad;          /**< CAD 参数 */
    LoraRfParams  rf;           /**< 射频参数 */

    void               *context;  /**< sx1262_handle_t / llcc68_handle_t */
    ap_lora_runtime_t  *rt;       /**< 专属运行时（1:1） */
    TaskHandle_t        task;     /**< 专属任务句柄 */
    SemaphoreHandle_t   irq_sem;  /**< DIO1 信号量 */

    uint8_t  *rx_buf;             /**< 接收缓冲 */
    uint16_t rx_len;
};
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