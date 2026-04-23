/*=============================================================================
 * 文件: lcd.c
 * 说明: ST7735S TFT-LCD（1.8寸 128×160/160×128）HAL库驱动实现
 * 芯片: STM32F103C8T6
 * 接口: 硬件SPI2（CubeMX配置）+ GPIO控制线（RES/DC/CS/BLK）
 * 改动: 原辰哥标准库版本已重写为HAL库版本：
 *       · 模拟SPI → HAL_SPI_Transmit(&hspi2, ...)
 *       · delay_ms() → HAL_Delay()
 *       · 标准库GPIO → HAL_GPIO_WritePin()
 *       · 新增 LCD_t 结构体，对外暴露统一的 LCD 操作接口
 *       · TFT_DrawRectangle 修正为绝对坐标语义（x1,y1,x2,y2）
 *=============================================================================*/
#include "lcd.h"
#include "lcdfont.h"   /* ASCII & 中文点阵字库（ascii_1206/1608/2412/3216，tfont12/16/24/32） */
#include "pic.h"       /* 图片数组（gImage_1 等） */
#include <stdlib.h>
#include <string.h>

/*---------------------------------------------------------------------------
 * 私有函数原型声明
 *---------------------------------------------------------------------------*/
static void LCD_Writ_Bus(uint8_t dat);
static void LCD_WriteBytes(const uint8_t *buf, uint16_t len);
static void LCD_BeginWrite(void);
static void LCD_EndWrite(void);
static void LCD_WR_DATA8(uint8_t dat);
static void LCD_WR_DATA16(uint16_t dat);
static void LCD_WR_REG(uint8_t dat);
static void LCD_Address_Set(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

static void LCD_Init(void);
static void LCD_FillColor(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, LCD_Color_t color);
static void LCD_DrawPoint(uint16_t x, uint16_t y, LCD_Color_t color);
static void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
static void LCD_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
static void LCD_ShowChar(uint16_t x, uint16_t y, const char ch,
                         uint16_t fc, uint16_t bc, ASCII_font_t font, font_overlay_t mode);
static void LCD_ShowString(uint16_t x, uint16_t y, const char *str,
                           uint16_t fc, uint16_t bc, ASCII_font_t font, font_overlay_t mode);
static void LCD_ShowChinese(uint16_t x, uint16_t y, const char *str,
                            uint16_t fc, uint16_t bc, CHN_font_t font, font_overlay_t mode);
static void LCD_ShowChineseString(uint16_t x, uint16_t y, const char *str,
                                  uint16_t fc, uint16_t bc, CHN_font_t font, font_overlay_t mode);
static void LCD_ShowCHNandENGstring(uint16_t x, uint16_t y, const char *str,
                                    uint16_t fc, uint16_t bc,
                                    CHN_font_t chn_font, ASCII_font_t asc_font,
                                    font_overlay_t mode);

/*---------------------------------------------------------------------------
 * 全局 LCD 操作对象（对外暴露，供 Display.c 等上层调用）
 *---------------------------------------------------------------------------*/
LCD_t LCD =
{
    LCD_Init,
    LCD_FillColor,
    LCD_DrawPoint,
    LCD_DrawLine,
    LCD_DrawRectangle,
    LCD_ShowChar,
    LCD_ShowString,
    LCD_ShowChinese,
    LCD_ShowChineseString,
    LCD_ShowCHNandENGstring
};

/*=============================================================================
 * 底层 SPI 通信函数
 *=============================================================================*/

/**
 * @brief  经硬件 SPI2 发送一个字节
 * @param  dat: 待发送字节
 * @note   CS 由本函数控制，确保每次传输均完整包裹在 CS 低电平内。
 */
static void LCD_Writ_Bus(uint8_t dat)
{
    HAL_SPI_Transmit(&hspi2, &dat, 1, 3);         /* 发送1字节，超时3ms */
}

/**
 * @brief  批量发送字节流
 */
static void LCD_WriteBytes(const uint8_t *buf, uint16_t len)
{
    HAL_SPI_Transmit(&hspi2, (uint8_t *)buf, len, 10);
}

/**
 * @brief  开始一次 LCD 写事务（CS 拉低）
 */
static void LCD_BeginWrite(void)
{
    LCD_CS_Clr;
}

/**
 * @brief  结束一次 LCD 写事务（CS 释放）
 */
static void LCD_EndWrite(void)
{
    LCD_CS_Set;
}

/**
 * @brief  向 LCD 写入一字节数据（DC=1）
 * @param  dat: 8位数据
 */
static void LCD_WR_DATA8(uint8_t dat)
{
    LCD_DC_Set;               /* DC=1：数据模式 */
    LCD_BeginWrite();
    LCD_Writ_Bus(dat);
    LCD_EndWrite();
}

/**
 * @brief  向 LCD 写入两字节数据（大端序，高字节先发）
 * @param  dat: 16位数据，典型用途：RGB565 颜色值
 */
static void LCD_WR_DATA16(uint16_t dat)
{
    uint8_t bytes[2];
    bytes[0] = (uint8_t)(dat >> 8);       /* 高字节 */
    bytes[1] = (uint8_t)(dat & 0xFF);     /* 低字节 */
    LCD_DC_Set;
    LCD_BeginWrite();
    LCD_WriteBytes(bytes, 2);
    LCD_EndWrite();
}

/**
 * @brief  向 LCD 发送一条命令字节（DC=0）
 * @param  dat: 命令字节
 */
static void LCD_WR_REG(uint8_t dat)
{
    LCD_DC_Clr;               /* DC=0：命令模式 */
    LCD_BeginWrite();
    LCD_Writ_Bus(dat);
    LCD_EndWrite();
    LCD_DC_Set;               /* 恢复数据模式，为后续数据做准备 */
}

/*=============================================================================
 * 地址窗口设置（CASET + RASET + RAMWR）
 *=============================================================================*/

/**
 * @brief  设置 LCD 显示地址窗口
 * @param  x1,y1: 起始坐标（含）
 * @param  x2,y2: 结束坐标（含）
 * @note   ST7735S 内部 RAM 有固定偏移量，需根据旋转方向进行补偿：
 *         · 竖屏（0/1）：列偏移+2，行偏移+1
 *         · 横屏（2/3）：列偏移+1，行偏移+2
 */
static void LCD_Address_Set(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    if(USE_HORIZONTAL == 0 || USE_HORIZONTAL == 1)
    {
        LCD_WR_REG(0x2A);                   /* CASET：列地址设置 */
        LCD_WR_DATA16(x1 + 2);
        LCD_WR_DATA16(x2 + 2);
        LCD_WR_REG(0x2B);                   /* RASET：行地址设置 */
        LCD_WR_DATA16(y1 + 1);
        LCD_WR_DATA16(y2 + 1);
    }
    else    /* USE_HORIZONTAL == 2 或 3 */
    {
        LCD_WR_REG(0x2A);
        LCD_WR_DATA16(x1 + 1);
        LCD_WR_DATA16(x2 + 1);
        LCD_WR_REG(0x2B);
        LCD_WR_DATA16(y1 + 2);
        LCD_WR_DATA16(y2 + 2);
    }
    LCD_WR_REG(0x2C);                       /* RAMWR：开始写入显存 */
}

/*=============================================================================
 * LCD 初始化（ST7735S 初始化序列）
 *=============================================================================*/

/**
 * @brief  LCD 完整初始化
 * @note   按照 ST7735S 数据手册配置帧率、电源、Gamma、颜色格式及扫描方向。
 *         初始化完成后清屏为白色并开启背光。
 *         GPIO 由 CubeMX 生成的 MX_GPIO_Init() 负责，本函数无需重复初始化。
 */
static void LCD_Init(void)
{
    LCD_BLK_OFF;        /* 初始化期间关闭背光，避免花屏 */

    /* 与原标准库一致：先拉到空闲态，避免上电后误命令 */
    LCD_CS_Set;
    LCD_DC_Set;
    LCD_RES_Set;
    HAL_Delay(10);

    /* --- 硬件复位 --- */
    LCD_RES_Clr;
    HAL_Delay(100);
    LCD_RES_Set;
    HAL_Delay(100);

    /* --- 退出睡眠模式 --- */
    LCD_WR_REG(0x11);   /* Sleep Out */
    HAL_Delay(120);     /* ST7735S 规范要求：Sleep Out 后至少等待 120ms */

    /* --- 帧率设置（正常/空闲/局部模式均配置相同帧率） --- */
    LCD_WR_REG(0xB1);
    LCD_WR_DATA8(0x05); LCD_WR_DATA8(0x3C); LCD_WR_DATA8(0x3C);
    LCD_WR_REG(0xB2);
    LCD_WR_DATA8(0x05); LCD_WR_DATA8(0x3C); LCD_WR_DATA8(0x3C);
    LCD_WR_REG(0xB3);
    LCD_WR_DATA8(0x05); LCD_WR_DATA8(0x3C); LCD_WR_DATA8(0x3C);
    LCD_WR_DATA8(0x05); LCD_WR_DATA8(0x3C); LCD_WR_DATA8(0x3C);

    /* --- 点反转（Dot Inversion） --- */
    LCD_WR_REG(0xB4);
    LCD_WR_DATA8(0x03);

    /* --- 电源控制序列 --- */
    LCD_WR_REG(0xC0);
    LCD_WR_DATA8(0x28); LCD_WR_DATA8(0x08); LCD_WR_DATA8(0x04);
    LCD_WR_REG(0xC1);
    LCD_WR_DATA8(0xC0);
    LCD_WR_REG(0xC2);
    LCD_WR_DATA8(0x0D); LCD_WR_DATA8(0x00);
    LCD_WR_REG(0xC3);
    LCD_WR_DATA8(0x8D); LCD_WR_DATA8(0x2A);
    LCD_WR_REG(0xC4);
    LCD_WR_DATA8(0x8D); LCD_WR_DATA8(0xEE);

    /* --- VCOM 电压配置 --- */
    LCD_WR_REG(0xC5);
    LCD_WR_DATA8(0x1A);

    /* --- 扫描方向（MADCTL），根据 USE_HORIZONTAL 配置 --- */
    LCD_WR_REG(0x36);
    if     (USE_HORIZONTAL == 0) LCD_WR_DATA8(0x00);
    else if(USE_HORIZONTAL == 1) LCD_WR_DATA8(0xC0);
    else if(USE_HORIZONTAL == 2) LCD_WR_DATA8(0x70);
    else                         LCD_WR_DATA8(0xA0);

    /* --- 正极 Gamma 曲线（影响颜色/对比度） --- */
    LCD_WR_REG(0xE0);
    LCD_WR_DATA8(0x04); LCD_WR_DATA8(0x22); LCD_WR_DATA8(0x07);
    LCD_WR_DATA8(0x0A); LCD_WR_DATA8(0x2E); LCD_WR_DATA8(0x30);
    LCD_WR_DATA8(0x25); LCD_WR_DATA8(0x2A); LCD_WR_DATA8(0x28);
    LCD_WR_DATA8(0x26); LCD_WR_DATA8(0x2E); LCD_WR_DATA8(0x3A);
    LCD_WR_DATA8(0x00); LCD_WR_DATA8(0x01); LCD_WR_DATA8(0x03);
    LCD_WR_DATA8(0x13);

    /* --- 负极 Gamma 曲线 --- */
    LCD_WR_REG(0xE1);
    LCD_WR_DATA8(0x04); LCD_WR_DATA8(0x16); LCD_WR_DATA8(0x06);
    LCD_WR_DATA8(0x0D); LCD_WR_DATA8(0x2D); LCD_WR_DATA8(0x26);
    LCD_WR_DATA8(0x23); LCD_WR_DATA8(0x27); LCD_WR_DATA8(0x27);
    LCD_WR_DATA8(0x25); LCD_WR_DATA8(0x2D); LCD_WR_DATA8(0x3B);
    LCD_WR_DATA8(0x00); LCD_WR_DATA8(0x01); LCD_WR_DATA8(0x04);
    LCD_WR_DATA8(0x13);

    /* --- 像素格式：16位 RGB565 --- */
    LCD_WR_REG(0x3A);
    LCD_WR_DATA8(0x05);

    /* --- 打开显示 --- */
    LCD_WR_REG(0x29);

    /* --- 清屏为白色，开启背光 --- */
    LCD_FillColor(0, 0, LCD_W, LCD_H, Color_WHITE);
    printf("ST7735S LCD Init OK!\r\n");
    LCD_BLK_ON;
}


/*=============================================================================
 * 基本绘图函数
 *=============================================================================*/

/**
 * @brief  在指定矩形区域内填充单色
 * @param  x1,y1: 起始坐标（含）
 * @param  x2,y2: 结束坐标（不含），即填充半开区间 [x1,x2) × [y1,y2)
 * @param  color: 填充颜色（LCD_Color_t 枚举，自动转 uint16_t）
 */
static void LCD_FillColor(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, LCD_Color_t color)
{
    uint16_t i;
    uint16_t width;
    uint16_t height;
    uint16_t px;
    uint8_t color_pair[2];
    uint8_t line_buf[320];

    /* 边界保护：坐标裁剪 + 空区间直接返回 */
    if(x1 >= LCD_W || y1 >= LCD_H)
    {
        return;
    }
    if(x2 > LCD_W)
    {
        x2 = LCD_W;
    }
    if(y2 > LCD_H)
    {
        y2 = LCD_H;
    }
    if(x2 <= x1 || y2 <= y1)
    {
        return;
    }

    width = (uint16_t)(x2 - x1);
    height = (uint16_t)(y2 - y1);
    color_pair[0] = (uint8_t)((uint16_t)color >> 8);
    color_pair[1] = (uint8_t)((uint16_t)color & 0xFF);

    /* 预填充一行像素缓冲，降低每像素调用开销并保证数据突发连续 */
    for(px = 0; px < width; px++)
    {
        line_buf[2U * px] = color_pair[0];
        line_buf[2U * px + 1U] = color_pair[1];
    }

    LCD_Address_Set(x1, y1, x2 - 1, y2 - 1);   /* 设置地址窗口（闭区间） */
    LCD_DC_Set;
    LCD_BeginWrite();
    for(i = 0; i < height; i++)
    {
        LCD_WriteBytes(line_buf, (uint16_t)(2U * width));
    }
    LCD_EndWrite();
}

/**
 * @brief  在指定坐标绘制一个像素点
 * @param  x,y:  像素坐标
 * @param  color: 颜色
 */
static void LCD_DrawPoint(uint16_t x, uint16_t y, LCD_Color_t color)
{
    LCD_Address_Set(x, y, x, y);
    LCD_WR_DATA16((uint16_t)color);
}

/**
 * @brief  绘制任意直线（Bresenham 算法）
 * @param  x1,y1: 起点坐标（含）
 * @param  x2,y2: 终点坐标（含）
 * @param  color: 线颜色（uint16_t，兼容 LCD_Color_t 枚举隐式转换）
 */
static void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    uint16_t t;
    int xerr = 0, yerr = 0, delta_x, delta_y, distance;
    int incx, incy, uRow, uCol;

    delta_x = (int)x2 - (int)x1;
    delta_y = (int)y2 - (int)y1;
    uRow = (int)x1;
    uCol = (int)y1;

    if(delta_x > 0)       incx = 1;
    else if(delta_x == 0) incx = 0;
    else { incx = -1; delta_x = -delta_x; }

    if(delta_y > 0)       incy = 1;
    else if(delta_y == 0) incy = 0;
    else { incy = -1; delta_y = -delta_y; }

    distance = (delta_x > delta_y) ? delta_x : delta_y;

    for(t = 0; t <= (uint16_t)distance; t++)
    {
        LCD_DrawPoint((uint16_t)uRow, (uint16_t)uCol, (LCD_Color_t)color);
        xerr += delta_x;
        yerr += delta_y;
        if(xerr > distance) { xerr -= distance; uRow += incx; }
        if(yerr > distance) { yerr -= distance; uCol += incy; }
    }
}

