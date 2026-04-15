#include "TFT_240.h"
#include "Font_ASCII.h"
#include "Font_CHN.h"
/* Private define-------------------------------------------------------------*/

/* Private variables----------------------------------------------------------*/

 static void TFT_Writ_Bus(uint8_t dat);//模拟SPI时序 
 static void TFT_WR_DATA8(uint8_t dat);//写入一个字节
 static void TFT_WR_DATA16(uint16_t dat);//写入两个字节
 static void TFT_WR_REG(uint8_t dat);//写入一个命令
 static void TFT_Address_Set(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2) ;
 static void TFT_Disp_Direction(void);     //设置TFT显示方向


static void  TFT_Init(void);                             //TFT屏幕初始化
static void  TFT_FillColor(uint16_t,uint16_t,uint16_t,uint16_t,LCD_Color_t); 
static void  TFT_DrawPoint(uint16_t ,uint16_t ,LCD_Color_t);         													 //TFT屏幕画点
static void  TFT_DrawLine(uint16_t ,uint16_t ,uint16_t ,uint16_t ,uint16_t );                   	//TFT屏幕画线
static void  TFT_DrawRectangle(uint16_t,uint16_t,uint16_t,uint16_t,uint16_t);                   						//TFT屏幕画矩形
static void  TFT_ShowChar(uint16_t,uint16_t,const char,uint16_t,uint16_t,ASCII_font_t,font_overlay_t);                 //在TFT屏幕上显示一个英文字符
static void  TFT_ShowString(uint16_t,uint16_t,const char *,uint16_t,uint16_t,ASCII_font_t,font_overlay_t);                       //在TFT屏幕上显示英文字符串

static void  TFT_ShowChinese(uint16_t ,uint16_t ,const char *,uint16_t ,uint16_t ,CHN_font_t ,font_overlay_t );        //在TFT屏幕上显示一个中文
static void  TFT_ShowChinesestring(uint16_t,uint16_t,const char *,uint16_t,uint16_t,CHN_font_t,font_overlay_t);        //在TFT屏幕上显示中文字符串
static void  TFT_ShowCHNandENGstring(uint16_t, uint16_t, const char *, uint16_t, uint16_t,CHN_font_t,ASCII_font_t,font_overlay_t);//在TFT屏幕上显示中英文字符串


/* Public variables-----------------------------------------------------------*/
TFT_LCD_t TFT_LCD = 
{
	TFT_Init,
	TFT_FillColor,
	TFT_DrawPoint,
	TFT_DrawLine,
	TFT_DrawRectangle,
	TFT_ShowChar,
	TFT_ShowString,
	TFT_ShowChinese,
	TFT_ShowChinesestring,
	TFT_ShowCHNandENGstring
};



 





/******************************************************************************
      函数说明：TFT串行数据写入函数
      入口数据：dat 要写入的串行数据
      返回值  ：无
******************************************************************************/
 static void TFT_Writ_Bus(uint8_t  dat) 
{	
	TFT_CS_Clr;
	//while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);  //检查接收标志位
	HAL_SPI_Transmit(&hspi2,&dat,1,3);	
 	//HAL_Delay(100);  //这句不用，是加在这里降低速度扫描观察的 
	TFT_CS_Set;
}

/******************************************************************************
      函数说明：TFT写入数据
      入口数据：dat 要写入的数据
      返回值  ：无
******************************************************************************/
 static void TFT_WR_DATA8(uint8_t dat)
{   

	TFT_DC_Set;//写数据
	TFT_Writ_Bus(dat);
}


/******************************************************************************
      函数说明：TFT写入数据
      入口数据：dat 要写入的数据
      返回值  ：无
******************************************************************************/
 static void TFT_WR_DATA16(uint16_t dat)
{
	TFT_WR_DATA8(dat>>8);
	TFT_WR_DATA8(dat);
}


