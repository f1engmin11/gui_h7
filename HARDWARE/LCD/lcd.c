#include "lcd.h"
#include "stdlib.h"
#include "font.h" 
#include "usart.h"	 
#include "delay.h"	 
#include "ltdc.h"
#include "Graphics.h"

//LCD的画笔颜色和背景色
u32 POINT_COLOR = 0xFF000000;	//画笔颜色
u32 BACK_COLOR = 0xFFFFFFFF;	//背景色 

//管理LCD重要参数
//默认为竖屏
_lcd_dev lcddev;

//---------------------------------------------------------------
//LCD开启显示
void LCD_DisplayOn(void)
{
	LTDC_Switch(1);//开启LCD
}

//---------------------------------------------------------------
//LCD关闭显示
void LCD_DisplayOff(void)
{
	LTDC_Switch(0);//关闭LCD
}

//---------------------------------------------------------------
//画点
//x,y:坐标
//POINT_COLOR:此点的颜色
void LCD_DrawPoint(u16 x, u16 y)
{
	LTDC_Draw_Point(x, y, POINT_COLOR);
}

//---------------------------------------------------------------
//快速画点
//x,y:坐标
//color:颜色
void LCD_Fast_DrawPoint(u16 x, u16 y, u32 color)
{
	LTDC_Draw_Point(x, y, color);
}

//---------------------------------------------------------------
//读取个某点的颜色值
//x,y:坐标
//返回值:此点的颜色
u32 LCD_ReadPoint(u16 x, u16 y)
{
	if( x>=lcddev.width || y>=lcddev.height )
		return 0;//超过了范围,直接返回
	else
		return LTDC_Read_Point(x, y);
}

//---------------------------------------------------------------
//设置LCD显示方向
//dir:0,竖屏；1,横屏
void LCD_Display_Dir(u8 dir)
{
	lcddev.dir = dir;         //横屏/竖屏

	LTDC_Display_Dir(dir);
	lcddev.width = lcdltdc.width;
	lcddev.height = lcdltdc.height;
}

//---------------------------------------------------------------
//初始化lcd
//该初始化函数可以初始化各种型号的LCD(详见本.c文件最前面的描述)
void LCD_Init(void)
{
	lcddev.id=LTDC_PanelID_Read();	//检查是否有RGB屏接入
	
	if( lcddev.id != 0 )
	{
		LTDC_Init();			    //ID非零,说明有RGB屏接入.
	}

	LCD_Display_Dir(0);		//默认为竖屏
	//LCD_Display_Dir(1);		//默认为竖屏
	LCD_LED(1);				//点亮背光
	LCD_Clear(WHITE_LCD);
}

//---------------------------------------------------------------
//清屏函数
//color:要清屏的填充色
void LCD_Clear(u32 color)
{
	LTDC_Clear(color);
}

//---------------------------------------------------------------
//在指定区域内填充单个颜色
//(sx,sy),(ex,ey):填充矩形对角坐标,区域大小为:(ex-sx+1)*(ey-sy+1)   
//color:要填充的颜色
void LCD_Fill(u16 sx, u16 sy, u16 ex, u16 ey, u32 color)
{
	LTDC_Fill(sx, sy, ex, ey, color);
}

//---------------------------------------------------------------
//在指定区域内填充指定颜色块			 
//(sx,sy),(ex,ey):填充矩形对角坐标,区域大小为:(ex-sx+1)*(ey-sy+1)   
//color:要填充的颜色
void LCD_Color_Fill(u16 sx, u16 sy, u16 ex, u16 ey, u16 *color)
{
	LTDC_Color_Fill(sx, sy, ex, ey, color);
}

