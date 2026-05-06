/* Includes ------------------------------------------------------------------*/
#include "MyAdc_Apply.h"
#include <math.h>



/* Private define-------------------------------------------------------------*/
// /* 输出电压ADC采样校准模型（最小二乘拟合，12组实测数据）:
//  * V_adc_raw = ADC_VO_FIT_K * V_real + ADC_VO_FIT_B
//  * 反向补偿: V_real = (V_adc_raw - ADC_VO_FIT_B) / ADC_VO_FIT_K
//  */
// #define ADC_VO_FIT_K    1.09930f   /* ADC采样增益误差（实测斜率） */
// #define ADC_VO_FIT_B    0.02788f   /* ADC采样偏置误差（实测截距，V） */

/* 输出电压ADC采样校准模型（最小二乘拟合，12组实测数据）:
 * V_adc_raw = ADC_VO_FIT_K * V_real + ADC_VO_FIT_B
 * 反向补偿: V_real = (V_adc_raw - ADC_VO_FIT_B) / ADC_VO_FIT_K
 */
#define ADC_VO_FIT_K    1.09530f   /* ADC采样增益误差（实测斜率） */
#define ADC_VO_FIT_B    0.0f   /* ADC采样偏置误差（实测截距，V） */


/* Private variables----------------------------------------------------------*/

/* Private function prototypes------------------------------------------------*/
//static void     Get_NTC_Voltage(void);       //获取温度电压
static void ADC_Initial_Setup (void);
static void ADC_GetNewSample (void);

/* Public variables-----------------------------------------------------------*/
MyADC_t  MyADC =
{
  {0},
  {0},
  {0},
  {0},
  {0},
  0.0,
  0.0,
  0.0,
  0.0,
  ADC_Initial_Setup,
  ADC_GetNewSample
};

static void ADC_Initial_Setup(void)  //ADC初始化设置
{
//HAL_TIM_Base_Start(&htim3);    //启动定时器3
  HAL_Delay(1);    //等待初始化稳定
  HAL_ADCEx_Calibration_Start(&hadc1); //校准ADC
  HAL_ADC_Start_DMA(&hadc1,(uint32_t*)MyADC.ADC_ConverValue,(uint32_t) 4);   //启动DAC，DMA模式

//HAL_DAC_Start(&DAC_HandleTypeDef, DAC_CHANNEL_1); //启动DAC通道1
//HAL_DMA_Start(&hdma, uint32_t SrcAddress, uint32_t DstAddress, uint32_t DataLength);
//HAL_DMA_Start_IT(DMA_HandleTypeDef *hdma, uint32_t SrcAddress, uint32_t DstAddress, uint32_t DataLength);

}

