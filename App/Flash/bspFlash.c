//
// Created by 114514 on 2026/11/15.
//

#include "bspFlash.h"
#include "stm32f4xx_hal_flash.h"
#include "stm32f4xx_hal_flash_ex.h"
#include "FreeRTOS.h"
#include "task.h"

static uint32_t AreaCount(FlashArea area)
{
    switch (area) {
        case FLASH_BOOTLOADER_DATA: return 2;   /* S0-1 */
        case FLASH_ETH_DATA:        return 1;   /* S11  */
        case FLASH_USER_DATA:       return 1;   /* S3   */
        case FLASH_APP_A:           return 4;   /* S4-7 */
        case FLASH_APP_B:           return 4;   /* S8-11*/
        default:                    return 1;
    }
}
/* Flash物理扇区号映射: FLASH_ETH_DATA 使用 Flash 尾部 S11,避开程序代码区 */
static uint32_t SectorNum(FlashArea area)
{
    switch (area) {
        case FLASH_BOOTLOADER_DATA: return 0;
        case FLASH_ETH_DATA:        return 11;
        case FLASH_USER_DATA:       return 3;
        case FLASH_APP_A:           return 4;
        case FLASH_APP_B:           return 8;
        default:                    return 0;
    }
}
static uint32_t SectorBase(FlashArea area)
{
    static const uint32_t base[12] = {
        0x08000000,0x08004000,0x08008000,0x0800C000,  /* S0-S3 16K */
        0x08010000,                                    /* S4 64K   */
        0x08020000,0x08040000,0x08060000,              /* S5-S7 128K */
        0x08080000,0x080A0000,0x080C0000,0x080E0000    /* S8-S11 128K */
    };
    if (area == FLASH_ETH_DATA) return 0x080E0000;   /* S11,Flash尾部 */
    return base[area];
}
uint8_t FlashEraseSector(FlashArea area) {
    taskENTER_CRITICAL();
    FLASH_EraseInitTypeDef erase;
    uint32_t sectorError = 0;

    erase.TypeErase = FLASH_TYPEERASE_SECTORS;
    erase.Sector = SectorNum(area);
    erase.NbSectors = AreaCount(area);
    erase.VoltageRange = VOLTAGE_RANGE_3;

    HAL_FLASH_Unlock();
    HAL_StatusTypeDef result = HAL_FLASHEx_Erase(&erase, &sectorError);
    HAL_FLASH_Lock();
    taskEXIT_CRITICAL();
    return result==HAL_OK?1:0;
}

uint8_t FlashAreaWrite(FlashArea area, const uint8_t *data, uint32_t len) {
    uint32_t addr = SectorBase(area);
    HAL_StatusTypeDef st = HAL_OK;
    uint32_t i;
    if (!FlashEraseSector(area)) return 0;


    HAL_FLASH_Unlock();
    for (i = 0; i < len; i++) {
        st = HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, addr + i, data[i]);
        if (st != HAL_OK) break;
    }
    HAL_FLASH_Lock();

    return (st == HAL_OK) ? 1 : 0;

}
uint8_t FlashAreaRead(FlashArea area, uint8_t *out, uint32_t len)
{
    uint32_t addr = SectorBase(area);
    uint32_t i;
    for (i = 0; i < len; i++) {
        out[i] = *(volatile uint8_t*)(addr + i);
    }
    return 1;
}
uint8_t FlashWriteEthData(LoraEthHeader *eth)
{
    return FlashAreaWrite(FLASH_ETH_DATA, (const uint8_t*)eth, sizeof(LoraEthHeader));
}
uint8_t FlashReadEthData(LoraEthHeader *out)
{
    uint8_t raw[sizeof(LoraEthHeader)];
    uint32_t i;
    uint8_t allFF = 1;

    for (i = 0; i < sizeof(raw); i++) {
        raw[i] = *(volatile uint8_t*)(SectorBase(FLASH_ETH_DATA) + i);
        if (raw[i] != 0xFF) allFF = 0;
    }
    if (allFF) return 0;                 /* 空扇区: 没配置 */

    memcpy(out, raw, sizeof(*out));
    if (out->crc != (uint8_t)crc32(raw, sizeof(raw)-1)) return 0;  /* CRC复核 */

    return 1;
}

/* 全局ETH配置缓存, 各文件共享 */
LoraEthHeader g_ethCfg;

uint8_t EthCfgInit(void)
{
    if (FlashReadEthData(&g_ethCfg)) {
        return 1;
    }
    return 0;
}