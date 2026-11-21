//
// Created by 114514 on 2026/11/11.
//

#include "FreeRtosHandler.h"
#include "iwdg.h"
#include "stm32f4xx_hal.h"
#include "cmsis_os.h"
#include "../../common/LoraManager.h"
#include "FreeRTOS.h"
#include "lwip.h"
#include "sockets.h"
#include "task.h"
#include "../../App/UsartHandler/usartHandler.h"
#include "stream_buffer.h"
#include "../../App/Tcp/TcpDataProcess.h"
osThreadId_t WatchDogTaskHandle;
osThreadId_t InitLWIPTaskHandle;
osThreadId_t IrqTaskHandle;
osThreadId_t UsartTaskHandle;
osThreadId_t TcpTaskHandle;
const osThreadAttr_t WatchDogTask_attributes = {
    .name = "WatchDogTask",
    .stack_size = 512 * 4,
    .priority = osPriorityRealtime,
};
const osThreadAttr_t InitLWIPTask_attributes = {
    .name = "InitLWIPTask",
    .stack_size = 1024 * 4,
    .priority = osPriorityAboveNormal,
};
const osThreadAttr_t IrqTask_attributes = {
    .name = "IrqTask",
    .stack_size = 1024 * 4,
    .priority = osPriorityAboveNormal,
};
const osThreadAttr_t UsartTask_attributes = {
    .name = "UsartTask",
    .stack_size = 512 * 4,
    .priority = osPriorityAboveNormal,
};
const osThreadAttr_t TcpTask_attributes = {
    .stack_size = 512 * 4,
    .priority = osPriorityNormal,
    .name = "TcpTask",
};

//ethTcp任务
osThreadId_t EthTcpConnectTaskHandle;
osThreadId_t EthTcpSendTaskHandle;
osThreadId_t EthTcpReceiveTaskHandle;
osThreadId_t EthTcpProcessTaskHandle;
const osThreadAttr_t EthTcpConnectTask_attributes = {
    .name = "EthTcpConnectTask",
    .stack_size = 512 * 4,
    .priority = osPriorityNormal3,
};
const osThreadAttr_t EthTcpSendTask_attributes = {
    .name = "EthTcpSendTask",
    .stack_size = 512 * 4,
    .priority = osPriorityNormal,
};
const osThreadAttr_t EthTcpReceiveTask_attributes = {
    .name = "EthTcpReceiveTask",
    .stack_size = 512 * 4,
    .priority = osPriorityNormal,
};
const osThreadAttr_t EthTcpProcessTask_attributes = {
    .name = "EthTcpProcessTask",
    .stack_size = 1024 * 4,
    .priority = osPriorityAboveNormal,
};



//irq中断任务
osThreadId_t Llcc68IrqTaskHandle;
osThreadId_t Sx1262IrqTaskHandle;
const osThreadAttr_t Sx1262IrqTask_attributes = {
    .name = "Sx1262IrqTask",
    .stack_size = 1024 * 4,
    .priority = osPriorityAboveNormal,
};
const osThreadAttr_t Llcc68IrqTask_attributes = {
    .name = "Llcc68IrqTask",
    .stack_size = 1024 * 4,
    .priority = osPriorityAboveNormal
};


void TcpTask(void *argument);
void WatchDogTask(void *argument);
void InitLWIPTask(void *argument);
void UsartTask(void *argument);
void CreateIrqTask(void *argument);


//lora任务
osThreadId_t LoraInitTaskHandle;
osThreadId_t LoraTestTaskHandle;
void LoraInitTask(void *argument);
void LoraTestTask(void *argument);
const osThreadAttr_t LoraInit_attributes = {
    .name = "LoraInit",
    .stack_size = 512 * 4,
    .priority = osPriorityNormal,
};
const osThreadAttr_t LoraTest_attributes = {
    .name = "LoraTest",
    .stack_size = 1024 * 4,
    .priority = osPriorityNormal,
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
        printf("WATCHDOG CREATE FAILED   free=%d\r\n", (int)xPortGetFreeHeapSize());
    }
    const uint16_t ux_task_get_stack_high_water_mark2 = uxTaskGetStackHighWaterMark2(WatchDogTaskHandle);
    printf("WATCHDOG STACK HIGH WATER MARK: %d\r\n", ux_task_get_stack_high_water_mark2);
}

