/* Includes ------------------------------------------------------------------*/
#include "Public.h"

/* Private define-------------------------------------------------------------*/

/* Private variables----------------------------------------------------------*/
static void Memory_Clr(uint8_t*,uint16_t);   //内存清零函数

/* Public variables-----------------------------------------------------------*/
Public_t  Public = 
{
  Memory_Clr
};

/* Private function prototypes------------------------------------------------*/      

/*
	* @name   Memory_Set
	* @brief  内存清零
	* @param  pucBuffer -> 目标缓冲区
						LEN       -> 缓冲区长度   
	* @retval None      
*/
static void Memory_Clr(uint8_t* pucBuffer,uint16_t LEN)
{
	uint16_t i;
	
	for(i=0;i<LEN;i++)
	{
		*(pucBuffer+i) = (uint8_t)0;
	}
}


/********************************************************
  End Of File
********************************************************/
