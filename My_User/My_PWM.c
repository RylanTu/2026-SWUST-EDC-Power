/* Includes ------------------------------------------------------------------*/
#include "My_PWM.h"

/* Private define-------------------------------------------------------------*/

/* Private variables----------------------------------------------------------*/

/* Private function prototypes------------------------------------------------*/
static void PWM_Init(void);
static void PWM_Stop(void);
static void PWM_Start(void);
static void PWM_Updata(uint16_t Duty_CV,uint16_t Duty_CC);
static void PWM_SetPinsToAfPp(void);
static void PWM_SetPinsToGpioLow(void);

#define PWM_SLEW_STEP_UP    12U
#define PWM_SLEW_STEP_DOWN  20U

static uint16_t pwm_cv_shadow = 0U;
static uint16_t pwm_cc_shadow = 0U;

static void PWM_SetPinsToAfPp(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	__HAL_RCC_GPIOA_CLK_ENABLE();
	GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_11;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

static void PWM_SetPinsToGpioLow(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	__HAL_RCC_GPIOA_CLK_ENABLE();
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8 | GPIO_PIN_11, GPIO_PIN_RESET);
	GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_11;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8 | GPIO_PIN_11, GPIO_PIN_RESET);
}

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
    
    //设置最大限制值(周期的98%)和最小限制值(周期的98%)和最小限制值(周期的1%)
	PWMSET.limitMax = 0.98f * PWMSET.period;  //1438
	PWMSET.limitMin = 0;                      //允许占空比降到0//2026.4.16 RylanTu:不确定改到0会不会炸
    
 	PWMSET.Status = Stop_State;   //将初始状态设置为停止状态
}
static void PWM_Start(void)
{
	PWM_SetPinsToAfPp();

	/* 先将比较值拉到0，再启动输出，避免启动瞬间沿用历史占空导致尖峰。 */
	__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_1,0U);
	__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_4,0U);
	pwm_cv_shadow = 0U;
	pwm_cc_shadow = 0U;

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
	PWM_SetPinsToGpioLow();
	pwm_cv_shadow = 0U;
	pwm_cc_shadow = 0U;
}


//CC_Duty=(float)((((Iout_val*0.025)/0.75+(Iout_val*0.0908*3))/3.31)*1440);
//CC_Duty=(float)((((Iout_val*0.0254)/0.75+((Iout_val*0.0254)/0.75/4*30))/3.3)*1440); //电流环占空比估算（参数待标定）
static void PWM_Updata( uint16_t Duty_CV , uint16_t Duty_CC)
{
	int16_t CV_duty=0,CC_duty=0;
	uint16_t target_cv;
	uint16_t target_cc;
	
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

		target_cv = (uint16_t)CV_duty;
		target_cc = (uint16_t)CC_duty;

		/* 软斜坡：抑制设定值突变带来的PWM跳变，减少上电/切换时峰值与纹波冲击。 */
		if(target_cv > pwm_cv_shadow)
		{
			uint16_t delta = (uint16_t)(target_cv - pwm_cv_shadow);
			pwm_cv_shadow += (delta > PWM_SLEW_STEP_UP) ? PWM_SLEW_STEP_UP : delta;
		}
		else
		{
			uint16_t delta = (uint16_t)(pwm_cv_shadow - target_cv);
			pwm_cv_shadow -= (delta > PWM_SLEW_STEP_DOWN) ? PWM_SLEW_STEP_DOWN : delta;
		}

		if(target_cc > pwm_cc_shadow)
		{
			uint16_t delta = (uint16_t)(target_cc - pwm_cc_shadow);
			pwm_cc_shadow += (delta > PWM_SLEW_STEP_UP) ? PWM_SLEW_STEP_UP : delta;
		}
		else
		{
			uint16_t delta = (uint16_t)(pwm_cc_shadow - target_cc);
			pwm_cc_shadow -= (delta > PWM_SLEW_STEP_DOWN) ? PWM_SLEW_STEP_DOWN : delta;
		}

		__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_1,pwm_cv_shadow);
		__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_4,pwm_cc_shadow);
	}	
}
