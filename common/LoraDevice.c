//
// Created by 114514 on 2026/11/18.
//

#include "LoraDevice.h"

#include <stdio.h>

/* 调试期用:apply_config 里任何一步失败都会打印,静默失败是排查大敌 */
#define CK(x) do { uint8_t _r = (x); if (_r != 0) printf("APPLY FAIL: %s -> %d\r\n", #x, _r); } while (0)

#include "driver_llcc68.h"
#include "driver_llcc68_interface.h"
#include "driver_sx1262.h"
#include "driver_sx1262_interface.h"

uint8_t lora_driver_bind(LoraInstance *inst, void *handle) {
    switch (inst->chip) {
        case LORA_CHIP_SX1262: {
            sx1262_handle_t *h = (sx1262_handle_t *)handle;
            DRIVER_SX1262_LINK_INIT(h, sx1262_handle_t);
            DRIVER_SX1262_LINK_SPI_INIT(h, sx1262_interface_spi_init);
            DRIVER_SX1262_LINK_SPI_DEINIT(h, sx1262_interface_spi_deinit);
            DRIVER_SX1262_LINK_SPI_WRITE_READ(h, sx1262_interface_spi_write_read);
            DRIVER_SX1262_LINK_RESET_GPIO_INIT(h, sx1262_interface_reset_gpio_init);
            DRIVER_SX1262_LINK_RESET_GPIO_DEINIT(h, sx1262_interface_reset_gpio_deinit);
            DRIVER_SX1262_LINK_RESET_GPIO_WRITE(h, sx1262_interface_reset_gpio_write);
            DRIVER_SX1262_LINK_BUSY_GPIO_INIT(h, sx1262_interface_busy_gpio_init);
            DRIVER_SX1262_LINK_BUSY_GPIO_DEINIT(h, sx1262_interface_busy_gpio_deinit);
            DRIVER_SX1262_LINK_BUSY_GPIO_READ(h, sx1262_interface_busy_gpio_read);
            DRIVER_SX1262_LINK_DELAY_MS(h, sx1262_interface_delay_ms);
            DRIVER_SX1262_LINK_DEBUG_PRINT(h, sx1262_interface_debug_print);
            DRIVER_SX1262_LINK_RECEIVE_CALLBACK(h,inst->callBack);
            inst->context = h;
            inst->rx_buf = h->receive_buf;
            if (sx1262_init(h) != 0) {
                return 2;   /* init 失败（如回调未挂 / SPI 链路不通） */
            }
            return 0;
        }
        case LORA_CHIP_LLCC68: {
            llcc68_handle_t *h = (llcc68_handle_t *)handle;
            DRIVER_LLCC68_LINK_INIT(h, llcc68_handle_t);
            DRIVER_LLCC68_LINK_SPI_INIT(h, llcc68_interface_spi_init);
            DRIVER_LLCC68_LINK_SPI_DEINIT(h, llcc68_interface_spi_deinit);
            DRIVER_LLCC68_LINK_SPI_WRITE_READ(h, llcc68_interface_spi_write_read);
            DRIVER_LLCC68_LINK_RESET_GPIO_INIT(h, llcc68_interface_reset_gpio_init);
            DRIVER_LLCC68_LINK_RESET_GPIO_DEINIT(h, llcc68_interface_reset_gpio_deinit);
            DRIVER_LLCC68_LINK_RESET_GPIO_WRITE(h, llcc68_interface_reset_gpio_write);
            DRIVER_LLCC68_LINK_BUSY_GPIO_INIT(h, llcc68_interface_busy_gpio_init);
            DRIVER_LLCC68_LINK_BUSY_GPIO_DEINIT(h, llcc68_interface_busy_gpio_deinit);
            DRIVER_LLCC68_LINK_BUSY_GPIO_READ(h, llcc68_interface_busy_gpio_read);
            DRIVER_LLCC68_LINK_DELAY_MS(h, llcc68_interface_delay_ms);
            DRIVER_LLCC68_LINK_DEBUG_PRINT(h, llcc68_interface_debug_print);
            DRIVER_LLCC68_LINK_RECEIVE_CALLBACK(h,inst->callBack);
            inst->context = h;
            inst->rx_buf = h->receive_buf;
            if (llcc68_init(h) != 0) {
                return 2;   /* init 失败 */
            }
            return 0;
        }
        default:
            return 1;
    }
    return 0;
}

