/* Includes ------------------------------------------------------------------*/
#include "My_KEY.h"

/* Private define-------------------------------------------------------------*/

/* Private variables----------------------------------------------------------*/

/* Private function prototypes------------------------------------------------*/


/* Public variables-----------------------------------------------------------*/
//结构体定义
//2026.4.15 RylanTu:这东西可能需要重构，没有使用外部中断
//static void Mode_Adjust ( ); //模式设置
static void KEY_ON_Detect(void);  //开关键
static void KEY_OK_Detect(void);  //开关键
static void KEY_UP_Detect(void);  //开关键
static void KEY_SET_Detect(void);  //开关键
static void KEY_DOWN_Detect(void);  //开关键
static void KEY_EC11_Detect(void);  //开关键
//static void Step_Adjust( ); //步长调节
//static void Encoder_Direction(Direction_Change_t Direction_Change);

//结构体定义
KEY_t  KEY_ON 	=	{FALSE,KEY_ON_Detect}; //
KEY_t  KEY_OK 	=	{FALSE,KEY_OK_Detect}; //
KEY_t  KEY_UP 	=	{FALSE,KEY_UP_Detect}; //
KEY_t  KEY_SET 	=	{FALSE,KEY_SET_Detect}; //
KEY_t  KEY_DOWN =	{FALSE,KEY_DOWN_Detect}; //
KEY_EC11_t  KEY_EC11 =	{KEY_EC11_Detect}; //


/*逻辑按键映射（物理引脚）：
*key1(K4)-向上选择
*key2(K3)-向下选择
*key3(K2)-设置输出调节
*key4(K5)-开关机调节
*key5(EC11按压)-步进调节
*编码器旋转-数值加减
*/
//2026.4.15 RylanTu:你的意思是这个东西用的定时器中断扫描而不是外部中断?

static void KEY_ON_Detect(void)
{
	if(KEY_ON.KEY_Flag==TRUE)
	{
		HAL_Delay(2);  //软件去抖
		if(HAL_GPIO_ReadPin(K2_GPIO_Port,K2_Pin) == GPIO_PIN_RESET)       
		{
			Function_SET.OUT_Switch_Adjust();  //输出开关调节
		}
		KEY_ON.KEY_Flag=FALSE; //清除标志位
	}
} 
static void KEY_OK_Detect(void)
{
	if(KEY_OK.KEY_Flag==TRUE)
	{
		HAL_Delay(2);  //软件去抖
		if(HAL_GPIO_ReadPin(EC11_GPIO_Port,EC11_Pin) == GPIO_PIN_RESET)
		{
			Function_SET.OK_Switch_Adjust();  //步进开关调节	
		}
		KEY_OK.KEY_Flag=FALSE; //清除标志位
	}
} 
static void KEY_UP_Detect(void)
{
	if(KEY_UP.KEY_Flag==TRUE)
	{
//		HAL_Delay(10);  //软件去抖
//		if(HAL_GPIO_ReadPin(KEY_UP_GPIO_Port,KEY_UP_Pin) == GPIO_PIN_RESET)
//		{
//			Function_SET.UP_Switch_Adjust();  //上键开关调节
//		}
//		KEY_UP.KEY_Flag=FALSE; //清除标志位
	}
} 
static void KEY_SET_Detect(void)
{
	if(KEY_SET.KEY_Flag==TRUE)
	{
		HAL_Delay(10);  //软件去抖
		if(HAL_GPIO_ReadPin(K5_GPIO_Port,K5_Pin) == GPIO_PIN_RESET)
		{
			Function_SET.SET_Switch_Adjust();  //输出开关调节
		}
		KEY_SET.KEY_Flag=FALSE; //清除标志位
	}
} 
static void KEY_DOWN_Detect(void)
{
	if(KEY_DOWN.KEY_Flag==TRUE)
	{
//		HAL_Delay(10);  //软件去抖
//		if(HAL_GPIO_ReadPin(KEY_DOWN_GPIO_Port,KEY_DOWN_Pin) == GPIO_PIN_RESET)
//		{
//			Function_SET.DOWN_Switch_Adjust();  //下键开关调节
//		}
//		KEY_DOWN.KEY_Flag=FALSE; //清除标志位
	}
} 

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