/******************************************************************************
      函数说明：TFT写入命令
      入口数据：dat 要写入的数据
      返回值  ：无
******************************************************************************/
 static void TFT_WR_REG(uint8_t dat)
{
	TFT_DC_Clr;//写命令
	TFT_Writ_Bus(dat);
	TFT_DC_Set;//写数据

}

/******************************************************************************
      函数说明：设置起始和结束地址
      入口数据：X1 X2 设置列的起始和结束地址
	           Y1 Y2 设置行的起始和结束地址
      返回值  ：无
******************************************************************************/
static void TFT_Address_Set(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2)
{
	if(USE_HORIZONTAL==0)
	{
	TFT_WR_REG(0x2a);////列地址设置
	TFT_WR_DATA16(x1);
	TFT_WR_DATA16(x2);
	TFT_WR_REG(0x2b);//行地址设置
	TFT_WR_DATA16(y1);
	TFT_WR_DATA16(y2);
	TFT_WR_REG(0x2c);//储存器写
	}
	else if(USE_HORIZONTAL==1)
	{
	TFT_WR_REG(0x2a);////列地址设置
	TFT_WR_DATA16(x1);
	TFT_WR_DATA16(x2);
	TFT_WR_REG(0x2b);//行地址设置
	TFT_WR_DATA16(y1+80);
	TFT_WR_DATA16(y2+80);
	TFT_WR_REG(0x2c);//储存器写
	}
	else if(USE_HORIZONTAL==2)
	{
	TFT_WR_REG(0x2a);////列地址设置
	TFT_WR_DATA16(x1);
	TFT_WR_DATA16(x2);
	TFT_WR_REG(0x2b);//行地址设置
	TFT_WR_DATA16(y1);
	TFT_WR_DATA16(y2);
	TFT_WR_REG(0x2c);//储存器写
	}
	else 
	{
	TFT_WR_REG(0x2a);////列地址设置
	TFT_WR_DATA16(x1+80);
	TFT_WR_DATA16(x2+80);
	TFT_WR_REG(0x2b);//行地址设置
	TFT_WR_DATA16(y1);
	TFT_WR_DATA16(y2);
	TFT_WR_REG(0x2c);//储存器写
	}

}


 static void TFT_Disp_Direction()     //设置TFT显示方向
   {
	   switch (USE_HORIZONTAL)
	   {
	   case 0 :TFT_WR_REG(0x36); TFT_WR_DATA8(0x00);break ;
	   case 1 :TFT_WR_REG(0x36); TFT_WR_DATA8(0xC0);break ;
	   case 2 :TFT_WR_REG(0x36); TFT_WR_DATA8(0x70);break ;
	   case 3 :TFT_WR_REG(0x36); TFT_WR_DATA8(0xA0);break ;
	   default:TFT_WR_REG(0x36); TFT_WR_DATA8(0xA0);break;	
	   }
   }








