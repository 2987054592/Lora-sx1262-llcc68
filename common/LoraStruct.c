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