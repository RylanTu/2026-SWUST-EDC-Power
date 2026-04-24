#include "Display.h"

/* 显示报警阈值 */
#define DISP_WARN_VOLT   12.0f   //输出过压报警阈值(V)
#define DISP_WARN_CURR    1.0f   //输出过流报警阈值(A)
#define DISP_WARN_TEMP   70.0f   //过温报警阈值(℃)

/* UI坐标布局 */
#define UI_X_SPLIT          80U
#define UI_Y_HEADER_END     24U
#define UI_Y_ROW1_END       48U
#define UI_Y_ROW2_END       72U
#define UI_Y_INFO_END      104U

#define UI_Y_HEADER_TEXT      4U
#define UI_Y_ROW1_TEXT       28U
#define UI_Y_ROW2_TEXT       52U
#define UI_Y_INFO1_TEXT      74U
#define UI_Y_INFO2_TEXT      88U

#define UI_PWR_BADGE_X1      40U
#define UI_PWR_BADGE_Y1       2U
#define UI_PWR_BADGE_X2      72U
#define UI_PWR_BADGE_Y2      22U

#define UI_MODE_BADGE_X1    126U
#define UI_MODE_BADGE_Y1      2U
#define UI_MODE_BADGE_X2    156U
#define UI_MODE_BADGE_Y2     22U

#define UI_BAR_X1             4U
#define UI_BAR_Y1           111U
#define UI_BAR_X2           156U
#define UI_BAR_Y2           124U

//static void Relay_State(void);


static void DisplayShow_Once  (void);       //只显示一次
static void DisplayShow_Device(void);       //设备运行状态


static void DisplayShow_Setval(void);       //显示设定值
static void DisplayShow_Outval(void);       //显示输出值
static void DisplayShow_Cursor(void);       //显示光标
static int32_t Display_RoundToScale(float value, int32_t scale);
static void Display_FormatFixed(char *buf, size_t len, const char *label,
                                int32_t scaled_val, int32_t scale,
                                uint8_t frac_digits, char unit);
static void Display_DrawStatusBadge(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,
                                    uint16_t bg_color, uint16_t txt_color,
                                    const char *txt, uint16_t txt_x, uint16_t txt_y);

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
  LCD.FillColor(0, 0, LCD_W, LCD_H, Color_WHITE);    /* 全屏清白（x2/y2为不含边界的终点）*/
  LCD.DrawRectangle(0, 0, LCD_W - 1U, LCD_H - 1U, Color_BLACK);

  // 分区分隔线。
  LCD.DrawLine(0, UI_Y_HEADER_END, LCD_W - 1U, UI_Y_HEADER_END, Color_BLACK);
  LCD.DrawLine(0, UI_Y_ROW1_END, LCD_W - 1U, UI_Y_ROW1_END, Color_BLACK);
  LCD.DrawLine(0, UI_Y_ROW2_END, LCD_W - 1U, UI_Y_ROW2_END, Color_BLACK);
  LCD.DrawLine(0, UI_Y_INFO_END, LCD_W - 1U, UI_Y_INFO_END, Color_BLACK);
  LCD.DrawLine(UI_X_SPLIT, UI_Y_ROW1_END, UI_X_SPLIT, UI_Y_ROW2_END, Color_BLACK);

  // 顶栏固定文本。
  LCD.ShowString(4, UI_Y_HEADER_TEXT, "PWR:", Color_BLACK, Color_WHITE, ASCII_font_16, font_overlay_OFF);
  LCD.ShowString(84, UI_Y_HEADER_TEXT, "MODE:", Color_BLACK, Color_WHITE, ASCII_font_16, font_overlay_OFF);

  // 进度条静态边框和标题。
  LCD.ShowString(4, 106, "BAR", Color_BLACK, Color_WHITE, ASCII_font_16, font_overlay_OFF);
  LCD.DrawRectangle(UI_BAR_X1 - 1U, UI_BAR_Y1 - 1U, UI_BAR_X2 + 1U, UI_BAR_Y2 + 1U, Color_BLACK);

  Display.Show_Once_Flag = FALSE;
}

static void Display_DrawStatusBadge(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,
                                    uint16_t bg_color, uint16_t txt_color,
                                    const char *txt, uint16_t txt_x, uint16_t txt_y)
{
  LCD.FillColor(x1, y1, x2, y2, (LCD_Color_t)bg_color);
  LCD.DrawRectangle(x1 - 1U, y1 - 1U, x2, y2, Color_BLACK);
  LCD.ShowString(txt_x, txt_y, txt, txt_color, (LCD_Color_t)bg_color, ASCII_font_16, font_overlay_OFF);
}