static void TFT_Init(void)
{
	TFT_BLK_OFF; //关背光

	TFT_RES_Clr;  //复位
	HAL_Delay(30);
	TFT_RES_Set;
	HAL_Delay(30);
		
	TFT_WR_REG(0x11); 
	HAL_Delay(30); 
	TFT_WR_REG(0x11); 
	HAL_Delay(30); 
	TFT_WR_REG(0x36);
   // TFT_WR_DATA8(0xC0); 
    TFT_Disp_Direction();     //设置TFT显示方向
 
	TFT_WR_REG(0x3A);			
	TFT_WR_DATA8(0x05);

	TFT_WR_REG(0xB2);			
	TFT_WR_DATA8(0x1F);
	TFT_WR_DATA8(0x0F); 
	TFT_WR_DATA8(0x00); 
	TFT_WR_DATA8(0x33); 
	TFT_WR_DATA8(0x33); 			

	TFT_WR_REG(0xB7);			
	TFT_WR_DATA8(0x35);

	TFT_WR_REG(0xBB);			
	TFT_WR_DATA8(0x2B); //Vcom=1.35V
					
	TFT_WR_REG(0xC2);
	TFT_WR_DATA8(0x01);

	TFT_WR_REG(0xC3);			
	TFT_WR_DATA8(0x0F); //GVDD=4.8V  ????
				
	TFT_WR_REG(0xC4);			
	TFT_WR_DATA8(0x20); //VDV, 0x20:0v

	TFT_WR_REG(0xC6);			
	TFT_WR_DATA8(0x13); //0x0F:60Hz        	

	TFT_WR_REG(0xD0);			
	TFT_WR_DATA8(0xA4);
	TFT_WR_DATA8(0xA1); 

	TFT_WR_REG(0xE0);
	TFT_WR_DATA8(0xF0);   
	TFT_WR_DATA8(0x04);   
	TFT_WR_DATA8(0x07);   
	TFT_WR_DATA8(0x04);   
	TFT_WR_DATA8(0x04);   
	TFT_WR_DATA8(0x04);   
	TFT_WR_DATA8(0x25);   
	TFT_WR_DATA8(0x33);   
	TFT_WR_DATA8(0x3C);   
	TFT_WR_DATA8(0x36);   
	TFT_WR_DATA8(0x14);   
	TFT_WR_DATA8(0x12);   
	TFT_WR_DATA8(0x29);   
	TFT_WR_DATA8(0x30);   

	TFT_WR_REG(0xE1);     
	TFT_WR_DATA8(0xF0);   
	TFT_WR_DATA8(0x02);   
	TFT_WR_DATA8(0x04);   
	TFT_WR_DATA8(0x05);   
	TFT_WR_DATA8(0x05);   
	TFT_WR_DATA8(0x21);   
	TFT_WR_DATA8(0x25);   
	TFT_WR_DATA8(0x32);   
	TFT_WR_DATA8(0x3B);   
	TFT_WR_DATA8(0x38);   
	TFT_WR_DATA8(0x12);   
	TFT_WR_DATA8(0x14);   
	TFT_WR_DATA8(0x27);   
	TFT_WR_DATA8(0x31);
	
    TFT_WR_REG(0xE4);     
	TFT_WR_DATA8(0x1D);   
	TFT_WR_DATA8(0x00);   
	TFT_WR_DATA8(0x00);   
	TFT_WR_DATA8(0x05); 
	
	TFT_WR_REG(0x21);   
	TFT_WR_REG(0x29);

    TFT_LCD.TFT_FillColor(0,0,LCD_W,LCD_H,Color_WHITE);
	printf("IPS240_TFT Init OK!\r\n"); //TFT初始化完成
	TFT_BLK_ON;
}



/******************************************************************************
      函数说明：在指定区域填充颜色
      入口数据：xsta,ysta   起始坐标
                xend,yend   终止坐标
								color       要填充的颜色
      返回值：  无
******************************************************************************/
static void TFT_FillColor(uint16_t xstar,uint16_t ystar,uint16_t xWidth,uint16_t yHeight,LCD_Color_t Color)
{          
	uint16_t i,j; 
	TFT_Address_Set(xstar,ystar,xWidth-1,yHeight-1);//设置显示范围
	for(i=ystar;i<yHeight;i++)
	{													   	 	
		for(j=xstar;j<xWidth;j++)
		{
			TFT_WR_DATA16(Color);
		}
	} 					  	    
}

/******************************************************************************
      函数说明：在指定区域画点
      入口数据：xsta,ysta   起始坐标
                
								color       要画点的颜色
      返回值：  无
******************************************************************************/
static void TFT_DrawPoint(uint16_t xstar,uint16_t ystar,LCD_Color_t Color)
{
TFT_Address_Set(xstar,ystar,xstar,ystar);//设置画点的坐标
TFT_WR_DATA16(Color);
}

/******************************************************************************
      函数说明：在指定区域画线
      入口数据：xsta,ysta   起始坐标
                xend,yend   终止坐标
								color       要填充的颜色
      返回值：  无
******************************************************************************/


