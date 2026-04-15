#ifndef __MyInit_H__
#define __MyInit_H__
#include "MyApplication.h"
typedef struct 
{
    void (*Peripheral_Set)(void);
    /* data */
}MyInit_t;

extern MyInit_t MyInit;
#endif /* __MyInit_H__ */