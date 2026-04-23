#ifndef __LED_H__
#define __LED_H__

#include "MyApplication.h"


//定义结构体类型

typedef struct 
{
    /* data */
    void (*RUN_LED_ON)(void);     //打开LED
    void (*RUN_LED_OFF)(void);    //关闭LED
    void (*RUN_LED_Flip)(void);   //翻转LED
} LED_t ;


/*extern variables--------------------------------------*/
extern LED_t   LED ;

#endif /* __LED__ */


/********************************************************/
/*                        End of File                   */
/********************************************************/