#include "ltdc.h"
#include "lcd.h"

//---------------------------------------------------------------
LTDC_HandleTypeDef  LTDC_Handler;	    //LTDC���
DMA2D_HandleTypeDef DMA2D_Handler; 	    //DMA2D���

//���ݲ�ͬ����ɫ��ʽ,����֡��������
// һ��1280*800������Ԫ�أ�ÿ��Ԫ�ض�Ӧһ�����أ�ÿ������ռ�������ֽڣ���u16
u16 ltdc_lcd_framebuf[1280][800] __attribute__((at(LCD_FRAME_BUF_ADDR)));	//����������ֱ���ʱ,LCD�����֡���������С,u16��ӦRGB565

u32 *ltdc_framebuf[2];					//LTDC LCD֡��������ָ��,����ָ���Ӧ��С���ڴ�����
_ltdc_dev lcdltdc;						//����LCD LTDC����Ҫ����

//---------------------------------------------------------------
//��LCD����
//lcd_switch:1 ��,0���ر�
void LTDC_Switch(u8 sw)
{
	if(sw==1)
		__HAL_LTDC_ENABLE(&LTDC_Handler);
	else if(sw==0)
		__HAL_LTDC_DISABLE(&LTDC_Handler);
}

//---------------------------------------------------------------
//����ָ����
//layerx:���,0,��һ��; 1,�ڶ���
//sw:1 ��;0�ر�
void LTDC_Layer_Switch(u8 layerx, u8 sw)
{
	if(sw==1)
		__HAL_LTDC_LAYER_ENABLE(&LTDC_Handler,layerx);
	else if(sw==0)
		__HAL_LTDC_LAYER_DISABLE(&LTDC_Handler,layerx);
	
	__HAL_LTDC_RELOAD_CONFIG(&LTDC_Handler);
}

//---------------------------------------------------------------
//ѡ���
//layerx:���;0,��һ��;1,�ڶ���;
void LTDC_Select_Layer(u8 layerx)
{
	lcdltdc.activelayer = layerx;
}

//---------------------------------------------------------------
//����LCD��ʾ����
//dir:0,������1,����
void LTDC_Display_Dir(u8 dir)
{
    lcdltdc.dir = dir; 	//��ʾ����
    
	if(dir==0)		//����
	{
		lcdltdc.width=lcdltdc.pheight;
		lcdltdc.height=lcdltdc.pwidth;
	}
	else if(dir==1)	//����
	{
		lcdltdc.width=lcdltdc.pwidth;
		lcdltdc.height=lcdltdc.pheight;
	}
}

//---------------------------------------------------------------
//���㺯��
//x,y:����
//color:��ɫֵ
void LTDC_Draw_Point(u16 x, u16 y, u32 color)
{
	if(lcdltdc.dir)	//����
	{
		*(u16*)((u32)ltdc_framebuf[lcdltdc.activelayer] + lcdltdc.pixsize*(lcdltdc.pwidth*y+x)) = color;
	}
	else 			//����
	{
		*(u16*)((u32)ltdc_framebuf[lcdltdc.activelayer] + lcdltdc.pixsize*(lcdltdc.pwidth*(lcdltdc.pheight-x-1)+y)) = color; 
	}
}

//---------------------------------------------------------------
//���㺯��
//����ֵ:��ɫֵ
u32 LTDC_Read_Point(u16 x, u16 y)
{
	if(lcdltdc.dir)	//����
	{
		return *(u16*)((u32)ltdc_framebuf[lcdltdc.activelayer] + lcdltdc.pixsize*(lcdltdc.pwidth*y+x));
	}
	else 			//����
	{
		return *(u16*)((u32)ltdc_framebuf[lcdltdc.activelayer] + lcdltdc.pixsize*(lcdltdc.pwidth*(lcdltdc.pheight-x-1)+y)); 
	}
}

