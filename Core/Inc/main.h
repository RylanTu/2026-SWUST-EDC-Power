/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "stm32f1xx_hal.h"

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
#define LED_RUN_Pin GPIO_PIN_13
#define LED_RUN_GPIO_Port GPIOC
#define FAN_Pin GPIO_PIN_0
#define FAN_GPIO_Port GPIOA
#define K2_Pin GPIO_PIN_1
#define K2_GPIO_Port GPIOA
#define K3_Pin GPIO_PIN_2
#define K3_GPIO_Port GPIOA
#define K4_Pin GPIO_PIN_3
#define K4_GPIO_Port GPIOA
#define ADC_NTC_Pin GPIO_PIN_6
#define ADC_NTC_GPIO_Port GPIOA
#define ADC_IOUT_Pin GPIO_PIN_7
#define ADC_IOUT_GPIO_Port GPIOA
#define ADC_VB_Pin GPIO_PIN_0
#define ADC_VB_GPIO_Port GPIOB
#define ADC_VOUT_Pin GPIO_PIN_1
#define ADC_VOUT_GPIO_Port GPIOB
#define TFT_BLK_Pin GPIO_PIN_2
#define TFT_BLK_GPIO_Port GPIOB
#define TFT_CS_Pin GPIO_PIN_10
#define TFT_CS_GPIO_Port GPIOB
#define TFT_DC_Pin GPIO_PIN_12
#define TFT_DC_GPIO_Port GPIOB
#define TFT_SCL_Pin GPIO_PIN_13
#define TFT_SCL_GPIO_Port GPIOB
#define TFT_RES_Pin GPIO_PIN_14
#define TFT_RES_GPIO_Port GPIOB
#define TFT_SDA_Pin GPIO_PIN_15
#define TFT_SDA_GPIO_Port GPIOB
#define K5_Pin GPIO_PIN_4
#define K5_GPIO_Port GPIOB
#define EC11_Pin GPIO_PIN_5
#define EC11_GPIO_Port GPIOB
#define IIC_SCL_Pin GPIO_PIN_8
#define IIC_SCL_GPIO_Port GPIOB
#define IIC_SDA_Pin GPIO_PIN_9
#define IIC_SDA_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
