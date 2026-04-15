/* Includes ------------------------------------------------------------------*/
#include "Public.h"

/* Private define-------------------------------------------------------------*/

/* Private variables----------------------------------------------------------*/
static void Memory_Clr(uint8_t*,uint16_t);   //????????????

/* Public variables-----------------------------------------------------------*/
Public_t  Public = 
{
  Memory_Clr
};

/* Private function prototypes------------------------------------------------*/      

/*
	* @name   Memory_Set
	* @brief  ????????????
	* @param  pucBuffer -> ?????¡Á???¡¤
						LEN       -> ?????¡è??   
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