/**
 * @brief  绘制矩形边框（绝对坐标，四条边均包含端点）
 * @param  x1,y1: 左上角坐标（含）
 * @param  x2,y2: 右下角坐标（含）
 * @param  color: 边框颜色
 * @note   参数语义为绝对终点坐标，而非宽高。Display.c 中所有调用均遵循此语义。
 */
static void LCD_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    LCD_DrawLine(x1, y1, x2, y1, color);   /* 上边 */
    LCD_DrawLine(x2, y1, x2, y2, color);   /* 右边 */
    LCD_DrawLine(x2, y2, x1, y2, color);   /* 下边 */
    LCD_DrawLine(x1, y2, x1, y1, color);   /* 左边 */
}

/*=============================================================================
 * 字符显示函数
 *=============================================================================*/

/**
 * @brief  显示一个 ASCII 字符
 * @param  x,y:   字符左上角坐标
 * @param  ch:    ASCII字符（' ' ~ '~'）
 * @param  fc:    前景色（字符笔画颜色）
 * @param  bc:    背景色
 * @param  font:  字体大小（ASCII_font_12/16/24/32）
 * @param  mode:  叠加模式（font_overlay_OFF/ON）
 */
static void LCD_ShowChar(uint16_t x, uint16_t y, const char ch,
                         uint16_t fc, uint16_t bc, ASCII_font_t font, font_overlay_t mode)
{
    uint8_t  temp, sizex, t, m = 0;
    uint16_t i, TypefaceNum;
    uint16_t x0 = x;
    uint8_t  idx = (uint8_t)((uint8_t)ch - ' ');   /* 字符偏移量（从空格开始） */

    sizex = (uint8_t)(font / 2);                               /* 字符宽度 = 字高/2 */
    TypefaceNum = (uint16_t)((sizex / 8 + (sizex % 8 ? 1 : 0)) * (uint8_t)font); /* 字模字节数 */

    LCD_Address_Set(x, y, x + sizex - 1, y + (uint16_t)font - 1);  /* 设置字符绘制窗口 */

    for(i = 0; i < TypefaceNum; i++)
    {
        /* 根据字体大小选取对应字模数组 */
        if     (font == ASCII_font_12) temp = ascii_1206[idx][i];   /* 6×12  */
        else if(font == ASCII_font_16) temp = ascii_1608[idx][i];   /* 8×16  */
        else if(font == ASCII_font_24) temp = ascii_2412[idx][i];   /* 12×24 */
        else if(font == ASCII_font_32) temp = ascii_3216[idx][i];   /* 16×32 */
        else return;

        for(t = 0; t < 8; t++)
        {
            if(!mode)   /* 非叠加：每位都输出颜色（前景或背景） */
            {
                if(temp & (0x01 << t)) LCD_WR_DATA16(fc);
                else                   LCD_WR_DATA16(bc);
                m++;
                if(m % sizex == 0) { m = 0; break; }
            }
            else        /* 叠加：仅前景位画点，背景透明 */
            {
                if(temp & (0x01 << t)) LCD_DrawPoint(x, y, (LCD_Color_t)fc);
                x++;
                if((x - x0) == sizex) { x = x0; y++; break; }
            }
        }
    }
}