static void TFT_DrawLine(uint16_t xsta,uint16_t ysta,uint16_t xend,uint16_t yend,uint16_t color)
{
	uint16_t t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance;
	int incx,incy,uRow,uCol;
	delta_x=xend-xsta; //计算坐标增量
	delta_y=yend-ysta;
	uRow=xsta;//画线起点坐标
	uCol=ysta;
	if(delta_x>0)incx=1; //计置单步方向
	else if (delta_x==0)incx=0;//垂直线
	else {incx=-1;delta_x=-delta_x;}
	if(delta_y>0)incy=1;
	else if (delta_y==0)incy=0;//水平线
	else {incy=-1;delta_y=-delta_y;}
	if(delta_x>delta_y)distance=delta_x; //选取基本增量坐标轴
	else distance=delta_y;
	for(t=0;t<distance+1;t++)
	{
		TFT_LCD.TFT_DrawPoint(uRow,uCol,color);//画点
		xerr+=delta_x;
		yerr+=delta_y;
		if(xerr>distance)
		{
			xerr-=distance;
			uRow+=incx;
		}
		if(yerr>distance)
		{
			yerr-=distance;
			uCol+=incy;
		}
	}
}


static void TFT_DrawRectangle(uint16_t xsta,uint16_t ysta,uint16_t xWidth,uint16_t yHeight,uint16_t color)
{
	TFT_LCD.TFT_DrawLine(xsta,ysta,xsta+xWidth-1,ysta,color);   //上横线
	TFT_LCD.TFT_DrawLine(xsta+xWidth-1,ysta,xsta+xWidth-1,ysta+yHeight-1,color);  //右竖线
	TFT_LCD.TFT_DrawLine(xsta+xWidth-1,ysta+yHeight-1,xsta,ysta+yHeight-1,color);  //下横线
	
	TFT_LCD.TFT_DrawLine(xsta,ysta+yHeight-1,xsta,ysta,color); //左竖线
}

  //TFT_LCD.TFT_DrawLine(0,212,240,212,Color_RED);        //电压的个位 

/******************************************************************************
      函数说明：显示单个字符
      入口数据：xsta,ysta   起始坐标
               cChar 要显示的字符
			   UsColor_Foreground字的颜色
			   usColor_Background背景的颜色
			   size 字号
			   mode 0非叠加模式 1叠加模式
      返回值：  无
******************************************************************************/
void TFT_ShowChar(uint16_t xstar,uint16_t ystar,const char cChar,uint16_t UsColor_Foreground,uint16_t usColor_Background,ASCII_font_t size,font_overlay_t mode)
{
	uint8_t temp,sizex,ucIndex,t,m=0;
	uint16_t i,TypefaceNum;//一个字符所占字节大小
	uint16_t x0=xstar;
	sizex=size/2;
	TypefaceNum=(sizex/8+((sizex%8)?1:0))*size;
	ucIndex=cChar-' ';    //得到偏移后的值
	TFT_Address_Set(xstar,ystar,xstar+sizex-1,ystar+size-1);  //设置光标位置
	for(i=0;i<TypefaceNum;i++)
	{ 
		if(size==ASCII_font_12)temp=ucAscii_1206[ucIndex][i];		       //调用6*12字体
		else if(size==ASCII_font_16)temp=ucAscii_1608[ucIndex][i];		 //调用8*16字体
		else if(size==ASCII_font_24)temp=ucAscii_2412[ucIndex][i];		 //调用12*24字体
		else if(size==ASCII_font_32)temp=ucAscii_3216[ucIndex][i];		 //调用16*32字体
		else return;
		
		for(t=0;t<8;t++)
		{
			if(!mode)//非叠加模式
			{
				if(temp&(0x01<<t))TFT_WR_DATA16(UsColor_Foreground);
				else TFT_WR_DATA16(usColor_Background);
				m++;
				if(m%sizex==0)
				{
					m=0;
					break;
				}
			}
			else
			{
				if(temp&(0x01<<t)) TFT_LCD.TFT_DrawPoint(xstar,ystar,UsColor_Foreground);//画点
				xstar++;
				if((xstar-x0)==sizex)
				{
					xstar=x0;
					ystar++;
					break;
				}
			}
		}
	}   	 	  
}




