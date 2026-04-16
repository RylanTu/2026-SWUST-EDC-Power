/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include <math.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define VOLT_DIV       11.0f	//电压分压倍数
#define PROTECT_TEMP   70.0f	//过温保护70℃
#define PROTECT_CURR   1.2f		//过流1.2A
#define PROTECT_VOLT   12.5f	//过压12.5V
#define PWM_MAX        999		//PWM最大值
#define VOLT_MAX       12.0f	//最大输出电压12V
#define CURR_MAX       1.0f		//最大输出电流1A

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

//全局变量
uint16_t adc_buf[4];
float real_v = 0.0f;	//真实输出电压
float real_i = 0.0f;	//真实输出电流
float real_t = 0.0f;	//板子温度
float real_b = 0.0f;	//电池电压（检测输入电源）
float set_v = 5.0f;		//设定电压
float set_i = 0.5f;		//设定电流
uint8_t mode = 0;			//工作模式（0恒压，1恒流）
uint8_t protect = 0;	//保护标识（0正常，1警告）
uint8_t protect_type = 0; //0正常，1过压，2过流，3过温
char buf[128];

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */


void ADC_Process(void);
void PWM_Out(void);
void Check_Protect(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void ADC_Process(void)
{
  //电压
  real_v = adc_buf[0] * 3.3f / 4095.0f * VOLT_DIV;

  //电流
  real_i = adc_buf[1] * 3.3f / 4095.0f * 1.0f;

  //温度
  float v_ntc = adc_buf[2] * 3.3f / 4095.0f;
  float r_ntc = 10000.0f * v_ntc / (3.3f - v_ntc);
  real_t = 1.0f/(log(r_ntc/10000.0f)/3435.0f + 1.0f/298.15f) - 273.15f;

  //电池输入电压
  real_b = adc_buf[3] * 3.3f / 4095.0f * VOLT_DIV;
}

void Check_Protect(void)
{
  //默认先认为没保护
  protect = 0;
  protect_type = 0;

  //过压
  if(real_v > PROTECT_VOLT)
  {
    protect = 1;
    protect_type = 1;
  }
  //过流
  else if(real_i > PROTECT_CURR)
  {
    protect = 1;
    protect_type = 2;
  }
  //过温
  else if(real_t > PROTECT_TEMP)
  {
    protect = 1;
    protect_type = 3;
  }
}

void PWM_Out(void)
{
  uint16_t pwm;

	//过载保护
  if(protect)
  {
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);
    return;
  }

	
  if(mode == 0)	//恒压模式
  {
    pwm = set_v / VOLT_MAX * PWM_MAX;
  }
  else	//恒流模式
  {
    pwm = set_i / CURR_MAX * PWM_MAX;
  }

	//安全限制（PWM最大值999）
  if(pwm > PWM_MAX) pwm = PWM_MAX;
  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, pwm);
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  uint8_t Key_num;
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */

	HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_buf, 4);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		ADC_Process();
    Check_Protect();
    PWM_Out();
		
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		HAL_Delay(100);
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
