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
osThreadId_t WatchDogTaskHandle;
osThreadId_t InitLWIPTaskHandle;
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

void CreateWatchDogTask() {
    WatchDogTaskHandle = osThreadNew(WatchDogTask, NULL, &WatchDogTask_attributes);
    if (WatchDogTaskHandle == NULL) {
        printf("WATCHDOG CREATE FAILED\r\n");
    }
    printf("WATCHDOG CREATE SUCCESS\r\n");
}

void CreateInitAllTask() {
    InitLWIPTaskHandle = osThreadNew(InitLWIPTask, NULL, &InitLWIPTask_attributes);
    if (InitLWIPTaskHandle == NULL) {
        printf("INIT LWIP CREATE FAILED\r\n");
    }
    printf("INIT LWIP CREATE SUCCESS\r\n");
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
