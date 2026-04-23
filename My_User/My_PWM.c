/* Includes ------------------------------------------------------------------*/
#include "My_PWM.h"

/* Private define-------------------------------------------------------------*/

/* Private variables----------------------------------------------------------*/

/* Private function prototypes------------------------------------------------*/
static void PWM_Init(void);
static void PWM_Stop(void);
static void PWM_Start(void);
static void PWM_Updata(uint16_t Duty_CV,uint16_t Duty_CC);

//初始为0
PWMSet_Type  PWMSET =
 {
	0,					
	0,					
	0,					
	0,					
	0,					
	0.0f,				
	0.0f,				  
	PWM_Init,			
	PWM_Start,			
	PWM_Stop,			
	PWM_Updata			 
 };

static void PWM_Init(void)
{
	//HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_1); // TIM1通道1输出PWM
	//HAL_TIMEx_PWMN_Start(&htim1,TIM_CHANNEL_1);  // TIM1通道1互补PWM输出
 	//HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_4);
    
 	PWMSET.PWM_Stop();    //停止PWM输出
	PWMSET.period = PWM_PERIOD_VAL;       //周期 1439
	PWMSET.halfPeriod = PWMSET.period >> 1;   //半周期 719
    
    //设置最大限制值(周期的95%)和最小限制值(周期的1%)
	PWMSET.limitMax = 0.95f * PWMSET.period;  //1367
	PWMSET.limitMin = 0;                      //允许占空比降到0//2026.4.16 RylanTu:不确定改到0会不会炸
    
 	PWMSET.Status = Stop_State;   //将初始状态设置为停止状态
}
static void PWM_Start(void)
{

	PWMSET.Status = Start_State;    //设置为启动状态
	HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_4);
	//__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_1,CV_Duty);  //CV
	//__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_4,CC_Duty);  //CC
	//printf("CV_Duty:%d\r\n\r\n",CV_Duty); 
	//printf("CC_Duty:%d\r\n\r\n",CC_Duty); 
}

static void PWM_Stop(void)
{
	PWMSET.Status = Stop_State;   //设置为停止状态
	HAL_TIM_PWM_Stop(&htim1,TIM_CHANNEL_1);
	HAL_TIM_PWM_Stop(&htim1,TIM_CHANNEL_4);
	__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_1,0);
	__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_4,0);
}


//CC_Duty=(float)((((Iout_val*0.025)/0.75+(Iout_val*0.0908*3))/3.31)*1440);
//CC_Duty=(float)((((Iout_val*0.0254)/0.75+((Iout_val*0.0254)/0.75/4*30))/3.3)*1440); //电流环占空比估算（参数待标定）
static void PWM_Updata( uint16_t Duty_CV , uint16_t Duty_CC)
{
	int16_t CV_duty=0,CC_duty=0;
	
	if(PWMSET.Status == Start_State)  //仅在PWM启动时更新占空比
	{
		if((int16_t)Duty_CV<0)
		{
			CV_duty = 0;
		}
		else
		{
			CV_duty = Duty_CV;
		}
		if((int16_t)Duty_CC<0)
		{
			CC_duty = 0;
		}
		else
		{
			CC_duty = Duty_CC;
		}

		PWM_Limit_Max(CV_duty, PWMSET.limitMax);
		PWM_Limit_Min(CV_duty, PWMSET.limitMin);
		PWM_Limit_Max(CC_duty, PWMSET.limitMax);
		PWM_Limit_Min(CC_duty, PWMSET.limitMin);

		__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_1,CV_duty);
		__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_4,CC_duty);
	}	
}
