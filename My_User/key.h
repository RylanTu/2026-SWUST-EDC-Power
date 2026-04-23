#ifndef _KEY_H
#define _KEY_H

#include "main.h"

#define KEY_CLICK		0x01    //单击
#define KEY_DOUBLE		0x02    //双击
#define KEY_LONG		0x04     //长按

void Key_Init(void);
uint8_t Key_Check(uint8_t n, uint8_t Event);
void Key_Clear(void);
void Key_Tick(void);






#endif