//---------------------------------------------------------------
//LTDC������,DMA2D���
//(sx,sy),(ex,ey):�����ζԽ�����,�����СΪ:(ex-sx+1)*(ey-sy+1)   
//ע��:sx,ex,���ܴ���lcddev.width-1;sy,ey,���ܴ���lcddev.height-1!!!
//color:Ҫ������ɫ
void LTDC_Fill(u16 sx, u16 sy, u16 ex, u16 ey, u32 color)
{ 
	u32 psx, psy, pex, pey;	//��LCD���Ϊ��׼������ϵ,����������仯���仯
	u32 timeout = 0; 
	u16 offline;
	u32 addr;
	
	//����ϵת��
	if( lcdltdc.dir )	//����
	{
		psx = sx;
		psy = sy;
		pex = ex;
		pey = ey;
	}
	else			//����
	{
		psx = sy;
		psy = lcdltdc.pheight - ex - 1;
		pex = ey;
		pey = lcdltdc.pheight - sx - 1;
	}
	
	offline = lcdltdc.pwidth - (pex - psx + 1);
	addr = ((u32)ltdc_framebuf[lcdltdc.activelayer] + lcdltdc.pixsize * (lcdltdc.pwidth * psy + psx));
	
	RCC->AHB1ENR |= 1 << 23;			//ʹ��DM2Dʱ��
	DMA2D->CR = 3 << 16;				//�Ĵ������洢��ģʽ
	DMA2D->OPFCCR = LCD_PIXFORMAT;	//������ɫ��ʽ
	DMA2D->OOR = offline;				//������ƫ��
	DMA2D->CR &= ~(1 << 0);				//��ֹͣDMA2D
	DMA2D->OMAR = addr;				//����洢����ַ
	DMA2D->NLR = (pey - psy + 1) | ((pex - psx + 1) << 16);	//�趨�����Ĵ���
	DMA2D->OCOLR = color;				//�趨�����ɫ�Ĵ��� 
	DMA2D->CR |= 1 << 0;				//����DMA2D
	
	while( (DMA2D->ISR & (1 << 1)) ==0 )	//�ȴ��������
	{
		timeout++;
		
		if( timeout > 0X1FFFFF )
			break;	//��ʱ�˳�
	}
	
	DMA2D->IFCR |= 1 << 1;				//���������ɱ�־
}

//---------------------------------------------------------------
//��ָ�����������ָ����ɫ��,DMA2D���
//�˺�����֧��u16,RGB565��ʽ����ɫ�������.
//(sx,sy),(ex,ey):�����ζԽ�����,�����СΪ:(ex-sx+1)*(ey-sy+1)  
//ע��:sx,ex,���ܴ���lcddev.width-1;sy,ey,���ܴ���lcddev.height-1!!!
//color:Ҫ������ɫ�����׵�ַ
void LTDC_Color_Fill(u16 sx, u16 sy, u16 ex, u16 ey, u16 *color)
{
	u32 psx, psy, pex, pey;	//��LCD���Ϊ��׼������ϵ,����������仯���仯
	u32 timeout = 0; 
	u16 offline;
	u32 addr;
	
	//����ϵת��
	if(lcdltdc.dir)	//����
	{
		psx=sx;
		psy=sy;
		pex=ex;
		pey=ey;
	}
	else			//����
	{
		psx=sy;
		psy=lcdltdc.pheight-ex-1;
		pex=ey;
		pey=lcdltdc.pheight-sx-1;
	}
	
	offline=lcdltdc.pwidth-(pex-psx+1);
	addr=((u32)ltdc_framebuf[lcdltdc.activelayer]+lcdltdc.pixsize*(lcdltdc.pwidth*psy+psx));
	
	RCC->AHB1ENR|=1<<23;			//ʹ��DM2Dʱ��
	DMA2D->CR=0<<16;				//�洢�����洢��ģʽ
	DMA2D->FGPFCCR=LCD_PIXFORMAT;	//������ɫ��ʽ
	DMA2D->FGOR=0;					//ǰ������ƫ��Ϊ0
	DMA2D->OOR=offline;				//������ƫ��
	DMA2D->CR&=~(1<<0);				//��ֹͣDMA2D
	DMA2D->FGMAR=(u32)color;		//Դ��ַ
	DMA2D->OMAR=addr;				//����洢����ַ
	DMA2D->NLR=(pey-psy+1)|((pex-psx+1)<<16);	//�趨�����Ĵ��� 
	DMA2D->CR|=1<<0;				//����DMA2D
	
	while((DMA2D->ISR&(1<<1))==0)	//�ȴ��������
	{
		timeout++;
		if(timeout>0X1FFFFF)break;	//��ʱ�˳�
	}
	
	DMA2D->IFCR|=1<<1;				//���������ɱ�־  	
} 