void CreateInitAllTask() {
    InitLWIPTaskHandle = osThreadNew(InitLWIPTask, NULL, &InitLWIPTask_attributes);
    if (InitLWIPTaskHandle == NULL) {
        printf("INIT LWIP CREATE FAILED   free=%d\r\n", (int)xPortGetFreeHeapSize());
    }
    uint16_t ux_task_get_stack_high_water_mark2 = uxTaskGetStackHighWaterMark2(InitLWIPTaskHandle);
    printf("INIT LWIP STACK HIGH WATER MARK: %d\r\n", ux_task_get_stack_high_water_mark2);

    IrqTaskHandle= osThreadNew(CreateIrqTask, NULL, &IrqTask_attributes);
    if (IrqTaskHandle == NULL) {
        printf("IRQ WRAPPER CREATE FAILED   free=%d\r\n", (int)xPortGetFreeHeapSize());
    }
    ux_task_get_stack_high_water_mark2 = uxTaskGetStackHighWaterMark2(IrqTaskHandle);
    printf("IRQQ STACK HIGH WATER MARK: %d\r\n", ux_task_get_stack_high_water_mark2);

    UsartTaskHandle = osThreadNew(UsartTask, NULL, &UsartTask_attributes);
    if (UsartTaskHandle == NULL) {
        printf("USART CREATE FAILED   free=%d\r\n", (int)xPortGetFreeHeapSize());
    }
    ux_task_get_stack_high_water_mark2 = uxTaskGetStackHighWaterMark2(UsartTaskHandle);
    printf("USART STACK HIGH WATER MARK: %d\r\n", ux_task_get_stack_high_water_mark2);

    TcpTaskHandle = osThreadNew(TcpTask, NULL, &TcpTask_attributes);
    if (TcpTaskHandle == NULL) {
        printf("TCP CREATE FAILED   free=%d\r\n", (int)xPortGetFreeHeapSize());
    }
    ux_task_get_stack_high_water_mark2 = uxTaskGetStackHighWaterMark2(TcpTaskHandle);
    printf("TCP STACK HIGH WATER MARK: %d\r\n", ux_task_get_stack_high_water_mark2);

    LoraInitTaskHandle = osThreadNew(LoraInitTask, NULL, &LoraInit_attributes);
    if (LoraInitTaskHandle == NULL) {
        printf("LORA INIT CREATE FAILED   free=%d\r\n", (int)xPortGetFreeHeapSize());
    }
    ux_task_get_stack_high_water_mark2 = uxTaskGetStackHighWaterMark2(LoraInitTaskHandle);
    printf("LORA INIT STACK HIGH WATER MARK: %d\r\n", ux_task_get_stack_high_water_mark2);


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
    UsartDataProcess();   /* 内部阻塞等信号量 + 非阻塞取数据 */
}
void Llcc68IrqTask(void *argument) {
    for (;;) {
        ulTaskNotifyTake(pdTRUE,pdMS_TO_TICKS(1000));
        LoraInstance * lora_instance = lora_get_llcc68();
        lora_irq_handler(lora_instance);
    }
}

void Sx1262IrqTask(void *argument) {
    for (;;) {
        ulTaskNotifyTake(pdTRUE,pdMS_TO_TICKS(1000));
        LoraInstance * lora_instance = lora_get_sx1262();
        lora_irq_handler(lora_instance);
    }
}

