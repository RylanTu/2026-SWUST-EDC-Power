/* Includes ------------------------------------------------------------------*/
#include "MyAdc_Apply.h"



/* Private define-------------------------------------------------------------*/

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

//HAL_DAC_Start(&DAC_HandleTypeDef, DAC_CHANNEL_1); //????DAC?¨??1
//HAL_DMA_Start(&hdma, uint32_t SrcAddress, uint32_t DstAddress, uint32_t DataLength);
//HAL_DMA_Start_IT(DMA_HandleTypeDef *hdma, uint32_t SrcAddress, uint32_t DstAddress, uint32_t DataLength);

}

static void ADC_GetNewSample (void) //获取ADC采样值
{
  uint16_t i=0;
  float SUM[4]= {0.0f,0.0f,0.0f,0.0f};
  float set_current;//2026.4.16 RylanTu:修改这个可以更改切换阈值
  float cc_enter_threshold;
  float cc_exit_threshold;

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
  MyADC.Vout[0]  = MyADC.ADC_ConverValue[3]; //PB1->输入电压

  SUM[0] += MyADC.Nin[0];
  SUM[1] += MyADC.Iout[0];
  SUM[2] += MyADC.Vin[0];
  SUM[3] += MyADC.Vout[0];

  MyADC.Ni = (SUM[0] / PW_ADC_SAMPLE_LEN) * (3.3/4095) ;//温度
  //MyADC.Io = (SUM[1] / PW_ADC_SAMPLE_LEN) * (3.3/4095) /34/0.025 ;//理论34
  //MyADC.Io = 2 * (3.3/4095) /34/0.025 ;//理论34
  //printf("S V:%d\r\n\r\n",SUM[1]);
  MyADC.Io = (SUM[1] / PW_ADC_SAMPLE_LEN) * (3.3/4095) /15/0.025 ;//运放增益约15，采样电阻0.025R
  MyADC.Vi = (SUM[2] / PW_ADC_SAMPLE_LEN) * (3.3/4095) *((100+5.1)/5.1);  //输入电压
  MyADC.Vo = (SUM[3] / PW_ADC_SAMPLE_LEN) * (3.3/4095)*110;  //ADC_VOUT经0.1倍运放和11分压链路折算

  //MyADC.Vo = (SUM[3] / PW_ADC_SAMPLE_LEN) * (3.3/4096) *((100+10)/10)/1.1;


  //MyADC.Io = (SUM[2] / PW_ADC_SAMPLE_LEN) * (3.3/4096) / 68.5 / 1.25 / 0.01 ;//理论37，第一版实测增益68.5，第二版实测增益68.5* 1.25
  if(MyADC.Vo<0.2)
  {
    MyADC.Vo = 0;
  }
  //LED.LED_Filp(LED_TEST); //TEST灯翻转一次
  if(MyADC.Io<0)
  {
    MyADC.Io = 0;
  }

  //在电流阈值附近加入迟滞，避免CV/CC状态来回抖动
  set_current = (float)Function_SET.Set_IOUT / 1000.0f;
  cc_enter_threshold = set_current * (1.0f + CC_HYS_ENTER_PCT / 100.0f);
  cc_exit_threshold = set_current * (1.0f - CC_HYS_EXIT_PCT / 100.0f);
  if(Function_SET.OutPutState == CC_State)
  {
    if(MyADC.Io <= cc_exit_threshold)
    {
      Function_SET.OutPutState=CV_State;//恒压
    }
  }
  else
  {
    if(MyADC.Io >= cc_enter_threshold)
    {
      Function_SET.OutPutState=CC_State;//恒流
    }
  }

}