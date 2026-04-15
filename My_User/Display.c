#include "Display.h"

static void Relay_State(void);


static void DisplayShow_Once  (void);       //ֻ��ʾһ��
static void DisplayShow_Device(void);       //�豸����״̬


static void DisplayShow_Setval(void);       //��ʾ����ֵ
static void DisplayShow_Outval(void);       //��ʾ���ֵ
static void DisplayShow_Cursor(void);       //��ʾ���

Display_Type Display=
{
  TRUE,       //��ʾһ�α�ʾ
  FALSE,      //�豸����״̬
  DisplayShow_Once,
  DisplayShow_Device,
  DisplayShow_Setval,
  DisplayShow_Outval,
  DisplayShow_Cursor
};

static void Relay_State(void)
{

}

static void DisplayShow_Once(void)
{
  for(int i=0; i<32; i++)
  {
    OLED_DrawCircle(63,31,i,OLED_UNFILLED);
    HAL_Delay(50);
    OLED_Update();
  }
  OLED_Clear();
  for(int i=32; i>0; i--)
  {
    OLED_DrawCircle(63,31,i,OLED_UNFILLED);
    HAL_Delay(50);
    OLED_Update();
  }
  HAL_Delay(500);

}

static void DisplayShow_Device(void)
{

}
static void DisplayShow_Cursor(void)
{


}

static void DisplayShow_Setval(void)
{
  if(Function_SET.PowrputState==ON_State)
  {
    OLED_ShowString(100,0,"ON",OLED_8X16);
  }
  else
  {
    OLED_ShowString(100,0,"OFF",OLED_8X16);
  }

}

static void DisplayShow_Outval(void)
{
  float I_draw=0;
  float I_show;
  float V_show;
  float P_OUT;                    //�������


  P_OUT = MyADC.Io * MyADC.Vo;

  if(Function_SET.SetMenuState==Menu_OUT_State)        //�˵����ģʽ
  {
    OLED_Printf(0,0,OLED_8X16,"VIN:%.2f",MyADC.Vin);
    OLED_Printf(0,16,OLED_8X16,"V_SET:%.2f",Function_SET.Set_VOUT);     //���õ�ѹ
    OLED_Printf(0,48,OLED_8X16,"P:%.2f",P_OUT);
    OLED_DrawRectangle(48,16,32,16,OLED_UNFILLED);
  }
  else               //�˵�����ģʽ
  {
    if(Function_SET.SetVIState==SET_V_State)   //��ѹ����
    {
      OLED_Printf(0,16,OLED_8X16,"V_SET:%.2f",Function_SET.Set_VOUT);     //���õ�ѹ
      V_show = Function_SET.Set_VOUT /25 * 100;
      I_draw = (int)(3.6*V_show-180);

      OLED_Printf(90,37,OLED_6X8,"%d",(int)V_show);
      OLED_ShowString(115,37,"%",OLED_6X8);
      OLED_DrawArc(105,40,22,180,I_draw,OLED_UNFILLED);         //��Բ��
    }
    else
    {
      OLED_Printf(0,16,OLED_8X16,"I_SET:%.2f",Function_SET.Set_IOUT);     //���õ���
      I_show = Function_SET.Set_VOUT /7.5 * 100;
      I_draw = (int)(3.6*I_show-180);

      OLED_Printf(90,37,OLED_6X8,"%d",(int)I_show);
      OLED_ShowString(115,37,"%",OLED_6X8);
      OLED_DrawArc(105,40,22,180,I_draw,OLED_UNFILLED);         //��Բ��
    }
    if(Function_SET.SetStepState==SET_State_First)
    {
      OLED_DrawRectangle(48,16,8,16,OLED_UNFILLED);      //��ʾ����
    }
    else if(Function_SET.SetStepState==SET_State_Second)
    {
      OLED_DrawRectangle(56,16,8,16,OLED_UNFILLED);
    }
    else
    {
      OLED_DrawRectangle(64,16,8,16,OLED_UNFILLED);
    }
  }

}

void My_DisplayTask(void)
{
  Display.DisplayShow_Once();    //��ʾһ��
  Display.DisplayShow_Setval();  //��ʾ����ֵ
  Display.DisplayShow_Outval();  //��ʾ���ֵ

  OLED_Update();
}