/* 一次性 LoRa 模式配置：init 只做复位自检，这里补齐运行前必须烧写的寄存器。
 * 关键：packet type 必须切 LoRa、DIO1 中断掩码必须打开，否则 cad_done / tx_done 永不置位。 */
uint8_t lora_radio_config(LoraInstance *inst) {
    switch (inst->chip) {
        case LORA_CHIP_SX1262: {
            sx1262_handle_t *h = (sx1262_handle_t *)inst->context;

            sx1262_set_standby(h, SX1262_CLOCK_SOURCE_XTAL_32MHZ);
            sx1262_set_stop_timer_on_preamble(h, SX1262_BOOL_FALSE);
            sx1262_set_regulator_mode(h, SX1262_REGULATOR_MODE_DC_DC_LDO);
            sx1262_set_pa_config(h, 0x02, 0x03);                          /* +17dBm */
            sx1262_set_rx_tx_fallback_mode(h, SX1262_RX_TX_FALLBACK_MODE_STDBY_XOSC);
            sx1262_clear_irq_status(h, 0x03FF);
            /* DIO1 全开：CAD/TX/RX 完成都走同一根 DIO1 进 EXTI */
            sx1262_set_dio_irq_params(h, 0x03FF, 0x03FF, 0x0000, 0x0000);
            if (sx1262_set_packet_type(h, SX1262_PACKET_TYPE_LORA) != 0) {
                return 1;   /* 切 LoRa 模式失败 */
            }
            sx1262_set_tx_params(h, 17, SX1262_RAMP_TIME_10US);           /* 17dBm */
            sx1262_set_buffer_base_address(h, 0x00, 0x00);
            sx1262_set_lora_symb_num_timeout(h, 0);
            sx1262_set_ocp(h, 0x38);                                      /* 140mA */
            return 0;
        }
        case LORA_CHIP_LLCC68: {
            llcc68_handle_t *h = (llcc68_handle_t *)inst->context;

            llcc68_set_standby(h, LLCC68_CLOCK_SOURCE_XTAL_32MHZ);
            llcc68_set_stop_timer_on_preamble(h, LLCC68_BOOL_FALSE);
            llcc68_set_regulator_mode(h, LLCC68_REGULATOR_MODE_DC_DC_LDO);
            llcc68_set_pa_config(h, 0x02, 0x03);                          /* +17dBm */
            llcc68_set_rx_tx_fallback_mode(h, LLCC68_RX_TX_FALLBACK_MODE_STDBY_XOSC);
            llcc68_clear_irq_status(h, 0x03FF);
            llcc68_set_dio_irq_params(h, 0x03FF, 0x03FF, 0x0000, 0x0000);
            if (llcc68_set_packet_type(h, LLCC68_PACKET_TYPE_LORA) != 0) {
                return 1;
            }
            llcc68_set_tx_params(h, 17, LLCC68_RAMP_TIME_10US);           /* 17dBm */
            llcc68_set_buffer_base_address(h, 0x00, 0x00);
            llcc68_set_lora_symb_num_timeout(h, 0);
            llcc68_set_ocp(h, 0x38);                                      /* 140mA */
            return 0;
        }
        default:
            return 1;
    }
    return 0;
}