/******************************************************************************
      函数说明：显示字符串
      入口数据：xsta,ysta   起始坐标
               *p 要显示的字符
			   UsColor_Foreground字的颜色
			   usColor_Background背景的颜色
			   size 字号
			   mode 0非叠加模式 1叠加模式
      返回值：  无
******************************************************************************/

void TFT_ShowString(uint16_t xstar,uint16_t ystar,const char *p,uint16_t UsColor_Foreground,uint16_t usColor_Background,ASCII_font_t size,font_overlay_t mode)
{         
	while(*p!='\0')
	{       
		TFT_LCD.TFT_ShowChar(xstar,ystar,*p,UsColor_Foreground,usColor_Background,size,mode);
		xstar+=size/2;
		p++;
	}  
}


/******************************************************************************
      函数说明：显示单个汉字
      入口数据：xsta,ysta   起始坐标
               *pStr 要显示的汉字
			   UsColor_Foreground字的颜色
			   usColor_Background背景的颜色
			   sizey 字号
			   mode 0非叠加模式 1叠加模式
      返回值：  无
******************************************************************************/

void TFT_ShowChinese(uint16_t xstar,uint16_t ystar,const char *pStr,uint16_t usColor_Background,uint16_t usColor_Foreground,CHN_font_t sizey,font_overlay_t mode)
{
	uint8_t i,j,m=0;
	uint16_t k;
	uint16_t HZnum;//汉字数目
	uint16_t TypefaceNum;//一个字符所占的字节大小
	uint16_t x0=xstar;
	TypefaceNum=(sizey/8+((sizey%8)?1:0))*sizey;
	if(sizey== CHN_font_12)
	{                         
	 HZnum=sizeof(FONT_CHN12)/sizeof(FONT_CHN12_t);	//统计汉字数目
	// printf("HZnum_TESET1 %d\r\n\r\n",sizeof(FONT_CHN12)); 
	// printf("HZnum_TESET2 %d\r\n\r\n",sizeof(FONT_CHN12_t)); 
	// printf("HZnum_TESET3 %d\r\n\r\n",*(pStr));   
	 //printf("HZnum_TESET4 %x\r\n\r\n",(FONT_CHN12[k].Index[0]));   
	 //printf("HZnum_TESET5 %x\r\n\r\n",(FONT_CHN12[k].Index[1]));   
	 for(k=0;k<HZnum;k++) 
	 {
		if((FONT_CHN12[k].Index[0]==*(pStr))&&(FONT_CHN12[k].Index[1]==*(pStr+1)))
		{ 	
			TFT_Address_Set(xstar,ystar,xstar+sizey-1,ystar+sizey-1);
			for(i=0;i<TypefaceNum;i++)
			{
				for(j=0;j<8;j++)
				{	
					if(!mode)//非叠加模式
					{
						if(FONT_CHN12[k].CHN_code[i]&(0x01<<j))TFT_WR_DATA16(usColor_Foreground);
						else TFT_WR_DATA16(usColor_Background);
						m++;
						if(m%sizey==0)
						{
							m=0;
							break;
						}
					}
					else//叠加模式
					{
						if(FONT_CHN12[k].CHN_code[i]&(0x01<<j))	TFT_LCD.TFT_DrawPoint(xstar,ystar,usColor_Foreground);//画一个点
						xstar++;
						if((xstar-x0)==sizey)
						{
							xstar=x0;
							ystar++;
							break;
						}
					}
				}
			}
		}				  	
		continue;  //查找到对应点阵字库立即退出，防止多个汉字重复取模带来影响
	 }
	}
	if(sizey== CHN_font_16)
	{                         
	 HZnum=sizeof(FONT_CHN16)/sizeof(FONT_CHN16_t);	//统计汉字数目
	 for(k=0;k<HZnum;k++) 
	 {
		if((FONT_CHN16[k].Index[0]==*(pStr))&&(FONT_CHN16[k].Index[1]==*(pStr+1)))
		{ 	
			TFT_Address_Set(xstar,ystar,xstar+sizey-1,ystar+sizey-1);
			for(i=0;i<TypefaceNum;i++)
			{
				for(j=0;j<8;j++)
				{	
					if(!mode)//非叠加模式
					{
						if(FONT_CHN16[k].CHN_code[i]&(0x01<<j))TFT_WR_DATA16(usColor_Foreground);
						else TFT_WR_DATA16(usColor_Background);
						m++;
						if(m%sizey==0)
						{
							m=0;
							break;
						}
					}
					else//叠加模式
					{
						if(FONT_CHN16[k].CHN_code[i]&(0x01<<j))	TFT_LCD.TFT_DrawPoint(xstar,ystar,usColor_Foreground);//画一个点
						xstar++;
						if((xstar-x0)==sizey)
						{
							xstar=x0;
							ystar++;
							break;
						}
					}
				}
			}
		}				  	
		continue;  //查找到对应点阵字库立即退出，防止多个汉字重复取模带来影响
	 }
	}
	if(sizey== CHN_font_24)
	{                         
	 HZnum=sizeof(FONT_CHN24)/sizeof(FONT_CHN24_t);	//统计汉字数目
	 for(k=0;k<HZnum;k++) 
	 {
		if((FONT_CHN24[k].Index[0]==*(pStr))&&(FONT_CHN24[k].Index[1]==*(pStr+1)))
		{ 	
			TFT_Address_Set(xstar,ystar,xstar+sizey-1,ystar+sizey-1);
			for(i=0;i<TypefaceNum;i++)
			{
				for(j=0;j<8;j++)
				{	
					if(!mode)//非叠加模式
					{
						if(FONT_CHN24[k].CHN_code[i]&(0x01<<j))TFT_WR_DATA16(usColor_Foreground);
						else TFT_WR_DATA16(usColor_Background);
						m++;
						if(m%sizey==0)
						{
							m=0;
							break;
						}
					}
					else//叠加模式
					{
						if(FONT_CHN24[k].CHN_code[i]&(0x01<<j))	TFT_LCD.TFT_DrawPoint(xstar,ystar,usColor_Foreground);//画一个点
						xstar++;
						if((xstar-x0)==sizey)
						{
							xstar=x0;
							ystar++;
							break;
						}
					}
				}
			}
		}				  	
		continue;  //查找到对应点阵字库立即退出，防止多个汉字重复取模带来影响
	 }
	}
	if(sizey== CHN_font_32)
	{                         
	 HZnum=sizeof(FONT_CHN32)/sizeof(FONT_CHN32_t);	//统计汉字数目
	 for(k=0;k<HZnum;k++) 
	 {
		if((FONT_CHN32[k].Index[0]==*(pStr))&&(FONT_CHN32[k].Index[1]==*(pStr+1)))
		{ 	
			TFT_Address_Set(xstar,ystar,xstar+sizey-1,ystar+sizey-1);
			for(i=0;i<TypefaceNum;i++)
			{
				for(j=0;j<8;j++)
				{	
					if(!mode)//非叠加模式
					{
						if(FONT_CHN32[k].CHN_code[i]&(0x01<<j))TFT_WR_DATA16(usColor_Foreground);
						else TFT_WR_DATA16(usColor_Background);
						m++;
						if(m%sizey==0)
						{
							m=0;
							break;
						}
					}
					else//叠加模式
					{
						if(FONT_CHN32[k].CHN_code[i]&(0x01<<j))	TFT_LCD.TFT_DrawPoint(xstar,ystar,usColor_Foreground);//画一个点
						xstar++;
						if((xstar-x0)==sizey)
						{
							xstar=x0;
							ystar++;
							break;
						}
					}
				}
			}
		}				  	
		continue;  //查找到对应点阵字库立即退出，防止多个汉字重复取模带来影响
	 }
	}
} 





