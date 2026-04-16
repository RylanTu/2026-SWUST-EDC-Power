#include "Display.h"

//static void Relay_State(void);


static void DisplayShow_Once  (void);       //只显示一次
static void DisplayShow_Device(void);       //设备运行状态


static void DisplayShow_Setval(void);       //显示设定值
static void DisplayShow_Outval(void);       //显示输出值
static void DisplayShow_Cursor(void);       //显示光标

Display_Type Display=
{
  TRUE,       //显示一次标志
  FALSE,      //设备运行状态
  DisplayShow_Once,
  DisplayShow_Device,
  DisplayShow_Setval,
  DisplayShow_Outval,
  DisplayShow_Cursor
};
/*
static void Relay_State(void)
{

}
*/
static void DisplayShow_Once(void)
{
  //开机动画：依次填充红绿蓝后清屏
  TFT_LCD.TFT_FillColor(0, 0, LCD_W-1, LCD_H-1, Color_RED);
  HAL_Delay(300);
  TFT_LCD.TFT_FillColor(0, 0, LCD_W-1, LCD_H-1, Color_GREEN);
  HAL_Delay(300);
  TFT_LCD.TFT_FillColor(0, 0, LCD_W-1, LCD_H-1, Color_BLUE);
  HAL_Delay(300);
  TFT_LCD.TFT_FillColor(0, 0, LCD_W-1, LCD_H-1, Color_BLACK);
  HAL_Delay(200);
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
    TFT_LCD.TFT_ShowString(120, 0, "ON ", Color_GREEN, Color_BLACK, ASCII_font_16, font_overlay_ON);
  }
  else
  {
    TFT_LCD.TFT_ShowString(120, 0, "OFF", Color_RED, Color_BLACK, ASCII_font_16, font_overlay_ON);
  }
}

static void DisplayShow_Outval(void)
{
  char buf[32];
  float P_OUT;                    //输出功率
  uint8_t percent;

  P_OUT = MyADC.Io * MyADC.Vo;

  if(Function_SET.SetMenuState==Menu_OUT_State)        //菜单输出模式
  {
    snprintf(buf, sizeof(buf), "VIN :%.2fV ", MyADC.Vi);
    TFT_LCD.TFT_ShowString(0, 0,  buf, Color_WHITE,  Color_BLACK, ASCII_font_16, font_overlay_ON);

    snprintf(buf, sizeof(buf), "VSET:%.2fV ", (float)Function_SET.Set_VOUT / 100.0f);   //设定电压
    TFT_LCD.TFT_ShowString(0, 32, buf, Color_YELLOW, Color_BLACK, ASCII_font_16, font_overlay_ON);

    snprintf(buf, sizeof(buf), "P   :%.2fW ", P_OUT);
    TFT_LCD.TFT_ShowString(0, 96, buf, Color_WHITE,  Color_BLACK, ASCII_font_16, font_overlay_ON);

    //光标矩形框：覆盖设定值行
    TFT_LCD.TFT_DrawRectangle(0, 32, LCD_W-1, 63, Color_YELLOW);
  }
  else               //菜单设置模式
  {
    if(Function_SET.SetVIState==SET_V_State)   //电压调节
    {
      snprintf(buf, sizeof(buf), "VSET:%.2fV ", (float)Function_SET.Set_VOUT / 100.0f);   //设定电压
      TFT_LCD.TFT_ShowString(0, 32, buf, Color_YELLOW, Color_BLACK, ASCII_font_16, font_overlay_ON);
      percent = (uint8_t)(Function_SET.Set_VOUT / 2700.0f * 100.0f);
    }
    else
    {
      snprintf(buf, sizeof(buf), "ISET:%.3fA ", (float)Function_SET.Set_IOUT / 1000.0f);   //设定电流
      TFT_LCD.TFT_ShowString(0, 32, buf, Color_CYAN,   Color_BLACK, ASCII_font_16, font_overlay_ON);
      percent = (uint8_t)(Function_SET.Set_IOUT / 7500.0f  * 100.0f);
    }
    if(percent > 100) percent = 100;

    //百分比文字
    snprintf(buf, sizeof(buf), "%3d%%", percent);
    TFT_LCD.TFT_ShowString(120, 64, buf, Color_WHITE, Color_BLACK, ASCII_font_16, font_overlay_ON);

    //进度条背景（灰色）
    TFT_LCD.TFT_FillColor(0, 108, LCD_W-1, 127, Color_GRAY);
    //进度条填充（绿色）
    if(percent > 0)
    {
      TFT_LCD.TFT_FillColor(0, 108, (uint16_t)((LCD_W-1) * percent / 100), 127, Color_GREEN);
    }

    //光标框：指示当前调节位
    if(Function_SET.SetStepState==SET_State_First)
    {
      TFT_LCD.TFT_DrawRectangle(48, 32, 55, 63, Color_WHITE);     //显示光标（最高位）
    }
    else if(Function_SET.SetStepState==SET_State_Second)
    {
      TFT_LCD.TFT_DrawRectangle(56, 32, 63, 63, Color_WHITE);
    }
    else
    {
      TFT_LCD.TFT_DrawRectangle(64, 32, 71, 63, Color_WHITE);
    }
  }
}

void My_DisplayTask(void)
{
  Display.DisplayShow_Once();    //显示一次
  Display.DisplayShow_Setval();  //显示设定值
  Display.DisplayShow_Outval();  //显示输出值
}
