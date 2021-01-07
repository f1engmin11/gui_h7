#include "sys.h"
#include "delay.h"
#include "usart.h" 
#include "led.h"
#include "key.h"
#include "lcd.h"
#include "sdram.h"
#include "touch.h"
#include "Graphics.h"
#include "TouchScreen.h"
/************************************************
 ALIENTEK ������STM32H7������ ʵ��15
 LTDC LCDʵ��-HAL�⺯����
 ����֧�֣�www.openedv.com
 �Ա����̣�http://eboard.taobao.com 
 ��ע΢�Ź���ƽ̨΢�źţ�"����ԭ��"����ѻ�ȡSTM32���ϡ�
 ������������ӿƼ����޹�˾  
 ���ߣ�����ԭ�� @ALIENTEK
************************************************/
//�����Ļ�������Ͻ���ʾ"RST"
void Load_Drow_Dialog(void)
{
	LCD_Clear(WHITE);//����	
	POINT_COLOR=BLUE;//��������Ϊ��ɫ 
	LCD_ShowString(lcddev.width-24,0,200,16,16,"RST");//��ʾ��������
	POINT_COLOR=RED;//���û�����ɫ 
}
////////////////////////////////////////////////////////////////////////////////
//���ݴ�����ר�в���
//��ˮƽ��
//x0,y0:����
//len:�߳���
//color:��ɫ
void gui_draw_hline(u16 x0,u16 y0,u16 len,u16 color)
{
	if(len==0)return;
	if((x0+len-1)>=lcddev.width)x0=lcddev.width-len-1;	//�������귶Χ
	if(y0>=lcddev.height)y0=lcddev.height-1;			//�������귶Χ
	LCD_Fill(x0,y0,x0+len-1,y0,color);	
}
//��ʵ��Բ
//x0,y0:����
//r:�뾶
//color:��ɫ
void gui_fill_circle(u16 x0,u16 y0,u16 r,u16 color)
{											  
	u32 i;
	u32 imax = ((u32)r*707)/1000+1;
	u32 sqmax = (u32)r*(u32)r+(u32)r/2;
	u32 x=r;
	gui_draw_hline(x0-r,y0,2*r,color);
	for (i=1;i<=imax;i++) 
	{
		if ((i*i+x*x)>sqmax)// draw lines from outside	
		{
			if (x>imax) 
			{
				gui_draw_hline (x0-i+1,y0+x,2*(i-1),color);
				gui_draw_hline (x0-i+1,y0-x,2*(i-1),color);
			}
			x--;
		}
		// draw lines from inside (center)	
		gui_draw_hline(x0-x,y0+i,2*x,color);
		gui_draw_hline(x0-x,y0-i,2*x,color);
	}
}  
//������֮��ľ���ֵ 
//x1,x2����ȡ��ֵ��������
//����ֵ��|x1-x2|
u16 my_abs(u16 x1,u16 x2)
{			 
	if(x1>x2)return x1-x2;
	else return x2-x1;
}  
//��һ������
//(x1,y1),(x2,y2):��������ʼ����
//size�������Ĵ�ϸ�̶�
//color����������ɫ
void lcd_draw_bline(u16 x1, u16 y1, u16 x2, u16 y2,u8 size,u16 color)
{
	u16 t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance; 
	int incx,incy,uRow,uCol; 
	if(x1<size|| x2<size||y1<size|| y2<size)return; 
	delta_x=x2-x1; //������������ 
	delta_y=y2-y1; 
	uRow=x1; 
	uCol=y1; 
	if(delta_x>0)incx=1; //���õ������� 
	else if(delta_x==0)incx=0;//��ֱ�� 
	else {incx=-1;delta_x=-delta_x;} 
	if(delta_y>0)incy=1; 
	else if(delta_y==0)incy=0;//ˮƽ�� 
	else{incy=-1;delta_y=-delta_y;} 
	if( delta_x>delta_y)distance=delta_x; //ѡȡ�������������� 
	else distance=delta_y; 
	for(t=0;t<=distance+1;t++ )//������� 
	{  
		gui_fill_circle(uRow,uCol,size,color);//���� 
		xerr+=delta_x ; 
		yerr+=delta_y ; 
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
////////////////////////////////////////////////////////////////////////////////
//5�����ص����ɫ(���ݴ�������)												 
const u16 POINT_COLOR_TBL[10]={RED_LCD,GREEN_LCD,BLUE_LCD,BROWN_LCD,MAGENTA_LCD,BLACK_LCD,MAGENTA_LCD,YELLOW_LCD,MAGENTA_LCD,CYAN_LCD};  
//���ݴ��������Ժ���
void ctp_test(void)
{
	u8 t=0;
	u8 i=0; 		
	u16 lastpos[5][2];		//���һ�ε����� 
	while(1)
	{
		tp_dev.scan(0);
		for(t=0;t<5;t++)
		{
			if((tp_dev.sta)&(1<<t))
			{
				//printf("X����:%d,Y����:%d\r\n",tp_dev.x[0],tp_dev.y[0]);
				if(tp_dev.x[t]<lcddev.width&&tp_dev.y[t]<lcddev.height)
				{
					if(lastpos[t][0]==0XFFFF)
					{
						lastpos[t][0] = tp_dev.x[t];
						lastpos[t][1] = tp_dev.y[t];
					}
					lcd_draw_bline(lastpos[t][0],lastpos[t][1],tp_dev.x[t],tp_dev.y[t],2,POINT_COLOR_TBL[t]);//����
					lastpos[t][0]=tp_dev.x[t];
					lastpos[t][1]=tp_dev.y[t];
					if(tp_dev.x[t]>(lcddev.width-24)&&tp_dev.y[t]<20)
					{
						Load_Drow_Dialog();//���
					}
				}
			}else lastpos[t][0]=0XFFFF;
		}
		
		delay_ms(5);i++;
		if(i%20==0)LED0_Toggle;
	}	
}

//-----------------------------------------------------------------------------------
uint32_t point_read = 0;
u16 color = 0XBC40;

const char L8145[] __attribute__((aligned(2))) = 
{
	0x00,0x00,0x20,0x00,0x4C,0x75,0x3E,0x17,0x00,0x05,0x20,0x00,0x00,0x07,0x37,0x00,0x00,0x15,0x4E,0x00,0x00,0x15,0x93,0x00,0x00,0x15,0xD8,0x00,0x00,0x15,0x1D,0x01,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x38,0x38,0x38,0x38,0x38,0x30,0x30,0x30,
	0x30,0x30,0x30,0x30,0x00,0x00,0x00,0x38,0x38,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x63,0x80,0x1C,0x63,0x00,0x0C,0x63,0x00,0x0C,0x63,0x00,0x0C,0x63,0x70,
	0x7F,0xFF,0xF8,0x0C,0x63,0x00,0x0C,0x63,0x00,0x0C,0x63,0x00,0x0C,0x63,0x00,0x0C,0x63,0x00,0x0C,0x63,0x00,0x0C,0x7F,0x00,0x0C,0x63,0x00,0x0C,0x00,0x00,0x0C,0x00,0x00,
	0x0C,0x00,0xE0,0x1C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x06,0x30,0x00,0x07,0x38,0x00,0x06,0x30,0x00,0x0E,0x30,0x00,0x0C,0x60,0x30,
	0x0C,0x7F,0xF8,0x1E,0xCC,0x60,0x1C,0xCC,0xC0,0x3D,0x8C,0x00,0x6D,0x0C,0x00,0x4C,0x7D,0x80,0x0C,0x6C,0xC0,0x0C,0x6C,0x60,0x0C,0xCC,0x70,0x0C,0xCC,0x30,0x0D,0x8C,0x30,
	0x0F,0x0C,0x30,0x0C,0x0C,0x00,0x0C,0x7C,0x00,0x0C,0x1C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0C,0x00,0x00,0x0C,0x00,0x60,0x0C,0xFF,0xF0,0x0C,0x00,0xC0,
	0x0D,0x81,0x80,0x7F,0xC3,0x00,0x19,0x87,0x00,0x19,0x86,0x00,0x1B,0x86,0x00,0x1B,0x06,0x70,0x3B,0x06,0x00,0x33,0x06,0x00,0x33,0x06,0x00,0x0E,0x06,0x00,0x07,0x06,0x00,
	0x0F,0x86,0x00,0x19,0x86,0x00,0x30,0x86,0x00,0x60,0x3E,0x00,0x00,0x0C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1F,0xFF,0xC0,0x0C,0x61,0x80,
	0x0C,0x61,0x80,0x0F,0xFF,0x80,0x0C,0x61,0x80,0x1C,0x61,0x80,0x1F,0xFF,0x80,0x1C,0x71,0x80,0x00,0xD8,0x00,0x01,0x8E,0x00,0x07,0x03,0x80,0x0D,0x8E,0xF0,0x31,0x8E,0x00,
	0x41,0x8E,0x00,0x03,0x8E,0x00,0x03,0x0E,0x00,0x06,0x0E,0x00,0x0C,0x0E,0x00,0x70,0x0C,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

const struct
{
	short mem; 
	const char* ptr;
	
} ChineseFont = {0,L8145};

XCHAR ChineseStr[] = {0x0023, 0x0024, 0x0022, 0x0025, 0x0021, 0x0000};    // In Chinese

GOL_SCHEME	*altScheme;
GOL_SCHEME *pScheme;
SLIDER *slider[3];
WORD state;
GOL_MSG msg; // GOL message structure to interact with GOL

// bitmap assumed to be declared externally
extern BITMAP_FLASH redRightArrow;
extern BITMAP_FLASH redLeftArrow;

// declare the alternative
// style scheme
// Global variable declarations
SLIDER *pSld;
// global Slider pointer
WORD update;// ������ͼ���±�־
// global variable for
// graphics update flag

BUTTON* ID_BTN_1 = NULL;
BUTTON* ID_BTN_2 = NULL;

void *pBitmap_1 = NULL;
void *pBitmap_2 = NULL;


int main(void)
{
	u8 lcd_id[12];
	
	Cache_Enable();                			//��L1-Cache
	HAL_Init();				        		//��ʼ��HAL��
	Stm32_Clock_Init(160,5,2,4);  		    //����ʱ��,400Mhz 
	delay_init(400);						//��ʱ��ʼ��
	uart_init(115200);						//���ڳ�ʼ��
	LED_Init();								//��ʼ��LED
	KEY_Init();								//��ʼ������
	SDRAM_Init();                   		//��ʼ��SDRAM
    //LCD_Init();                     		//��ʼ��LCD
	//POINT_COLOR=RED; 
	GOLInit();
	TouchInit();
	sprintf((char*)lcd_id,"LCD ID:%04X",lcddev.id);//��LCD ID��ӡ��lcd_id���顣
	
#if 1
{
	volatile uint32_t sys_clk, hclk, pclk, clk;

	sys_clk = HAL_RCC_GetSysClockFreq();
	hclk = HAL_RCC_GetHCLKFreq();
	pclk = HAL_RCC_GetPCLK1Freq();

	clk = 0;
}
#endif

	GOLFree();

	SetColor(WHITE);
	SetLineType(SOLID_LINE);
	SetLineThickness(THICK_LINE);
	
	//Bar(200,300,240*2,320*2);// ʵ�ķ���
	//FillCircle(100,100,10);// ʵ��Բ
	//Circle(400,1000,100);// ����Բ
	//Line(50,50,100,100);// ֱ��
	//Arc(50,50,100,100,20,30,0xff);// Բ�ǿ��ľ���
	//Bevel(100,100,150,150,20);// Բ�ǿ��ľ���

	//SetColor(LIGHTGREEN);// ������ɫ
	//SetLineType(DOTTED_LINE);// �������ͣ�����
	//Rectangle(10,10,80,80);// ��ͼ

	//SetFont((void*)&ChineseFont);
	//MoveTo(10,1000);
	//OutText(ChineseStr);

#if 0
	altScheme = GOLCreateScheme();// Create alternative style scheme
	altScheme->TextColor0 = BLACK;// set text color 0
	altScheme->TextColor1 = BRIGHTBLUE;// set text color 1
	
	BtnCreate( ID_BTN1,// 1st Button ID
				20, 160, 150, 210,// Object's dimensio
				0,
				BTN_DRAW,// set state of the object:draw the object
				NULL,// no bitmap used
				"LEFT",// use this text
				NULL);// use default style scheme
	
	BtnCreate( ID_BTN2,// 2nd Button ID
				170, 160, 300, 210,
				0,
				BTN_DRAW,
				NULL,
				"RIGHT",
				NULL);
	
	SldCreate( ID_SLD1,// Slider ID
				20, 105, 300, 150,// Object's dimension
				SLD_DRAW,// set state of the object:draw the object
				100,// range
				5,// page
				50,// initial position
				NULL);// use default style scheme
	
	while(1)
	{
		if( GOLDraw() )
		{
			// Draw GOL object
			TouchGetMsg(&msg);// Get message from touch screen
			GOLMsg(&msg);// Process message
		}
	}
#else
	altScheme = GOLCreateScheme();// Create alternative style scheme
	altScheme->TextColor0 = BLACK;// set text color 0
	altScheme->TextColor1 = BRIGHTBLUE;// set text color 1
	
	ID_BTN_1 = BtnCreate(	ID_BTN1,// 1st Button ID
				20, 160, 150, 210,// Object's dimension
				0,
				BTN_DRAW,// set state of the object:draw the object
				NULL,//&redLeftArrow,// no bitmap used
				"LEFT",// use this text
				altScheme);// use default style scheme
	
	ID_BTN_2 = BtnCreate(	ID_BTN2,// 2nd Button ID
				170, 160, 300, 210,
				0,
				BTN_DRAW,
				NULL,//&redRightArrow,
				"RIGHT",
				altScheme);
	
	pSld = SldCreate(	ID_SLD1,// Slider ID
						20, 105, 300, 150,// Object's dimension
						SLD_DRAW,// set state of the object:draw the object
						100,// range
						5,// page
						50,// initial position
						NULL);// use default style scheme
	
	update = 1;// to initialize the user,graphics update flag

	pBitmap_1 = BtnGetBitmap(ID_BTN_1);
	pBitmap_2 = BtnGetBitmap(ID_BTN_2);
	
	while(1)
	{
		if (GOLDraw())
		{
			// Draw GOL object
			TouchGetMsg(&msg);// Get message from touch screen
			printf("pMsg->uiEvent: %d\r\n", msg.uiEvent);
			GOLMsg(&msg);// Process message

			delay_ms(10);
		}
	}
#endif
}

