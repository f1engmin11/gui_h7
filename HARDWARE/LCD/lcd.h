#ifndef __LCD_H
#define __LCD_H		
#include "sys.h"	 
#include "stdlib.h" 

//LCD重要参数集
typedef struct  
{
	u16 width;			//LCD 宽度
	u16 height;			//LCD 高度
	u16 id;				//LCD ID
	u8  dir;			//横屏还是竖屏控制：0，竖屏；1，横屏。
	u16	wramcmd;		//开始写gram指令
	u16 setxcmd;		//设置x坐标指令
	u16 setycmd;		//设置y坐标指令
	
} _lcd_dev;

//LCD参数
extern _lcd_dev lcddev;	//管理LCD重要参数
//LCD的画笔颜色和背景色	   
extern u32  POINT_COLOR;//默认红色    
extern u32  BACK_COLOR; //背景颜色.默认为白色

//LCD背光	PB5	
#define LCD_LED(n) (n?HAL_GPIO_WritePin(GPIOB,GPIO_PIN_5,GPIO_PIN_SET):HAL_GPIO_WritePin(GPIOB,GPIO_PIN_5,GPIO_PIN_RESET)) 	    

//画笔颜色
#define WHITE_LCD         	 0xFFFF
#define BLACK_LCD         	 0x0000	  
#define BLUE_LCD        	 0x001F  
#define BRED_LCD             0XF81F
#define GRED_LCD 			 0XFFE0
#define GBLUE_LCD			 0X07FF
#define RED_LCD           	 0xF800
#define MAGENTA_LCD       	 0xF81F
#define GREEN_LCD         	 0x07E0
#define CYAN_LCD          	 0x7FFF
#define YELLOW_LCD        	 0xFFE0
#define BROWN_LCD 			 0XBC40 //棕色
#define BRRED_LCD 			 0XFC07 //棕红色
#define GRAY_LCD  			 0X8430 //灰色
//GUI颜色

#define DARKBLUE_LCD      	 0X01CF	//深蓝色
#define LIGHTBLUE_LCD      	 0X7D7C	//浅蓝色  
#define GRAYBLUE_LCD       	 0X5458 //灰蓝色
//以上三色为PANEL的颜色 
 
#define LIGHTGREEN_LCD     	 0X841F //浅绿色
//#define LIGHTGRAY_LCD        0XEF5B //浅灰色(PANNEL)
#define LGRAY_LCD 			 0XC618 //浅灰色(PANNEL),窗体背景色

#define LGRAYBLUE_LCD        0XA651 //浅灰蓝色(中间层颜色)
#define LBBLUE_LCD           0X2B12 //浅棕蓝色(选择条目的反色)

void LCD_Init(void);													   	//初始化
void LCD_DisplayOn(void);													//开显示
void LCD_DisplayOff(void);													//关显示
void LCD_Clear(u32 Color);	 												//清屏
void LCD_DrawPoint(u16 x,u16 y);											//画点
void LCD_Fast_DrawPoint(u16 x,u16 y,u32 color);								//快速画点
u32  LCD_ReadPoint(u16 x,u16 y); 											//读点 
void LCD_Draw_Circle(u16 x0,u16 y0,u8 r);						 			//画圆
void LCD_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2);							//画线
void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2);		   				//画矩形
void LCD_Fill(u16 sx,u16 sy,u16 ex,u16 ey,u32 color);		   				//填充单色
void LCD_Color_Fill(u16 sx,u16 sy,u16 ex,u16 ey,u16 *color);				//填充指定颜色
void LCD_ShowChar(u16 x,u16 y,u8 num,u8 size,u8 mode);						//显示一个字符
void LCD_ShowNum(u16 x,u16 y,u32 num,u8 len,u8 size);  						//显示一个数字
void LCD_ShowxNum(u16 x,u16 y,u32 num,u8 len,u8 size,u8 mode);				//显示 数字
void LCD_ShowString(u16 x,u16 y,u16 width,u16 height,u8 size,u8 *p);		//显示一个字符串,12/16字体

//LCD分辨率设置
#define SSD_HOR_RESOLUTION		800		//LCD水平分辨率
#define SSD_VER_RESOLUTION		480		//LCD垂直分辨率

//LCD驱动参数设置
#define SSD_HOR_PULSE_WIDTH		1		//水平脉宽
#define SSD_HOR_BACK_PORCH		46		//水平前廊
#define SSD_HOR_FRONT_PORCH		210		//水平后廊

#define SSD_VER_PULSE_WIDTH		1		//垂直脉宽
#define SSD_VER_BACK_PORCH		23		//垂直前廊
#define SSD_VER_FRONT_PORCH		22		//垂直前廊

//如下几个参数，自动计算
#define SSD_HT	(SSD_HOR_RESOLUTION+SSD_HOR_BACK_PORCH+SSD_HOR_FRONT_PORCH)
#define SSD_HPS	(SSD_HOR_BACK_PORCH)
#define SSD_VT 	(SSD_VER_RESOLUTION+SSD_VER_BACK_PORCH+SSD_VER_FRONT_PORCH)
#define SSD_VPS (SSD_VER_BACK_PORCH)

//-----------------------------------------------
#define ID_BTN1 10
#define ID_BTN2 11
#define ID_SLD1 12

#endif