//---------------------------------------------------------------
//LCD����
//color:��ɫֵ
void LTDC_Clear(u32 color)
{
	LTDC_Fill(0, 0, lcdltdc.width-1, lcdltdc.height-1, color);
}

//---------------------------------------------------------------
//LTDCʱ��(Fdclk)���ú���
//PLL3_VCO Input=HSE_VALUE/PLL3M
//PLL3_VCO Output=PLL3_VCO Input * PLL3N
//PLLLCDCLK = PLL3_VCO Output/PLL3R
//����HSE_VALUE=25MHz��PLL3M=5��PLL3N=160,PLL3R=88
//LTDCLK=PLLLCDCLK=25/5*160/88=9MHz
//����ֵ:0,�ɹ�;1,ʧ�ܡ�
u8 LTDC_Clk_Set(u32 pll3m, u32 pll3n, u32 pll3r)
{
	RCC_PeriphCLKInitTypeDef PeriphClkIniture;
	
    PeriphClkIniture.PeriphClockSelection = RCC_PERIPHCLK_LTDC;
    PeriphClkIniture.PLL3.PLL3M = pll3m;
    PeriphClkIniture.PLL3.PLL3N = pll3n;
    PeriphClkIniture.PLL3.PLL3P = 2;
    PeriphClkIniture.PLL3.PLL3Q = 2;
    PeriphClkIniture.PLL3.PLL3R = pll3r;
	
	if(HAL_RCCEx_PeriphCLKConfig(&PeriphClkIniture)==HAL_OK) //��������ʱ�ӣ���������Ϊʱ��Ϊ18.75MHZ
		return 0;   //�ɹ�
	else
		return 1;  //ʧ��
}

//---------------------------------------------------------------
//LTDC,���մ�������,������LCD�������ϵΪ��׼
//ע��:�˺���������LTDC_Layer_Parameter_Config֮��������.
//layerx:��ֵ,0/1.
//sx,sy:��ʼ����
//width,height:��Ⱥ͸߶�
void LTDC_Layer_Window_Config(u8 layerx, u16 sx, u16 sy, u16 width, u16 height)
{
	HAL_LTDC_SetWindowPosition(&LTDC_Handler, sx, sy, layerx);  //���ô��ڵ�λ��
	HAL_LTDC_SetWindowSize(&LTDC_Handler, width, height, layerx);//���ô��ڴ�С    
}

