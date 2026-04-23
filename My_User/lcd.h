/*=============================================================================
 * 文件: lcd.h
 * 说明: ST7735S TFT-LCD（1.8寸 128×160）HAL库驱动头文件
 * 芯片: STM32F103C8T6
 * 接口: 硬件SPI2 + GPIO控制线（RES/DC/CS/BLK，引脚名由CubeMX生成）
 * 注意: 本文件替代原 TFT_240.h，对外暴露 LCD_t 结构体接口（全局实例 LCD），
 *       Display.c 等上层代码无需修改。
 *=============================================================================*/
#ifndef __LCD_HAL_H__
#define __LCD_HAL_H__

#include "MyApplication.h"

/*---------------------------------------------------------------------------
 * 显示方向配置
 *   0: 竖屏（W=128, H=160），正向扫描
 *   1: 竖屏（W=128, H=160），翻转扫描
 *   2: 横屏（W=160, H=128），逆时针90°  ← 当前使用
 *   3: 横屏（W=160, H=128），顺时针90°
 *---------------------------------------------------------------------------*/
#define USE_HORIZONTAL  2

#if USE_HORIZONTAL == 0 || USE_HORIZONTAL == 1
#define LCD_W  128   /* 屏幕像素宽度 */
#define LCD_H  160   /* 屏幕像素高度 */
#else
#define LCD_W  160
#define LCD_H  128
#endif

/*---------------------------------------------------------------------------
 * HAL GPIO 控制宏
 * 引脚标号由 STM32CubeMX 自动生成并导出至 gpio.h。
 * 若引脚名称发生变更，只需在 CubeMX 中重新配置，无需修改此处。
 *---------------------------------------------------------------------------*/
#define LCD_RES_Clr  HAL_GPIO_WritePin(TFT_RES_GPIO_Port, TFT_RES_Pin, GPIO_PIN_RESET)  /* 复位拉低 */
#define LCD_RES_Set  HAL_GPIO_WritePin(TFT_RES_GPIO_Port, TFT_RES_Pin, GPIO_PIN_SET)    /* 复位释放 */

#define LCD_DC_Clr   HAL_GPIO_WritePin(TFT_DC_GPIO_Port,  TFT_DC_Pin,  GPIO_PIN_RESET)  /* DC=0：命令 */
#define LCD_DC_Set   HAL_GPIO_WritePin(TFT_DC_GPIO_Port,  TFT_DC_Pin,  GPIO_PIN_SET)    /* DC=1：数据 */

#define LCD_CS_Clr   HAL_GPIO_WritePin(TFT_CS_GPIO_Port,  TFT_CS_Pin,  GPIO_PIN_RESET)  /* 片选使能 */
#define LCD_CS_Set   HAL_GPIO_WritePin(TFT_CS_GPIO_Port,  TFT_CS_Pin,  GPIO_PIN_SET)    /* 片选释放 */

/* 背光极性：高=开，低=关（与原标准库 LCD_BLK_Set 打开背光保持一致） */
#define LCD_BLK_OFF  HAL_GPIO_WritePin(TFT_BLK_GPIO_Port, TFT_BLK_Pin, GPIO_PIN_RESET)  /* 关闭背光 */
#define LCD_BLK_ON   HAL_GPIO_WritePin(TFT_BLK_GPIO_Port, TFT_BLK_Pin, GPIO_PIN_SET)    /* 开启背光 */

/*---------------------------------------------------------------------------
 * 颜色定义（RGB565 格式，16位）
 *---------------------------------------------------------------------------*/
typedef enum
{
    Color_WHITE      = 0xFFFF,   /* 白色   */
    Color_BLACK      = 0x0000,   /* 黑色   */
    Color_BLUE       = 0x001F,   /* 蓝色   */
    Color_BRED       = 0xF81F,   /* 紫色   */
    Color_GBLUE      = 0x07FF,   /* 青色   */
    Color_RED        = 0xF800,   /* 红色   */
    Color_MAGENTA    = 0xF81F,   /* 品红   */
    Color_GREEN      = 0x07E0,   /* 绿色   */
    Color_CYAN       = 0x7FFF,   /* 青蓝色 */
    Color_YELLOW     = 0xFFE0,   /* 黄色   */
    Color_BROWN      = 0xBC40,   /* 棕色   */
    Color_BRRED      = 0xFC07,   /* 棕红色 */
    Color_GRAY       = 0x8430,   /* 灰色   */
    Color_DARKBLUE   = 0x01CF,   /* 深蓝色 */
    Color_LIGHTBLUE  = 0x7D7C,   /* 浅蓝色 */
    Color_GRAYBLUE   = 0x5458,   /* 灰蓝色 */
    Color_LIGHTGREEN = 0x841F,   /* 浅绿色 */
    Color_LGRAY      = 0xC618,   /* 浅灰色 */
    Color_LGRAYBLUE  = 0xA651,   /* 浅灰蓝 */
    Color_LBBLUE     = 0x2B12,   /* 浅棕蓝 */
} LCD_Color_t;