/******************************************************************************
      函数说明：显示汉字串
      入口数据：xsta,ysta   起始坐标
               *pStr 要显示的汉字
			   UsColor_Foreground字的颜色
			   usColor_Background背景的颜色
			   size 字号
			   mode 0非叠加模式 1叠加模式
      返回值：  无
******************************************************************************/

/*******************************************************************************/
static void TFT_ShowChinesestring(uint16_t xstar,uint16_t ystar,const char *pStr,uint16_t usColor_Foreground,uint16_t usColor_Background,CHN_font_t sizey,font_overlay_t mode)
{

    printf("sizeof_test1 %d\r\n\r\n",sizeof(pStr));                 
	printf("strlen_test1 %d\r\n\r\n",strlen(pStr));             
	while(*pStr!=0)
	{	  
		TFT_LCD.TFT_ShowChinese(xstar,ystar,pStr,usColor_Foreground,usColor_Background,sizey,mode);
		pStr+=2;
		xstar+=sizey;
	}
}

/*
	* @name   LCD_ShowCHNandENGstring
	* @brief  在LCD屏幕上显示中英文字符串
	* @param  usX起始X坐标
*             usY起始Y坐标
*           pStr:要显示的中英文字符串的首地址
*           usColor_Background：选择字符的背景色
*           usColor_Foreground：选择字符的前景色
*           font_CHN：中文字体选择
*             ??????CHN_font_16：16号字体
*                   CHN_font_24：24号字体
*           font_ASCII??ASCII码字体选择
*             ??????ASCII_font_16：16号字体
*                   ASCII_font_24：24号字体
	* @retval None      
*/
static void TFT_ShowCHNandENGstring(uint16_t xstar, uint16_t ystar, const char * pStr, uint16_t usColor_Background, uint16_t usColor_Foreground,CHN_font_t font_CHN,ASCII_font_t font_ASCII,font_overlay_t mode)
{
	while (* pStr != '\0')
	{
		//中文字符
		if((* pStr) > 127) //首地址从0X80开始为中文
		{
			//自动换行
			if ((xstar + font_CHN) > LCD_W)
			{
				xstar = 0;
				ystar += font_CHN;
			} 
			//自动换页
			if ((ystar + font_CHN) > LCD_H)
			{
				xstar = 0;
				ystar = 0;
			}
			//显示中文字符
			TFT_LCD.TFT_ShowChinese(xstar,ystar,pStr,usColor_Foreground,usColor_Background,font_CHN,mode);
			//更新位置
			pStr += 2;       //一个汉字占两个字节
			xstar += font_CHN;
		}
		//英文字符
		else
		{
			if((* pStr == '\r') | (* pStr == '\n'))
		  {
				//前面的字符为中文
				if((* (pStr-1)) > 127)
				{
					//自动换行
					xstar = 0;
					ystar += font_CHN;
				}
				//前面的字符为英文
				else
				{
					//换行
					xstar = 0;
					ystar += font_ASCII;	
				}							
			}
			else
			{
				//自动换行
				if ((xstar + font_ASCII/2) > LCD_W)
				{
					xstar = 0;
					ystar += font_ASCII;
				} 
				//×自动换页
				if ((ystar + font_ASCII) > LCD_H)
				{
					xstar = 0;
					ystar = 0;
				}
				//显示字符
				TFT_LCD.TFT_ShowChar(xstar, ystar, * pStr, usColor_Background, usColor_Foreground,font_ASCII,mode);
				//更新位置
				xstar += font_ASCII/2;
			}
			//指向下一个字符
     pStr ++; 			
		}		
	}
}

#if 0
#endif