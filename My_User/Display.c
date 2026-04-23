#include "Display.h"

/* 显示报警阈值 */
#define DISP_WARN_VOLT   12.0f   //输出过压报警阈值(V)
#define DISP_WARN_CURR    1.0f   //输出过流报警阈值(A)
#define DISP_WARN_TEMP   70.0f   //过温报警阈值(℃)

//static void Relay_State(void);


static void DisplayShow_Once  (void);       //只显示一次
static void DisplayShow_Device(void);       //设备运行状态


static void DisplayShow_Setval(void);       //显示设定值
static void DisplayShow_Outval(void);       //显示输出值
static void DisplayShow_Cursor(void);       //显示光标
static float Display_GetSetVoltage(void);
static float Display_GetSetCurrent(void);

static uint8_t ui_blink_phase = 0;
static uint8_t ui_blink_div = 0;

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
  // 仅首次清屏并绘制静态边框，避免显示任务反复整屏重绘。
  LCD.FillColor(0, 0, LCD_W, LCD_H, Color_BLACK);    /* 全屏清黑（x2/y2为不含边界的终点）*/
  LCD.DrawRectangle(0, 0, LCD_W-1, LCD_H-1, Color_WHITE);
  LCD.DrawRectangle(0, 108, LCD_W-1, 127, Color_GRAY);

  // 分区分隔线：增强层次感与可读性。
  LCD.DrawLine(0, 30, LCD_W-1, 30, Color_GRAY);
  LCD.DrawLine(0, 62, LCD_W-1, 62, Color_GRAY);
  LCD.DrawLine(0, 78, LCD_W-1, 78, Color_GRAY);
  LCD.DrawLine(0, 106, LCD_W-1, 106, Color_GRAY);
  LCD.DrawLine(76, 63, 76, 77, Color_GRAY);

  Display.Show_Once_Flag = FALSE;
}

static void DisplayShow_Device(void)
{
  char buf[32];

  // 状态行：左侧电源状态，右侧工作模式。
  snprintf(buf, sizeof(buf), "PWR:%s ",
           (Function_SET.PowrputState == ON_State) ? "ON " : "OFF");
  LCD.ShowString(0, 64, buf,
                         (Function_SET.PowrputState == ON_State) ? Color_GREEN : Color_RED,
                         Color_BLACK, ASCII_font_16, font_overlay_ON);

  snprintf(buf, sizeof(buf), "MODE:%s ",
           (Function_SET.OutPutState == CV_State) ? "CV" : "CC");
  LCD.ShowString(80, 64, buf,
                         (Function_SET.OutPutState == CV_State) ? Color_YELLOW : Color_CYAN,
                         Color_BLACK, ASCII_font_16, font_overlay_ON);
}

static void DisplayShow_Cursor(void)
{

}

static float Display_GetSetVoltage(void)
{
  // 内部电压单位：10mV。
  return (float)Function_SET.Set_VOUT / 100.0f;
}

static float Display_GetSetCurrent(void)
{
  // 内部电流单位：1mA。
  return (float)Function_SET.Set_IOUT / 1000.0f;
}

static void DisplayShow_Setval(void)
{
  char buf[32];
  uint16_t color_vset = (Function_SET.SetVIState == SET_V_State) ? Color_YELLOW : Color_GRAY;
  uint16_t color_iset = (Function_SET.SetVIState == SET_I_State) ? Color_CYAN : Color_GRAY;

  snprintf(buf, sizeof(buf), "VSET:%5.2fV", Display_GetSetVoltage());
  LCD.ShowString(0, 32, buf, color_vset, Color_BLACK, ASCII_font_16, font_overlay_ON);

  snprintf(buf, sizeof(buf), "ISET:%5.3fA", Display_GetSetCurrent());
  LCD.ShowString(0, 48, buf, color_iset, Color_BLACK, ASCII_font_16, font_overlay_ON);
}