//---------------------------------------------------------------
//LTDC,������������.
//ע��:�˺���,������LTDC_Layer_Window_Config֮ǰ����.
//layerx:��ֵ,0/1.
//bufaddr:����ɫ֡������ʼ��ַ
//pixformat:��ɫ��ʽ.0,ARGB8888;1,RGB888;2,RGB565;3,ARGB1555;4,ARGB4444;5,L8;6;AL44;7;AL88
//alpha:����ɫAlphaֵ,0,ȫ͸��;255,��͸��
//alpha0:Ĭ����ɫAlphaֵ,0,ȫ͸��;255,��͸��
//bfac1:���ϵ��1,4(100),�㶨��Alpha;6(101),����Alpha*�㶨Alpha
//bfac2:���ϵ��2,5(101),�㶨��Alpha;7(111),����Alpha*�㶨Alpha
//bkcolor:��Ĭ����ɫ,32λ,��24λ��Ч,RGB888��ʽ
//����ֵ:��
void LTDC_Layer_Parameter_Config(u8 layerx, u32 bufaddr, u8 pixformat, u8 alpha, u8 alpha0, u8 bfac1, u8 bfac2, u32 bkcolor)
{
	LTDC_LayerCfgTypeDef pLayerCfg;
	
	pLayerCfg.WindowX0 = 0;                       //������ʼX����
	pLayerCfg.WindowY0 = 0;                       //������ʼY����
	pLayerCfg.WindowX1 = lcdltdc.pwidth;          //������ֹX����
	pLayerCfg.WindowY1 = lcdltdc.pheight;         //������ֹY����
	pLayerCfg.PixelFormat = pixformat;		      //���ظ�ʽ
	pLayerCfg.Alpha = alpha;				      //Alphaֵ���ã�0~255,255Ϊ��ȫ��͸��
	pLayerCfg.Alpha0 = alpha0;			        //Ĭ��Alphaֵ
	pLayerCfg.BlendingFactor1 = (u32)bfac1 << 8;//���ò���ϵ��
	pLayerCfg.BlendingFactor2 = (u32)bfac2 << 8;//���ò���ϵ��
	pLayerCfg.FBStartAdress = bufaddr;	        //���ò���ɫ֡������ʼ��ַ
	pLayerCfg.ImageWidth = lcdltdc.pwidth;        //������ɫ֡�������Ŀ��    
	pLayerCfg.ImageHeight = lcdltdc.pheight;      //������ɫ֡�������ĸ߶�
	
	pLayerCfg.Backcolor.Red = (u8)(bkcolor&0X00FF0000)>>16;   //������ɫ��ɫ����
	pLayerCfg.Backcolor.Green = (u8)(bkcolor&0X0000FF00)>>8;  //������ɫ��ɫ����
	pLayerCfg.Backcolor.Blue = (u8)bkcolor&0X000000FF;        //������ɫ��ɫ����
	
	HAL_LTDC_ConfigLayer(&LTDC_Handler, &pLayerCfg, layerx);  //������ѡ�еĲ�
}  

//---------------------------------------------------------------
//��ȡ������,�������ж�����ȫһ���Ų�
//PG6=R7(M0);PI2=G7(M1);PI7=B7(M2);
//M2:M1:M0
//0 :0 :0	//4.3��480*272 RGB��,ID=0X4342
//0 :0 :1	//7��800*480 RGB��,ID=0X7084
//0 :1 :0	//7��1024*600 RGB��,ID=0X7016
//0 :1 :1	//7��1280*800 RGB��,ID=0X7018
//1 :0 :0	//4.3��800*480 RGB��,ID=0X4384
//1 :0 :1   //10.1��1280*800,RGB��,ID=0X1018
//1 :1 :1   //VGA��ʾ��
//����ֵ:LCD ID:0,�Ƿ�;����ֵ,ID;
u16 LTDC_PanelID_Read(void)
{
	u8 idx=0;
	GPIO_InitTypeDef GPIO_Initure;
	__HAL_RCC_GPIOG_CLK_ENABLE();       //ʹ��GPIOGʱ��
	__HAL_RCC_GPIOI_CLK_ENABLE();       //ʹ��GPIOIʱ��

	GPIO_Initure.Pin=GPIO_PIN_6;        //PG6
	GPIO_Initure.Mode=GPIO_MODE_INPUT;  //����
	GPIO_Initure.Pull=GPIO_PULLUP;      //����
	GPIO_Initure.Speed=GPIO_SPEED_FREQ_VERY_HIGH; //����
	HAL_GPIO_Init(GPIOG,&GPIO_Initure); //��ʼ��

	GPIO_Initure.Pin=GPIO_PIN_2|GPIO_PIN_7; //PI2,7
	HAL_GPIO_Init(GPIOI,&GPIO_Initure);     //��ʼ��

	// B7 G7 R7
	idx=(u8)HAL_GPIO_ReadPin(GPIOG,GPIO_PIN_6); //��ȡM0
	idx|=(u8)HAL_GPIO_ReadPin(GPIOI,GPIO_PIN_2)<<1;//��ȡM1
	idx|=(u8)HAL_GPIO_ReadPin(GPIOI,GPIO_PIN_7)<<2;//��ȡM2
	if(idx==0) return 0X4342;	    //4.3����,480*272�ֱ���
	else if(idx==1)return 0X7084;	//7����,800*480�ֱ���
	else if(idx==2)return 0X7016;	//7����,1024*600�ֱ���
	else if(idx==3)return 0X7018;	//7����,1280*800�ֱ���
	else if(idx==4)return 0X4384;	//4.3����,800*480�ֱ���
	else if(idx==5)return 0X1018;	//10.1����,1280*800�ֱ���		
	else return 0;
}

