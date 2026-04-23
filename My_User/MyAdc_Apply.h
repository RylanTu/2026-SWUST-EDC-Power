#ifndef __MyAdc_Apply_H__
#define __MyAdc_Apply_H__

#include "MyApplication.h"
//宏定义
# define  PW_ADC_SAMPLE_LEN 8   //ADC样本空间长度
# define  CC_HYS_ENTER_PCT 1.0f //进入CC阈值百分比
# define  CC_HYS_EXIT_PCT  1.0f //退出CC阈值百分比
//定义枚举类型
 
//定义结构体类型
typedef struct
{
   volatile uint16_t ADC_ConverValue[4];
   uint16_t Nin [PW_ADC_SAMPLE_LEN]; 
 	 uint16_t Iout[PW_ADC_SAMPLE_LEN];
	 uint16_t Vin [PW_ADC_SAMPLE_LEN];
   uint16_t Vout[PW_ADC_SAMPLE_LEN];

	 float Ni;
	 float Io;
	 float Vi;
   float Vo;
   void  (*ADC_Initial_Setup)(void); //ADC初始化设置
   void  (*ADC_GetNewSample)(void);   // ADC采集值

} MyADC_t;
/* extern variables-----------------------------------------------------------*/
extern MyADC_t  MyADC;


/* extern function prototypes-------------------------------------------------*/

#endif
/********************************************************
  End Of File
********************************************************/