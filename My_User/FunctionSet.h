#ifndef __FunctionSet_H__
#define __FunctionSet_H__

#include "MyApplication.h"

#define SET_VOUT_MIN 0U       // 0.00V (unit: 10mV)
#define SET_VOUT_MAX 1200U    // 12.00V (unit: 10mV)
#define SET_IOUT_MIN 0U       // 0.000A (unit: 1mA)
#define SET_IOUT_MAX 1300U    // 1.300A (unit: 1mA)

#define SET_VOUT_DEFAULT 600U // 6.00V (unit: 10mV)
#define SET_IOUT_DEFAULT 500U // 0.500A (unit: 1mA)

//方向
typedef enum
{
	Idle_State = 	(uint8_t)0,     //闲置状态
	Forward_State = (uint8_t)2,  //顺时方向
	Reverse_State = (uint8_t)1,  //逆时方向

}Direction_Change_t;


typedef struct  
{
	uint8_t PowrputState ;           	//电源模式    有开机模式 关机模式
	uint8_t OutPutState;				//实际输出模式	  有恒压模式 恒流模式
	uint8_t SelectOutPutState;			//用户选择模式  有恒压模式 恒流模式
	uint8_t SetMenuState;               //菜单模式状态 输出 设置
	uint8_t SetVIState;               //设置电压步进模式  有 退出 第一位 第二位 
	uint8_t SetStepState;               //设置步进模式: VSET三档/ISET四档(含整数位)
	uint8_t Encoder_State;                 //编码器状态  闲置 正转 反转
	uint8_t ProtectState;                  //保护状态 0正常 1过压 2过流 3过温
	uint16_t Set_VOUT  ;                  //设置电压值(单位:10mV)
	uint16_t Set_IOUT  ;                  //设置电流值(单位:1mA)

      
	void (*OUT_Switch_Adjust)(void);       	//电源开/关机
	void (*SET_Switch_Adjust)(void);       	//输出/设置模式
	void (*UP_Switch_Adjust)(void); 		//上键开关调节
 	void (*DOWN_Switch_Adjust)(void);		//下键开关调节
	void (*OK_Switch_Adjust)(void);	   		//步进开关调节
	void (*Encoder_Direction_Adjust)(Direction_Change_t); //编码器方向调整 
	void (*OUT_VAL_Ctrl)(void);  						  //输出电压 电流控制
	void (*Check_Protect)(void);                          //保护检查

}FunctionSet_Type;

//状态
typedef enum
{
 OFF_State  = (uint8_t)0,   //关机状态
 ON_State  	= (uint8_t)1,   //开机状态
}OUT_Switch_Status_t;       //输出开关状态


//状态
typedef enum
{
	SET_State_First  	= 	(uint8_t)0,   //第一位(0.1步进)
	SET_State_Second	=	(uint8_t)1,   //第二位(0.01步进)
	SET_State_Thirdly	=	(uint8_t)2,   //第三位(0.001步进)
	SET_State_Fourth	=	(uint8_t)3,   //整数位(1.000步进,VSET专用)
}SET_Step_Status_t;         //设置开关状态

//状态
typedef enum
{
 CV_State	= 	(uint8_t)0,    //恒压状态
 CC_State	=	(uint8_t)1,    //恒流状态
}OUT_Pattern_Status_t;         //输出模式状态

//状态
typedef enum
{
 Menu_OUT_State	= 	(uint8_t)0,    //菜单输出状态
 Menu_SET_State	=	(uint8_t)1,    //菜单设置状态
}SET_Menu_Status_t;       //设置菜单模式状态


//状态
typedef enum
{
 SET_V_State	= 	(uint8_t)0,    //菜单设置状态
 SET_I_State	=	(uint8_t)1,    //菜单设置状态
}SET_VI_Status_t;       //设置菜单模式状态




extern FunctionSet_Type	Function_SET;    //功能设置

void FunctionSet_SyncSetVIWithMode(void);


#endif /* __FunctionSet__ */


/********************************************************/
/*                        End of File                   */
/********************************************************/