static void ADC_GetNewSample (void) //获取ADC采样值
{
  uint16_t i=0;
  float SUM[4]= {0.0f,0.0f,0.0f,0.0f};
  float set_current;
  float cc_enter_threshold;
  float cc_exit_threshold;
  static uint8_t cc_enter_cnt = 0;
  static uint8_t cc_exit_cnt = 0;

  for(i=PW_ADC_SAMPLE_LEN-1; i>0; i--)//滑动（递推）平均滤波
  {
    MyADC.Nin[i] = MyADC.Nin[i-1];  //NTC温度
    MyADC.Iout[i] = MyADC.Iout[i-1]; //输出电流
    MyADC.Vin[i]  = MyADC.Vin[i-1]; //输入电压
    MyADC.Vout[i] = MyADC.Vout[i-1]; //输出电压

    SUM[0] += MyADC.Nin[i];
    SUM[1] += MyADC.Iout[i];
    SUM[2] += MyADC.Vin[i];
    SUM[3] += MyADC.Vout[i];



  }
  MyADC.Nin[0]   = MyADC.ADC_ConverValue[0]; //PA6->输入温度
  MyADC.Iout[0]  = MyADC.ADC_ConverValue[1]; //PA7->输出电流
  MyADC.Vin [0]  = MyADC.ADC_ConverValue[2]; //PB0->输入电压
  MyADC.Vout[0]  = MyADC.ADC_ConverValue[3]; //PB1->输出电压

  SUM[0] += MyADC.Nin[0];
  SUM[1] += MyADC.Iout[0];
  SUM[2] += MyADC.Vin[0];
  SUM[3] += MyADC.Vout[0];

  {
    float v_ntc = (SUM[0] / PW_ADC_SAMPLE_LEN) * (3.3f / 4095.0f);
    float r_ntc;

    if(v_ntc >= 3.299f)
    {
      v_ntc = 3.299f;
    }
    r_ntc = 10000.0f * v_ntc / (3.3f - v_ntc);
    MyADC.Ni = 1.0f / (logf(r_ntc / 10000.0f) / 3435.0f + 1.0f / 298.15f) - 273.15f;//温度(℃)//2026.4.17 RylanTu:基于pjz的基础上修改
  }
  //MyADC.Io = (SUM[1] / PW_ADC_SAMPLE_LEN) * (3.3/4095) /34/0.025 ;//理论34
  //MyADC.Io = 2 * (3.3/4095) /34/0.025 ;//理论34
  //printf("S V:%d\r\n\r\n",SUM[1]);
  //MyADC.Io = (SUM[1] / PW_ADC_SAMPLE_LEN) * (3.3/4095) /15/0.025;//运放增益，采样电阻0.025R
  MyADC.Io = (SUM[1] / PW_ADC_SAMPLE_LEN) * (3.313f / 4095.0f) * 2.468189f;   //输出电流
  MyADC.Vi = (SUM[2] / PW_ADC_SAMPLE_LEN) * (3.29f / 4095.0f) * 11.0f;  //输入电压
  


  {
    float vo_raw = (SUM[3] / PW_ADC_SAMPLE_LEN) * (3.29f / 4095.0f) * 11.0f;
    MyADC.Vo = (vo_raw - ADC_VO_FIT_B) / ADC_VO_FIT_K;  //输出电压（ADC校准补偿后）
  }

  //MyADC.Vo = (SUM[3] / PW_ADC_SAMPLE_LEN) * (3.3/4096) *((100+10)/10)/1.1;


  //MyADC.Io = (SUM[2] / PW_ADC_SAMPLE_LEN) * (3.3/4096) / 68.5 / 1.25 / 0.01 ;//理论37，第一版实测增益68.5，第二版实测增益68.5* 1.25
  if(MyADC.Vo < 0.15f)  //输出电压偏置补偿：0.15V以下视为0
  {
    MyADC.Vo = 0.0f;
  }
  //LED.LED_Filp(LED_TEST); //TEST灯翻转一次
  if(MyADC.Io < 0.003f)  //无负载偏置补偿：0.003A以下视为0
  {
    MyADC.Io = 0.0f;
  }

  // 负载变化时自动在CV/CC间平滑切换：
  // 仅在输出界面生效，避免设置界面下手动切换被自动逻辑抢回。
  if((Function_SET.PowrputState == ON_State) &&
     (Function_SET.SetMenuState == Menu_OUT_State))
  {
    set_current = (float)Function_SET.Set_IOUT / 1000.0f;

    if(Function_SET.Set_IOUT == SET_IOUT_MIN)
    {
      Function_SET.OutPutState = CV_State;
      cc_enter_cnt = 0U;
      cc_exit_cnt = 0U;
      return;
    }

    cc_enter_threshold = set_current * (1.0f + CC_HYS_ENTER_PCT / 100.0f);
    cc_exit_threshold = set_current * (1.0f - CC_HYS_EXIT_PCT / 100.0f);

    if(Function_SET.OutPutState == CC_State)
    {
      if(MyADC.Io <= cc_exit_threshold)
      {
        if(++cc_exit_cnt >= 3U)
        {
          Function_SET.OutPutState = CV_State;
          cc_exit_cnt = 0U;
        }
      }
      else
      {
        cc_exit_cnt = 0U;
      }
      cc_enter_cnt = 0U;
    }
    else
    {
      if(MyADC.Io >= cc_enter_threshold)
      {
        if(++cc_enter_cnt >= 3U)
        {
          Function_SET.OutPutState = CC_State;
          cc_enter_cnt = 0U;
        }
      }
      else
      {
        cc_enter_cnt = 0U;
      }
      cc_exit_cnt = 0U;
    }
  }
  else
  {
    cc_enter_cnt = 0U;
    cc_exit_cnt = 0U;
  }

}