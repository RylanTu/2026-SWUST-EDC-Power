/*Include--------------------------------------------------------*/
#include "LED.h"

/*private define-------------------------------------------------*/

/*private variables----------------------------------------------*/

static void RUN_LED_ON(void);   
static void RUN_LED_OFF(void);
static void RUN_LED_Flip(void);

/*public variables-----------------------------------------------*/

LED_t LED =
{

    RUN_LED_ON,
    RUN_LED_OFF,
    RUN_LED_Flip
};

/*private function prototypes -----------------------------------*/


/*
* @name    LED_ON
* @brief   LED打开
* @param   Num->编号
* @retval  None
*/
static void RUN_LED_ON (void)
{   
    HAL_GPIO_WritePin(LED_RUN_GPIO_Port,LED_RUN_Pin,GPIO_PIN_SET);
}


/*
* @name    LED_OFF
* @brief   LED关闭
* @param   Num->编号
* @retval  None
*/

static void RUN_LED_OFF (void)
{
    HAL_GPIO_WritePin(LED_RUN_GPIO_Port,LED_RUN_Pin,GPIO_PIN_RESET);  

}


/*
* @name    LED_Flip
* @brief   LED取反
* @param   Num->编号
* @retval  None
*/

static void RUN_LED_Flip (void)
{
    HAL_GPIO_TogglePin(LED_RUN_GPIO_Port,LED_RUN_Pin);
}



/********************************************************/
/*                        End of File                   */
/********************************************************/