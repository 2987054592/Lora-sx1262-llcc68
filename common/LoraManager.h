//
// Created by 114514 on 2026/11/19.
//

#ifndef LORACOPY_LORAMANAGER_H
#define LORACOPY_LORAMANAGER_H
#include "main.h"
#include "LoraStruct.h"
#include "LoraDevice.h"

/* 双芯片管理器：每个实例 1:1 持有自己的驱动句柄（context），
 * 初始化顺序 = bind(LINK+init) -> radio_config(一次性LoRa配置)。 */

uint8_t LoraInit(void);                          // 初始化两颗芯片，0=成功
LoraInstance *lora_get_sx1262(void);             // 取 SX1262 实例（上位机下发参数用）
LoraInstance *lora_get_llcc68(void);             // 取 LLCC68 实例
void lora_irq_dispatch(uint16_t GPIO_Pin);       // EXTI 引脚 -> 对应芯片 irq_handler
void LoraTest(void);                            // 测试任务

#endif //LORACOPY_LORAMANAGER_H