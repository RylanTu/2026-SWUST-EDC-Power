#include "Display.h"
#include <string.h>

/*
 * 告警参数集中配置（便于后续统一调参）
 * 说明：
 * 1) VOUT/IOUT 红色由保护状态驱动，不直接使用显示层阈值；
 * 2) 温度红色既可由温度红阈值触发，也可在过温保护后保持一段时间。
 */
#define DISP_WARN_VOUT_ORANGE_V   13.0f   //VOUT橙色阈值(V)
#define DISP_WARN_IOUT_ORANGE_A    1.3f   //IOUT橙色阈值(A)
#define DISP_WARN_TEMP_ORANGE_C   45.0f   //温度橙色阈值(℃)
#define DISP_WARN_TEMP_RED_C      70.0f   //温度红色阈值(℃)

#define DISP_WARN_ORANGE_HOLD_CYCLES  50U  //橙色保持时长：1s（50×20ms）
#define DISP_PROTECT_RED_HOLD_CYCLES 150U  //保护红色保持时长：3s（150×20ms）
#define DISP_UI_ORANGE_COLOR      ((LCD_Color_t)0xFD20U) // RGB565 橙色，用于预警和模式不一致提示

#define DISP_OUTVAL_AVG_SAMPLES 50U //输出值累计50次（20ms/次）后更新=1Hz显示且为1s平均

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

/* 数值区局部刷新窗口（按字符宽度裁剪，避免整行清屏） */
#define UI_ASCII_W             6U
#define UI_VSET_VAL_X         34U
#define UI_VSET_VAL_X2        76U
#define UI_ISET_VAL_X         34U
#define UI_ISET_VAL_X2        76U
#define UI_VOUT_VAL_X        114U
#define UI_VOUT_VAL_X2       156U
#define UI_IOUT_VAL_X        114U
#define UI_IOUT_VAL_X2       158U
#define UI_VOUT_BOX_X1        (UI_X_SPLIT + 1U)
#define UI_VOUT_BOX_Y1        (UI_Y_HEADER_END + 1U)
#define UI_VOUT_BOX_X2        (LCD_W - 1U)
#define UI_VOUT_BOX_Y2        (UI_Y_ROW1_END - 1U)
#define UI_IOUT_BOX_X1        (UI_X_SPLIT + 1U)
#define UI_IOUT_BOX_Y1        (UI_Y_ROW1_END + 1U)
#define UI_IOUT_BOX_X2        (LCD_W - 1U)
#define UI_IOUT_BOX_Y2        (UI_Y_ROW2_END - 1U)
#define UI_TMP_BOX_X1         (UI_COL_RIGHT_X1 + 1U)
#define UI_TMP_BOX_Y1         (UI_Y_ROW2_END + 1U)
#define UI_TMP_BOX_X2         (LCD_W - 1U)
#define UI_TMP_BOX_Y2         (UI_Y_INFO_END - 1U)
#define UI_VOUT_LABEL_X       84U
#define UI_IOUT_LABEL_X       84U
#define UI_T_LABEL_X          (UI_COL_RIGHT_X1 + 2U)
#define UI_VIN_VAL_X          26U
#define UI_VIN_VAL_X2         44U
#define UI_PWR_VAL_X          (UI_COL_MID_X1 + 14U)
#define UI_PWR_VAL_X2         98U
#define UI_TMP_VAL_X          (UI_COL_RIGHT_X1 + 14U)
#define UI_TMP_VAL_X2         158U

//static void Relay_State(void);


static void DisplayShow_Once  (void);       //只显示一次
static void DisplayShow_Device(void);       //设备运行状态


static void DisplayShow_Setval(void);       //显示设定值
static void DisplayShow_Outval(void);       //显示输出值
static void DisplayShow_Cursor(void);       //显示光标
static uint16_t Display_ModeThemeColor(void);
static uint16_t Display_TextColorForBg(uint16_t bg_color);
static uint8_t Display_ModeTextState(void);
static uint8_t Display_ModeMismatch(void);
static int32_t Display_RoundToScale(float value, int32_t scale);

