/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LC68_CS_Pin GPIO_PIN_4
#define LC68_CS_GPIO_Port GPIOA
#define LC68_REST_Pin GPIO_PIN_0
#define LC68_REST_GPIO_Port GPIOB
#define LC68_RX_Pin GPIO_PIN_1
#define LC68_RX_GPIO_Port GPIOB
#define LC68_TX_Pin GPIO_PIN_11
#define LC68_TX_GPIO_Port GPIOF
#define LC_IQ_Pin GPIO_PIN_7
#define LC_IQ_GPIO_Port GPIOE
#define LC_IQ_EXTI_IRQn EXTI9_5_IRQn
#define SX_RX_Pin GPIO_PIN_10
#define SX_RX_GPIO_Port GPIOE
#define SX_TX_Pin GPIO_PIN_11
#define SX_TX_GPIO_Port GPIOE
#define SX_REST_Pin GPIO_PIN_14
#define SX_REST_GPIO_Port GPIOE
#define SX_BUSY_Pin GPIO_PIN_14
#define SX_BUSY_GPIO_Port GPIOB
#define USART_TX_Pin GPIO_PIN_6
#define USART_TX_GPIO_Port GPIOC
#define USART_RX_Pin GPIO_PIN_7
#define USART_RX_GPIO_Port GPIOC
#define LC68_BUSY_Pin GPIO_PIN_8
#define LC68_BUSY_GPIO_Port GPIOB
#define SX_CS_Pin GPIO_PIN_9
#define SX_CS_GPIO_Port GPIOB
#define SX_IQ_Pin GPIO_PIN_0
#define SX_IQ_GPIO_Port GPIOE
#define SX_IQ_EXTI_IRQn EXTI0_IRQn

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