/**
 * @brief  显示 ASCII 字符串（逐字符调用 LCD_ShowChar）
 * @param  x,y:   起始左上角坐标
 * @param  str:   字符串指针（以 '\0' 结尾）
 * @param  fc,bc: 前景/背景色
 * @param  font:  字体大小
 * @param  mode:  叠加模式
 */
static void LCD_ShowString(uint16_t x, uint16_t y, const char *str,
                           uint16_t fc, uint16_t bc, ASCII_font_t font, font_overlay_t mode)
{
    while(*str != '\0')
    {
        LCD_ShowChar(x, y, *str, fc, bc, font, mode);
        x += (uint16_t)(font / 2);   /* 移动到下一个字符位置 */
        str++;
    }
}

/*=============================================================================
 * 中文字符显示函数
 *=============================================================================*/

/**
 * @brief  显示单个汉字（逐级查询 lcdfont.h 中对应尺寸字库）
 * @param  x,y:   字符左上角坐标
 * @param  str:   指向2字节 GBK 编码汉字的指针
 * @param  fc:    前景色（笔画颜色）
 * @param  bc:    背景色
 * @param  font:  字体大小（CHN_font_12/16/24/32）
 * @param  mode:  叠加模式
 * @note   字库定义于 lcdfont.h（tfont12/16/24/32），需在 CubeMX 中使用 GBK 编码。
 */