//---------------------------------------------------------------
//LCD��ʼ������
void LTDC_Init(void)
{
	u16 lcdid = 0;
	
	lcdid = LTDC_PanelID_Read();			//��ȡLCD���ID
	
	if(lcdid==0X1018)				//10.1��1280*800 RGB��	
	{
		lcdltdc.pwidth = 1280;			//�����,��λ:����
		lcdltdc.pheight = 800;			//���߶�,��λ:����
		lcdltdc.hbp = 140;				//ˮƽ����
		lcdltdc.hfp = 10;		    	//ˮƽǰ��
		lcdltdc.hsw = 10;				//ˮƽͬ�����
		lcdltdc.vbp = 10;				//��ֱ����
		lcdltdc.vfp = 10;				//��ֱǰ��
		lcdltdc.vsw = 3;				//��ֱͬ�����
		LTDC_Clk_Set(5, 160, 16);		//��������ʱ�� 25/5 * 160 / 16 = 50MHz
		//LTDC_Clk_Set(1, 6, 3);		//��������ʱ��
	}

	lcddev.width = lcdltdc.pwidth;
	lcddev.height = lcdltdc.pheight;

	lcdltdc.pixsize = 2;//ÿ������ռ2���ֽ�
	ltdc_framebuf[0] = (u32*)&ltdc_lcd_framebuf;

	//LTDC����
	LTDC_Handler.Instance = LTDC;
	LTDC_Handler.Init.HSPolarity = LTDC_HSPOLARITY_AL;         //ˮƽͬ������
	LTDC_Handler.Init.VSPolarity = LTDC_VSPOLARITY_AL;         //��ֱͬ������
	LTDC_Handler.Init.DEPolarity = LTDC_DEPOLARITY_AL;         //����ʹ�ܼ���
	LTDC_Handler.Init.PCPolarity = LTDC_PCPOLARITY_IPC;        //����ʱ�Ӽ���
	
	LTDC_Handler.Init.HorizontalSync = lcdltdc.hsw - 1;          //ˮƽͬ�����
	LTDC_Handler.Init.VerticalSync = lcdltdc.vsw - 1;            //��ֱͬ�����
	LTDC_Handler.Init.AccumulatedHBP = lcdltdc.hsw + lcdltdc.hbp - 1; //ˮƽͬ�����ؿ��
	LTDC_Handler.Init.AccumulatedVBP = lcdltdc.vsw + lcdltdc.vbp - 1; //��ֱͬ�����ظ߶�
	LTDC_Handler.Init.AccumulatedActiveW = lcdltdc.hsw + lcdltdc.hbp + lcdltdc.pwidth - 1;//��Ч���
	LTDC_Handler.Init.AccumulatedActiveH = lcdltdc.vsw + lcdltdc.vbp + lcdltdc.pheight - 1;//��Ч�߶�
	LTDC_Handler.Init.TotalWidth = lcdltdc.hsw + lcdltdc.hbp + lcdltdc.pwidth + lcdltdc.hfp - 1;   //�ܿ��
	LTDC_Handler.Init.TotalHeigh = lcdltdc.vsw + lcdltdc.vbp + lcdltdc.pheight + lcdltdc.vfp - 1;  //�ܸ߶�
	
	LTDC_Handler.Init.Backcolor.Red = 0;           //��Ļ�������ɫ����
	LTDC_Handler.Init.Backcolor.Green = 0;         //��Ļ��������ɫ����
	LTDC_Handler.Init.Backcolor.Blue = 0;          //��Ļ����ɫ��ɫ����
	
	HAL_LTDC_Init(&LTDC_Handler);

	//������
	LTDC_Layer_Parameter_Config(0, (u32)ltdc_framebuf[0], LCD_PIXFORMAT, 255, 0, 6, 7, 0X000000);//���������
	LTDC_Layer_Window_Config(0, 0, 0, lcdltdc.pwidth, lcdltdc.pheight);//�㴰������,��LCD�������ϵΪ��׼,��Ҫ����޸�!	

	LTDC_Display_Dir(0);			//Ĭ������
	LTDC_Select_Layer(0); 			//ѡ���1��
	LCD_LED(1);         		    //��������
	LTDC_Clear(0XFFFFFFFF);			//����
}