uint8_t lora_apply_config(LoraInstance *inst) {
    switch (inst->chip) {
        case LORA_CHIP_SX1262: {
            sx1262_handle_t *h = (sx1262_handle_t *)inst->context;
            uint32_t frf_reg;

            CK(sx1262_set_lora_modulation_params(h,inst->mod.sf,inst->mod.bw,inst->mod.cr,inst->mod.ldro));
            CK(sx1262_set_lora_packet_params(h,
                inst->pkt.preamble_len,
                (sx1262_lora_header_t)inst->pkt.header,
                inst->pkt.payload_len,
                (sx1262_lora_crc_type_t)inst->pkt.crc,
                (sx1262_bool_t)inst->pkt.iq));

            /* CAD 参数 */
            CK(sx1262_set_cad_params(h,
                (sx1262_lora_cad_symbol_num_t)inst->cad.symbol_num,
                inst->cad.det_peak,
                inst->cad.det_min,
                (sx1262_lora_cad_exit_mode_t)inst->cad.exit_mode,
                inst->cad.timeout));

            CK(sx1262_set_tx_params(h, 0, SX1262_RAMP_TIME_10US));
            CK(sx1262_frequency_convert_to_register(h,inst->rf.freq_hz,&frf_reg));
            CK(sx1262_set_rf_frequency(h, frf_reg));
            CK(sx1262_set_lora_sync_word(h, inst->rf.sync_word));
            CK(sx1262_set_rx_gain(h, inst->rf.rx_gain));
            return 0;
        }
        case LORA_CHIP_LLCC68: {
            llcc68_handle_t *hs = (llcc68_handle_t *)inst->context;
            uint32_t frf_regs;

            CK(llcc68_set_lora_modulation_params(hs,inst->mod.sf,inst->mod.bw,inst->mod.cr,inst->mod.ldro));
            CK(llcc68_set_lora_packet_params(hs,
                inst->pkt.preamble_len,
                (llcc68_lora_header_t)inst->pkt.header,
                inst->pkt.payload_len,
                (llcc68_lora_crc_type_t)inst->pkt.crc,
                (llcc68_bool_t)inst->pkt.iq));

            /* CAD 参数 */
            CK(llcc68_set_cad_params(hs,
                (llcc68_lora_cad_symbol_num_t)inst->cad.symbol_num,
                inst->cad.det_peak,
                inst->cad.det_min,
                (llcc68_lora_cad_exit_mode_t)inst->cad.exit_mode,
                inst->cad.timeout));

            CK(llcc68_set_tx_params(hs, 0, LLCC68_RAMP_TIME_10US));

            CK(llcc68_frequency_convert_to_register(hs,inst->rf.freq_hz,&frf_regs));
            CK(llcc68_set_rf_frequency(hs, frf_regs));
            CK(llcc68_set_lora_sync_word(hs, inst->rf.sync_word));
            CK(llcc68_set_rx_gain(hs, inst->rf.rx_gain));
            return 0;
        }
    }
    return 1;
}
/* 把扫描表一项灌入实例（只改该动的字段，CAD 的 peak/min/exit 保持默认） */
void lora_load_scan_item(LoraInstance *inst, const lora_scan_item_t *item) {
    inst->mod.sf = item->sf;
    inst->mod.bw = item->bw;
    inst->mod.cr = item->cr;
    inst->pkt.crc = item->crc;
    inst->pkt.iq = item->iq;
    inst->pkt.payload_len = item->payload_len;
    inst->cad.symbol_num = item->cad_sym;
    inst->rf.freq_hz = item->freq_hz;
}
uint8_t lora_start_cad(LoraInstance *inst) {
    inst->pkt.iq = LORA_IQ_NORMAL;        /* 强制非翻转，参考 lora_common.c L460-466 */
    if (lora_apply_config(inst) != 0) {
        return 1;
    }
    switch (inst->chip) {
        case LORA_CHIP_SX1262: {
            sx1262_handle_t *h = (sx1262_handle_t *)inst->context;
            sx1262_clear_irq_status(h,0x03FF);
            h->cad_done = 0;
            h->cad_detected = 0;
            if (sx1262_set_cad(h) != 0) {
                return 1;
            }
            break;
        }
        case LORA_CHIP_LLCC68: {
            llcc68_handle_t *hs = (llcc68_handle_t *)inst->context;
            llcc68_clear_irq_status(hs,0x03FF);
            hs->cad_done = 0;
            hs->cad_detected = 0;
            if (llcc68_set_cad(hs) != 0) {
                return 1;
            }
            break;
        }
        default:
            return 1;
    }
    return 0;
}