static uint8_t ui_blink_phase = 0;
static uint8_t ui_blink_div = 0;
static uint8_t ui_last_pwr_state = 0xFFU;
static uint8_t ui_last_mode_state = 0xFFU;
static uint8_t ui_last_selected_mode_state = 0xFFU;
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
static uint8_t ui_force_bar_refresh = 0U;

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

  // 静态标签仅绘制一次，后续仅更新数值区域。
  LCD.ShowString(4U, UI_Y_ROW1_TEXT, "VSET:", Color_BLACK, Color_WHITE, ASCII_font_12, font_overlay_OFF);
  LCD.ShowString(4U, UI_Y_ROW2_TEXT, "ISET:", Color_BLACK, Color_WHITE, ASCII_font_12, font_overlay_OFF);
  LCD.ShowString(84U, UI_Y_ROW1_TEXT, "VOUT:", Color_BLACK, Color_WHITE, ASCII_font_12, font_overlay_OFF);
  LCD.ShowString(84U, UI_Y_ROW2_TEXT, "IOUT:", Color_BLACK, Color_WHITE, ASCII_font_12, font_overlay_OFF);
  LCD.ShowString(2U, UI_Y_INFO1_TEXT, "VIN:", Color_BLACK, Color_WHITE, ASCII_font_12, font_overlay_OFF);
  LCD.ShowString((uint16_t)(UI_COL_MID_X1 + 2U), UI_Y_INFO1_TEXT, "P:",
                 Color_BLACK, Color_WHITE, ASCII_font_12, font_overlay_OFF);
  LCD.ShowString((uint16_t)(UI_COL_RIGHT_X1 + 2U), UI_Y_INFO1_TEXT, "T:",
                 Color_BLACK, Color_WHITE, ASCII_font_12, font_overlay_OFF);

  // 首帧静态界面完成后清空缓存，确保动态数值下一周期必定重绘。
  ui_last_pwr_state = 0xFFU;
  ui_last_mode_state = 0xFFU;
  ui_last_selected_mode_state = 0xFFU;
  ui_last_percent = 0xFFU;
  ui_last_setvi_state = 0xFFU;
  ui_last_theme_mode_state = 0xFFU;
  ui_last_bar_mode_state = 0xFFU;
  ui_last_cursor_menu_state = 0xFFU;
  ui_last_cursor_vi_state = 0xFFU;
  ui_last_cursor_step_state = 0xFFU;
  ui_last_alarm_ov = 0xFFU;
  ui_last_alarm_oc = 0xFFU;
  ui_last_alarm_ot = 0xFFU;
  ui_force_bar_refresh = 1U;
  ui_last_vset[0] = '\0';
  ui_last_iset[0] = '\0';
  ui_last_vout[0] = '\0';
  ui_last_iout[0] = '\0';
  ui_last_vin[0] = '\0';
  ui_last_pwr[0] = '\0';
  ui_last_tmp[0] = '\0';

  Display.Show_Once_Flag = FALSE;
}

static uint16_t Display_ModeThemeColor(void)
{
  if(Display_ModeMismatch() != 0U)
  {
    return DISP_UI_ORANGE_COLOR;
  }

  return (Display_ModeTextState() == CV_State) ? Color_YELLOW : Color_BLUE;
}

static uint16_t Display_TextColorForBg(uint16_t bg_color)
{
  if((bg_color == Color_RED) || (bg_color == Color_BLUE))
  {
    return Color_WHITE;
  }

  return Color_BLACK;
}

static uint8_t Display_ModeTextState(void)
{
  return Function_SET.SelectOutPutState;
}

static uint8_t Display_ModeMismatch(void)
{
  if(Function_SET.PowrputState != ON_State)
  {
    return 0U;
  }

  if(Function_SET.SetMenuState != Menu_OUT_State)
  {
    return 0U;
  }

  return (Function_SET.SelectOutPutState != Function_SET.OutPutState) ? 1U : 0U;
}

