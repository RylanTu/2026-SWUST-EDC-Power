#ifndef __My_KEY_H__
#define __My_KEY_H__

#include "MyApplication.h"


typedef struct  
{
	uint8_t volatile KEY_Flag; 
	void (*KEY_Detect)(void);  
}KEY_t;

typedef struct  
{
	void (*KEY_EC11_Detect)(void);  
}KEY_EC11_t;


extern KEY_t	KEY_ON;    //开关键
extern KEY_t	KEY_OK;    //确认键
extern KEY_t	KEY_SET;   //设置键
extern KEY_t	KEY_UP;    //上键
extern KEY_t	KEY_DOWN;   //下键
extern KEY_EC11_t	KEY_EC11;   //编码器

void KEY_ALL_Detect(void);  //按键检测系统
#endif /* __My_KEY__ */


/********************************************************/
/*                        End of File                   */
/********************************************************/