//---------------------------------------------------------------
//画线
//x1,y1:起点坐标
//x2,y2:终点坐标  
void LCD_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2)
{
	u16 t;
	int xerr = 0, yerr = 0, delta_x, delta_y, distance; 
	int incx, incy, uRow, uCol; 
	delta_x = x2 - x1; //计算坐标增量 
	delta_y = y2 - y1; 
	uRow = x1; 
	uCol = y1;
	
	if( delta_x > 0 )
		incx = 1; //设置单步方向 
	else if( delta_x == 0 )
		incx = 0;//垂直线 
	else 
	{
		incx = -1;
		delta_x = -delta_x;
	}
	
	if( delta_y > 0 )
		incy = 1;
	else if( delta_y == 0 )
		incy = 0;//水平线 
	else
	{
		incy = -1;
		delta_y = -delta_y;
	} 
	if( delta_x > delta_y )
		distance = delta_x; //选取基本增量坐标轴 
	else
		distance = delta_y; 
	
	for( t=0; t<=distance+1; t++ )//画线输出 
	{  
		LCD_DrawPoint(uRow, uCol);//画点 
		xerr += delta_x;
		yerr += delta_y;
		
		if( xerr > distance )
		{ 
			xerr -= distance;
			uRow += incx;
		}
		
		if( yerr > distance ) 
		{ 
			yerr -= distance; 
			uCol += incy; 
		}
	}
}

//---------------------------------------------------------------
//画矩形	  
//(x1,y1),(x2,y2):矩形的对角坐标
void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2)
{
	LCD_DrawLine(x1, y1, x2, y1);
	LCD_DrawLine(x1, y1, x1, y2);
	LCD_DrawLine(x1, y2, x2, y2);
	LCD_DrawLine(x2, y1, x2, y2);
}

//---------------------------------------------------------------
//在指定位置画一个指定大小的圆
//(x,y):中心点
//r    :半径
void LCD_Draw_Circle(u16 x0,u16 y0,u8 r)
{
	int a, b;
	int di;
	a = 0;
	b = r;	  
	di = 3 - (r<<1);             //判断下个点位置的标志
	
	while( a <= b )
	{
		LCD_DrawPoint(x0+a, y0-b);             //5
 		LCD_DrawPoint(x0+b, y0-a);             //0           
		LCD_DrawPoint(x0+b, y0+a);             //4               
		LCD_DrawPoint(x0+a, y0+b);             //6 
		LCD_DrawPoint(x0-a, y0+b);             //1       
 		LCD_DrawPoint(x0-b, y0+a);             
		LCD_DrawPoint(x0-a, y0-b);             //2             
  		LCD_DrawPoint(x0-b, y0-a);             //7     	         
		a++;
		
		//使用Bresenham算法画圆
		if( di < 0 )
			di += 4 * a + 6;
		else
		{
			di += 10 + 4 * (a - b);
			b--;
		}
	}
}

//---------------------------------------------------------------
//在指定位置显示一个字符
//x,y:起始坐标
//num:要显示的字符:" "--->"~"
//size:字体大小 12/16/24/32
//mode:叠加方式(1)还是非叠加方式(0)
void LCD_ShowChar(u16 x,u16 y,u8 num,u8 size,u8 mode)
{  							  
    u8 temp, t1, t;
	u16 y0 = y;
	
	u8 csize = (size/8+((size%8)?1:0)) * (size/2);//得到字体一个字符对应点阵集所占的字节数	
 	num = num - ' ';//得到偏移后的值（ASCII字库是从空格开始取模，所以-' '就是对应字符的字库）

	for( t=0; t<csize; t++ )
	{
		if(size==12)
			temp = asc2_1206[num][t]; 	 	//调用1206字体
		else if(size==16)
			temp = asc2_1608[num][t];	//调用1608字体
		else if(size==24)
			temp = asc2_2412[num][t];	//调用2412字体
		else if(size==32)
			temp = asc2_3216[num][t];	//调用3216字体
		else 
			return;								//没有的字库
			
		for( t1=0; t1<8; t1++ )
		{			    
			if( temp & 0x80 )
				LCD_Fast_DrawPoint(x, y, POINT_COLOR);
			else if( mode == 0 )
				LCD_Fast_DrawPoint(x, y, BACK_COLOR);
			
			temp <<= 1;
			y++;
			
			if( y >= lcddev.height )
				return;		//超区域了
				
			if( (y-y0) == size )
			{
				y = y0;
				x++;
				if( x >= lcddev.width )
					return;	//超区域了
					
				break;
			}
		}
	}
}