uint8_t lora_start_rx(LoraInstance *inst) {
    if (lora_apply_config(inst) != 0) {
        return 1;
    }
    switch (inst->chip) {
        case LORA_CHIP_SX1262: {
            sx1262_handle_t *h = (sx1262_handle_t *)inst->context;
            HAL_GPIO_WritePin(SX_RX_GPIO_Port, SX_RX_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(SX_TX_GPIO_Port, SX_TX_Pin, GPIO_PIN_RESET);
            h->timeout = 0;
            h->crc_error = 0;
            if (sx1262_continuous_receive(h) !=0) {
                return 1;
            }
            break;
        }
        case LORA_CHIP_LLCC68: {
            llcc68_handle_t *hs = (llcc68_handle_t *)inst->context;
            HAL_GPIO_WritePin(LC68_RX_GPIO_Port, LC68_RX_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(LC68_TX_GPIO_Port, LC68_TX_Pin, GPIO_PIN_RESET);
            hs->timeout = 0;
            hs->crc_error = 0;
            if (llcc68_continuous_receive(hs) !=0) {
                return 1;
            }
            break;
        }
        default:
            return 1;
    }
    return 0;
}

uint8_t lora_send(LoraInstance *inst, const uint8_t *buf, uint16_t len) {
    if (lora_apply_config(inst) != 0) {
        return 1;
    }
    switch (inst->chip) {
        case LORA_CHIP_SX1262: {
            sx1262_handle_t *h = (sx1262_handle_t *)inst->context;
            HAL_GPIO_WritePin(SX_TX_GPIO_Port, SX_TX_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(SX_RX_GPIO_Port, SX_RX_Pin, GPIO_PIN_RESET);
            h->tx_done = 0;
            if (sx1262_write_buffer(h, 0, (uint8_t *)buf, len) != 0) {
                return 1;
            }
            if (sx1262_set_tx(h, 0) != 0) {
                return 1;
            }
            break;
        }
        case LORA_CHIP_LLCC68: {
            llcc68_handle_t *hs = (llcc68_handle_t *)inst->context;
            HAL_GPIO_WritePin(LC68_TX_GPIO_Port, LC68_TX_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(LC68_RX_GPIO_Port, LC68_RX_Pin, GPIO_PIN_RESET);
            hs->tx_done = 0;
            if (llcc68_write_buffer(hs, 0, (uint8_t *)buf, len) != 0) {
                return 1;
            }
            if (llcc68_set_tx(hs, 0) != 0) {
                return 1;
            }
            break;
        }
        default:
            return 1;
    }
    return 0;
}

uint8_t lora_irq_handler(LoraInstance *inst) {
    switch (inst->chip) {
        case LORA_CHIP_SX1262: {
            sx1262_handle_t *h = (sx1262_handle_t *)inst->context;
            if (sx1262_irq_handler(h)!=0) {
                return 1;
            }
            break;
        }

        case LORA_CHIP_LLCC68: {
            llcc68_handle_t *hs = (llcc68_handle_t *)inst->context;
            if (llcc68_irq_handler(hs)!=0) {
                return 1;
            }
            break;
        }

    }
    return 0;
}

uint8_t lora_set_idle(LoraInstance *inst) {
    switch (inst->chip) {
        case LORA_CHIP_SX1262:
            return (sx1262_set_standby((sx1262_handle_t *)inst->context,
                                       SX1262_CLOCK_SOURCE_XTAL_32MHZ) == 0) ? 0 : 1;
        case LORA_CHIP_LLCC68:
            return (llcc68_set_standby((llcc68_handle_t *)inst->context,
                                       LLCC68_CLOCK_SOURCE_XTAL_32MHZ) == 0) ? 0 : 1;
        default:
            return 1;
    }
}

int16_t lora_get_rssi(LoraInstance *inst) {
    uint8_t rssi_raw, signal_raw;
    int8_t snr_raw;
    float rssi, snr, signal;

    switch (inst->chip) {
        case LORA_CHIP_SX1262:
            if (sx1262_get_lora_packet_status((sx1262_handle_t *)inst->context,
                                              &rssi_raw, &snr_raw, &signal_raw,
                                              &rssi, &snr, &signal) != 0) {
                return INT16_MIN;   /* 失败返回极值 */
            }
            return (int16_t)rssi;
        case LORA_CHIP_LLCC68:
            if (llcc68_get_lora_packet_status((llcc68_handle_t *)inst->context,
                                              &rssi_raw, &snr_raw, &signal_raw,
                                              &rssi, &snr, &signal) != 0) {
                return INT16_MIN;
            }
            return (int16_t)rssi;
        default:
            return INT16_MIN;
    }
}