/* Includes ------------------------------------------------------------------*/
#include "MyApplication.h"

/* Private define-------------------------------------------------------------*/

/* Private variables----------------------------------------------------------*/

/* Private function prototypes------------------------------------------------*/      
static void Timer2_Start_IT(void);  //定时器2以中断模式启动
	
/* Public variables-----------------------------------------------------------*/
My_Timer2_t  My_Timer2 = 
{
	0,
	0,	
	Timer2_Start_IT     
};

/*
	* @name   Timer2_Start_IT
	* @brief  定时器2以中断模式启动
	* @param  None
	* @retval None      
*/
static void Timer2_Start_IT(void)
{
	HAL_TIM_Base_Start_IT(&htim2); //启动定时器2
}
/********************************************************
  End Of File
********************************************************/
