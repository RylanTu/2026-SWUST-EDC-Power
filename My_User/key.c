#include "key.h"

uint8_t Key_Code[6];

uint8_t Key_ReadPin(uint8_t n)
{  
	if (n == 1)
	{
		return HAL_GPIO_ReadPin(KEY1_GPIO_Port, KEY1_Pin);
	}
	if (n == 2)
	{
		return HAL_GPIO_ReadPin(KEY2_GPIO_Port, KEY2_Pin);
	}
	if (n == 3)
	{
		return HAL_GPIO_ReadPin(KEY3_GPIO_Port, KEY3_Pin);
	}
	if (n == 4)
	{
		return HAL_GPIO_ReadPin(KEY4_GPIO_Port, KEY4_Pin);
	}
    if (n == 5)
	{
		return HAL_GPIO_ReadPin(KEY5_GPIO_Port, KEY5_Pin);
	}
	return 1;
}

uint8_t Key_Check(uint8_t n, uint8_t Event)
{
	if (Key_Code[n] & Event)
	{
		Key_Code[n] &= ~Event;
		return 1;
	}
	return 0;
}

void Key_Clear(void)
{
	uint8_t i;
	for (i = 1; i < 6; i ++)
	{
		Key_Code[i] = 0;
	}
}

//放到定时器中断中进行扫描
void Key_Tick(void)
{
	static uint8_t Count;
	static uint8_t PrevState[6], CurrState[6];
	static uint8_t S[6];
	static uint8_t KeyCount[6];
	uint8_t i;
	
	Count ++;
	if (Count >= 20)
	{
		Count = 0;
		
		for (i = 1; i < 6; i ++)
		{
			PrevState[i] = CurrState[i];
			CurrState[i] = Key_ReadPin(i);
			
			switch (S[i])
			{
				case 0:
					if (PrevState[i] == 1 && CurrState[i] == 0)
					{
						S[i] = 1;
						KeyCount[i] = 0;
					}
				break;
				case 1:
					KeyCount[i] ++;
					if (KeyCount[i] >= 50)		//100*20=2000ms
					{
						S[i] = 0;
						Key_Code[i] |= KEY_LONG;
					}
					if (PrevState[i] == 0 && CurrState[i] == 1)
					{
						S[i] = 2;
						KeyCount[i] = 0;
					}
				break;
				case 2:
					KeyCount[i] ++;
					if (KeyCount[i] >= 1)		//1*20=20ms
					{
						S[i] = 0;
						Key_Code[i] |= KEY_CLICK;
					}
					if (PrevState[i] == 0 && CurrState[i] == 1)
					{
						S[i] = 0;
						Key_Code[i] |= KEY_DOUBLE;
					}
				break;
			}
		}
	}
}