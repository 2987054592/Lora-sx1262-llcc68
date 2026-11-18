//
// Created by 114514 on 2026/11/10.
//

#include "LoraStruct.h"

static uint32_t crc32_table[256];
static uint8_t crc32_table_init = 0;

static void crc32_table_gen(void) {
    uint32_t crc;
    for (uint32_t i = 0; i < 256; i++) {
        crc = i;
        for (uint8_t j = 0; j < 8; j++) {
            crc = (crc & 1U) ? (crc >> 1U) ^ 0xEDB88320UL : (crc >> 1U);
        }
        crc32_table[i] = crc;
    }
    crc32_table_init = 1;
}

uint32_t crc32(const uint8_t *data, uint32_t len) {
    if (!crc32_table_init) {
        crc32_table_gen();
    }
    uint32_t crc = 0xFFFFFFFFUL;
    for (uint32_t i = 0; i < len; i++) {
        crc = (crc >> 8U) ^ crc32_table[(crc ^ data[i]) & 0xFFU];
    }
    return crc ^ 0xFFFFFFFFUL;
}

/* ============================ 433M 盲扫总表 ============================ */
/* 每个频点展开 SF x BW x CR 全组合：
 *   SX1262 : SF5..SF12 x 125/250/500 x CR4/5..4/8 = 96 条
 *   LLCC68 : SF5..SF11 x 125/250/500 x CR4/5..4/8 = 84 条（无 SF12）
 * CRC/IQ/payload_len/cad_sym 为全表固定默认值。 */

/* 展开单条 CR 组合 */
#define LORA_SCAN_CRS(f, sf, bw, crc, iq, pl, cad)                          \
    { (f), (sf), (bw), LORA_CR_4_5, (crc), (iq), (pl), (cad) },             \
    { (f), (sf), (bw), LORA_CR_4_6, (crc), (iq), (pl), (cad) },             \
    { (f), (sf), (bw), LORA_CR_4_7, (crc), (iq), (pl), (cad) },             \
    { (f), (sf), (bw), LORA_CR_4_8, (crc), (iq), (pl), (cad) },

/* 展开单条 SF 的 3 种带宽 */
#define LORA_SCAN_BWS(f, sf, crc, iq, pl, cad)                              \
    LORA_SCAN_CRS(f, sf, LORA_BW_125K, crc, iq, pl, cad)                    \
    LORA_SCAN_CRS(f, sf, LORA_BW_250K, crc, iq, pl, cad)                    \
    LORA_SCAN_CRS(f, sf, LORA_BW_500K, crc, iq, pl, cad)

/* 展开全部 SF（SX1262：含 SF12） */
#define LORA_SCAN_SFS_SX(f, crc, iq, pl, cad)                               \
    LORA_SCAN_BWS(f, LORA_SF_5,  crc, iq, pl, cad)                          \
    LORA_SCAN_BWS(f, LORA_SF_6,  crc, iq, pl, cad)                          \
    LORA_SCAN_BWS(f, LORA_SF_7,  crc, iq, pl, cad)                          \
    LORA_SCAN_BWS(f, LORA_SF_8,  crc, iq, pl, cad)                          \
    LORA_SCAN_BWS(f, LORA_SF_9,  crc, iq, pl, cad)                          \
    LORA_SCAN_BWS(f, LORA_SF_10, crc, iq, pl, cad)                          \
    LORA_SCAN_BWS(f, LORA_SF_11, crc, iq, pl, cad)                          \
    LORA_SCAN_BWS(f, LORA_SF_12, crc, iq, pl, cad)

/* 展开全部 SF（LLCC68：到 SF11 止） */
#define LORA_SCAN_SFS_LC(f, crc, iq, pl, cad)                               \
    LORA_SCAN_BWS(f, LORA_SF_5,  crc, iq, pl, cad)                          \
    LORA_SCAN_BWS(f, LORA_SF_6,  crc, iq, pl, cad)                          \
    LORA_SCAN_BWS(f, LORA_SF_7,  crc, iq, pl, cad)                          \
    LORA_SCAN_BWS(f, LORA_SF_8,  crc, iq, pl, cad)                          \
    LORA_SCAN_BWS(f, LORA_SF_9,  crc, iq, pl, cad)                          \
    LORA_SCAN_BWS(f, LORA_SF_10, crc, iq, pl, cad)                          \
    LORA_SCAN_BWS(f, LORA_SF_11, crc, iq, pl, cad)

/* 433M 频点列表（占用位示例，按实际需求增删） */
const lora_scan_item_t g_lora_sx1262_scan_table[] = {
    LORA_SCAN_SFS_SX(433125000U, LORA_CRC_ENABLE, LORA_IQ_NORMAL, 8, LORA_CAD_SYMBOL_8)
    LORA_SCAN_SFS_SX(433375000U, LORA_CRC_ENABLE, LORA_IQ_NORMAL, 8, LORA_CAD_SYMBOL_8)
    LORA_SCAN_SFS_SX(433625000U, LORA_CRC_ENABLE, LORA_IQ_NORMAL, 8, LORA_CAD_SYMBOL_8)
    LORA_SCAN_SFS_SX(433875000U, LORA_CRC_ENABLE, LORA_IQ_NORMAL, 8, LORA_CAD_SYMBOL_8)
    LORA_SCAN_SFS_SX(434125000U, LORA_CRC_ENABLE, LORA_IQ_NORMAL, 8, LORA_CAD_SYMBOL_8)
    LORA_SCAN_SFS_SX(434375000U, LORA_CRC_ENABLE, LORA_IQ_NORMAL, 8, LORA_CAD_SYMBOL_8)
    LORA_SCAN_SFS_SX(434625000U, LORA_CRC_ENABLE, LORA_IQ_NORMAL, 8, LORA_CAD_SYMBOL_8)
};
const uint16_t g_lora_sx1262_scan_table_len =
        sizeof(g_lora_sx1262_scan_table) / sizeof(lora_scan_item_t);

const lora_scan_item_t g_lora_llcc68_scan_table[] = {
    LORA_SCAN_SFS_LC(433125000U, LORA_CRC_ENABLE, LORA_IQ_NORMAL, 8, LORA_CAD_SYMBOL_8)
    LORA_SCAN_SFS_LC(433375000U, LORA_CRC_ENABLE, LORA_IQ_NORMAL, 8, LORA_CAD_SYMBOL_8)
    LORA_SCAN_SFS_LC(433625000U, LORA_CRC_ENABLE, LORA_IQ_NORMAL, 8, LORA_CAD_SYMBOL_8)
    LORA_SCAN_SFS_LC(433875000U, LORA_CRC_ENABLE, LORA_IQ_NORMAL, 8, LORA_CAD_SYMBOL_8)
    LORA_SCAN_SFS_LC(434125000U, LORA_CRC_ENABLE, LORA_IQ_NORMAL, 8, LORA_CAD_SYMBOL_8)
    LORA_SCAN_SFS_LC(434375000U, LORA_CRC_ENABLE, LORA_IQ_NORMAL, 8, LORA_CAD_SYMBOL_8)
    LORA_SCAN_SFS_LC(434625000U, LORA_CRC_ENABLE, LORA_IQ_NORMAL, 8, LORA_CAD_SYMBOL_8)
};
const uint16_t g_lora_llcc68_scan_table_len =
        sizeof(g_lora_llcc68_scan_table) / sizeof(lora_scan_item_t);