void CreateIrqTask(void *argument) {
    Sx1262IrqTaskHandle = osThreadNew(Sx1262IrqTask,NULL,&Sx1262IrqTask_attributes);
    if (Sx1262IrqTaskHandle == NULL) {
        printf("SX1262 IRQ CREATE FAILED  free=%d\r\n", (int)xPortGetFreeHeapSize());
    }
    uint16_t ux_task_get_stack_high_water_mark2 = uxTaskGetStackHighWaterMark2(Sx1262IrqTaskHandle);
    printf("SX1262 IRQ STACK HIGH WATER MARK: %d\r\n", ux_task_get_stack_high_water_mark2);

    Llcc68IrqTaskHandle = osThreadNew(Llcc68IrqTask,NULL,&Llcc68IrqTask_attributes);
    if (Llcc68IrqTaskHandle == NULL) {
        printf("LLCC68 IRQ CREATE FAILED  free=%d\r\n", (int)xPortGetFreeHeapSize());
    }
    ux_task_get_stack_high_water_mark2 = uxTaskGetStackHighWaterMark2(Llcc68IrqTaskHandle);
    printf("LLCC68 IRQ STACK HIGH WATER MARK: %d\r\n", ux_task_get_stack_high_water_mark2);

    vTaskDelete(NULL);
}

/**
 * 发送任务
 * 接收任务
 * 建立连接任务
 */
void EthTcpReceiveTask(void *argument);
void EthTcpConnectTask(void *argument);
void EthTcpSendTask(void *argument);
void EthTcpProcessTask(void *argument);
void TcpTask(void *argument) {
    /* 先初始化任务间通信组件 (stream/queue) */
    EthTcpComInit();

    EthTcpConnectTaskHandle = osThreadNew(EthTcpConnectTask, NULL, &EthTcpConnectTask_attributes);
    if (EthTcpConnectTaskHandle==NULL) {
        printf("ETH TCP CONNECT TASK CREATE FAILED   free=%d\r\n", (int)xPortGetFreeHeapSize());
    }
    EthTcpReceiveTaskHandle = osThreadNew(EthTcpReceiveTask, NULL, &EthTcpReceiveTask_attributes);
    if (EthTcpReceiveTaskHandle==NULL) {
        printf("ETH TCP RECEIVE TASK CREATE FAILED   free=%d\r\n", (int)xPortGetFreeHeapSize());
    }
    EthTcpProcessTaskHandle = osThreadNew(EthTcpProcessTask, NULL, &EthTcpProcessTask_attributes);
    if (EthTcpProcessTaskHandle==NULL) {
        printf("ETH TCP PROCESS TASK CREATE FAILED   free=%d\r\n", (int)xPortGetFreeHeapSize());
    }
    EthTcpSendTaskHandle = osThreadNew(EthTcpSendTask, NULL, &EthTcpSendTask_attributes);
    if (EthTcpSendTaskHandle==NULL) {
        printf("ETH TCP SEND TASK CREATE FAILED   free=%d\r\n", (int)xPortGetFreeHeapSize());
    }
    vTaskDelete(NULL);
}
void EthTcpReceiveTask(void *argument) {
    EthTcpReceiveTaskLoop();
}
void EthTcpProcessTask(void *argument) {
    EthTcpProcessTaskLoop();
}
void EthTcpConnectTask(void *argument) {
    for (;;) {
        while (!netif_is_link_up(&gnetif)) {
            vTaskDelay(pdMS_TO_TICKS(500));
        }

        if (EthTcpGetSock() < 0) {
            if (EthTcpConnect() == 0) {
                printf("ETH TCP CONNECT SUCCESS\r\n");
            }
        } else {
            vTaskDelay(pdMS_TO_TICKS(500));
        }

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
void EthTcpSendTask(void *argument) {
    EthTcpSendTaskLoop();
}

void LoraInitTask(void *argument) {
    //初始化芯片
    uint8_t lora_init = LoraInit();
    if (lora_init == 0) {
        printf("LORA INIT SUCCESS\r\n");
    } else {
        printf("LORA INIT FAILED\r\n");
    }
    //测试任务
    LoraTestTaskHandle = osThreadNew(LoraTestTask, NULL, &LoraTest_attributes);
    if (LoraTestTaskHandle == NULL) {
        printf("LORA TEST CREATE FAILED   free=%d\r\n", (int)xPortGetFreeHeapSize());
    }
    vTaskDelete(NULL);

}
void LoraTestTask(void *argument) {
    LoraTest();
}