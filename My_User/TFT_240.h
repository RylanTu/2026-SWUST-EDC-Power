#ifndef __TFT_240_H
#define __TFT_240_H		

#include "MyApplication.h"

//2026.4.15 RylanTu:如果花屏考虑一下CPOL和CPHA的配置，现在是默认，以及prescaler也要注意一下

//显示方向选择，可选（0 1 2 3）四个方向 设置横屏或者竖屏显示 
//#define USE_HORIZONTAL                  0  // 竖屏，逆时针旋转0度，原点在屏幕左上角  X*Y=135*240
//#define USE_HORIZONTAL                  1  // 竖屏，逆时针旋转180度，原点在屏幕右下角  X*Y=135*240
#define USE_HORIZONTAL                  2  // 横屏，逆时针旋转90度，原点在屏幕右上角  X*Y=240*135
//#define USE_HORIZONTAL                  3  // 横屏，顺时针旋转90度，原点在屏幕左下角  X*Y=240*135
//2026.4.15 RylanTu:方向这个还不确定

//2026.4.15 RylanTu:显示屏分辨率是128*160
#define LCD_W 160   //X轴长度
#define LCD_H 128   //Y轴长度




#define TFT_RES_Clr  HAL_GPIO_WritePin(TFT_RES_GPIO_Port,TFT_RES_Pin,RESET)  //
#define TFT_RES_Set  HAL_GPIO_WritePin(TFT_RES_GPIO_Port,TFT_RES_Pin,SET)  //

#define TFT_DC_Clr  HAL_GPIO_WritePin(TFT_DC_GPIO_Port,TFT_DC_Pin,RESET)  //
#define TFT_DC_Set  HAL_GPIO_WritePin(TFT_DC_GPIO_Port,TFT_DC_Pin,SET)  //

#define TFT_CS_Clr  HAL_GPIO_WritePin(TFT_CS_GPIO_Port,TFT_CS_Pin,RESET)  //
#define TFT_CS_Set  HAL_GPIO_WritePin(TFT_CS_GPIO_Port,TFT_CS_Pin,SET)  //


#define TFT_BLK_OFF HAL_GPIO_WritePin(TFT_BLK_GPIO_Port,TFT_BLK_Pin,GPIO_PIN_SET)  //
#define TFT_BLK_ON  HAL_GPIO_WritePin(TFT_BLK_GPIO_Port,TFT_BLK_Pin,GPIO_PIN_RESET)  //



//定义枚举类型
//颜色定义
typedef enum
{
Color_WHITE      =    	 0xFFFF,   //白色
Color_BLACK      =   	   0x0000,   //黑色 
Color_BLUE       =    	 0x001F,   //蓝色 
Color_BRED       =       0XF81F,   //紫色
Color_GBLUE	     =       0X07FF,   //青色
Color_RED        =	     0xF800,   //红色
Color_MAGENTA    =	     0xF81F,   //品红
Color_GREEN      =	     0x07E0,   //绿色
Color_CYAN       =	     0x7FFF,   //青蓝色
Color_YELLOW     =	     0xFFE0,   //黄色
Color_BROWN      =       0XBC40,   //棕色
Color_BRRED      =       0XFC07,   //棕红色
Color_GRAY       =       0X8430,   //灰色
Color_DARKBLUE 	 =       0X01CF,	 //深蓝色
Color_LIGHTBLUE  =       0X7D7C,	 //浅蓝色
Color_GRAYBLUE   =       0X5458,   //灰蓝色
Color_LIGHTGREEN =     	 0X841F,   //浅绿色
Color_LGRAY 	   =       0XC618,   //浅灰色(PANNEL),窗体背景色
Color_LGRAYBLUE  =       0XA651,   //浅灰蓝色(中间层颜色)
Color_LBBLUE     =       0X2B12,   //浅棕蓝色(选择条目的反色)
}LCD_Color_t;

//ASCII码字体
typedef enum
{
  ASCII_font_12 = 12,
  ASCII_font_16 = 16,
  ASCII_font_24 = 24,
  ASCII_font_32 = 32,
} ASCII_font_t;

//字体叠加
typedef enum
{
  font_overlay_OFF = 0,
  font_overlay_ON = 1,

} font_overlay_t;




#define IS_ASCII_font(font)   (((font) == ASCII_font_12) || ((font) == ASCII_font_16)|| ((font) == ASCII_font_24)|| ((font) == ASCII_font_32))

//中文字体
typedef enum
{
  CHN_font_12 = 12,
  CHN_font_16 = 16,
  CHN_font_24 = 24,
  CHN_font_32 = 32,
} CHN_font_t;
#define IS_CHN_font(font)   (((font) == CHN_font_12) ||(font) == CHN_font_16) || ((font) == CHN_font_24)|| ((font) == CHN_font_32))



//定义结构体类型
typedef struct
{
	void  (*TFT_Init)(void);                                                                                               //LCD初始化
	void  (*TFT_FillColor)(uint16_t,uint16_t,uint16_t,uint16_t,LCD_Color_t);                                               //LCD屏幕填充色
  void  (*TFT_DrawPoint)(uint16_t,uint16_t,LCD_Color_t);                                                             //LCD屏幕画点
  void  (*TFT_DrawLine)(uint16_t,uint16_t,uint16_t,uint16_t,uint16_t);                                               //LCD屏幕画线
  void  (*TFT_DrawRectangle)(uint16_t,uint16_t,uint16_t,uint16_t,uint16_t);                                          //LCD屏幕画矩形
  void  (*TFT_ShowChar)(uint16_t,uint16_t,const char,uint16_t,uint16_t,ASCII_font_t,font_overlay_t);                 //在LCD屏幕上显示一个英文字符
  void  (*TFT_ShowString)(uint16_t,uint16_t,const char *,uint16_t,uint16_t,ASCII_font_t,font_overlay_t);             //在LCD屏幕上显示英文字符串
  void  (*TFT_ShowChinese)(uint16_t ,uint16_t ,const char *,uint16_t ,uint16_t ,CHN_font_t ,font_overlay_t );        //在LCD屏幕上显示一个中文
  void  (*TFT_ShowChinesestring)(uint16_t,uint16_t,const char *,uint16_t,uint16_t,CHN_font_t,font_overlay_t);        //在LCD屏幕上显示中文字符串
  void  (*TFT_ShowCHNandENGstring)(uint16_t, uint16_t, const char *, uint16_t, uint16_t,CHN_font_t,ASCII_font_t,font_overlay_t);//在LCD屏幕上显示中英文字符串
 } TFT_LCD_t;

extern TFT_LCD_t  TFT_LCD;


#endif