static void DisplayShow_Device(void)
{
  uint16_t mode_txt_color;

  if((ui_last_pwr_state == Function_SET.PowrputState) &&
	 (ui_last_mode_state == Function_SET.OutPutState) &&
     (ui_last_selected_mode_state == Function_SET.SelectOutPutState))
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

  mode_txt_color = Display_TextColorForBg(Display_ModeThemeColor());
  LCD.FillColor(UI_X_SPLIT, UI_MODE_BADGE_Y1, LCD_W, UI_MODE_BADGE_Y2, Display_ModeThemeColor());
  LCD.ShowString(84, UI_Y_HEADER_TEXT, "MODE:", mode_txt_color, Display_ModeThemeColor(), ASCII_font_16, font_overlay_OFF);

  if(Display_ModeTextState() == CV_State)
  {
    LCD.ShowString(132, UI_Y_HEADER_TEXT, "CV", mode_txt_color, Display_ModeThemeColor(), ASCII_font_16, font_overlay_OFF);
  }
  else
  {
    LCD.ShowString(132, UI_Y_HEADER_TEXT, "CC", mode_txt_color, Display_ModeThemeColor(), ASCII_font_16, font_overlay_OFF);
  }

  // MODE区发生重绘时，强制下一帧同步重绘进度条，避免色块不同步。
  ui_force_bar_refresh = 1U;

  ui_last_pwr_state = Function_SET.PowrputState;
  ui_last_mode_state = Function_SET.OutPutState;
  ui_last_selected_mode_state = Function_SET.SelectOutPutState;
}