static void DisplayShow_Outval(void)
{
  char buf[32];
  float p_out;                    //输出功率
  float temperature;
  uint8_t percent;
  uint16_t row_color;
  uint16_t cursor_color;
  uint16_t io_color;
  uint16_t temp_color;
  uint8_t alarm_oc;
  uint8_t alarm_ot;

  p_out = MyADC.Io * MyADC.Vo;
  temperature = MyADC.Ni;

  // UI层闪烁节拍：仅用于告警显示，不影响控制逻辑。
  if(++ui_blink_div >= 25)
  {
    ui_blink_div = 0;
    ui_blink_phase ^= 1U;
  }

  alarm_oc = (MyADC.Io > DISP_WARN_CURR) ? 1U : 0U;
  alarm_ot = (temperature > DISP_WARN_TEMP) ? 1U : 0U;
  io_color = alarm_oc ? (ui_blink_phase ? Color_RED : Color_BLACK) : Color_WHITE;
  temp_color = alarm_ot ? (ui_blink_phase ? Color_RED : Color_BLACK) : Color_WHITE;

  // 实时输出值（固定在独立行，避免与设定值和状态行冲突）。
  snprintf(buf, sizeof(buf), "VO:%5.2fV ", MyADC.Vo);
  LCD.ShowString(0, 0, buf,
                         (MyADC.Vo > DISP_WARN_VOLT) ? Color_RED : Color_WHITE,
                         Color_BLACK, ASCII_font_16, font_overlay_ON);

  snprintf(buf, sizeof(buf), "IO:%5.3fA ", MyADC.Io);
  LCD.ShowString(0, 16, buf,
                         io_color,
                         Color_BLACK, ASCII_font_16, font_overlay_ON);

  // 第二状态区：输入电压、输出功率、温度。
  snprintf(buf, sizeof(buf), "VIN:%5.2fV", MyADC.Vi);
  LCD.ShowString(0, 80, buf, Color_WHITE, Color_BLACK, ASCII_font_16, font_overlay_ON);

  snprintf(buf, sizeof(buf), "P:%4.2fW T:%4.1fC", p_out, temperature);
  LCD.ShowString(0, 96, buf,
                         temp_color,
                         Color_BLACK, ASCII_font_16, font_overlay_ON);

  // 底部进度条：依据当前调节对象显示百分比。
  if(Function_SET.SetVIState == SET_V_State)
  {
    percent = (uint8_t)(Function_SET.Set_VOUT / (float)SET_VOUT_MAX * 100.0f);
    row_color = Color_YELLOW;
  }
  else
  {
    percent = (uint8_t)(Function_SET.Set_IOUT / (float)SET_IOUT_MAX * 100.0f);
    row_color = Color_CYAN;
  }
  if(percent > 100) percent = 100;

  LCD.FillColor(0, 108, LCD_W, 128, Color_GRAY);
  if(percent > 0)
  {
    LCD.FillColor(0, 108, (uint16_t)(LCD_W * percent / 100), 128, row_color);
  }

  snprintf(buf, sizeof(buf), "%3d%%", percent);
  LCD.ShowString(120, 80, buf, row_color, Color_BLACK, ASCII_font_16, font_overlay_ON);

  // 设置模式下，用矩形高亮当前调节行。
  if(Function_SET.SetMenuState == Menu_SET_State)
  {
    // 先擦除两条候选高亮边框，避免切换时残留旧框。
    LCD.DrawRectangle(0, 32, LCD_W-1, 47, Color_BLACK);
    LCD.DrawRectangle(0, 48, LCD_W-1, 63, Color_BLACK);

    cursor_color = (Function_SET.SetVIState == SET_V_State) ? Color_YELLOW : Color_CYAN;
    if(Function_SET.SetVIState == SET_V_State)
    {
      LCD.DrawRectangle(0, 32, LCD_W-1, 47, cursor_color);
    }
    else
    {
      LCD.DrawRectangle(0, 48, LCD_W-1, 63, cursor_color);
    }
  }
}

void My_DisplayTask(void)
{
  static uint8_t disp_tick = 0;

  if(Display.Show_Once_Flag == TRUE)
  {
    Display.DisplayShow_Once();  //显示一次
  }

  disp_tick++;

  /* 状态和设定值低频刷新，减少不必要的重复刷屏 */
  if((disp_tick % 5U) == 0U)
  {
    Display.DisplayShow_Device();  //显示设备状态
    Display.DisplayShow_Setval();  //显示设定值
  }

  /* 输出值中频刷新，兼顾实时性与总线稳定性 */
  if((disp_tick % 2U) == 0U)
  {
    Display.DisplayShow_Outval();  //显示输出值
  }
}
