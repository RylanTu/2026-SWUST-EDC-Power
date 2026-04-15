#ifndef __FAN_H__
#define __FAN_H__

#include "MyApplication.h"
//定义枚举类型


//定义结构体类型

typedef struct 
{
    /* data */
    void (*FAN_ON)(void);     //打开FAN
    void (*FAN_OFF)(void);    //关闭FAN
    void (*FAN_Filp)(void);   //翻转FAN
} FAN_t ;


/*extern variables--------------------------------------*/
extern FAN_t   FAN ;

#endif /* __FAN__ */


/********************************************************/
/*                        End of File                   */
/********************************************************/

