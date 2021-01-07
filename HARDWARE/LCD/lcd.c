#include "lcd.h"
#include "stdlib.h"
#include "font.h" 
#include "usart.h"	 
#include "delay.h"	 
#include "ltdc.h"
#include "Graphics.h"

//LCD�Ļ�����ɫ�ͱ���ɫ
u32 POINT_COLOR = 0xFF000000;	//������ɫ
u32 BACK_COLOR = 0xFFFFFFFF;	//����ɫ 

//����LCD��Ҫ����
//Ĭ��Ϊ����
_lcd_dev lcddev;

//---------------------------------------------------------------
//LCD������ʾ
void LCD_DisplayOn(void)
{
	LTDC_Switch(1);//����LCD
}

//---------------------------------------------------------------
//LCD�ر���ʾ
void LCD_DisplayOff(void)
{
	LTDC_Switch(0);//�ر�LCD
}

//---------------------------------------------------------------
//����
//x,y:����
//POINT_COLOR:�˵����ɫ
void LCD_DrawPoint(u16 x, u16 y)
{
	LTDC_Draw_Point(x, y, POINT_COLOR);
}

//---------------------------------------------------------------
//���ٻ���
//x,y:����
//color:��ɫ
void LCD_Fast_DrawPoint(u16 x, u16 y, u32 color)
{
	LTDC_Draw_Point(x, y, color);
}

//---------------------------------------------------------------
//��ȡ��ĳ�����ɫֵ
//x,y:����
//����ֵ:�˵����ɫ
u32 LCD_ReadPoint(u16 x, u16 y)
{
	if( x>=lcddev.width || y>=lcddev.height )
		return 0;//�����˷�Χ,ֱ�ӷ���
	else
		return LTDC_Read_Point(x, y);
}

//---------------------------------------------------------------
//����LCD��ʾ����
//dir:0,������1,����
void LCD_Display_Dir(u8 dir)
{
	lcddev.dir = dir;         //����/����

	LTDC_Display_Dir(dir);
	lcddev.width = lcdltdc.width;
	lcddev.height = lcdltdc.height;
}

//---------------------------------------------------------------
//��ʼ��lcd
//�ó�ʼ���������Գ�ʼ�������ͺŵ�LCD(�����.c�ļ���ǰ�������)
void LCD_Init(void)
{
	lcddev.id=LTDC_PanelID_Read();	//����Ƿ���RGB������
	
	if( lcddev.id != 0 )
	{
		LTDC_Init();			    //ID����,˵����RGB������.
	}

	LCD_Display_Dir(0);		//Ĭ��Ϊ����
	//LCD_Display_Dir(1);		//Ĭ��Ϊ����
	LCD_LED(1);				//��������
	LCD_Clear(WHITE_LCD);
}

//---------------------------------------------------------------
//��������
//color:Ҫ���������ɫ
void LCD_Clear(u32 color)
{
	LTDC_Clear(color);
}

//---------------------------------------------------------------
//��ָ����������䵥����ɫ
//(sx,sy),(ex,ey):�����ζԽ�����,�����СΪ:(ex-sx+1)*(ey-sy+1)   
//color:Ҫ������ɫ
void LCD_Fill(u16 sx, u16 sy, u16 ex, u16 ey, u32 color)
{
	LTDC_Fill(sx, sy, ex, ey, color);
}

//---------------------------------------------------------------
//��ָ�����������ָ����ɫ��			 
//(sx,sy),(ex,ey):�����ζԽ�����,�����СΪ:(ex-sx+1)*(ey-sy+1)   
//color:Ҫ������ɫ
void LCD_Color_Fill(u16 sx, u16 sy, u16 ex, u16 ey, u16 *color)
{
	LTDC_Color_Fill(sx, sy, ex, ey, color);
}

