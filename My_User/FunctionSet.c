/* Includes ------------------------------------------------------------------*/
#include "FunctionSet.h"

/* Private define-------------------------------------------------------------*/

/* Private variables----------------------------------------------------------*/

/* Private function prototypes------------------------------------------------*/


/* Public variables-----------------------------------------------------------*/
//结构体定义

//static void Mode_Adjust ( ); //模式设置
static void OUT_Switch_Adjust(void);  	//电源开/关机
static void SET_Switch_Adjust(void);  	//输出/设置模式
static void UP_Switch_Adjust(void); 	//上键开关调节
static void DOWN_Switch_Adjust(void);	//下键开关调节
static void OK_Switch_Adjust(void);	  	 //步进开关调节
static void Encoder_Direction_Adjust(Direction_Change_t Direction_Change);//编码器开关调节
static void OUT_VAL_Ctrl(void);				//输出电压 电流控制


FunctionSet_Type  Function_SET =    //功能设置
{
	OFF_State,           	//电压模式    有开机模式  关机模式
	CV_State,				//输出模式	  有恒压模式  恒流模式
	Menu_OUT_State,			//设置菜单输出状态
	SET_V_State,		 	//设置电压模式
	SET_State_First,		//设置步进第一位
	Idle_State,				//编码器闲置状态
	SET_VOUT_DEFAULT ,		//设置电压默认值 6.00V
	SET_IOUT_DEFAULT ,   	//设置电流默认值 0.500A

	OUT_Switch_Adjust,       	//电源开/关机
	SET_Switch_Adjust,			//输出/设置模式
	UP_Switch_Adjust,  			//上键开关调节
	DOWN_Switch_Adjust, 		//下键开关调节
	OK_Switch_Adjust,	   		//步进开关调节
	Encoder_Direction_Adjust,
	OUT_VAL_Ctrl      			 //输出电压 电流控制
};

static void OUT_Switch_Adjust(void)  //输出开关调节
{ 
	if(Function_SET.PowrputState==OFF_State)   //开机状态
    {
		//PWMSET.BUCK_POWER_Start();	  //执行开机任务
		PWMSET.PWM_Start();
		AT24CXX.Write_SET_VAL(Function_SET.Set_VOUT,Function_SET.Set_IOUT);//备份设置电压
		printf(" The KEY_ON button is ON!\r\n\r\n"); 
		Function_SET.PowrputState=ON_State;  //关机模式 
		Function_SET.SetMenuState=Menu_OUT_State; //菜单输出模式
    }
	else
    {
		//PWMSET.BUCK_POWER_Stop();     //执行关机任务
		PWMSET.PWM_Stop();
		printf(" The KEY_ON button is OFF!\r\n\r\n");
		Function_SET.PowrputState=OFF_State;  //关机模式 
		Function_SET.SetMenuState=Menu_OUT_State;  //菜单输出模式
    }  
}
static void SET_Switch_Adjust(void)  //输出/设置模式
{	
	if(Function_SET.PowrputState==OFF_State)  //关机模式
	{
		if(Function_SET.SetMenuState==Menu_OUT_State)  //输出模式 
		{
			Function_SET.SetMenuState=Menu_SET_State;
		}
		else
		{
			Function_SET.SetMenuState=Menu_OUT_State;
		}
	} 	
}

static void UP_Switch_Adjust(void)  //上键开关调节
{
	if(Function_SET.SetMenuState==Menu_SET_State)
	{
		Function_SET.SetVIState=SET_V_State;
	}
	
}    	

static void DOWN_Switch_Adjust(void)  //下键开关调节
{
	if(Function_SET.SetMenuState==Menu_SET_State)
	{
		Function_SET.SetVIState=SET_I_State;
	}
}

static void OK_Switch_Adjust(void)	   //步进开关调节
{
	if(Function_SET.SetMenuState!=Menu_OUT_State) //设置模式
	{
		switch(Function_SET.SetStepState)   // 步进位
     	{
      		case SET_State_First:Function_SET.SetStepState=SET_State_Second; break; //   第一位 
			case SET_State_Second:Function_SET.SetStepState=SET_State_Thirdly; break; //   第二位 
			case SET_State_Thirdly:Function_SET.SetStepState=SET_State_First; break; //   第三位 
      		default:Function_SET.SetStepState=SET_State_First;
     	}
	}
}





