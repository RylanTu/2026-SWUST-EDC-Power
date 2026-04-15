/*Include--------------------------------------------------------*/
#include "FAN.h"

/*private define-------------------------------------------------*/

/*private variables----------------------------------------------*/

static void FAN_ON(void);   //静态函数
static void FAN_OFF(void);
static void FAN_Flip(void);

/*public variables-----------------------------------------------*/

FAN_t FAN =
{
    FAN_ON,
    FAN_OFF,
    FAN_Flip
};

/*private function prototypes -----------------------------------*/


/*
* @name    FAN_ON
* @brief   FAN打开
* @param   None
* @retval  None
*/
static void FAN_ON(void)
{
  HAL_GPIO_WritePin(FAN_GPIO_Port,FAN_Pin,GPIO_PIN_SET); 
}


/*
* @name    FAN_OFF
* @brief   FAN关闭
* @param   None
* @retval  None
*/

static void FAN_OFF(void)
{
   HAL_GPIO_WritePin(FAN_GPIO_Port,FAN_Pin,GPIO_PIN_RESET);
}


/*
* @name    FAN_Flip
* @brief   FAN取反
* @param   None
* @retval  None
*/

static void FAN_Flip (void)
{
    HAL_GPIO_TogglePin(FAN_GPIO_Port,FAN_Pin);
}



/********************************************************/
/*                        End of File                   */
/********************************************************/