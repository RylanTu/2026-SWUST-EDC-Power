/* Includes ------------------------------------------------------------------*/
#include "My_KEY.h"

/* Private define-------------------------------------------------------------*/

/* Private variables----------------------------------------------------------*/

/* Private function prototypes------------------------------------------------*/
static void KEY_EC11_Detect(void);


/* Public variables-----------------------------------------------------------*/
//旋转编码器
static void KEY_EC11_Detect(void)
{
	if(Function_SET.Encoder_State==Reverse_State)
	{
		Function_SET.Encoder_Direction_Adjust(Reverse_State); //逆时针
		printf(" The Encoder_A button is pressed!\r\n\r\n");

		Function_SET.Encoder_State=Idle_State; //清除标志位
	}
	else if(Function_SET.Encoder_State==Forward_State)
	{		
		Function_SET.Encoder_Direction_Adjust(Forward_State); //顺时针
		printf(" The Encoder_B button is pressed!\r\n\r\n");

		Function_SET.Encoder_State=Idle_State; //清除标志位
	}	

}
/*逻辑按键映射（物理引脚）：
*key1(K4)-VSET/ISET切换
*key2(K3)-CV/CC模式切换(仅SET状态)
*key3(K2)-设置输出调节
*key4(K5)-开关机调节
*key5(EC11按压)-步进调节
*编码器旋转-数值加减
*/
//2026.4.15 RylanTu:你的意思是这个东西用的定时器中断扫描而不是外部中断?


void KEY_ALL_Detect(void)
{
    if(Key_Check(1,KEY_CLICK)) Function_SET.UP_Switch_Adjust();
    else if(Key_Check(2,KEY_CLICK))   Function_SET.DOWN_Switch_Adjust();
    else if(Key_Check(3,KEY_CLICK))   Function_SET.SET_Switch_Adjust();
    else if(Key_Check(4,KEY_CLICK))   Function_SET.OUT_Switch_Adjust();
    else if(Key_Check(5,KEY_CLICK))   Function_SET.OK_Switch_Adjust();  
    KEY_EC11_Detect();
    Key_Clear();
}