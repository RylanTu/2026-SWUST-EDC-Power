#ifndef __System_H__
#define __System_H__
#include "MyApplication.h"
//定义结构体类型

typedef struct 
{
    void (*Run)(void);
    void (*Error_Handler)(void);
    void (*Assert_Failed)(void);
    /* data */
}System_t;

extern System_t System;

#endif /* __System_H__ */