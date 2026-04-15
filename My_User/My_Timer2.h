#ifndef __My_Timer2_H__
#define __My_Timer2_H__

#include "MyApplication.h"

//定义枚举类型
typedef enum
{
	TIMER2_10mS  	= (uint16_t)2,
	TIMER2_20mS  	= (uint16_t)4,
	TIMER2_50mS  	= (uint16_t)10,
	TIMER2_100mS	= (uint16_t)20,
	TIMER2_200mS	= (uint16_t)40,
	TIMER2_500mS	= (uint16_t)100,
	TIMER2_1S     = (uint16_t)200,
	TIMER2_2S     = (uint16_t)400,
	TIMER2_3S     = (uint16_t)600,
	TIMER2_5S     = (uint16_t)1000,
	TIMER2_10S    = (uint16_t)2000,
	TIMER2_30S    = (uint16_t)6000,
	TIMER2_3min   = (uint16_t)36000,
}TIMER2_Value_t;

//定义结构体类垿
typedef struct
{
    uint16_t volatile usMCU_Run_Timer;  //系统运迌定时器
	uint16_t volatile usDelay_Timer;    //延时定时噿
	void (*Timer2_Start_IT)(void);      //定时噿2以中斿模式吿势
} My_Timer2_t;

/* extern variables-----------------------------------------------------------*/
extern My_Timer2_t  My_Timer2;

#endif
/********************************************************
  End Of File
********************************************************/
