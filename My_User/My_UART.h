#ifndef __MY_UART_H_
#define __MY_UART_H_

#include "MyApplication.h"

//定义枚举类型
typedef enum
{	
	TTL       = (uint8_t)0,    //TTL模式
	RS_485	  = (uint8_t)1,    //RS485模式
	RS_232	  = (uint8_t)2,    //RS232模式
}Interface_Type_t;
/* extern variables-----------------------------------------------------------*/

typedef struct 
{
	uint8_t* pucSend_Buffer;           //发送缓存指针
	uint8_t* pucRec_Buffer;            //接收缓存指针
	
	void (*SendArray)(uint8_t*,uint16_t);  //串口发送数组
	void (*SendString)(uint8_t*);          //串口发送字符串

	uint8_t	Interface_Type;            //接口类型
	void (*RS485_Set_SendMode)(void);  //RS-485接口设置为发送模式
	void (*RS485_Set_RecMode)(void);   //RS-485接口设置为接收模式
} My_UART_t ;
/* extern variables-----------------------------------------------------------*/

/* extern function prototypes-------------------------------------------------*/

#endif
/********************************************************
  End Of File
********************************************************/