//---------------------------------------------------------------
//m^n函数
//返回值:m^n次方.
u32 LCD_Pow(u8 m,u8 n)
{
	u32 result = 1;
	while(n--)
		result *= m;
	
	return result;
}

//---------------------------------------------------------------
//显示数字,高位为0,则不显示
//x,y :起点坐标	 
//len :数字的位数
//size:字体大小
//color:颜色 
//num:数值(0~4294967295);	 
void LCD_ShowNum(u16 x, u16 y, u32 num, u8 len, u8 size)
{
	u8 t, temp;
	u8 enshow = 0;
	for( t=0; t<len; t++ )
	{
		temp = (num / LCD_Pow(10, len - t - 1)) % 10;
		
		if( enshow == 0 && t < (len-1) )
		{
			if( temp == 0 )
			{
				LCD_ShowChar(x+(size/2)*t, y, ' ', size, 0);
				continue;
			}
			else
				enshow=1;
			
		}
		
	 	LCD_ShowChar(x+(size/2)*t,y,temp+'0',size,0); 
	}
}

//---------------------------------------------------------------
//显示数字,高位为0,还是显示
//x,y:起点坐标
//num:数值(0~999999999);	 
//len:长度(即要显示的位数)
//size:字体大小
//mode:
//[7]:0,不填充;1,填充0.
//[6:1]:保留
//[0]:0,非叠加显示;1,叠加显示.
void LCD_ShowxNum(u16 x,u16 y,u32 num,u8 len,u8 size,u8 mode)
{  
	u8 t,temp;
	u8 enshow=0;						   
	for(t=0;t<len;t++)
	{
		temp=(num/LCD_Pow(10,len-t-1))%10;
		if(enshow==0&&t<(len-1))
		{
			if(temp==0)
			{
				if(mode&0X80)LCD_ShowChar(x+(size/2)*t,y,'0',size,mode&0X01);  
				else LCD_ShowChar(x+(size/2)*t,y,' ',size,mode&0X01);  
 				continue;
			}else enshow=1; 
		 	 
		}
	 	LCD_ShowChar(x+(size/2)*t,y,temp+'0',size,mode&0X01); 
	}
}

//---------------------------------------------------------------
//显示字符串
//x,y:起点坐标
//width,height:区域大小  
//size:字体大小
//*p:字符串起始地址		  
void LCD_ShowString(u16 x, u16 y, u16 width, u16 height, u8 size, u8 *p)
{
	u8 x0 = x;
	width += x;
	height += y;
	
    while( (*p<='~') && (*p>=' ') )//判断是不是非法字符!
    {
        if( x >= width )
		{ 
			x = x0;
			y += size;
		}
        if( y >= height )
			break;//退出
			
        LCD_ShowChar(x, y, *p, size, 0);
        x += size / 2;
        p++;
    }  
}

UINT16 Touch_callback_cnt = 0;

// bitmap assumed to be declared externally
extern BITMAP_FLASH redRightArrow;
extern BITMAP_FLASH redLeftArrow;

extern SLIDER *pSld;
extern WORD update;

TRANS_MSG objMsg_1 = 0;

