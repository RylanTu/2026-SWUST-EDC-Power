#include "Display.h"
#include <string.h>

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

#define UI_COL_LEFT_X2       54U
#define UI_COL_MID_X1        54U
#define UI_COL_MID_X2       108U
#define UI_COL_RIGHT_X1     108U

#define UI_Y_HEADER_TEXT      4U
#define UI_Y_ROW1_TEXT       30U
#define UI_Y_ROW2_TEXT       54U
#define UI_Y_INFO1_TEXT      82U

#define UI_PWR_BADGE_X1      36U
#define UI_PWR_BADGE_Y1       1U
#define UI_PWR_BADGE_X2      UI_X_SPLIT
#define UI_PWR_BADGE_Y2      24U

#define UI_MODE_BADGE_X1    124U
#define UI_MODE_BADGE_Y1      1U
#define UI_MODE_BADGE_X2    LCD_W
#define UI_MODE_BADGE_Y2     24U

#define UI_BAR_X1             0U
#define UI_BAR_Y1          (UI_Y_INFO_END + 1U)
#define UI_BAR_TEXT_X       126U
#define UI_BAR_X2           (UI_BAR_TEXT_X - 2U)
#define UI_BAR_Y2           LCD_H

//static void Relay_State(void);


static void DisplayShow_Once  (void);       //只显示一次
static void DisplayShow_Device(void);       //设备运行状态


static void DisplayShow_Setval(void);       //显示设定值
static void DisplayShow_Outval(void);       //显示输出值
static void DisplayShow_Cursor(void);       //显示光标
static uint16_t Display_ModeThemeColor(void);
static int32_t Display_RoundToScale(float value, int32_t scale);
static void Display_FormatFixed(char *buf, size_t len, const char *label,
                                int32_t scaled_val, int32_t scale,
                                uint8_t frac_digits, char unit);

static uint8_t ui_blink_phase = 0;
static uint8_t ui_blink_div = 0;
static uint8_t ui_last_pwr_state = 0xFFU;
static uint8_t ui_last_mode_state = 0xFFU;
static uint8_t ui_last_percent = 0xFFU;
static uint8_t ui_last_setvi_state = 0xFFU;
static uint8_t ui_last_theme_mode_state = 0xFFU;
static uint8_t ui_last_bar_mode_state = 0xFFU;
static uint8_t ui_last_cursor_menu_state = 0xFFU;
static uint8_t ui_last_cursor_vi_state = 0xFFU;
static uint8_t ui_last_cursor_step_state = 0xFFU;
static uint8_t ui_last_alarm_ov = 0xFFU;
static uint8_t ui_last_alarm_oc = 0xFFU;
static uint8_t ui_last_alarm_ot = 0xFFU;

static char ui_last_vset[32] = "";
static char ui_last_iset[32] = "";
static char ui_last_vout[32] = "";
static char ui_last_iout[32] = "";
static char ui_last_vin[24] = "";
static char ui_last_pwr[24] = "";
static char ui_last_tmp[24] = "";

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
  LCD.DrawLine(UI_X_SPLIT, 0, UI_X_SPLIT, UI_Y_ROW2_END, Color_BLACK);
  LCD.DrawLine(UI_COL_LEFT_X2 - 1U, UI_Y_ROW2_END, UI_COL_LEFT_X2 - 1U, UI_Y_INFO_END, Color_BLACK);
  LCD.DrawLine(UI_COL_MID_X2 - 1U, UI_Y_ROW2_END, UI_COL_MID_X2 - 1U, UI_Y_INFO_END, Color_BLACK);

  // 顶部状态色块与进度区初始清空。
  LCD.FillColor(UI_BAR_X1, UI_BAR_Y1, UI_BAR_X2, UI_BAR_Y2, Color_GRAY);
  LCD.FillColor(UI_BAR_X2, UI_BAR_Y1, LCD_W, UI_BAR_Y2, Color_WHITE);

  Display.Show_Once_Flag = FALSE;
}

static uint16_t Display_ModeThemeColor(void)
{
  return (Function_SET.OutPutState == CV_State) ? Color_YELLOW : Color_BLUE;
}

