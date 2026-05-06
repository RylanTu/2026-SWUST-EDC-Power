/* Includes ------------------------------------------------------------------*/
#include "FunctionSet.h"
#include "Display.h"
#include <string.h>
/* Private define-------------------------------------------------------------*/
#define PROTECT_VOLT   15.0f   //过压保护阈值(V)
#define PROTECT_CURR    3.0f   //过流保护阈值(A)
#define PROTECT_TEMP   60.0f   //过温保护阈值(℃)
#define MIN_SHUTDOWN_TIME_MS  300U  //最小关机时间(ms) 防止快速开关导致保护反复触发

// #define CC_RSENSE        0.02468189f    // 采样电阻 25mΩ
// #define CC_AMP_GAIN      11.40f     // 电流环放大倍数
// #define CC_VREF          3.29f

#define CC_RSENSE        0.02468189f    // 采样电阻 25mΩ
#define CC_AMP_GAIN      11.40f     // 电流环放大倍数
#define CC_VREF          3.29f

// /* 电压校准模型（最小二乘拟合）: V_meas = k * V_set + b
//  * 反向补偿: V_set_comp = (V_target - b) / k
//  */
// #define VOUT_FIT_K      0.993364f
// #define VOUT_FIT_B      0.081634f

// /* 电压校准模型（最小二乘拟合）: V_meas = k * V_set + b
//  * 反向补偿: V_set_comp = (V_target - b) / k
//  * 精度：绝对误差 ≤ 0.0008V，相对误差 ＜ 0.01%
//  */
// #define VOUT_FIT_K      0.99725758f//成功
// #define VOUT_FIT_B      0.11257576f


/* 电压校准模型（最小二乘拟合）: V_meas = k * V_set + b
 * 反向补偿: V_set_comp = (V_target - b) / k
 * 精度：绝对误差 ≤ 0.0008V，相对误差 ＜ 0.01%
 */
#define VOUT_FIT_K      0.99725758f
#define VOUT_FIT_B      0.11257576f

/* Private variables----------------------------------------------------------*/
static uint16_t shutdown_timer_ms = 0U;  //关机计时器(ms) 用于实现最小关机时间防护

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
static void Check_Protect(void);			//保护检查


FunctionSet_Type  Function_SET =    //功能设置
{
	OFF_State,           	//电压模式    有开机模式  关机模式
	CV_State,				//输出模式	  有恒压模式  恒流模式
	CV_State,				//用户选择模式  有恒压模式  恒流模式
	Menu_OUT_State,			//设置菜单输出状态
	SET_V_State,		 	//设置电压模式
	SET_State_First,		//设置步进第一位
	Idle_State,				//编码器闲置状态
	0,                      //ProtectState 初始正常
	SET_VOUT_DEFAULT ,		//设置电压默认值 6.00V
	SET_IOUT_DEFAULT ,   	//设置电流默认值 0.500A

	OUT_Switch_Adjust,       	//电源开/关机
	SET_Switch_Adjust,			//输出/设置模式
	UP_Switch_Adjust,  			//上键开关调节
	DOWN_Switch_Adjust, 		//下键开关调节K3
	OK_Switch_Adjust,	   		//步进开关调节
	Encoder_Direction_Adjust,
	OUT_VAL_Ctrl,      			 //输出电压 电流控制
	Check_Protect                //保护检查
};

void FunctionSet_SyncSetVIWithMode(void)
{
	if(Function_SET.OutPutState == CV_State)
	{
		Function_SET.SetVIState = SET_V_State;
	}
	else
	{
		Function_SET.SetVIState = SET_I_State;
	}
}

