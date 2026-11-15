//
// Created by 114514 on 2026/11/15.
//

#ifndef LORACOPY_BSPFLASH_H
#define LORACOPY_BSPFLASH_H
#include "main.h"
#include "../../common/LoraStruct.h"
typedef enum{
    FLASH_BOOTLOADER_DATA = 0, // Sector 0-1
    FLASH_ETH_DATA = 2, // Sector 2
    FLASH_USER_DATA = 3, // Sector 3
    FLASH_APP_A = 4,     // Sector 4-7
    FLASH_APP_B = 8,     // Sector 8-11
} FlashArea;
uint8_t FlashWriteEthData(LoraEthHeader *eth);
uint8_t FlashReadEthData(LoraEthHeader *out);
uint8_t EthCfgInit(void);

/* 全局ETH配置缓存, 各文件共享 */
extern LoraEthHeader g_ethCfg;
#endif //LORACOPY_BSPFLASH_H