//
// Created by 114514 on 2026/11/19.
//

#include "LoraManager.h"

#include "cmsis_os2.h"
#include "driver_llcc68.h"
#include "driver_sx1262.h"
#include "spi.h"

extern osThreadId_t Sx1262IrqTaskHandle;
extern osThreadId_t Llcc68IrqTaskHandle;
static sx1262_handle_t s_sx1262_h;
static llcc68_handle_t s_llcc68_h;

static LoraInstance s_sx1262_inst;
static LoraInstance s_llcc68_inst;

/* 各自独立回调：驱动 receive_callback 不带实例参数，必须按芯片分开 */
static void lora_llcc68_event(uint16_t type, uint8_t *buf, uint16_t len) {
    if (type == LLCC68_IRQ_RX_DONE) {
        uint8_t r1, r2, r3; float a, b, c;
        llcc68_get_lora_packet_status(lora_get_llcc68()->context, &r1, &r2, &r3, &a, &b, &c);
        printf("LLCC68 RX len=%u rssi=%d snr=%d\r\n", len, -(r1 >> 1), (int8_t)r2 / 4);
        if (len > 0) printf("data: %.*s\r\n", len, buf);
        lora_start_rx(lora_get_llcc68());
    }
}

static void lora_sx1262_event(uint16_t type, uint8_t *buf, uint16_t len) {
    if (type == SX1262_IRQ_RX_DONE) {
        uint8_t r1, r2, r3; float a, b, c;
        sx1262_get_lora_packet_status(lora_get_sx1262()->context, &r1, &r2, &r3, &a, &b, &c);
        printf("SX1262 RX len=%u rssi=%d snr=%d\r\n", len, -(r1 >> 1), (int8_t)r2 / 4);
        if (len > 0) printf("data: %.*s\r\n", len, buf);
        lora_start_rx(lora_get_sx1262());
    }
}



LoraInstance *lora_get_sx1262(void) {
    return &s_sx1262_inst;
}

LoraInstance *lora_get_llcc68(void) {
    return &s_llcc68_inst;
}

/* EXTI 引脚 -> 对应芯片 IRQ 处理。SX_IQ / LC_IQ 定义在 main.h */
void lora_irq_dispatch(uint16_t GPIO_Pin) {
    BaseType_t xHigherPriorityTaskWoken  = pdFALSE;
    if (GPIO_Pin == SX_IQ_Pin) {
        vTaskNotifyGiveFromISR(Sx1262IrqTaskHandle,&xHigherPriorityTaskWoken);
    }else if (GPIO_Pin == LC_IQ_Pin) {
        vTaskNotifyGiveFromISR(Llcc68IrqTaskHandle,&xHigherPriorityTaskWoken);
    }
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/* 覆盖 HAL 弱回调：HAL_GPIO_EXTI_IRQHandler 会回这里，转发到驱动侧
 * sx1262_irq_handler 置位 cad_done/tx_done 等标志；已是中断上下文，勿加 printf */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    lora_irq_dispatch(GPIO_Pin);
}

uint8_t LoraInit(void) {
    /* ---------- SX1262 ---------- */
    s_sx1262_inst.chip = LORA_CHIP_SX1262;
    s_sx1262_inst.callBack = lora_sx1262_event;
    if (lora_driver_bind(&s_sx1262_inst, &s_sx1262_h) != 0) {
        return 1;
    }
    if (lora_radio_config(&s_sx1262_inst) != 0) {
        return 2;
    }

    /* ---------- LLCC68 ---------- */
    s_llcc68_inst.chip = LORA_CHIP_LLCC68;
    s_llcc68_inst.callBack = lora_llcc68_event;
    if (lora_driver_bind(&s_llcc68_inst, &s_llcc68_h) != 0) {
        return 3;
    }
    if (lora_radio_config(&s_llcc68_inst) != 0) {
        return 4;
    }

    return 0;
}
void loraTestSetup(LoraInstance *inst,const lora_scan_item_t *table,uint16_t idx) {
    lora_load_scan_item(inst,&table[idx]);

    inst->mod.ldro         = LORA_LDRO_DISABLE;   /* SF7/125K 符号时间 ~1ms，不需要低速优化 */
    inst->pkt.preamble_len = 8;
    inst->pkt.header       = LORA_HEADER_EXPLICIT;
    inst->rf.sync_word     = 0x3444;
    inst->rf.rx_gain       = 0x94;
    inst->cad.det_peak     = 10;                  /* 只收发不用 CAD，给个常规值即可 */
    inst->cad.det_min      = 10;
    inst->cad.exit_mode    = LORA_CAD_EXIT_ONLY;
    inst->cad.timeout      = 0;
}
void LoraTest(void) {
    loraTestSetup(&s_llcc68_inst, g_lora_llcc68_scan_table, 0);
    loraTestSetup(&s_sx1262_inst, g_lora_sx1262_scan_table, 0);

    /* SX1262(SPI2):接收 */
    lora_set_idle(&s_sx1262_inst);
    lora_apply_config(&s_sx1262_inst);
    if (lora_start_rx(&s_sx1262_inst) != 0) {
        printf("SX1262 start rx failed\r\n");
    }

    /* LLCC68(SPI1):发射 */
    lora_set_idle(&s_llcc68_inst);
    lora_apply_config(&s_llcc68_inst);

    llcc68_handle_t *hl = (llcc68_handle_t *)s_llcc68_inst.context;
    uint8_t msg[] = "hello-lo";
    for (;;) {
        hl->tx_done = 0;
        if (lora_send(&s_llcc68_inst, msg, sizeof(msg) - 1) != 0) {
            printf("LLCC68 send failed\r\n");
        }
        for (int i = 0; i < 500 && hl->tx_done == 0; i++) vTaskDelay(1);
        printf("LLCC68 TX done=%u %s\r\n", (unsigned)hl->tx_done, hl->tx_done ? "OK" : "TIMEOUT");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