static void LCD_ShowChinese(uint16_t x, uint16_t y, const char *str,
                            uint16_t fc, uint16_t bc, CHN_font_t font, font_overlay_t mode)
{
    uint8_t  i, j, m = 0;
    uint16_t k, HZnum;
    uint16_t TypefaceNum;
    uint16_t x0 = x;

    TypefaceNum = (uint16_t)((font / 8 + (font % 8 ? 1 : 0)) * (uint8_t)font);

    if(font == CHN_font_12)
    {
        HZnum = sizeof(tfont12) / sizeof(typFNT_GB12);
        for(k = 0; k < HZnum; k++)
        {
            if((tfont12[k].Index[0] == (uint8_t)str[0]) &&
               (tfont12[k].Index[1] == (uint8_t)str[1]))
            {
                LCD_Address_Set(x, y, x + (uint16_t)font - 1, y + (uint16_t)font - 1);
                for(i = 0; i < (uint8_t)TypefaceNum; i++)
                {
                    for(j = 0; j < 8; j++)
                    {
                        if(!mode)
                        {
                            if(tfont12[k].Msk[i] & (0x01 << j)) LCD_WR_DATA16(fc);
                            else                                  LCD_WR_DATA16(bc);
                            m++;
                            if(m % (uint8_t)font == 0) { m = 0; break; }
                        }
                        else
                        {
                            if(tfont12[k].Msk[i] & (0x01 << j)) LCD_DrawPoint(x, y, (LCD_Color_t)fc);
                            x++;
                            if((x - x0) == (uint16_t)font) { x = x0; y++; break; }
                        }
                    }
                }
                return;   /* 匹配到后立即返回，避免重复渲染 */
            }
        }
    }
    else if(font == CHN_font_16)
    {
        HZnum = sizeof(tfont16) / sizeof(typFNT_GB16);
        for(k = 0; k < HZnum; k++)
        {
            if((tfont16[k].Index[0] == (uint8_t)str[0]) &&
               (tfont16[k].Index[1] == (uint8_t)str[1]))
            {
                LCD_Address_Set(x, y, x + (uint16_t)font - 1, y + (uint16_t)font - 1);
                for(i = 0; i < (uint8_t)TypefaceNum; i++)
                {
                    for(j = 0; j < 8; j++)
                    {
                        if(!mode)
                        {
                            if(tfont16[k].Msk[i] & (0x01 << j)) LCD_WR_DATA16(fc);
                            else                                  LCD_WR_DATA16(bc);
                            m++;
                            if(m % (uint8_t)font == 0) { m = 0; break; }
                        }
                        else
                        {
                            if(tfont16[k].Msk[i] & (0x01 << j)) LCD_DrawPoint(x, y, (LCD_Color_t)fc);
                            x++;
                            if((x - x0) == (uint16_t)font) { x = x0; y++; break; }
                        }
                    }
                }
                return;
            }
        }
    }
    else if(font == CHN_font_24)
    {
        HZnum = sizeof(tfont24) / sizeof(typFNT_GB24);
        for(k = 0; k < HZnum; k++)
        {
            if((tfont24[k].Index[0] == (uint8_t)str[0]) &&
               (tfont24[k].Index[1] == (uint8_t)str[1]))
            {
                LCD_Address_Set(x, y, x + (uint16_t)font - 1, y + (uint16_t)font - 1);
                for(i = 0; i < (uint8_t)TypefaceNum; i++)
                {
                    for(j = 0; j < 8; j++)
                    {
                        if(!mode)
                        {
                            if(tfont24[k].Msk[i] & (0x01 << j)) LCD_WR_DATA16(fc);
                            else                                  LCD_WR_DATA16(bc);
                            m++;
                            if(m % (uint8_t)font == 0) { m = 0; break; }
                        }
                        else
                        {
                            if(tfont24[k].Msk[i] & (0x01 << j)) LCD_DrawPoint(x, y, (LCD_Color_t)fc);
                            x++;
                            if((x - x0) == (uint16_t)font) { x = x0; y++; break; }
                        }
                    }
                }
                return;
            }
        }
    }
    else if(font == CHN_font_32)
    {
        HZnum = sizeof(tfont32) / sizeof(typFNT_GB32);
        for(k = 0; k < HZnum; k++)
        {
            if((tfont32[k].Index[0] == (uint8_t)str[0]) &&
               (tfont32[k].Index[1] == (uint8_t)str[1]))
            {
                LCD_Address_Set(x, y, x + (uint16_t)font - 1, y + (uint16_t)font - 1);
                for(i = 0; i < (uint8_t)TypefaceNum; i++)
                {
                    for(j = 0; j < 8; j++)
                    {
                        if(!mode)
                        {
                            if(tfont32[k].Msk[i] & (0x01 << j)) LCD_WR_DATA16(fc);
                            else                                  LCD_WR_DATA16(bc);
                            m++;
                            if(m % (uint8_t)font == 0) { m = 0; break; }
                        }
                        else
                        {
                            if(tfont32[k].Msk[i] & (0x01 << j)) LCD_DrawPoint(x, y, (LCD_Color_t)fc);
                            x++;
                            if((x - x0) == (uint16_t)font) { x = x0; y++; break; }
                        }
                    }
                }
                return;
            }
        }
    }
}

