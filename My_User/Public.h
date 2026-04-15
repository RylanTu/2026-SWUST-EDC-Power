#ifndef __PUBLIC_H_
#define __PUBLIC_H_
#include "MyApplication.h"
/* Public define-------------------------------------------------------------*/
#define SoftWare_Version 	(float)1.1  //版本号

//定义枚举类型 -> TRUE/FALSE
typedef enum 
{
  FALSE = 0U, 
  TRUE = !FALSE
} FlagStatus_t;

typedef enum 
{
  FAILED = 0U, 
  PASSED = !FAILED
} TestStatus_t;

//定义结构体类型
typedef struct
{
	void (*Memory_Clr)(uint8_t*,uint16_t); //内存清除函数
} Public_t;

/* extern variables-----------------------------------------------------------*/
extern Public_t Public;


/* extern variables-----------------------------------------------------------*/

/*******预编译宏定义*******/
//#define Monitor_Run_Code   //代码运行监视器
//#define Hardware_TEST      //硬件测试

#endif
/********************************************************
  End Of File
********************************************************/