static void OUT_Switch_Adjust(void)  //输出开关调节
{ 
	if(Function_SET.PowrputState==OFF_State)   //开机状态
    {
		//检查最小关机时间 防止快速开关导致保护反复触发
		if(shutdown_timer_ms < MIN_SHUTDOWN_TIME_MS)
		{
			//关机时间不足 忽略启动请求
			return;
		}
		
		//PWMSET.BUCK_POWER_Start();	  //执行开机任务
		PWMSET.PWM_Start();
		AT24CXX.Write_SET_VAL(Function_SET.Set_VOUT,Function_SET.Set_IOUT);//备份设置电压
		printf(" The KEY_ON button is ON!\r\n\r\n"); 
		Function_SET.PowrputState=ON_State;  //开机模式 
		Function_SET.SetMenuState=Menu_OUT_State; //菜单输出模式
		shutdown_timer_ms = 0U; //复位关机计时器
    }
	else
    {
		//PWMSET.BUCK_POWER_Stop();     //执行关机任务
		PWMSET.PWM_Stop();
		printf(" The KEY_ON button is OFF!\r\n\r\n");
		Function_SET.PowrputState=OFF_State;  //关机模式 
		Function_SET.SetMenuState=Menu_OUT_State;  //菜单输出模式
		shutdown_timer_ms = 0U; //启动关机计时
    }  
}

static void SET_Switch_Adjust(void)  //输出/设置模式
{	
	if(Function_SET.PowrputState==OFF_State)  //关机模式
	{
		if(Function_SET.SetMenuState==Menu_OUT_State)  //输出模式 
		{
			Function_SET.SetMenuState=Menu_SET_State;
			FunctionSet_SyncSetVIWithMode();
		}
		else
		{
			Function_SET.SetMenuState=Menu_OUT_State;
		}
	} 	
}

static void UP_Switch_Adjust(void)  
{
	if(Function_SET.SetMenuState==Menu_SET_State)
	{
		// K4: 在设置菜单下切换 VSET/ISET
		if(Function_SET.SetVIState == SET_V_State)
		{
			Function_SET.SetVIState = SET_I_State;
		}
		else
		{
			Function_SET.SetVIState = SET_V_State;
		}
	}
	
}    	

static void DOWN_Switch_Adjust(void)  
{
	// K3: 仅在设置菜单下允许切换恒压/恒流模式
	if(Function_SET.SetMenuState==Menu_SET_State)
	{
		if(Function_SET.OutPutState == CV_State)
		{
			Function_SET.OutPutState = CC_State;			
		}
		else
		{
			Function_SET.OutPutState = CV_State;			
		}
		 FunctionSet_SyncSetVIWithMode();		
	}
}

static void OK_Switch_Adjust(void)	   //步进开关调节
{
	if(Function_SET.SetMenuState!=Menu_OUT_State) //设置模式
	{
		if(Function_SET.SetVIState == SET_I_State)
		{
			switch(Function_SET.SetStepState)   // ISET: 四档(含整数位)
	     	{
	      		case SET_State_First:Function_SET.SetStepState=SET_State_Second; break;
				case SET_State_Second:Function_SET.SetStepState=SET_State_Thirdly; break;
				case SET_State_Thirdly:Function_SET.SetStepState=SET_State_Fourth; break;
				case SET_State_Fourth:Function_SET.SetStepState=SET_State_First; break;
	      		default:Function_SET.SetStepState=SET_State_First;
	     	}
		}
		else
		{
			switch(Function_SET.SetStepState)   // VSET: 三档
	     	{
	      		case SET_State_First:Function_SET.SetStepState=SET_State_Second; break;
				case SET_State_Second:Function_SET.SetStepState=SET_State_Thirdly; break;
				case SET_State_Thirdly:Function_SET.SetStepState=SET_State_First; break;
				default:Function_SET.SetStepState=SET_State_First;
	     	}
		}
	}
}