static void DisplayShow_Device(void)
{
  // 顶栏状态色块：PWR和MODE。
  if(Function_SET.PowrputState == ON_State)
  {
    Display_DrawStatusBadge(UI_PWR_BADGE_X1, UI_PWR_BADGE_Y1, UI_PWR_BADGE_X2, UI_PWR_BADGE_Y2,
                            Color_GREEN, Color_BLACK, "ON", 48, UI_Y_HEADER_TEXT);
  }
  else
  {
    Display_DrawStatusBadge(UI_PWR_BADGE_X1, UI_PWR_BADGE_Y1, UI_PWR_BADGE_X2, UI_PWR_BADGE_Y2,
                            Color_RED, Color_WHITE, "OFF", 44, UI_Y_HEADER_TEXT);
  }

  if(Function_SET.OutPutState == CV_State)
  {
    Display_DrawStatusBadge(UI_MODE_BADGE_X1, UI_MODE_BADGE_Y1, UI_MODE_BADGE_X2, UI_MODE_BADGE_Y2,
                            Color_YELLOW, Color_BLACK, "CV", 132, UI_Y_HEADER_TEXT);
  }
  else
  {
    Display_DrawStatusBadge(UI_MODE_BADGE_X1, UI_MODE_BADGE_Y1, UI_MODE_BADGE_X2, UI_MODE_BADGE_Y2,
                            Color_BLUE, Color_WHITE, "CC", 132, UI_Y_HEADER_TEXT);
  }
}

static void DisplayShow_Cursor(void)
{

}

static int32_t Display_RoundToScale(float value, int32_t scale)
{
  if(value >= 0.0f)
  {
    return (int32_t)(value * (float)scale + 0.5f);
  }
  return (int32_t)(value * (float)scale - 0.5f);
}

static void Display_FormatFixed(char *buf, size_t len, const char *label,
                                int32_t scaled_val, int32_t scale,
                                uint8_t frac_digits, char unit)
{
  int32_t abs_val = (scaled_val < 0) ? -scaled_val : scaled_val;
  int32_t integer_part = abs_val / scale;
  int32_t frac_part = abs_val % scale;
  const char *sign = (scaled_val < 0) ? "-" : "";

  if(frac_digits == 3U)
  {
    if(unit != 0)
    {
      snprintf(buf, len, "%s:%s%ld.%03ld%c", label, sign,
               (long)integer_part, (long)frac_part, unit);
    }
    else
    {
      snprintf(buf, len, "%s:%s%ld.%03ld", label, sign,
               (long)integer_part, (long)frac_part);
    }
  }
  else if(frac_digits == 2U)
  {
    if(unit != 0)
    {
      snprintf(buf, len, "%s:%s%ld.%02ld%c", label, sign,
               (long)integer_part, (long)frac_part, unit);
    }
    else
    {
      snprintf(buf, len, "%s:%s%ld.%02ld", label, sign,
               (long)integer_part, (long)frac_part);
    }
  }
  else if(frac_digits == 1U)
  {
    if(unit != 0)
    {
      snprintf(buf, len, "%s:%s%ld.%01ld%c", label, sign,
               (long)integer_part, (long)frac_part, unit);
    }
    else
    {
      snprintf(buf, len, "%s:%s%ld.%01ld", label, sign,
               (long)integer_part, (long)frac_part);
    }
  }
  else
  {
    if(unit != 0)
    {
      snprintf(buf, len, "%s:%s%ld%c", label, sign,
               (long)integer_part, unit);
    }
    else
    {
      snprintf(buf, len, "%s:%s%ld", label, sign,
               (long)integer_part);
    }
  }
}