static void DisplayShow_Device(void)
{
  uint16_t mode_txt_color;

  if((ui_last_pwr_state == Function_SET.PowrputState) &&
     (ui_last_mode_state == Function_SET.OutPutState))
  {
    return;
  }

  // 顶栏整格铺色：包含 PWR:/MODE: 标签区域。
  if(Function_SET.PowrputState == ON_State)
  {
    LCD.FillColor(0, UI_PWR_BADGE_Y1, UI_X_SPLIT, UI_PWR_BADGE_Y2, Color_GREEN);
    LCD.ShowString(4, UI_Y_HEADER_TEXT, "PWR:", Color_BLACK, Color_GREEN, ASCII_font_16, font_overlay_OFF);
    LCD.ShowString(48, UI_Y_HEADER_TEXT, "ON", Color_BLACK, Color_GREEN, ASCII_font_16, font_overlay_OFF);
  }
  else
  {
    LCD.FillColor(0, UI_PWR_BADGE_Y1, UI_X_SPLIT, UI_PWR_BADGE_Y2, Color_RED);
    LCD.ShowString(4, UI_Y_HEADER_TEXT, "PWR:", Color_WHITE, Color_RED, ASCII_font_16, font_overlay_OFF);
    LCD.ShowString(44, UI_Y_HEADER_TEXT, "OFF", Color_WHITE, Color_RED, ASCII_font_16, font_overlay_OFF);
  }

  mode_txt_color = (Function_SET.OutPutState == CV_State) ? Color_BLACK : Color_WHITE;
  LCD.FillColor(UI_X_SPLIT, UI_MODE_BADGE_Y1, LCD_W, UI_MODE_BADGE_Y2, Display_ModeThemeColor());
  LCD.ShowString(84, UI_Y_HEADER_TEXT, "MODE:", mode_txt_color, Display_ModeThemeColor(), ASCII_font_16, font_overlay_OFF);

  if(Function_SET.OutPutState == CV_State)
  {
    LCD.ShowString(132, UI_Y_HEADER_TEXT, "CV", Color_BLACK, Display_ModeThemeColor(), ASCII_font_16, font_overlay_OFF);
  }
  else
  {
    LCD.ShowString(132, UI_Y_HEADER_TEXT, "CC", Color_WHITE, Display_ModeThemeColor(), ASCII_font_16, font_overlay_OFF);
  }

  ui_last_pwr_state = Function_SET.PowrputState;
  ui_last_mode_state = Function_SET.OutPutState;
}

static void DisplayShow_Cursor(void)
{
  char set_buf[16];
  uint16_t x_base;
  uint16_t y_base;
  uint16_t x_sel;
  uint8_t idx;

  if(Function_SET.SetMenuState != Menu_SET_State)
  {
    return;
  }

  if(Function_SET.SetVIState == SET_V_State)
  {
    // 与 DisplayShow_Setval 保持相同格式，避免小于10V时索引错位。
    snprintf(set_buf, sizeof(set_buf), "%lu.%02lu",
             (unsigned long)(Function_SET.Set_VOUT / 100U),
             (unsigned long)(Function_SET.Set_VOUT % 100U));
    x_base = 4U + (5U * 6U); // "VSET:" 后
    y_base = UI_Y_ROW1_TEXT;
    if(Function_SET.SetStepState == SET_State_First)
    {
      idx = (uint8_t)(strlen(set_buf) - 1U);
    }
    else if(Function_SET.SetStepState == SET_State_Second)
    {
      idx = (uint8_t)(strlen(set_buf) - 2U);
    }
    else
    {
      if((Function_SET.Set_VOUT / 100U) >= 10U)
      {
        idx = 1U;
      }
      else
      {
        idx = 0U;
      }
    }
  }
  else
  {
    // 固定宽度："0.500"
    snprintf(set_buf, sizeof(set_buf), "%1lu.%03lu",
             (unsigned long)(Function_SET.Set_IOUT / 1000U),
             (unsigned long)(Function_SET.Set_IOUT % 1000U));
    x_base = 4U + (5U * 6U); // "ISET:" 后
    y_base = UI_Y_ROW2_TEXT;
    if(Function_SET.SetStepState == SET_State_First)
    {
      idx = 4U;
    }
    else if(Function_SET.SetStepState == SET_State_Second)
    {
      idx = 3U;
    }
    else
    {
      idx = 2U;
    }
  }

  x_sel = (uint16_t)(x_base + idx * 6U);

  // 按位反显：黑底白字 + 白色细边框
  LCD.ShowChar(x_sel, y_base,
               set_buf[idx], Color_WHITE, Color_BLACK,
               ASCII_font_12, font_overlay_OFF);
  LCD.DrawRectangle(x_sel, y_base, (uint16_t)(x_sel + 5U), (uint16_t)(y_base + 11U), Color_WHITE);

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
  char buf[48];
  uint16_t color_vset = Color_BLACK;
  uint16_t color_iset = Color_BLACK;

  Display_FormatFixed(buf, sizeof(buf), "VSET", (int32_t)Function_SET.Set_VOUT,
                      100, 2, 'V');
  if(strcmp(ui_last_vset, buf) != 0)
  {
    LCD.ShowString(4, UI_Y_ROW1_TEXT, buf, color_vset, Color_WHITE, ASCII_font_12, font_overlay_OFF);
    strncpy(ui_last_vset, buf, sizeof(ui_last_vset) - 1U);
    ui_last_vset[sizeof(ui_last_vset) - 1U] = '\0';
  }
    else if((ui_last_setvi_state != Function_SET.SetVIState) ||
      (ui_last_theme_mode_state != Function_SET.OutPutState) ||
      (ui_last_cursor_menu_state != Function_SET.SetMenuState) ||
      (ui_last_cursor_vi_state != Function_SET.SetVIState) ||
      (ui_last_cursor_step_state != Function_SET.SetStepState))
  {
    LCD.ShowString(4, UI_Y_ROW1_TEXT, buf, color_vset, Color_WHITE, ASCII_font_12, font_overlay_OFF);
  }

  Display_FormatFixed(buf, sizeof(buf), "ISET", (int32_t)Function_SET.Set_IOUT,
                      1000, 3, 'A');
  if(strcmp(ui_last_iset, buf) != 0)
  {
    LCD.ShowString(4, UI_Y_ROW2_TEXT, buf, color_iset, Color_WHITE, ASCII_font_12, font_overlay_OFF);
    strncpy(ui_last_iset, buf, sizeof(ui_last_iset) - 1U);
    ui_last_iset[sizeof(ui_last_iset) - 1U] = '\0';
  }
  else if((ui_last_setvi_state != Function_SET.SetVIState) ||
          (ui_last_cursor_menu_state != Function_SET.SetMenuState) ||
          (ui_last_cursor_vi_state != Function_SET.SetVIState) ||
          (ui_last_cursor_step_state != Function_SET.SetStepState))
  {
    LCD.ShowString(4, UI_Y_ROW2_TEXT, buf, color_iset, Color_WHITE, ASCII_font_12, font_overlay_OFF);
  }

  DisplayShow_Cursor();

  ui_last_setvi_state = Function_SET.SetVIState;
  ui_last_theme_mode_state = Function_SET.OutPutState;
  ui_last_cursor_menu_state = Function_SET.SetMenuState;
  ui_last_cursor_vi_state = Function_SET.SetVIState;
  ui_last_cursor_step_state = Function_SET.SetStepState;
}