//---------------------------------------------------------------
//����
//x1,y1:�������
//x2,y2:�յ�����  
void LCD_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2)
{
	u16 t;
	int xerr = 0, yerr = 0, delta_x, delta_y, distance; 
	int incx, incy, uRow, uCol; 
	delta_x = x2 - x1; //������������ 
	delta_y = y2 - y1; 
	uRow = x1; 
	uCol = y1;
	
	if( delta_x > 0 )
		incx = 1; //���õ������� 
	else if( delta_x == 0 )
		incx = 0;//��ֱ�� 
	else 
	{
		incx = -1;
		delta_x = -delta_x;
	}
	
	if( delta_y > 0 )
		incy = 1;
	else if( delta_y == 0 )
		incy = 0;//ˮƽ�� 
	else
	{
		incy = -1;
		delta_y = -delta_y;
	} 
	if( delta_x > delta_y )
		distance = delta_x; //ѡȡ�������������� 
	else
		distance = delta_y; 
	
	for( t=0; t<=distance+1; t++ )//������� 
	{  
		LCD_DrawPoint(uRow, uCol);//���� 
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
//������	  
//(x1,y1),(x2,y2):���εĶԽ�����
void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2)
{
	LCD_DrawLine(x1, y1, x2, y1);
	LCD_DrawLine(x1, y1, x1, y2);
	LCD_DrawLine(x1, y2, x2, y2);
	LCD_DrawLine(x2, y1, x2, y2);
}

//---------------------------------------------------------------
//��ָ��λ�û�һ��ָ����С��Բ
//(x,y):���ĵ�
//r    :�뾶
void LCD_Draw_Circle(u16 x0,u16 y0,u8 r)
{
	int a, b;
	int di;
	a = 0;
	b = r;	  
	di = 3 - (r<<1);             //�ж��¸���λ�õı�־
	
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
		
		//ʹ��Bresenham�㷨��Բ
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
//��ָ��λ����ʾһ���ַ�
//x,y:��ʼ����
//num:Ҫ��ʾ���ַ�:" "--->"~"
//size:�����С 12/16/24/32
//mode:���ӷ�ʽ(1)���Ƿǵ��ӷ�ʽ(0)
void LCD_ShowChar(u16 x,u16 y,u8 num,u8 size,u8 mode)
{  							  
    u8 temp, t1, t;
	u16 y0 = y;
	
	u8 csize = (size/8+((size%8)?1:0)) * (size/2);//�õ�����һ���ַ���Ӧ������ռ���ֽ���	
 	num = num - ' ';//�õ�ƫ�ƺ��ֵ��ASCII�ֿ��Ǵӿո�ʼȡģ������-' '���Ƕ�Ӧ�ַ����ֿ⣩

	for( t=0; t<csize; t++ )
	{
		if(size==12)
			temp = asc2_1206[num][t]; 	 	//����1206����
		else if(size==16)
			temp = asc2_1608[num][t];	//����1608����
		else if(size==24)
			temp = asc2_2412[num][t];	//����2412����
		else if(size==32)
			temp = asc2_3216[num][t];	//����3216����
		else 
			return;								//û�е��ֿ�
			
		for( t1=0; t1<8; t1++ )
		{			    
			if( temp & 0x80 )
				LCD_Fast_DrawPoint(x, y, POINT_COLOR);
			else if( mode == 0 )
				LCD_Fast_DrawPoint(x, y, BACK_COLOR);
			
			temp <<= 1;
			y++;
			
			if( y >= lcddev.height )
				return;		//��������
				
			if( (y-y0) == size )
			{
				y = y0;
				x++;
				if( x >= lcddev.width )
					return;	//��������
					
				break;
			}
		}
	}
}

//---------------------------------------------------------------
//m^n����
//����ֵ:m^n�η�.
u32 LCD_Pow(u8 m,u8 n)
{
	u32 result = 1;
	while(n--)
		result *= m;
	
	return result;
}

//---------------------------------------------------------------
//��ʾ����,��λΪ0,����ʾ
//x,y :�������	 
//len :���ֵ�λ��
//size:�����С
//color:��ɫ 
//num:��ֵ(0~4294967295);	 
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
//��ʾ����,��λΪ0,������ʾ
//x,y:�������
//num:��ֵ(0~999999999);	 
//len:����(��Ҫ��ʾ��λ��)
//size:�����С
//mode:
//[7]:0,�����;1,���0.
//[6:1]:����
//[0]:0,�ǵ�����ʾ;1,������ʾ.
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
//��ʾ�ַ���
//x,y:�������
//width,height:�����С  
//size:�����С
//*p:�ַ�����ʼ��ַ		  
void LCD_ShowString(u16 x, u16 y, u16 width, u16 height, u8 size, u8 *p)
{
	u8 x0 = x;
	width += x;
	height += y;
	
    while( (*p<='~') && (*p>=' ') )//�ж��ǲ��ǷǷ��ַ�!
    {
        if( x >= width )
		{ 
			x = x0;
			y += size;
		}
        if( y >= height )
			break;//�˳�
			
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
			// ����Ҫ��ʾ��λͼ
			BtnSetBitmap(pObj, &redLeftArrow);
			// �ı�����Ϊ�Ҷ���
			SetState(pObj, BTN_TEXTRIGHT);
			#if 1
			pSldObj = (SLIDER*)GOLFindObject(ID_SLD1);
			SldDecPos(pSldObj);
			SetState(pSldObj, SLD_DRAW_THUMB);
			#endif
		}
		else /*if (objMsg ==  BTN_MSG_RELEASED)*/ {
			// ��ȥλͼ
			BtnSetBitmap(pObj, NULL);
			// ���м�����ı�
			ClrState(pObj, BTN_TEXTRIGHT);
		}
	}
	if (objectID == ID_BTN2) {
		if (objMsg == BTN_MSG_PRESSED) {
			// ����Ҫ��ʾ��λͼ
			BtnSetBitmap(pObj, &redRightArrow);
			// �ı�����Ϊ�����
			SetState(pObj, BTN_TEXTLEFT);
			#if 1
			pSldObj = (SLIDER*)GOLFindObject(ID_SLD1);
			SldIncPos(pSldObj);
			SetState(pSldObj, SLD_DRAW_THUMB);
			#endif
		}
		else {
			// ��ȥλͼ
			BtnSetBitmap(pObj, NULL);
			// ���м�����ı�
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