/*---------------------------------------------------------------------------
 * ASCII 字体大小枚举（单位：像素高度，宽度 = 高度/2）
 *---------------------------------------------------------------------------*/
typedef enum
{
    ASCII_font_12 = 12,   /* 6×12  字体 */
    ASCII_font_16 = 16,   /* 8×16  字体 */
    ASCII_font_24 = 24,   /* 12×24 字体 */
    ASCII_font_32 = 32,   /* 16×32 字体 */
} ASCII_font_t;

/*---------------------------------------------------------------------------
 * 字体叠加模式
 *---------------------------------------------------------------------------*/
typedef enum
{
    font_overlay_OFF = 0,   /* 非叠加：字符区域完全覆盖（含背景色） */
    font_overlay_ON  = 1,   /* 叠加  ：仅绘制前景像素，背景保持原样 */
} font_overlay_t;

/*---------------------------------------------------------------------------
 * 中文字体大小枚举（正方形字模，单位：像素）
 *---------------------------------------------------------------------------*/
typedef enum
{
    CHN_font_12 = 12,
    CHN_font_16 = 16,
    CHN_font_24 = 24,
    CHN_font_32 = 32,
} CHN_font_t;

/*---------------------------------------------------------------------------
 * LCD 功能结构体（函数指针集合）
 *
 * 参数语义说明：
 *   FillColor(x1, y1, x2, y2, color)
 *       — 填充矩形，x2/y2 为不含边界（半开区间 [x1,x2) × [y1,y2)）
 *   DrawRectangle(x1, y1, x2, y2, color)
 *       — 画矩形边框，x2/y2 为包含边界（闭区间 [x1,x2]×[y1,y2]）
 *   DrawLine(x1, y1, x2, y2, color)
 *       — 画直线，起点/终点均为包含坐标
 *   ShowChar(x, y, ch, fc, bc, font, mode)
 *   ShowString(x, y, str, fc, bc, font, mode)
 *   ShowChinese(x, y, str, fc, bc, font, mode)        — 单个汉字（2字节GBK）
 *   ShowChineseString(x, y, str, fc, bc, font, mode)  — 汉字串
 *   ShowCHNandENGString(x, y, str, fc, bc, chn_font, asc_font, mode)
 *---------------------------------------------------------------------------*/
typedef struct
{
    void (*Init)(void);
    void (*FillColor)(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, LCD_Color_t color);
    void (*DrawPoint)(uint16_t x, uint16_t y, LCD_Color_t color);
    void (*DrawLine)(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
    void (*DrawRectangle)(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
    void (*ShowChar)(uint16_t x, uint16_t y, const char ch,
                     uint16_t fc, uint16_t bc, ASCII_font_t font, font_overlay_t mode);
    void (*ShowString)(uint16_t x, uint16_t y, const char *str,
                       uint16_t fc, uint16_t bc, ASCII_font_t font, font_overlay_t mode);
    void (*ShowChinese)(uint16_t x, uint16_t y, const char *str,
                        uint16_t fc, uint16_t bc, CHN_font_t font, font_overlay_t mode);
    void (*ShowChineseString)(uint16_t x, uint16_t y, const char *str,
                              uint16_t fc, uint16_t bc, CHN_font_t font, font_overlay_t mode);
    void (*ShowCHNandENGString)(uint16_t x, uint16_t y, const char *str,
                                uint16_t fc, uint16_t bc,
                                CHN_font_t chn_font, ASCII_font_t asc_font,
                                font_overlay_t mode);
} LCD_t;

extern LCD_t LCD;   /* 全局 LCD 操作对象，在 lcd.c 中定义 */

#endif /* __LCD_HAL_H__ */