static void DisplayShow_Cursor(void)
{
  char set_buf[16];
  uint16_t x_base;
  uint16_t y_base;
  uint16_t x_sel;
  uint16_t char_span;
  uint16_t x_box1;
  uint16_t y_box1;
  uint16_t x_box2;
  uint16_t y_box2;
  uint16_t i;
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
    char_span = 1U;
    if(Function_SET.SetStepState == SET_State_First)
    {
      idx = 0U;
      if((Function_SET.Set_VOUT / 100U) >= 10U)
      {
        char_span = 2U;
      }
    }
    else if(Function_SET.SetStepState == SET_State_Second)
    {
      idx = ((Function_SET.Set_VOUT / 100U) >= 10U) ? 3U : 2U;
    }
    else
    {
      idx = ((Function_SET.Set_VOUT / 100U) >= 10U) ? 4U : 3U;
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
    char_span = 1U;
    if(Function_SET.SetStepState == SET_State_First)
    {
      idx = 2U;
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
  x_box1 = (x_sel > 0U) ? (uint16_t)(x_sel - 1U) : x_sel;
  y_box1 = (y_base > 0U) ? (uint16_t)(y_base - 1U) : y_base;
  x_box2 = (uint16_t)(x_sel + 6U * char_span);
  y_box2 = (uint16_t)(y_base + 12U);

  // 按位反显：放大一圈黑底高亮，提升位选可读性。
  LCD.FillColor(x_box1, y_box1, x_box2, y_box2, Color_BLACK);
  for(i = 0U; i < char_span; i++)
  {
    LCD.ShowChar((uint16_t)(x_sel + i * 6U), y_base,
                 set_buf[idx + i], Color_WHITE, Color_BLACK,
                 ASCII_font_12, font_overlay_OFF);
  }
  LCD.DrawRectangle(x_box1, y_box1, x_box2, y_box2, Color_WHITE);

}

static int32_t Display_RoundToScale(float value, int32_t scale)
{
  if(value >= 0.0f)
  {
    return (int32_t)(value * (float)scale + 0.5f);
  }
  return (int32_t)(value * (float)scale - 0.5f);
}

static void DisplayShow_Setval(void)
{
  char buf[48];
  uint16_t color_vset = Color_BLACK;
  uint16_t color_iset = Color_BLACK;

  snprintf(buf, sizeof(buf), "%lu.%02luV",
           (unsigned long)(Function_SET.Set_VOUT / 100U),
           (unsigned long)(Function_SET.Set_VOUT % 100U));
  if(strcmp(ui_last_vset, buf) != 0)
  {
    LCD.FillColor(UI_VSET_VAL_X, UI_Y_HEADER_END + 1U, UI_VSET_VAL_X2, UI_Y_ROW1_END, Color_WHITE);
    LCD.ShowString(UI_VSET_VAL_X, UI_Y_ROW1_TEXT, buf, color_vset, Color_WHITE, ASCII_font_12, font_overlay_OFF);
    strncpy(ui_last_vset, buf, sizeof(ui_last_vset) - 1U);
    ui_last_vset[sizeof(ui_last_vset) - 1U] = '\0';
  }
    else if((ui_last_setvi_state != Function_SET.SetVIState) ||
      (ui_last_theme_mode_state != Function_SET.SelectOutPutState) ||
      (ui_last_cursor_menu_state != Function_SET.SetMenuState) ||
      (ui_last_cursor_vi_state != Function_SET.SetVIState) ||
      (ui_last_cursor_step_state != Function_SET.SetStepState))
  {
    LCD.FillColor(UI_VSET_VAL_X, UI_Y_HEADER_END + 1U, UI_VSET_VAL_X2, UI_Y_ROW1_END, Color_WHITE);
    LCD.ShowString(UI_VSET_VAL_X, UI_Y_ROW1_TEXT, buf, color_vset, Color_WHITE, ASCII_font_12, font_overlay_OFF);
  }

  snprintf(buf, sizeof(buf), "%lu.%03luA",
           (unsigned long)(Function_SET.Set_IOUT / 1000U),
           (unsigned long)(Function_SET.Set_IOUT % 1000U));
  if(strcmp(ui_last_iset, buf) != 0)
  {
    LCD.FillColor(UI_ISET_VAL_X, UI_Y_ROW1_END + 1U, UI_ISET_VAL_X2, UI_Y_ROW2_END, Color_WHITE);
    LCD.ShowString(UI_ISET_VAL_X, UI_Y_ROW2_TEXT, buf, color_iset, Color_WHITE, ASCII_font_12, font_overlay_OFF);
    strncpy(ui_last_iset, buf, sizeof(ui_last_iset) - 1U);
    ui_last_iset[sizeof(ui_last_iset) - 1U] = '\0';
  }
  else if((ui_last_setvi_state != Function_SET.SetVIState) ||
          (ui_last_cursor_menu_state != Function_SET.SetMenuState) ||
          (ui_last_cursor_vi_state != Function_SET.SetVIState) ||
          (ui_last_cursor_step_state != Function_SET.SetStepState))
  {
    LCD.FillColor(UI_ISET_VAL_X, UI_Y_ROW1_END + 1U, UI_ISET_VAL_X2, UI_Y_ROW2_END, Color_WHITE);
    LCD.ShowString(UI_ISET_VAL_X, UI_Y_ROW2_TEXT, buf, color_iset, Color_WHITE, ASCII_font_12, font_overlay_OFF);
  }

  DisplayShow_Cursor();

  ui_last_setvi_state = Function_SET.SetVIState;
  ui_last_theme_mode_state = Function_SET.SelectOutPutState;
  ui_last_cursor_menu_state = Function_SET.SetMenuState;
  ui_last_cursor_vi_state = Function_SET.SetVIState;
  ui_last_cursor_step_state = Function_SET.SetStepState;
}

static void DisplayShow_Outval(void)
{
  char buf[64];
  float p_out = 0.0f;             //输出功率 初始化为0防止未定义行为
  float temperature = 0.0f;       //温度 初始化为0
  uint8_t percent;
  uint16_t row_color;
  uint16_t io_color;
  uint16_t temp_color;
  uint16_t vo_color;
  uint16_t vout_bg;
  uint16_t iout_bg;
  uint16_t temp_bg;
  uint8_t alarm_oc_level;
  uint8_t temp_alarm_level;
  uint8_t alarm_ov_level;
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

  // 显示值每1秒发布一次：显示内容为该1秒窗口平均值。
  {
    static uint16_t avg_count = 0U;
    static float sum_vo = 0.0f;
    static float sum_io = 0.0f;
    static float sum_vi = 0.0f;
    static float sum_ni = 0.0f;
    static float disp_vo = 0.0f;
    static float disp_io = 0.0f;
    static float disp_vi = 0.0f;
    static float disp_ni = 0.0f;
    static uint8_t avg_ready = 0U;

    sum_vo += MyADC.Vo;
    sum_io += MyADC.Io;
    sum_vi += MyADC.Vi;
    sum_ni += MyADC.Ni;
    avg_count++;

    if(avg_count >= DISP_OUTVAL_AVG_SAMPLES)
    {
      disp_vo = sum_vo / (float)avg_count;
      disp_io = sum_io / (float)avg_count;
      disp_vi = sum_vi / (float)avg_count;
      disp_ni = sum_ni / (float)avg_count;
      sum_vo = 0.0f;
      sum_io = 0.0f;
      sum_vi = 0.0f;
      sum_ni = 0.0f;
      avg_count = 0U;
      avg_ready = 1U;
    }

    if(avg_ready == 0U)
    {
      // 首秒未满前先显示即时值，避免上电后长时间空白。
      disp_vo = MyADC.Vo;
      disp_io = MyADC.Io;
      disp_vi = MyADC.Vi;
      disp_ni = MyADC.Ni;
    }

    p_out = disp_io * disp_vo;
    temperature = disp_ni;
    vo_100  = Display_RoundToScale(disp_vo, 100);
    io_1000 = Display_RoundToScale(disp_io, 1000);
    vin_100 = Display_RoundToScale(disp_vi, 100);
  }

  // UI层闪烁节拍：仅用于告警显示，不影响控制逻辑。
  if(++ui_blink_div >= 25)
  {
    ui_blink_div = 0;
    ui_blink_phase ^= 1U;
  }

  // VOUT/IOUT两级告警：超阈值橙色；触发对应保护后红色（红色保持数秒再恢复）。
  {
    static uint16_t ov_red_hold = 0U;
    static uint16_t oc_red_hold = 0U;
    static uint16_t temp_red_hold = 0U;
    static uint16_t ov_orange_hold = 0U;
    static uint16_t oc_orange_hold = 0U;
    static uint16_t temp_orange_hold = 0U;

    if(Function_SET.ProtectState == 1U)
    {
      ov_red_hold = DISP_PROTECT_RED_HOLD_CYCLES;
    }
    else if(ov_red_hold > 0U)
    {
      ov_red_hold--;
    }

    if(Function_SET.ProtectState == 2U)
    {
      oc_red_hold = DISP_PROTECT_RED_HOLD_CYCLES;
    }
    else if(oc_red_hold > 0U)
    {
      oc_red_hold--;
    }

    if(Function_SET.ProtectState == 3U)
    {
      temp_red_hold = DISP_PROTECT_RED_HOLD_CYCLES;
    }
    else if(temp_red_hold > 0U)
    {
      temp_red_hold--;
    }

    if(ov_red_hold > 0U)
    {
      alarm_ov_level = 2U;
    }
    else if(MyADC.Vo > DISP_WARN_VOUT_ORANGE_V)
    {
      ov_orange_hold = DISP_WARN_ORANGE_HOLD_CYCLES;
      alarm_ov_level = 1U;
    }
    else if(ov_orange_hold > 0U)
    {
      ov_orange_hold--;
      alarm_ov_level = 1U;
    }
    else
    {
      alarm_ov_level = 0U;
    }

    if(oc_red_hold > 0U)
    {
      alarm_oc_level = 2U;
    }
    else if(MyADC.Io > DISP_WARN_IOUT_ORANGE_A)
    {
      oc_orange_hold = DISP_WARN_ORANGE_HOLD_CYCLES;
      alarm_oc_level = 1U;
    }
    else if(oc_orange_hold > 0U)
    {
      oc_orange_hold--;
      alarm_oc_level = 1U;
    }
    else
    {
      alarm_oc_level = 0U;
    }

    if(temp_red_hold > 0U)
    {
      temp_alarm_level = 2U; // 过温保护触发后红色保持3s
    }
    else if(MyADC.Ni > DISP_WARN_TEMP_RED_C)
    {
      temp_alarm_level = 2U; // 红色告警
    }
    else if(MyADC.Ni >= DISP_WARN_TEMP_ORANGE_C)
    {
      temp_orange_hold = DISP_WARN_ORANGE_HOLD_CYCLES;
      temp_alarm_level = 1U; // 橙色预警
    }
    else if(temp_orange_hold > 0U)
    {
      temp_orange_hold--;
      temp_alarm_level = 1U;
    }
    else
    {
      temp_alarm_level = 0U;
    }
  }
  vout_bg = (alarm_ov_level == 2U) ? Color_RED :
            ((alarm_ov_level == 1U) ? DISP_UI_ORANGE_COLOR : Color_WHITE);
  iout_bg = (alarm_oc_level == 2U) ? Color_RED :
            ((alarm_oc_level == 1U) ? DISP_UI_ORANGE_COLOR : Color_WHITE);
  temp_bg = (temp_alarm_level == 2U) ? Color_RED :
            ((temp_alarm_level == 1U) ? DISP_UI_ORANGE_COLOR : Color_WHITE);
  vo_color = Display_TextColorForBg(vout_bg);
  io_color = Display_TextColorForBg(iout_bg);
  temp_color = Display_TextColorForBg(temp_bg);

  p_100 = Display_RoundToScale(p_out, 100);
  t_10  = Display_RoundToScale(temperature, 10);

  // SET状态下跟随当前编辑项；退出SET后跟随当前模式：CV显示VSET，CC显示ISET。
  if(Function_SET.SetMenuState == Menu_SET_State)
  {
    if(Function_SET.SetVIState == SET_V_State)
    {
      percent = (uint8_t)(Function_SET.Set_VOUT / (float)SET_VOUT_MAX * 100.0f);
    }
    else
    {
      percent = (uint8_t)(Function_SET.Set_IOUT / (float)SET_IOUT_MAX * 100.0f);
    }
  }
  else if(Display_ModeTextState() == CV_State)
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

  // 仅刷新数值窗口，避免覆盖静态标签（VOUT/IOUT/T）。

  snprintf(buf, sizeof(buf), "%ld.%02ldV",
           (long)(vo_100 / 100),
           (long)((vo_100 >= 0) ? (vo_100 % 100) : (-(vo_100 % 100))));
  if((strcmp(ui_last_vout, buf) != 0) || (ui_last_alarm_ov != alarm_ov_level))
  {
    LCD.FillColor(UI_VOUT_BOX_X1, UI_VOUT_BOX_Y1, UI_VOUT_BOX_X2, UI_VOUT_BOX_Y2, (LCD_Color_t)vout_bg);
    LCD.ShowString(UI_VOUT_LABEL_X, UI_Y_ROW1_TEXT, "VOUT:", vo_color, vout_bg, ASCII_font_12, font_overlay_OFF);
    LCD.ShowString(UI_VOUT_VAL_X, UI_Y_ROW1_TEXT, buf, vo_color, vout_bg, ASCII_font_12, font_overlay_OFF);
    strncpy(ui_last_vout, buf, sizeof(ui_last_vout) - 1U);
    ui_last_vout[sizeof(ui_last_vout) - 1U] = '\0';
  }

  snprintf(buf, sizeof(buf), "%ld.%03ldA",
           (long)(io_1000 / 1000),
           (long)((io_1000 >= 0) ? (io_1000 % 1000) : (-(io_1000 % 1000))));
  if((strcmp(ui_last_iout, buf) != 0) || (ui_last_alarm_oc != alarm_oc_level))
  {
    LCD.FillColor(UI_IOUT_BOX_X1, UI_IOUT_BOX_Y1, UI_IOUT_BOX_X2, UI_IOUT_BOX_Y2, (LCD_Color_t)iout_bg);
    LCD.ShowString(UI_IOUT_LABEL_X, UI_Y_ROW2_TEXT, "IOUT:", io_color, iout_bg, ASCII_font_12, font_overlay_OFF);
    LCD.ShowString(UI_IOUT_VAL_X, UI_Y_ROW2_TEXT, buf, io_color, iout_bg, ASCII_font_12, font_overlay_OFF);
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

  snprintf(buf, sizeof(buf), "%ldV",
           (long)(vin_100 / 100));
  if(strcmp(ui_last_vin, buf) != 0)
  {
    LCD.FillColor(UI_VIN_VAL_X, UI_Y_ROW2_END + 1U, UI_VIN_VAL_X2, UI_Y_INFO_END, Color_WHITE);
    LCD.ShowString(UI_VIN_VAL_X, UI_Y_INFO1_TEXT, buf, Color_BLACK, Color_WHITE, ASCII_font_12, font_overlay_OFF);
    strncpy(ui_last_vin, buf, sizeof(ui_last_vin) - 1U);
    ui_last_vin[sizeof(ui_last_vin) - 1U] = '\0';
  }

  snprintf(buf, sizeof(buf), "%ld.%01ldW", (long)p_int, (long)(p_frac / 10));
  if(strcmp(ui_last_pwr, buf) != 0)
  {
    LCD.FillColor(UI_PWR_VAL_X, UI_Y_ROW2_END + 1U, UI_PWR_VAL_X2, UI_Y_INFO_END, Color_WHITE);
    LCD.ShowString(UI_PWR_VAL_X, UI_Y_INFO1_TEXT, buf,
                   Color_BLACK, Color_WHITE, ASCII_font_12, font_overlay_OFF);
    strncpy(ui_last_pwr, buf, sizeof(ui_last_pwr) - 1U);
    ui_last_pwr[sizeof(ui_last_pwr) - 1U] = '\0';
  }

  snprintf(buf, sizeof(buf), "%s%ld.%01ldC", t_sign, (long)t_int, (long)t_dec);
  if((strcmp(ui_last_tmp, buf) != 0) || (ui_last_alarm_ot != temp_alarm_level))
  {
    LCD.FillColor(UI_TMP_BOX_X1, UI_TMP_BOX_Y1, UI_TMP_BOX_X2, UI_TMP_BOX_Y2, (LCD_Color_t)temp_bg);
    LCD.ShowString(UI_T_LABEL_X, UI_Y_INFO1_TEXT, "T:", temp_color, temp_bg, ASCII_font_12, font_overlay_OFF);
    LCD.ShowString(UI_TMP_VAL_X, UI_Y_INFO1_TEXT, buf,
                   temp_color, temp_bg, ASCII_font_12, font_overlay_OFF);
    strncpy(ui_last_tmp, buf, sizeof(ui_last_tmp) - 1U);
    ui_last_tmp[sizeof(ui_last_tmp) - 1U] = '\0';
  }

  // 底部进度条颜色：CV棕色，CC蓝色（白底下对比度更高）。
  row_color = Display_ModeThemeColor();

    if((ui_force_bar_refresh != 0U) ||
      (ui_last_percent != percent) ||
     (ui_last_bar_mode_state != Function_SET.OutPutState) ||
     (ui_last_theme_mode_state != Function_SET.SelectOutPutState))
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
    ui_last_theme_mode_state = Function_SET.SelectOutPutState;
    ui_force_bar_refresh = 0U;
  }

  ui_last_alarm_ov = alarm_ov_level;
  ui_last_alarm_oc = alarm_oc_level;
  ui_last_alarm_ot = temp_alarm_level;
}

void My_DisplayTask(void)
{
  static uint8_t disp_tick = 0;

  if(Display.Show_Once_Flag == TRUE)
  {
    Display.DisplayShow_Once();  //显示一次
  }

  disp_tick++;

  /* 状态、设定值、输出值同频刷新，避免模式色不同步产生闪烁 */
  if((disp_tick % 2U) == 0U)
  {
    Display.DisplayShow_Device();  //显示设备状态
    Display.DisplayShow_Setval();  //显示设定值
    Display.DisplayShow_Outval();  //显示输出值
  }
}
