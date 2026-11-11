//
// Created by 114514 on 2026/11/10.
//

#ifndef LORACOPY_LORASTRUCT_H
#define LORACOPY_LORASTRUCT_H
#include "stm32f4xx_hal.h"

typedef struct __SPI_HandleTypeDef SPI_HandleTypeDef;
typedef struct {
    GPIO_TypeDef *Gpio; //GPIO端口
    uint16_t pin;  //Pin引脚
}GpioPin;

typedef struct {
    SPI_HandleTypeDef* spi; //spi句柄
    GpioPin cs;
    GpioPin rest;
    GpioPin busy;
        

}LoraConfig;
#endif //LORACOPY_LORASTRUCT_H