/**
 * @brief  显示连续汉字串（逐字调用 LCD_ShowChinese）
 * @param  x,y:   起始坐标
 * @param  str:   GBK 编码汉字串，每个汉字占 2 字节，以 '\0' 结尾
 * @param  fc,bc: 前景/背景色
 * @param  font:  字体大小
 * @param  mode:  叠加模式
 */
static void LCD_ShowChineseString(uint16_t x, uint16_t y, const char *str,
                                  uint16_t fc, uint16_t bc, CHN_font_t font, font_overlay_t mode)
{
    while(str[0] != '\0' && str[1] != '\0')
    {
        LCD_ShowChinese(x, y, str, fc, bc, font, mode);
        str += 2;                    /* 每个汉字跨越 2 字节 */
        x   += (uint16_t)font;       /* 向右移动一个字宽 */
    }
}

/**
 * @brief  显示中英文混合字符串（自动识别 ASCII / 汉字）
 * @param  x,y:       起始坐标
 * @param  str:       混合字符串（ASCII < 0x80，中文首字节 ≥ 0x80）
 * @param  fc,bc:     前景/背景色
 * @param  chn_font:  中文字体大小
 * @param  asc_font:  ASCII 字体大小
 * @param  mode:      叠加模式
 * @note   遇 '\r'/'\n' 自动换行；超出屏幕宽/高度时自动换行或回顶。
 */