static void  Encoder_Direction_Adjust(Direction_Change_t Direction_Change)  //编码器调节
{
	Direction_Change_t real_dir;

	/* 旋转方向反转：保留中断上报定义，在数值调节层做统一翻转。 */
	real_dir = (Direction_Change == Forward_State) ? Reverse_State : Forward_State;

	if(Function_SET.SetMenuState==Menu_SET_State) 
	{
		if(Function_SET.SetVIState==SET_V_State)  //设置电压
		{
			if(real_dir==Reverse_State)  //逆时针转动 减
			{
				//电压减
				switch(Function_SET.SetStepState)
				{
					case SET_State_First:
						if(Function_SET.Set_VOUT>=100)
					{
						Function_SET.Set_VOUT-= 100;
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
						if(Function_SET.Set_VOUT > SET_VOUT_MIN)
					{
						Function_SET.Set_VOUT-= 1;
					}
					else
					{
							Function_SET.Set_VOUT= SET_VOUT_MIN;
					}
					break; //   第三位 
					default:
						if(Function_SET.Set_VOUT>=100)
						{
							Function_SET.Set_VOUT-= 100;
						}
						else
						{
							Function_SET.Set_VOUT= SET_VOUT_MIN;
						}
						break;
				}		
			}
			else  //顺时针转动 加
			{
				//电压加
				switch(Function_SET.SetStepState)
				{
					case SET_State_First:
						if(Function_SET.Set_VOUT <= (SET_VOUT_MAX - 100U))
					{
						Function_SET.Set_VOUT+= 100;
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
						if(Function_SET.Set_VOUT < SET_VOUT_MAX)
					{
						Function_SET.Set_VOUT+= 1;
					}
					else
					{
							Function_SET.Set_VOUT= SET_VOUT_MAX;
					}
					break; //   第三位 
					default:
						if(Function_SET.Set_VOUT <= (SET_VOUT_MAX - 100U))
						{
							Function_SET.Set_VOUT+= 100;
						}
						else
						{
							Function_SET.Set_VOUT= SET_VOUT_MAX;
						}
						break;
				}
			}
			printf(" Function_SET.Set_VOUT: %d\r\n\r\n",Function_SET.Set_VOUT);
	
			
		}
		else////设置电流
		{
			if(real_dir==Reverse_State)  //逆时针转动 减
			{
				// 电流减
				switch(Function_SET.SetStepState)
				{
					case SET_State_Fourth:
					if(Function_SET.Set_IOUT>=1000)
					{
						Function_SET.Set_IOUT-= 1000;   //1.000A
					}
					else
					{
						Function_SET.Set_IOUT= SET_IOUT_MIN;
					}
					break;
					case SET_State_First:
					if(Function_SET.Set_IOUT>=100)
					{
						Function_SET.Set_IOUT-= 100;    //0.100A
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
					if(Function_SET.Set_IOUT > SET_IOUT_MIN)
					{
						Function_SET.Set_IOUT-= 1;    // 0.001A
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
					case SET_State_Fourth:
					if(Function_SET.Set_IOUT <= (SET_IOUT_MAX - 1000U))
					{
						Function_SET.Set_IOUT+= 1000;
					}
					else
					{
						Function_SET.Set_IOUT= SET_IOUT_MAX;
					}
					break;
					case SET_State_First:
					if(Function_SET.Set_IOUT <= (SET_IOUT_MAX - 100U))
					{
						Function_SET.Set_IOUT+= 100;
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
					if(Function_SET.Set_IOUT < SET_IOUT_MAX)
					{
						Function_SET.Set_IOUT+= 1;
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
	uint16_t CV_Duty;         
	uint16_t CC_Duty;
	uint16_t Vout_val;	
	uint16_t Iout_val;
	float target_v;
	float comp_v;
	float SET_Vout_val;
	float SET_Iout_val;
	if(Function_SET.PowrputState==ON_State)
	{
		Vout_val=Function_SET.Set_VOUT;
		Iout_val=Function_SET.Set_IOUT;
		//printf("Vout_val:%d\r\n\r\n",Vout_val);
		//printf("Iout_val:%d\r\n\r\n",Iout_val);

		// Set_VOUT单位10mV: /100转换为V，先做反向补偿，再匹配11分压比
		target_v = (float)Vout_val / 100.0f;
		comp_v = (target_v - VOUT_FIT_B) / VOUT_FIT_K;
		if(comp_v < 0.0f)
		{
			comp_v = 0.0f;
		}
		if(comp_v > ((float)SET_VOUT_MAX / 100.0f))
		{
			comp_v = (float)SET_VOUT_MAX / 100.0f;
		}
		SET_Vout_val = comp_v / 11.0f / 3.3f * 1440.0f;
		//printf("SET_Vout_val:%f\r\n\r\n",SET_Vout_val); 
		CV_Duty=(uint16_t)SET_Vout_val;
		//CC_Duty=(float)((((Iout_val/100*0.0254)/0.75+((Iout_val/100*0.0254)/0.75/4*30))/3.3)*1440); //采样电阻0.025R
		//CC_Duty=(float)(((Iout_val/100*0.025)/0.75+((Iout_val/100*0.025)/0.75/4*30))/3.3*1440); //采样电阻0.025R
		//CC_Duty=(uint16_t)((Iout_val/100*0.025)/3*34)/3.3*1440;

		// Set_IOUT单位1mA: /1000转换为A；采样电阻25毫欧，电流环放大约16倍
		// SET_Iout_val=(float)Iout_val/1000*0.025*16.0f/3.3*1440;
		SET_Iout_val = (float)Iout_val*0.001f * CC_RSENSE * CC_AMP_GAIN / CC_VREF * 1440.0f;
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

static void Check_Protect(void)  //保护检查
{
	static uint8_t ov_cnt = 0U;
	static uint8_t oc_cnt = 0U;
	static uint8_t ot_cnt = 0U;
	const uint8_t protect_confirm_cnt = 5U; // 10ms循环下约50ms确认，抑制瞬时毛刺误触发

	//更新最小关机时间计时器 每10ms递增
	if(Function_SET.PowrputState == OFF_State && shutdown_timer_ms < MIN_SHUTDOWN_TIME_MS)
	{
		shutdown_timer_ms += 10U;
	}

	//默认无保护
	Function_SET.ProtectState = 0;

	//过压保护
	if(MyADC.Vo > PROTECT_VOLT)
	{
		if(ov_cnt < protect_confirm_cnt)
		{
			ov_cnt++;
		}
	}
	else
	{
		ov_cnt = 0U;
	}
	//过流保护
	if(MyADC.Io > PROTECT_CURR)
	{
		if(oc_cnt < protect_confirm_cnt)
		{
			oc_cnt++;
		}
	}
	else
	{
		oc_cnt = 0U;
	}
	//过温保护
	if(MyADC.Ni > PROTECT_TEMP)
	{
		if(ot_cnt < protect_confirm_cnt)
		{
			ot_cnt++;
		}
	}
	else
	{
		ot_cnt = 0U;
	}

	if(ov_cnt >= protect_confirm_cnt)
	{
		Function_SET.ProtectState = 1;
	}
	else if(oc_cnt >= protect_confirm_cnt)
	{
		Function_SET.ProtectState = 2;
	}
	else if(ot_cnt >= protect_confirm_cnt)
	{
		Function_SET.ProtectState = 3;
	}

	if(Function_SET.ProtectState != 0)
	{
		if(Function_SET.PowrputState == ON_State)
		{
			PWMSET.PWM_Stop();
			Function_SET.PowrputState = OFF_State;
			shutdown_timer_ms = 0U; //保护触发关机时复位计时器
			printf("[PROTECT] state=%d Vo=%.3fV Io=%.3fA T=%.2fC\r\n",
			       Function_SET.ProtectState,
			       MyADC.Vo,
			       MyADC.Io,
			       MyADC.Ni);
			ov_cnt = 0U;
			oc_cnt = 0U;
			ot_cnt = 0U;
		}
	}
}


