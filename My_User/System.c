/*Include---------------------------------------------------------*/
#include "System.h"
/*private define--------------------------------------------------*/

/*private variables-----------------------------------------------*/
static void My_Run(void);
static void My_Error_Handler(void);
static void My_Assert_Failed(void);

 
/*public variables-----------------------------------------------*/
System_t System =
{
    My_Run,
    My_Error_Handler,
    My_Assert_Failed
};

/*private function prototypes -----------------------------------------------*/
//static void Disp_SHT30(void);
//static void TFT_Show(void);               //TFT显示（待实现）

/*
* @name    Run
* @brief   系统运行
* @param   None
* @retval  None
*/
static void My_Run()
{  
    LED.RUN_LED_Flip();
    My_DisplayTask(); //显示
    printf("MyADC.Ni:%02f\r\n\r\n",MyADC.Ni);
    //printf("MyADC.Vo:%02f\r\n\r\n",MyADC.Vo); 
    //printf("MyADC.Io:%02f\r\n\r\n",MyADC.Io); 
    My_Timer2.usDelay_Timer=0;
    while(1)
    {
	if(My_Timer2.usDelay_Timer >= TIMER2_10mS)
	break;
    }   

}



/*
* @name    Erro_Handler
* @brief   系统出错
* @param   None
* @retval  None
*/
static void My_Error_Handler()
{

}


/*
* @name    Assert_Failed
* @brief   参数出错
* @param   None
* @retval  None
*/
static void My_Assert_Failed()
{

}


/********************************************************/
/*                        End of File                   */
/********************************************************/