//
// Created by 114514 on 2026/11/18.
//

#ifndef LORACOPY_LORADEVICE_H
#define LORACOPY_LORADEVICE_H
#include "main.h"
#include "LoraStruct.h"
uint8_t lora_driver_bind(LoraInstance *inst, void *handle);   // 绑定底层库句柄 + LINK 宏填函数指针 + init
uint8_t lora_apply_config(LoraInstance *inst);                // mod/pkg/cad/rf 全参数一次写入芯片
void    lora_load_scan_item(LoraInstance *inst, const lora_scan_item_t *item);  // 扫描表项→实例参数分组
uint8_t lora_start_cad(LoraInstance *inst);                   // 异步发 SET_CAD（不等待）
uint8_t lora_start_rx(LoraInstance *inst);                    // 异步发 SET_RX（single/continuous）
uint8_t lora_send(LoraInstance *inst, const uint8_t *buf, uint16_t len);  // 写buffer+SET_TX
uint8_t lora_irq_handler(LoraInstance *inst);                 // 包一层 sx1262_irq_handler
uint8_t lora_set_idle(LoraInstance *inst);                    // standby
int16_t lora_get_rssi(LoraInstance *inst);                    // 读上次RSSI
#endif //LORACOPY_LORADEVICE_H