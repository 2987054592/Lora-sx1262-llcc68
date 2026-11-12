//
// Created by 114514 on 2026/11/11.
//

#include "FreeRtosHandler.h"
#include "iwdg.h"
#include "stm32f4xx_hal.h"
#include "cmsis_os.h"
#include "FreeRTOS.h"
#include "lwip.h"
#include "task.h"
#include "../../App/UsartHandler/usartHandler.h"
#include "stream_buffer.h"
osThreadId_t WatchDogTaskHandle;
osThreadId_t InitLWIPTaskHandle;
osThreadId_t IrqTaskHandle;
osThreadId_t UsartTaskHandle;
const osThreadAttr_t WatchDogTask_attributes = {
    .name = "WatchDogTask",
    .stack_size = 128 * 4,
    .priority = osPriorityRealtime,
};
const osThreadAttr_t InitLWIPTask_attributes = {
    .name = "InitLWIPTask",
    .stack_size = 1024 * 4,
    .priority = osPriorityAboveNormal,
};
const osThreadAttr_t IrqTask_attributes = {
    .name = "IrqTask",
    .stack_size = 128 * 4,
    .priority = osPriorityAboveNormal,
};
const osThreadAttr_t UsartTask_attributes = {
    .name = "UsartTask",
    .stack_size = 512 * 4,
    .priority = osPriorityNormal,
};

//irq中断任务
osThreadId_t Llcc68IrqTaskHandle;
osThreadId_t Sx1262IrqTaskHandle;
const osThreadAttr_t Sx1262IrqTask_attributes = {
    .name = "Sx1262IrqTask",
    .stack_size = 128 * 4,
    .priority = osPriorityNormal,
};
const osThreadAttr_t Llcc68IrqTask_attributes = {
    .name = "Llcc68IrqTask",
    .stack_size = 128 * 4,
    .priority = osPriorityNormal
};




/**
 * irq中断任务（两个lora芯片的中断）
 * 看门狗任务
 * 初始化lora芯片的
 *
 */
void CreateWatchDogTask() {
    WatchDogTaskHandle = osThreadNew(WatchDogTask, NULL, &WatchDogTask_attributes);
    if (WatchDogTaskHandle == NULL) {
        printf("WATCHDOG CREATE FAILED\r\n");
    }
    const uint16_t ux_task_get_stack_high_water_mark2 = uxTaskGetStackHighWaterMark2(WatchDogTaskHandle);
    printf("WATCHDOG STACK HIGH WATER MARK: %d\r\n", ux_task_get_stack_high_water_mark2);
}

void CreateInitAllTask() {
    InitLWIPTaskHandle = osThreadNew(InitLWIPTask, NULL, &InitLWIPTask_attributes);
    if (InitLWIPTaskHandle == NULL) {
        printf("INIT LWIP CREATE FAILED\r\n");
    }
    uint16_t ux_task_get_stack_high_water_mark2 = uxTaskGetStackHighWaterMark2(InitLWIPTaskHandle);
    printf("INIT LWIP STACK HIGH WATER MARK: %d\r\n", ux_task_get_stack_high_water_mark2);

    IrqTaskHandle= osThreadNew(CreateIrqTask, NULL, &IrqTask_attributes);
    if (IrqTaskHandle == NULL) {
        printf("IRQQ CREATE FAILED\r\n");
    }
    ux_task_get_stack_high_water_mark2 = uxTaskGetStackHighWaterMark2(IrqTaskHandle);
    printf("IRQQ STACK HIGH WATER MARK: %d\r\n", ux_task_get_stack_high_water_mark2);

    UsartTaskHandle = osThreadNew(UsartTask, NULL, &UsartTask_attributes);
    if (UsartTaskHandle == NULL) {
        printf("USART IRQ CREATE FAILED\r\n");
    }
    ux_task_get_stack_high_water_mark2 = uxTaskGetStackHighWaterMark2(UsartTaskHandle);
    printf("USART STACK HIGH WATER MARK: %d\r\n", ux_task_get_stack_high_water_mark2);


    vTaskDelete(NULL);
}

void WatchDogTask(void *argument) {
    for(;;) {
        HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
        HAL_IWDG_Refresh(&hiwdg);
        vTaskDelay(500);
    }
}

void InitLWIPTask(void *argument) {
    MX_LWIP_Init();
    vTaskDelete(NULL);
}

void UsartTask(void *argument) {
    InitUsart();
    while (1) {
        UsartDataProcess(50);
        vTaskDelay(500);
    }
}
void Llcc68IrqTask(void *argument) {
}

void Sx1262IrqTask(void *argument) {
}

void CreateIrqTask(void *argument) {
    Sx1262IrqTaskHandle = osThreadNew(Sx1262IrqTask,NULL,&Sx1262IrqTask_attributes);
    if (Sx1262IrqTaskHandle == NULL) {
        printf("SX1262 IRQ CREATE FAILED\r\n");
    }
    uint16_t ux_task_get_stack_high_water_mark2 = uxTaskGetStackHighWaterMark2(Sx1262IrqTaskHandle);
    printf("SX1262 IRQ STACK HIGH WATER MARK: %d\r\n", ux_task_get_stack_high_water_mark2);

    Llcc68IrqTaskHandle = osThreadNew(Llcc68IrqTask,NULL,&Llcc68IrqTask_attributes);
    if (Llcc68IrqTaskHandle == NULL) {
        printf("LLCC68 IRQ CREATE FAILED\r\n");
    }
    ux_task_get_stack_high_water_mark2 = uxTaskGetStackHighWaterMark2(Llcc68IrqTaskHandle);
    printf("LLCC68 IRQ STACK HIGH WATER MARK: %d\r\n", ux_task_get_stack_high_water_mark2);

    vTaskDelete(NULL);
}

