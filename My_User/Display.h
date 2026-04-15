#ifndef __Display_H__
#define __Display_H__

#include "MyApplication.h"

typedef struct  
{
uint8_t Show_Once_Flag;                 //显示一次标志位
uint8_t DeivceState;                    //显示设备在线标志 
void (*DisplayShow_Once)(void);         //只显示一次内容
void (*DisplayShow_Device)(void);       //显示设备在线状态
void (*DisplayShow_Setval)(void);       //显示设置值
void (*DisplayShow_Outval)(void);       //显示输出值
void (*DisplayShow_Cursor)(void);       //显示调节光标
}Display_Type;


//状态
typedef enum
{
 Offline_State  =   (uint8_t)0,   //离线状态
 Online_State   =   (uint8_t)1,   //在线状态
 
}Device_State_t;   //设备状态



extern Display_Type	Display;  // 显示


void My_DisplayTask(void);   //显示系统

#endif /* __Display__ */