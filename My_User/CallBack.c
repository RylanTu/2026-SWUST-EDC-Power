/* Includes ------------------------------------------------------------------*/
#include "MyApplication.h"

/* Private define-------------------------------------------------------------*/

/* Private variables----------------------------------------------------------*/

/* Public variables-----------------------------------------------------------*/

/* Private function prototypes------------------------------------------------*/      
/* Private function prototypes------------------------------------------------*/      
/******************************************************************
*函数名称:	HAL_GPIO_EXTI_Callback   外部中断回调函数
*函数功能:	按键中断
*函数参数:	按键值
*返 回 值:	无
*******************************************************************/
/******************************************************************
*函数名称:	HAL_TIM_PeriodElapsedCallback   定时器中断回调函数
*函数功能:	
*函数参数:	定时器4
*返 回 值:	无
*******************************************************************/
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)    //定时器4
{
     if(htim->Instance == htim4.Instance)
     {
          if(__HAL_TIM_IS_TIM_COUNTING_DOWN(&htim4)==1)  //向下记数 = 逆时针 = Reverse_State = 减
          {	          
			//Function_SET.Encoder_Direction_Adjust(Reverse_State); //逆时针
               Function_SET.Encoder_State=Reverse_State;
               //printf(" The Encoder_B button is pressed!\r\n\r\n");
               //__HAL_TIM_SET_COUNTER(&htim4,0);            
          }
          else if(__HAL_TIM_IS_TIM_COUNTING_DOWN(&htim4)==0)   //向上记数 = 顺时针 = Forward_State = 加
          {
               //Function_SET.Encoder_Direction_Adjust(Forward_State); //顺时针
               Function_SET.Encoder_State=Forward_State;
               //printf(" The Encoder_A button is pressed!\r\n\r\n");
               //__HAL_TIM_SET_COUNTER(&htim4,0);           
          }   
           
     }
}


/******************************************************************
*函数名称:	HAL_TIM_PeriodElapsedCallback   定时器中断回调函数
*函数功能:	ADC采集  执行控制任务  按键扫描  
*函数参数:	定时器3  定时器2       定时器4
*返 回 值:	无
*******************************************************************/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)    //定时器2     定时器3-这里不使用
{
 
	if(htim->Instance == htim2.Instance)    //5ms
	{
          Key_Tick();                     //按键扫描
          My_Timer2.usDelay_Timer++;     //延时
          MyADC.ADC_GetNewSample ();       //ADC采集一次        
          if(++My_Timer2.usMCU_Run_Timer>=TIMER2_10mS)  //10mS秒钟 
          {
               KEY_ALL_Detect();   //按键检测作用
               Function_SET.OUT_VAL_Ctrl(); //输出电压电流调整
               My_Timer2.usMCU_Run_Timer=0;   
          }  
	}
     
}