static void DisplayShow_Setval(void)
{
  char buf[32];
  uint16_t color_vset = (Function_SET.SetVIState == SET_V_State) ? Color_DARKBLUE : Color_GRAYBLUE;
  uint16_t color_iset = (Function_SET.SetVIState == SET_I_State) ? Color_BRRED : Color_GRAYBLUE;

  Display_FormatFixed(buf, sizeof(buf), "VSET", (int32_t)Function_SET.Set_VOUT,
                      100, 2, 0);
  LCD.ShowString(4, UI_Y_ROW1_TEXT, buf, color_vset, Color_WHITE, ASCII_font_16, font_overlay_OFF);

  Display_FormatFixed(buf, sizeof(buf), "ISET", (int32_t)Function_SET.Set_IOUT,
                      1000, 3, 0);
  LCD.ShowString(4, UI_Y_ROW2_TEXT, buf, color_iset, Color_WHITE, ASCII_font_16, font_overlay_OFF);
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
  uint16_t vo_color;
  uint8_t alarm_oc;
  uint8_t alarm_ot;
  int32_t vo_100;
  int32_t io_1000;
  int32_t vin_100;
  int32_t p_100;
  int32_t t_10;
  int32_t p_int;
  int32_t p_frac;
  int32_t t_abs;
  const char *t_sign;

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
  io_color = alarm_oc ? (ui_blink_phase ? Color_RED : Color_BLACK) : Color_BLACK;
  temp_color = alarm_ot ? (ui_blink_phase ? Color_RED : Color_BLACK) : Color_BLACK;
  vo_color = (MyADC.Vo > DISP_WARN_VOLT) ? Color_RED : Color_BLACK;

  vo_100 = Display_RoundToScale(MyADC.Vo, 100);
  io_1000 = Display_RoundToScale(MyADC.Io, 1000);
  vin_100 = Display_RoundToScale(MyADC.Vi, 100);
  p_100 = Display_RoundToScale(p_out, 100);
  t_10 = Display_RoundToScale(temperature, 10);

  // 进度百分比沿用当前调节对象（电压/电流）
  if(Function_SET.SetVIState == SET_V_State)
  {
    percent = (uint8_t)(Function_SET.Set_VOUT / (float)SET_VOUT_MAX * 100.0f);
  }
  else
  {
    percent = (uint8_t)(Function_SET.Set_IOUT / (float)SET_IOUT_MAX * 100.0f);
  }
  if(percent > 100U)
  {
    percent = 100U;
  }

  // 右侧实测值与左侧设定值对齐显示。
  Display_FormatFixed(buf, sizeof(buf), "VOUT", vo_100, 100, 2, 0);
  LCD.ShowString(84, UI_Y_ROW1_TEXT, buf, vo_color, Color_WHITE, ASCII_font_16, font_overlay_OFF);

  Display_FormatFixed(buf, sizeof(buf), "IOUT", io_1000, 1000, 3, 0);
  LCD.ShowString(84, UI_Y_ROW2_TEXT, buf, io_color, Color_WHITE, ASCII_font_16, font_overlay_OFF);

  // 信息区：百分比/VIN/P/T。
  snprintf(buf, sizeof(buf), "%%:%3d", percent);
  LCD.ShowString(4, UI_Y_INFO1_TEXT, buf, Color_DARKBLUE, Color_WHITE, ASCII_font_16, font_overlay_OFF);

  Display_FormatFixed(buf, sizeof(buf), "VIN", vin_100, 100, 2, 0);
  LCD.ShowString(52, UI_Y_INFO1_TEXT, buf, Color_BLACK, Color_WHITE, ASCII_font_16, font_overlay_OFF);

  p_int = p_100 / 100;
  p_frac = p_100 % 100;
  if(p_frac < 0)
  {
    p_frac = -p_frac;
  }
  t_sign = (t_10 < 0) ? "-" : "";
  t_abs = (t_10 < 0) ? -t_10 : t_10;
  snprintf(buf, sizeof(buf), "P:%ld.%02ld T:%s%ld.%01ld",
           (long)p_int, (long)p_frac,
           t_sign, (long)(t_abs / 10), (long)(t_abs % 10));
  LCD.ShowString(4, UI_Y_INFO2_TEXT, buf,
                         temp_color,
                         Color_WHITE, ASCII_font_16, font_overlay_OFF);

  // 底部进度条颜色：CV棕色，CC蓝色（白底下对比度更高）。
  row_color = (Function_SET.OutPutState == CV_State) ? Color_BROWN : Color_BLUE;

  LCD.FillColor(UI_BAR_X1, UI_BAR_Y1, UI_BAR_X2, UI_BAR_Y2, Color_GRAY);
  if(percent > 0)
  {
    LCD.FillColor(UI_BAR_X1, UI_BAR_Y1,
                  (uint16_t)(UI_BAR_X1 + ((UI_BAR_X2 - UI_BAR_X1) * percent / 100U)),
                  UI_BAR_Y2, row_color);
  }

  snprintf(buf, sizeof(buf), "%3d%%", percent);
  LCD.ShowString(124, 106, buf, Color_DARKBLUE, Color_WHITE, ASCII_font_16, font_overlay_OFF);

  // 设置模式下，用矩形高亮当前调节行。
  if(Function_SET.SetMenuState == Menu_SET_State)
  {
    // 先擦除两条候选高亮边框，避免切换时残留旧框。
    LCD.DrawRectangle(0, UI_Y_ROW1_END - 16U, LCD_W - 1U, UI_Y_ROW1_END - 1U, Color_WHITE);
    LCD.DrawRectangle(0, UI_Y_ROW2_END - 16U, LCD_W - 1U, UI_Y_ROW2_END - 1U, Color_WHITE);

    cursor_color = (Function_SET.SetVIState == SET_V_State) ? Color_DARKBLUE : Color_BRRED;
    if(Function_SET.SetVIState == SET_V_State)
    {
      LCD.DrawRectangle(0, UI_Y_ROW1_END - 16U, LCD_W - 1U, UI_Y_ROW1_END - 1U, cursor_color);
    }
    else
    {
      LCD.DrawRectangle(0, UI_Y_ROW2_END - 16U, LCD_W - 1U, UI_Y_ROW2_END - 1U, cursor_color);
    }
  }
  else
  {
    LCD.DrawRectangle(0, UI_Y_ROW1_END - 16U, LCD_W - 1U, UI_Y_ROW1_END - 1U, Color_WHITE);
    LCD.DrawRectangle(0, UI_Y_ROW2_END - 16U, LCD_W - 1U, UI_Y_ROW2_END - 1U, Color_WHITE);
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