static void DisplayShow_Outval(void)
{
  char buf[64];
  float p_out;                    //输出功率
  float temperature;
  uint8_t percent;
  uint16_t row_color;
  uint16_t io_color;
  uint16_t temp_color;
  uint16_t vo_color;
  uint16_t vout_bg;
  uint16_t iout_bg;
  uint16_t temp_bg;
  uint8_t alarm_oc;
  uint8_t alarm_ot;
  uint8_t alarm_ov;
  int32_t vo_100;
  int32_t io_1000;
  int32_t vin_100;
  int32_t p_100;
  int32_t t_10;
  int32_t p_int;
  int32_t p_frac;
  int32_t t_abs;
  const char *t_sign;
  int32_t t_int;
  int32_t t_dec;

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
  alarm_ov = (MyADC.Vo > DISP_WARN_VOLT) ? 1U : 0U;
  vo_color = alarm_ov ? Color_WHITE : Color_BLACK;
  io_color = alarm_oc ? Color_WHITE : Color_BLACK;
  temp_color = alarm_ot ? Color_WHITE : Color_BLACK;
  vout_bg = alarm_ov ? Color_RED : Color_WHITE;
  iout_bg = alarm_oc ? Color_RED : Color_WHITE;
  temp_bg = alarm_ot ? Color_RED : Color_WHITE;

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
  if((ui_last_alarm_ov != alarm_ov) || (ui_last_alarm_oc != alarm_oc) || (ui_last_alarm_ot != alarm_ot))
  {
    LCD.FillColor(UI_X_SPLIT + 1U, UI_Y_HEADER_END + 1U, LCD_W, UI_Y_ROW1_END, (LCD_Color_t)vout_bg);
    LCD.FillColor(UI_X_SPLIT + 1U, UI_Y_ROW1_END + 1U, LCD_W, UI_Y_ROW2_END, (LCD_Color_t)iout_bg);
    LCD.FillColor(UI_COL_RIGHT_X1, UI_Y_ROW2_END + 1U, LCD_W, UI_Y_INFO_END, (LCD_Color_t)temp_bg);
  }

  Display_FormatFixed(buf, sizeof(buf), "VOUT", vo_100, 100, 2, 'V');
  if((strcmp(ui_last_vout, buf) != 0) || (ui_last_alarm_ov != alarm_ov))
  {
    LCD.ShowString(84, UI_Y_ROW1_TEXT, buf, vo_color, vout_bg, ASCII_font_12, font_overlay_OFF);
    strncpy(ui_last_vout, buf, sizeof(ui_last_vout) - 1U);
    ui_last_vout[sizeof(ui_last_vout) - 1U] = '\0';
  }

  Display_FormatFixed(buf, sizeof(buf), "IOUT", io_1000, 1000, 3, 'A');
  if((strcmp(ui_last_iout, buf) != 0) || (ui_last_alarm_oc != alarm_oc))
  {
    LCD.ShowString(84, UI_Y_ROW2_TEXT, buf, io_color, iout_bg, ASCII_font_12, font_overlay_OFF);
    strncpy(ui_last_iout, buf, sizeof(ui_last_iout) - 1U);
    ui_last_iout[sizeof(ui_last_iout) - 1U] = '\0';
  }

  // 信息区：VIN/P/T 三列显示。
  p_int = p_100 / 100;
  p_frac = p_100 % 100;
  if(p_frac < 0)
  {
    p_frac = -p_frac;
  }
  t_sign = (t_10 < 0) ? "-" : "";
  t_abs = (t_10 < 0) ? -t_10 : t_10;
  t_int = t_abs / 10;
  t_dec = t_abs % 10;

  snprintf(buf, sizeof(buf), "VIN:%ldV",
           (long)(vin_100 / 100));
  if(strcmp(ui_last_vin, buf) != 0)
  {
    LCD.ShowString(2, UI_Y_INFO1_TEXT, buf, Color_BLACK, Color_WHITE, ASCII_font_12, font_overlay_OFF);
    strncpy(ui_last_vin, buf, sizeof(ui_last_vin) - 1U);
    ui_last_vin[sizeof(ui_last_vin) - 1U] = '\0';
  }

  snprintf(buf, sizeof(buf), "P:%ld.%01ldW", (long)p_int, (long)(p_frac / 10));
  if(strcmp(ui_last_pwr, buf) != 0)
  {
    LCD.ShowString((uint16_t)(UI_COL_MID_X1 + 2U), UI_Y_INFO1_TEXT, buf,
                   Color_BLACK, Color_WHITE, ASCII_font_12, font_overlay_OFF);
    strncpy(ui_last_pwr, buf, sizeof(ui_last_pwr) - 1U);
    ui_last_pwr[sizeof(ui_last_pwr) - 1U] = '\0';
  }

  snprintf(buf, sizeof(buf), "T:%s%ld.%01ldC", t_sign, (long)t_int, (long)t_dec);
  if((strcmp(ui_last_tmp, buf) != 0) || (ui_last_alarm_ot != alarm_ot))
  {
    LCD.ShowString((uint16_t)(UI_COL_RIGHT_X1 + 2U), UI_Y_INFO1_TEXT, buf,
                   temp_color, temp_bg, ASCII_font_12, font_overlay_OFF);
    strncpy(ui_last_tmp, buf, sizeof(ui_last_tmp) - 1U);
    ui_last_tmp[sizeof(ui_last_tmp) - 1U] = '\0';
  }

  // 底部进度条颜色：CV棕色，CC蓝色（白底下对比度更高）。
  row_color = Display_ModeThemeColor();

  if((ui_last_percent != percent) || (ui_last_bar_mode_state != Function_SET.OutPutState))
  {
    LCD.FillColor(UI_BAR_X1, UI_BAR_Y1, UI_BAR_X2, UI_BAR_Y2, Color_GRAY);
    LCD.FillColor(UI_BAR_X2, UI_BAR_Y1, LCD_W, UI_BAR_Y2, Color_WHITE);
    if(percent > 0)
    {
      LCD.FillColor(UI_BAR_X1, UI_BAR_Y1,
                    (uint16_t)(UI_BAR_X1 + ((UI_BAR_X2 - UI_BAR_X1) * percent / 100U)),
                    UI_BAR_Y2, row_color);
    }

    snprintf(buf, sizeof(buf), "%3d%%", percent);
    LCD.ShowString(UI_BAR_TEXT_X, UI_BAR_Y1 + 2U, buf, Color_BLACK, Color_WHITE, ASCII_font_16, font_overlay_OFF);
    ui_last_percent = percent;
    ui_last_bar_mode_state = Function_SET.OutPutState;
  }

  ui_last_alarm_ov = alarm_ov;
  ui_last_alarm_oc = alarm_oc;
  ui_last_alarm_ot = alarm_ot;
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