static void LCD_ShowCHNandENGstring(uint16_t x, uint16_t y, const char *str,
                                    uint16_t fc, uint16_t bc,
                                    CHN_font_t chn_font, ASCII_font_t asc_font,
                                    font_overlay_t mode)
{
    while(*str != '\0')
    {
        if((uint8_t)(*str) > 127)   /* 中文字符（GBK 首字节 ≥ 0x80） */
        {
            if((x + (uint16_t)chn_font) > LCD_W) { x = 0; y += (uint16_t)chn_font; }
            if((y + (uint16_t)chn_font) > LCD_H) { x = 0; y = 0; }
            LCD_ShowChinese(x, y, str, fc, bc, chn_font, mode);
            str += 2;
            x   += (uint16_t)chn_font;
        }
        else                        /* ASCII 字符 */
        {
            if(*str == '\r' || *str == '\n')
            {
                x  = 0;
                y += (uint16_t)asc_font;
            }
            else
            {
                if((x + (uint16_t)asc_font / 2) > LCD_W) { x = 0; y += (uint16_t)asc_font; }
                if((y + (uint16_t)asc_font) > LCD_H)     { x = 0; y = 0; }
                LCD_ShowChar(x, y, *str, fc, bc, asc_font, mode);
                x += (uint16_t)asc_font / 2;
            }
            str++;
        }
    }
}

/*=============================================================================
 * 图片显示
 *=============================================================================*/

/**
 * @brief  在指定区域显示图片（RGB565 格式，大端序，高字节在前）
 * @param  x,y:     左上角起点坐标
 * @param  length:  图片宽度（像素）
 * @param  width:   图片高度（像素）
 * @param  pic:     图片数组指针（每像素2字节）
 */
void LCD_ShowPicture(uint16_t x, uint16_t y, uint16_t length, uint16_t width, const uint8_t *pic)
{
    uint32_t k = 0;
    LCD_Address_Set(x, y, x + length - 1, y + width - 1);
    for(uint16_t i = 0; i < length; i++)
    {
        for(uint16_t j = 0; j < width; j++)
        {
            LCD_WR_DATA8(pic[k * 2]);
            LCD_WR_DATA8(pic[k * 2 + 1]);
            k++;
        }
    }
}











