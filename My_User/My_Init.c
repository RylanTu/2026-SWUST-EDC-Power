/*Include-----------------------------------------------*/
#include "My_Init.h"
/*˵����-----------------------------------------------*/
/*��Ϊ���·�ʹ�õ���LCD������ʾ���ڱ��뷽ʽѡ����GB2312*/


static void Peripheral_Set(void);


MyInit_t MyInit =
{
    Peripheral_Set
};

static void Peripheral_Set()
{   
    //OLED_Init();  // 已切换至TFT，不再使用OLED
    LCD.Init();
    LED.RUN_LED_ON();
    FAN.FAN_ON();
    HAL_Delay(100);  
    LED.RUN_LED_OFF();
    FAN.FAN_OFF();
    HAL_Delay(100);  
    MyADC.ADC_Initial_Setup() ;   //ADC初始化
    
    //定时器初始化   
    My_Timer2.Timer2_Start_IT();
    HAL_TIM_Encoder_Start(&htim4,TIM_CHANNEL_1|TIM_CHANNEL_2);
    HAL_TIM_Encoder_Start_IT(&htim4,TIM_CHANNEL_1);
    printf("Software version is V%.1f\r\n\r\n",SoftWare_Version);    
    if(AT24CXX.AT24CXX_IsDeviceReady()==HAL_OK)
    {
        printf("AT24CXX_DeviceReady!!!\r\n\r\n");
    }
    HAL_Delay(100);
    AT24CXX.Read_SET_VAL();  //读取存储信息
    HAL_Delay(100);
    PWMSET.PWM_Init();  

}
