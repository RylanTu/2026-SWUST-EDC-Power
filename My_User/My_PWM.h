#ifndef __My_PWM_H__
#define __My_PWM_H__

#include "MyApplication.h"


typedef enum
{
	Start_State  	= (uint8_t)1,  //开始
	Stop_State  	= (uint8_t)0,  //停止
}Status_t;

//PWM
#define PWM_COUNT_FRE			72000000		
#define PWM_FREQUENCY			100000	   //开关频率
#define PWM_PERIOD_VAL	(PWM_COUNT_FRE / PWM_FREQUENCY - 1)
#define PWM1_PULSE  TIM1->CCR1 
//#define PWM2_PULSE  TIM1->CCR4 

#define PWM_Limit_Max(a,b)		a=(a>b)?(b):(a+0)
#define PWM_Limit_Min(a,b)		a=(a<b)?(b):(a+0)

#define TIM_OCPolarity_High                ((uint16_t)0x0000)
#define TIM_OCPolarity_Low                 ((uint16_t)0x0002)

#define PWM_Polarity_channel1  TIM_CCER_CC1P
#define PWM_Polarity_channel2  TIM_CCER_CC2P
#define PWM_Polarity_channel3  TIM_CCER_CC3P
#define PWM_Polarity_channel4  TIM_CCER_CC4P
typedef struct  
{
	uint16_t halfPeriod;  //半周
	uint16_t period;      //周期
	int16_t limitMin;     //最小极限
	int16_t limitMax;     //最大极限
	uint8_t Status;        //停止标志位
    float PWMDuty;         //占控比
	float  ControlPWM;     //pwm增量  
    void (*PWM_Init)(void);   //PWM初始化
    void (*PWM_Start)(void);  //PWM开始
    void (*PWM_Stop)(void);   //PWM停止
	void (*PWM_Updata)(uint16_t, uint16_t);   //更新PWM
}PWMSet_Type;

extern PWMSet_Type	PWMSET;



#endif /* __KEY__ */