static void  Encoder_Direction_Adjust(Direction_Change_t Direction_Change)  //编码器调节
{
	if(Function_SET.SetMenuState==Menu_SET_State) 
	{
		if(Function_SET.SetVIState==SET_V_State)  //设置电压
		{
			if(Direction_Change==Reverse_State)  //逆时针转动 减(2026.4.15 RylanTu:why前面的判断逻辑要反着写?)
			{
				//电压减
				switch(Function_SET.SetStepState)
				{
					case SET_State_First:
						if(Function_SET.Set_VOUT > SET_VOUT_MIN)
					{
						Function_SET.Set_VOUT-= 1;
					}
					else
					{
							Function_SET.Set_VOUT= SET_VOUT_MIN;
					}
					break;
					case SET_State_Second:
						if(Function_SET.Set_VOUT>=10)
					{
						Function_SET.Set_VOUT-= 10;
					}
					else
					{
							Function_SET.Set_VOUT= SET_VOUT_MIN;
					}
					break; //   第二位 
					case SET_State_Thirdly:
						if(Function_SET.Set_VOUT>=100)
					{
						Function_SET.Set_VOUT-= 100;
					}
					else
					{
							Function_SET.Set_VOUT= SET_VOUT_MIN;
					}
					break; //   第三位 
				}		
			}
			else  //顺时针转动 加
			{
				//电压加
				switch(Function_SET.SetStepState)
				{
					case SET_State_First:
						if(Function_SET.Set_VOUT < SET_VOUT_MAX)
					{
						Function_SET.Set_VOUT+= 1;
					}
					else
					{
							Function_SET.Set_VOUT= SET_VOUT_MAX;
					}
					break;
					case SET_State_Second:
						if(Function_SET.Set_VOUT <= (SET_VOUT_MAX - 10U))
					{
						Function_SET.Set_VOUT+= 10;
					}
					else
					{
							Function_SET.Set_VOUT= SET_VOUT_MAX;
					}
					break; //   第二位 
					case SET_State_Thirdly:
						if(Function_SET.Set_VOUT <= (SET_VOUT_MAX - 100U))
					{
						Function_SET.Set_VOUT+= 100;
					}
					else
					{
							Function_SET.Set_VOUT= SET_VOUT_MAX;
					}
					break; //   第三位 
				}
			}
			printf(" Function_SET.Set_VOUT: %d\r\n\r\n",Function_SET.Set_VOUT);
	
			
		}
		else////设置电流
		{
			if(Direction_Change==Reverse_State)  //逆时针转动 减
			{
				// 电流减
				switch(Function_SET.SetStepState)
				{
					case SET_State_First:
					if(Function_SET.Set_IOUT > SET_IOUT_MIN)
					{
						Function_SET.Set_IOUT-= 1;    //0.001A
					}
					else
					{
						Function_SET.Set_IOUT= SET_IOUT_MIN;
					}
					break;
					case SET_State_Second:
					if(Function_SET.Set_IOUT>=10)
					{
						Function_SET.Set_IOUT-= 10;    //0.010A
					}
					else
					{
						Function_SET.Set_IOUT= SET_IOUT_MIN;
					}
					break;
					case SET_State_Thirdly:
					if(Function_SET.Set_IOUT>=100)
					{
						Function_SET.Set_IOUT-= 100;    // 0.100A
					}
					else
					{
						Function_SET.Set_IOUT= SET_IOUT_MIN;
					}
					break;
					//Function_SET.Set_IOUT=(Function_SET.Set_IOUT<=0)?0:Function_SET.Set_IOUT;  //设置电压小于0时等于0
				}
			}
			else
			{
				//电流加
				switch(Function_SET.SetStepState)
				{
					case SET_State_First:
					if(Function_SET.Set_IOUT < SET_IOUT_MAX)
					{
						Function_SET.Set_IOUT+= 1;
					}
					else
					{
						Function_SET.Set_IOUT= SET_IOUT_MAX;
					}
					break;
					case SET_State_Second:
					if(Function_SET.Set_IOUT <= (SET_IOUT_MAX - 10U))
					{
						Function_SET.Set_IOUT+= 10;
					}
					else
					{
						Function_SET.Set_IOUT= SET_IOUT_MAX;
					}
					break;
					case SET_State_Thirdly:
					if(Function_SET.Set_IOUT <= (SET_IOUT_MAX - 100U))
					{
						Function_SET.Set_IOUT+= 100;
					}
					else
					{
						Function_SET.Set_IOUT= SET_IOUT_MAX;
					}
					break;
					//Function_SET.Set_IOUT=(Function_SET.Set_IOUT<=0)?0:Function_SET.Set_IOUT;  //设置电压小于0时等于0
				}
			}
		printf(" Function_SET.Set_IOUT: %d\r\n\r\n",Function_SET.Set_IOUT);
		}
	}
}


static void  OUT_VAL_Ctrl(void)  //输出电压 电流控制
{
	uint16_t CV_Duty;         //
	uint16_t CC_Duty;         //
	uint16_t Vout_val;	
	uint16_t Iout_val;
	float SET_Vout_val;
	float SET_Iout_val;
	if(Function_SET.PowrputState==ON_State)
	{
		Vout_val=Function_SET.Set_VOUT;
		Iout_val=Function_SET.Set_IOUT;
		//printf("Vout_val:%d\r\n\r\n",Vout_val);
		//printf("Iout_val:%d\r\n\r\n",Iout_val);

		// Set_VOUT单位10mV: /100转换为V，再匹配11分压比
		SET_Vout_val=(float)Vout_val/100/11/3.3*1440;
		//printf("SET_Vout_val:%f\r\n\r\n",SET_Vout_val); 
		CV_Duty=(uint16_t)SET_Vout_val;
		//CC_Duty=(float)((((Iout_val/100*0.0254)/0.75+((Iout_val/100*0.0254)/0.75/4*30))/3.3)*1440); //采样电阻0.025R
		//CC_Duty=(float)(((Iout_val/100*0.025)/0.75+((Iout_val/100*0.025)/0.75/4*30))/3.3*1440); //采样电阻0.025R
		//CC_Duty=(uint16_t)((Iout_val/100*0.025)/3*34)/3.3*1440;

		// Set_IOUT单位1mA: /1000转换为A；采样电阻0.025R，电流环放大约15倍
		SET_Iout_val=(float)Iout_val/1000*0.025*15/3.3*1440;
		//printf("SET_Iout_val:%f\r\n\r\n",SET_Iout_val); 
		CC_Duty=(uint16_t)SET_Iout_val;

		PWMSET.PWM_Updata(CV_Duty,CC_Duty);  //更新输出电流 电压
	
		//printf("CV_Duty:%d\r\n\r\n",CV_Duty); 
		//printf("CC_Duty:%d\r\n\r\n",CC_Duty); 
	}
	else
	{
		Vout_val=0;
		Iout_val=0;
	}
	
}