//---------------------------------------------------------------
// Call back functions must be defined and return a value of 1
// even though they are not used
WORD GOLMsgCallback(WORD objMsg, OBJ_HEADER* pObj, GOL_MSG* pMsg){
	WORD objectID;
	SLIDER *pSldObj;
	objMsg_1 = objMsg;
	objectID = GetObjID(pObj);
	if (objectID == ID_BTN1) {
		if (objMsg == BTN_MSG_PRESSED) {
			// 设置要显示的位图
			BtnSetBitmap(pObj, &redLeftArrow);
			// 文本设置为右对齐
			SetState(pObj, BTN_TEXTRIGHT);
			#if 1
			pSldObj = (SLIDER*)GOLFindObject(ID_SLD1);
			SldDecPos(pSldObj);
			SetState(pSldObj, SLD_DRAW_THUMB);
			#endif
		}
		else /*if (objMsg ==  BTN_MSG_RELEASED)*/ {
			// 移去位图
			BtnSetBitmap(pObj, NULL);
			// 在中间放置文本
			ClrState(pObj, BTN_TEXTRIGHT);
		}
	}
	if (objectID == ID_BTN2) {
		if (objMsg == BTN_MSG_PRESSED) {
			// 设置要显示的位图
			BtnSetBitmap(pObj, &redRightArrow);
			// 文本设置为左对齐
			SetState(pObj, BTN_TEXTLEFT);
			#if 1
			pSldObj = (SLIDER*)GOLFindObject(ID_SLD1);
			SldIncPos(pSldObj);
			SetState(pSldObj, SLD_DRAW_THUMB);
			#endif
		}
		else {
			// 移去位图
			BtnSetBitmap(pObj, NULL);
			// 在中间放置文本
			ClrState(pObj, BTN_TEXTLEFT);
		}
	}
	
	Touch_callback_cnt += 1;
	//printf("Touch_callback_cnt: %d\r\n", Touch_callback_cnt);
	update = 1;
	//printf("SldGetPos: %d\r\n", SldGetPos(pSld));
	return 1;
}

#if 1
WORD GOLDrawCallback()
{
	WORD value, y, x;// variables for the slider position
	static WORD prevValue = 0;// maintains the previous value of the Slider
	if (update) {
		/* User defined graphics:
		This draws a series of bars indicating the value/position of the
		slider's thumb. The height of the bars follows the equation of a
		parabola "(y-k)^2 = 4a(x-h) with vertex at (k, h) at (60,100) on
		the display. The value 110 is the 4*a constant. x & y are calculated
		based on the value of the slider thumb. The bars are drawn from
		60 to 260 in the x-axis and 10 to 100 in the y-axis. Bars are drawn
		every 6 pixels with width of 4 pixels.
		Only the bars that are added or removed are drawn. This may require
		extra computation. However, it results in significant less data movement.
		Thus resulting in an overall efficient customized drawing.
		*/
		// check the value of slider
		value = SldGetPos(pSld);
		// remove bars if there the new value is less
		// than the previous
		SetColor(BLACK)
		if (value < prevValue) {
			while (prevValue > value) {
				// get the height of the bar to be removed
				y = (prevValue*prevValue)/110;
				// bars are drawn every 6 pixels with width = 4 pixels.
				x = (prevValue*2);
				x = x - (x%6);
				// draw a BLACK colored bar to remove the current bar drawn
				Bar(x+60,100-y, x+64,100);
				// decrement by three since we are drawing every 6 pixels
				prevValue -= 3;
			}
		}
		// Draw bars if there the new value is greater
		// than the previous
		else {
			while (prevValue < value) {
				// set the color of the bar drawn
				if (prevValue < 60) {
					SetColor(BRIGHTGREEN);
				} else if ((prevValue < 80) && (prevValue >= 60)) {
					SetColor(BRIGHTYELLOW);
				} else if (prevValue >= 80) {
					SetColor(BRIGHTRED);
				}
				// get the height of the bar to be drawn
				y = (prevValue*prevValue)/110;
				// bars are drawn every 6 pixels with width = 4 pixels.
				x = (prevValue*2);
				x = x - (x%6);
				// draw a bar to show increase in value
				Bar(x+60,100-y, x+64,100);
				// increment by three since we are drawing every 6 pixels
				prevValue += 3;
			}
		}
	}
	// prevValue will have the current value after drawing or removing bars.
	// reset the update flag
	update = 0;
	
	return 1;
}
#else
//---------------------------------------------------------------
WORD GOLDrawCallback(){
	return 1;
}
#endif