//---------------------------------------------------------------
//LTDC�ײ�IO��ʼ����ʱ��ʹ��
//�˺����ᱻHAL_LTDC_Init()����
//hltdc:LTDC���
void HAL_LTDC_MspInit(LTDC_HandleTypeDef* hltdc)
{
	GPIO_InitTypeDef GPIO_Initure;

	__HAL_RCC_LTDC_CLK_ENABLE();                //ʹ��LTDCʱ��
	__HAL_RCC_DMA2D_CLK_ENABLE();               //ʹ��DMA2Dʱ��
	__HAL_RCC_GPIOB_CLK_ENABLE();               //ʹ��GPIOBʱ��
	__HAL_RCC_GPIOF_CLK_ENABLE();               //ʹ��GPIOFʱ��
	__HAL_RCC_GPIOG_CLK_ENABLE();               //ʹ��GPIOGʱ��
	__HAL_RCC_GPIOH_CLK_ENABLE();               //ʹ��GPIOHʱ��
	__HAL_RCC_GPIOI_CLK_ENABLE();               //ʹ��GPIOIʱ��

	//��ʼ��PB5����������
	GPIO_Initure.Pin=GPIO_PIN_5;                //PB5������������Ʊ���
	GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;      //�������
	GPIO_Initure.Pull=GPIO_PULLUP;              //����
	GPIO_Initure.Speed=GPIO_SPEED_FREQ_VERY_HIGH; //����
	HAL_GPIO_Init(GPIOB,&GPIO_Initure);

	//��ʼ��PF10��LCD_DE
	GPIO_Initure.Pin=GPIO_PIN_10; 
	GPIO_Initure.Mode=GPIO_MODE_AF_PP;          //����
	GPIO_Initure.Pull=GPIO_NOPULL;              
	GPIO_Initure.Speed=GPIO_SPEED_FREQ_VERY_HIGH; //����
	GPIO_Initure.Alternate=GPIO_AF14_LTDC;      //����ΪLTDC
	HAL_GPIO_Init(GPIOF,&GPIO_Initure);

	//��ʼ��PG6,7,11��LCD_R7��LCD_CLK��LCD_B3
	GPIO_Initure.Pin=GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_11;
	HAL_GPIO_Init(GPIOG,&GPIO_Initure);

	//��ʼ��PH9,10,11,12,13,14,15��LCD_R3~LCD_R6��LCD_G2~LCD_G4
	GPIO_Initure.Pin=GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11|\
					GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
	HAL_GPIO_Init(GPIOH,&GPIO_Initure);

	//��ʼ��PI0,1,2,4,5,6,7,9,10��LCD_G5~LCD_G7��LCD_B4~LCD_B7��LCD_VSYNC��LCD_HSYNC
	GPIO_Initure.Pin=GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_4|GPIO_PIN_5|\
					GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_9|GPIO_PIN_10;
	HAL_GPIO_Init(GPIOI,&GPIO_Initure); 
}

