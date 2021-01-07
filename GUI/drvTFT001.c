/*****************************************************************************
 *  Module for Microchip Graphics Library
 *  LCD controller driver
 *  LG LGDP4531 
 *  Renesas R61505 
 *  Renesas R61580
 *  Samsung S6D0129
 *  Samsung S6D0139
 *  Orise Tech. SPFD5408
 *  Ilitek ILI9320
 *****************************************************************************
 * FileName:        drvTFT001.c
 * Dependencies:    Graphics.h
 * Processor:       PIC24, PIC32
 * Compiler:       	MPLAB C30, MPLAB C32
 * Linker:          MPLAB LINK30, MPLAB LINK32
 * Company:         Microchip Technology Incorporated
 *
 * Software License Agreement
 *
 * Copyright ?2008 Microchip Technology Inc.  All rights reserved.
 * Microchip licenses to you the right to use, modify, copy and distribute
 * Software only when embedded on a Microchip microcontroller or digital
 * signal controller, which is integrated into your product or third party
 * product (pursuant to the sublicense terms in the accompanying license
 * agreement).  
 *
 * You should refer to the license agreement accompanying this Software
 * for additional information regarding your rights and obligations.
 *
 * SOFTWARE AND DOCUMENTATION ARE PROVIDED “AS IS?WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY
 * OF MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR
 * PURPOSE. IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR
 * OBLIGATED UNDER CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION,
 * BREACH OF WARRANTY, OR OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT
 * DAMAGES OR EXPENSES INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL,
 * INDIRECT, PUNITIVE OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA,
 * COST OF PROCUREMENT OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY
 * CLAIMS BY THIRD PARTIES (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF),
 * OR OTHER SIMILAR COSTS.
 *
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Anton Alkhimenok     11/12/07	Version 1.0 release
 * Anton Alkhimenok	    01/30/08	combined version for landscape and portrait
 * Sean Justice         01/30/08    PIC32 support
 * Jayanth Murthy       06/25/09    dsPIC & PIC24H support 
 * Anton Alkhimenok     06/26/09    16-bit PMP support
 *****************************************************************************/
#include "Graphics.h"
// Color
WORD   _color;
// Clipping region control
SHORT _clipRgn;
// Clipping region borders
SHORT _clipLeft;
SHORT _clipTop;
SHORT _clipRight;
SHORT _clipBottom;

/////////////////////// LOCAL FUNCTIONS PROTOTYPES ////////////////////////////
void SetReg(WORD index, WORD value);
void PutImage1BPP(SHORT left, SHORT top, FLASH_BYTE* bitmap, BYTE stretch);
void PutImage4BPP(SHORT left, SHORT top, FLASH_BYTE* bitmap, BYTE stretch);
void PutImage8BPP(SHORT left, SHORT top, FLASH_BYTE* bitmap, BYTE stretch);
void PutImage16BPP(SHORT left, SHORT top, FLASH_BYTE* bitmap, BYTE stretch);

void PutImage1BPPExt(SHORT left, SHORT top, void* bitmap, BYTE stretch);
void PutImage4BPPExt(SHORT left, SHORT top, void* bitmap, BYTE stretch);
void PutImage8BPPExt(SHORT left, SHORT top, void* bitmap, BYTE stretch);
void PutImage16BPPExt(SHORT left, SHORT top, void* bitmap, BYTE stretch);


/*********************************************************************
* Overview: Additioal hardware-accelerated functions can be implemented
*           in the driver. These definitions exclude the PutPixel()-based
*           functions in the primitives layer (Primitive.c file) from compilation.
*********************************************************************/

// Define this to implement Font related functions in the driver.
//#define USE_DRV_FONT

// Define this to implement Line function in the driver.
//#define USE_DRV_LINE

// Define this to implement Circle function in the driver.
//#define USE_DRV_CIRCLE

// Define this to implement FillCircle function in the driver.
//#define USE_DRV_FILLCIRCLE

// Define this to implement Bar function in the driver.
//#define USE_DRV_BAR

// Define this to implement ClearDevice function in the driver.
//#define USE_DRV_CLEARDEVICE

// Define this to implement PutImage function in the driver.
//#define USE_DRV_PUTIMAGE


#ifndef DISP_HOR_RESOLUTION
#error  DISP_HOR_RESOLUTION must be defined in GraphicsConfig.h
#endif

#ifndef DISP_VER_RESOLUTION
#error  DISP_VER_RESOLUTION must be defined in GraphicsConfig.h
#endif

#ifndef COLOR_DEPTH
#error  COLOR_DEPTH must be defined in GraphicsConfig.h
#endif

#ifndef DISP_ORIENTATION
#error  DISP_ORIENTATION must be defined in GraphicsConfig.h
#endif


/*********************************************************************
* Overview: Horizontal and vertical screen size.
*********************************************************************/
#if (DISP_HOR_RESOLUTION != 240)
#error This driver doesn't supports this resolution. Horisontal resolution must be 240 pixels.
#endif
#if (DISP_VER_RESOLUTION != 320)
#error This driver doesn't supports this resolution. Vertical resolution must be 320 pixels.
#endif

/*********************************************************************
* Overview: Display orientation.
*********************************************************************/
#if (DISP_ORIENTATION != 0)&&(DISP_ORIENTATION != 90)
#error This driver doesn't support this orientation.
#endif

/*********************************************************************
* Overview: Color depth.
*********************************************************************/
#if (COLOR_DEPTH != 16)
#error This driver doesn't support this color depth. It should be 16.
#endif

/*********************************************************************
* Overview: Clipping region control codes to be used with SetClip(...)
*           function. 
*********************************************************************/
#define CLIP_DISABLE       0 	// Disables clipping.
#define CLIP_ENABLE        1	// Enables clipping.

/*********************************************************************
* Overview: Some basic colors definitions.
*********************************************************************/

#define BLACK               RGB565CONVERT(0,    0,      0)
#define BRIGHTBLUE          RGB565CONVERT(0,    0,      255)
#define BRIGHTGREEN         RGB565CONVERT(0,    255,    0)
#define BRIGHTCYAN          RGB565CONVERT(0,    255,    255)
#define BRIGHTRED           RGB565CONVERT(255,  0,      0)
#define BRIGHTMAGENTA       RGB565CONVERT(255,  0,      255)
#define BRIGHTYELLOW        RGB565CONVERT(255,  255,    0)
#define BLUE                RGB565CONVERT(0,    0,      128)
#define GREEN               RGB565CONVERT(0,    128,    0)
#define CYAN                RGB565CONVERT(0,    128,    128)
#define RED                 RGB565CONVERT(128,  0,      0)
#define MAGENTA             RGB565CONVERT(128,  0,      128)
#define BROWN               RGB565CONVERT(255,  128,    0)
#define LIGHTGRAY           RGB565CONVERT(128,  128,    128)
#define DARKGRAY            RGB565CONVERT(64,   64,     64)
#define LIGHTBLUE           RGB565CONVERT(128,  128,    255)
#define LIGHTGREEN          RGB565CONVERT(128,  255,    128)
#define LIGHTCYAN           RGB565CONVERT(128,  255,    255)
#define LIGHTRED            RGB565CONVERT(255,  128,    128)
#define LIGHTMAGENTA        RGB565CONVERT(255,  128,    255)
#define YELLOW              RGB565CONVERT(255,  255,    128)
#define WHITE               RGB565CONVERT(255,  255,    255)
                            
#define GRAY0       	    RGB565CONVERT(224,  224,    224)
#define GRAY1         	    RGB565CONVERT(192,  192,    192)   
#define GRAY2               RGB565CONVERT(160,  160,    160)   
#define GRAY3               RGB565CONVERT(128,  128,    128)
#define GRAY4               RGB565CONVERT(96,   96,     96)
#define GRAY5               RGB565CONVERT(64,   64,     64)
#define GRAY6	            RGB565CONVERT(32,   32,     32)

// Memory pitch for line
#define LINE_MEM_PITCH      0x100 

// Color
extern WORD _color;

/*********************************************************************
* Overview: Clipping region control and border settings.
*
*********************************************************************/
// Clipping region enable control
extern SHORT _clipRgn;

// Left clipping region border
extern SHORT _clipLeft;
// Top clipping region border
extern SHORT _clipTop;
// Right clipping region border
extern SHORT _clipRight;
// Bottom clipping region border
extern SHORT _clipBottom;

/*********************************************************************
* Macros:  PMPWaitBusy()
*
* Overview: Delays execution for PMP cycle time.
*
* PreCondition: none
*
* Input: none
*
* Output: none
*
* Side Effects: none
*
********************************************************************/
#define PMPWaitBusy()  

/*********************************************************************
* Macros:  SetIndex(index)
*
* Overview: Writes index register.
*
* PreCondition: none
*
* Input: index - 16-bit index register.
*
* Output: none
*
* Side Effects: none
*
********************************************************************/
#define SetIndex(index)   


/*********************************************************************
* Macros:  WriteData(data)
*
* Overview: Writes data.
*
* PreCondition: none
*
* Input: data - data 16-bit word.
*
* Output: none
*
* Side Effects: none
*
********************************************************************/
#define WriteData(data) 

/*********************************************************************
* Macros:  SetAddress(addr)
*
* Overview: Writes address pointer.
*
* PreCondition: none
*
* Input: add0 -  32-bit address.
*
* Output: none
*
* Side Effects: none
*
********************************************************************/

#define SetAddress(addr)   ili9320_SetCursor(((WORD)addr&0x00ff),((WORD)((DWORD)addr>>8)));LCD_WriteRAM_Prepare()

/*********************************************************************
* Function:  void ResetDevice()
*
* Overview: Initializes LCD module.
*
* PreCondition: none
*
* Input: none
*
* Output: none
*
* Side Effects: none
*
********************************************************************/
void ResetDevice(void);

/*********************************************************************
* Macros:  GetMaxX()
*
* Overview: Returns maximum horizontal coordinate.
*
* PreCondition: none
*
* Input: none
*
* Output: Maximum horizontal coordinate.
*
* Example:
*   <CODE> 
*
*	// Create a window that will occupy the whole screen.
*	WndCreate(0xFF,				    	// ID
*             0,0,
*			  GetMaxX(),GetMaxY(),		// dimension
*             WND_DRAW,					// will be dislayed after creation
*             (void*)&mchpIcon,         // use icon used
*             pText,	   				// set to text pointed to by pText
*             NULL);					// use default scheme 
*
*  </CODE>
*
* Side Effects: none
*
********************************************************************/
#if (DISP_ORIENTATION == 90)

#define	GetMaxX() (DISP_VER_RESOLUTION-1)

#elif (DISP_ORIENTATION == 0)

#define	GetMaxX() (DISP_HOR_RESOLUTION-1)

#endif

/*********************************************************************
* Macros:  GetMaxY()
*
* Overview: Returns maximum vertical coordinate.
*
* PreCondition: none
*
* Input: none
*
* Output: Maximum vertical coordinate.
*
* Example: (see GetMaxX()) example.
*
* Side Effects: none
*
********************************************************************/
#if (DISP_ORIENTATION == 90)

#define	GetMaxY() (DISP_HOR_RESOLUTION-1)

#elif (DISP_ORIENTATION == 0)

#define	GetMaxY() (DISP_VER_RESOLUTION-1)

#endif

/*********************************************************************
* Macros:  SetColor(color)
*
* Overview: Sets current drawing color.
*
* PreCondition: none
*
* Input: color - Color coded in 5:6:5 RGB format.
*
* Output: none
*
* Side Effects: none
*
********************************************************************/
#define SetColor(color) _color = color;

/*********************************************************************
* Macros:  GetColor()
*
* Overview: Returns current drawing color.
*
* PreCondition: none
*
* Input: none
*
* Output: Color coded in 5:6:5 RGB format.
*
* Side Effects: none
*
********************************************************************/
#define GetColor() _color

/*********************************************************************
* Macros:  SetActivePage(page)
*
* Overview: Sets active graphic page.
*
* PreCondition: none
*
* Input: page - Graphic page number.
*
* Output: none
*
* Side Effects: none
*
********************************************************************/
#define SetActivePage(page)

/*********************************************************************
* Macros: SetVisualPage(page)
*
* Overview: Sets graphic page to display.
*
* PreCondition: none
*
* Input: page - Graphic page number
*
* Output: none
*
* Side Effects: none
*
********************************************************************/
#define SetVisualPage(page)

/*********************************************************************
* Function: void PutPixel(SHORT x, SHORT y)
*
* Overview: Puts pixel with the given x,y coordinate position.
*
* PreCondition: none
*
* Input: x - x position of the pixel.
*		 y - y position of the pixel.
*
* Output: none
*
* Side Effects: none
*
********************************************************************/
void PutPixel(SHORT x, SHORT y);

/*********************************************************************
* Function: WORD GetPixel(SHORT x, SHORT y)
*
* Overview: Returns pixel color at the given x,y coordinate position.
*
* PreCondition: none
*
* Input: x - x position of the pixel.
*		 y - y position of the pixel.
*
* Output: pixel color
*
* Side Effects: none
*
********************************************************************/
WORD GetPixel(SHORT x, SHORT y);

/*********************************************************************
* Macros: SetClipRgn(left, top, right, bottom)
*
* Overview: Sets clipping region.
*
* PreCondition: none
*
* Input: left - Defines the left clipping region border.
*		 top - Defines the top clipping region border.
*		 right - Defines the right clipping region border.
*	     bottom - Defines the bottom clipping region border.
*
* Output: none
*
* Side Effects: none
*
********************************************************************/
#define SetClipRgn(left,top,right,bottom) _clipLeft=left; _clipTop=top; _clipRight=right; _clipBottom=bottom;

/*********************************************************************
* Macros: GetClipLeft()
*
* Overview: Returns left clipping border.
*
* PreCondition: none
*
* Input: none
*
* Output: Left clipping border.
*
* Side Effects: none
*
********************************************************************/
#define GetClipLeft() _clipLeft

/*********************************************************************
* Macros: GetClipRight()
*
* Overview: Returns right clipping border.
*
* PreCondition: none
*
* Input: none
*
* Output: Right clipping border.
*
* Side Effects: none
*
********************************************************************/
#define GetClipRight() _clipRight

/*********************************************************************
* Macros: GetClipTop()
*
* Overview: Returns top clipping border.
*
* PreCondition: none
*
* Input: none
*
* Output: Top clipping border.
*
* Side Effects: none
*
********************************************************************/
#define GetClipTop() _clipTop

/*********************************************************************
* Macros: GetClipBottom()
*
* Overview: Returns bottom clipping border.
*
* PreCondition: none
*
* Input: none
*
* Output: Bottom clipping border.
*
* Side Effects: none
*
********************************************************************/
#define GetClipBottom() _clipBottom

/*********************************************************************
* Macros: SetClip(control)
*
* Overview: Enables/disables clipping.
*
* PreCondition: none
*
* Input: control - Enables or disables the clipping.
*			- 0: Disable clipping
*			- 1: Enable clipping
*
* Output: none
*
* Side Effects: none
*
********************************************************************/
#define SetClip(control) _clipRgn=control;

/*********************************************************************
* Macros: IsDeviceBusy()
*
* Overview: Returns non-zero if LCD controller is busy 
*           (previous drawing operation is not completed).
*
* PreCondition: none
*
* Input: none
*
* Output: Busy status.
*
* Side Effects: none
*
********************************************************************/
#define IsDeviceBusy()  0

/*********************************************************************
* Macros: SetPalette(colorNum, color)
*
* Overview:  Sets palette register.
*
* PreCondition: none
*
* Input: colorNum - Register number.
*        color - Color.
*
* Output: none
*
* Side Effects: none
*
********************************************************************/
#define SetPalette(colorNum, color)

/*********************************************************************
* Function:  void  DelayMs(WORD time)
*
* Overview: Delays execution on time specified in milliseconds.
*           The delay is correct only for 16MIPS.
*
* PreCondition: none
*
* Input: time - Delay in milliseconds.
*
* Output: none
*
* Side Effects: none
*
********************************************************************/
void DelayMs(WORD time);


/*********************************************************************
* Function:  void  DelayMs(WORD time)
*
* PreCondition: none
*
* Input: time - delay in ms
*
* Output: none
*
* Side Effects: none
*
* Overview: delays execution on time specified in ms
*
* Note: none
*
********************************************************************/
void  DelayMs(WORD time)
{
delay_ms(time);
}
/*********************************************************************
* Function:  void  SetReg(WORD index, WORD value)
*
* PreCondition: none
*
* Input: index - register number
*        value - value to be set
*
* Output: none
*
* Side Effects: none
*
* Overview: sets graphics controller register
*
* Note: none
*
********************************************************************/
void  SetReg(WORD index, WORD value){
//LCD_WriteReg(index,value); // likw
}

/*********************************************************************
* Function:  void ResetDevice()
*
* PreCondition: none
*
* Input: none
*
* Output: none
*
* Side Effects: none
*
* Overview: resets LCD, initializes PMP
*
* Note: none
*
********************************************************************/
void ResetDevice(void){
//ili9320_Initializtion(); // likw
LCD_Init();
}

/*********************************************************************
* Function: void PutPixel(SHORT x, SHORT y)
*
* PreCondition: none
*
* Input: x,y - pixel coordinates
*
* Output: none
*
* Side Effects: none
*
* Overview: puts pixel
*
* Note: none
*
********************************************************************/
void PutPixel(SHORT x, SHORT y){
//ili9320_SetPoint(x,y,_color);// likw
LTDC_Draw_Point(x,y,_color);
}

/*********************************************************************
* Function: WORD GetPixel(SHORT x, SHORT y)
*
* PreCondition: none
*
* Input: x,y - pixel coordinates 
*
* Output: pixel color
*
* Side Effects: none
*
* Overview: returns pixel color at x,y position
*
* Note: none
*
********************************************************************/
WORD GetPixel(SHORT x, SHORT y){
//return ili9320_GetPoint(x,y);// likw
return LTDC_Read_Point(x,y);
}



/*********************************************************************
* Function: WORD Bar(SHORT left, SHORT top, SHORT right, SHORT bottom)
*
* PreCondition: none
*
* Input: left,top - top left corner coordinates,
*        right,bottom - bottom right corner coordinates
*
* Output: For NON-Blocking configuration:
*         - Returns 0 when device is busy and the shape is not yet completely drawn.
*         - Returns 1 when the shape is completely drawn.
*         For Blocking configuration:
*         - Always return 1.
*
* Side Effects: none
*
* Overview: draws rectangle filled with current color
*
* Note: none
*
********************************************************************/
WORD Bar(SHORT left, SHORT top, SHORT right, SHORT bottom){
register SHORT  x,y;

    #ifndef USE_NONBLOCKING_CONFIG
        while(IsDeviceBusy() != 0); /* Ready */
    #else
        if(IsDeviceBusy() != 0) return 0;
    #endif

	if(_clipRgn){
        if(left<_clipLeft)
           left = _clipLeft;
        if(right>_clipRight)
           right= _clipRight;
        if(top<_clipTop)
           top = _clipTop;
        if(bottom>_clipBottom)
           bottom = _clipBottom;
    }
#if (DISP_ORIENTATION == 0)

   
    for(y=top; y<bottom+1; y++)
        for(x=left; x<right+1; x++)
		PutPixel(x,y);
          
#else

	top = GetMaxY() - top;
    bottom = GetMaxY() - bottom;
 
    for(y=bottom; y<top+1; y++)
        for(x=left; x<right+1; x++)
           PutPixel(x,y);

#endif
    return 1;       
}

/*********************************************************************
* Function: void ClearDevice(void)
*
* PreCondition: none
*
* Input: none
*
* Output: none
*
* Side Effects: none
*
* Overview: clears screen with current color 
*
* Note: none
*
********************************************************************/
void ClearDevice(void){

// ili9320_Clear(_color);// likw
LTDC_Clear(_color);
}

/*********************************************************************
* Function: WORD PutImage(SHORT left, SHORT top, void* bitmap, BYTE stretch)
*
* PreCondition: none
*
* Input: left,top - left top image corner,
*        bitmap - image pointer,
*        stretch - image stretch factor
*
* Output: For NON-Blocking configuration:
*         - Returns 0 when device is busy and the image is not yet completely drawn.
*         - Returns 1 when the image is completely drawn.
*         For Blocking configuration:
*         - Always return 1.
*
* Side Effects: none
*
* Overview: outputs image starting from left,top coordinates
*
* Note: image must be located in flash
*
********************************************************************/
#ifdef USE_DRV_PUTIMAGE

WORD PutImage(SHORT left, SHORT top, void* bitmap, BYTE stretch){
FLASH_BYTE* flashAddress;
BYTE colorDepth;
WORD colorTemp;

    #ifndef USE_NONBLOCKING_CONFIG
        while(IsDeviceBusy() != 0); /* Ready */
    #else
        if(IsDeviceBusy() != 0) return 0;
    #endif

#if (DISP_ORIENTATION == 90)
	top = GetMaxY() - top; 
#endif

    // Save current color
    colorTemp = _color;

    switch(*((SHORT*)bitmap))
    {
#ifdef USE_BITMAP_FLASH
        case FLASHv:
            // Image address
            flashAddress = ((BITMAP_FLASH*)bitmap)->address;
            // Read color depth
            colorDepth = *(flashAddress+1);
            // Draw picture
            switch(colorDepth){
                case 1:
                    PutImage1BPP(left, top, flashAddress, stretch);
                    break;
                case 4:
                    PutImage4BPP(left, top, flashAddress, stretch);
                    break;
                case 8:
                    PutImage8BPP(left, top, flashAddress, stretch);
                    break;
                case 16:
                    PutImage16BPP(left, top, flashAddress, stretch);
                    break;
            }
            break;
#endif
#ifdef USE_BITMAP_EXTERNAL
        case EXTERNAL:
            // Get color depth
            ExternalMemoryCallback(bitmap, 1, 1, &colorDepth);
            // Draw picture
            switch(colorDepth){
                case 1:
                    PutImage1BPPExt(left, top, bitmap, stretch);
                    break;
                case 4:
                    PutImage4BPPExt(left, top, bitmap, stretch);
                    break;
                case 8:
                    PutImage8BPPExt(left, top, bitmap, stretch);
                    break;
                case 16:
                    PutImage16BPPExt(left, top, bitmap, stretch);
                    break;
                default:
                    break;
            }
            break;
#endif
        default:
            break;
    }

    // Restore current color
    _color = colorTemp;
    return 1;
}


#ifdef USE_BITMAP_FLASH
/*********************************************************************
* Function: void PutImage1BPP(SHORT left, SHORT top, FLASH_BYTE* bitmap, BYTE stretch)
*
* PreCondition: none
*
* Input: left,top - left top image corner,
*        bitmap - image pointer,
*        stretch - image stretch factor
*
* Output: none
*
* Side Effects: none
*
* Overview: outputs monochrome image starting from left,top coordinates
*
* Note: image must be located in flash
*
********************************************************************/
void PutImage1BPP(SHORT left, SHORT top, FLASH_BYTE* bitmap, BYTE stretch){
register DWORD address;
register FLASH_BYTE* flashAddress;
register FLASH_BYTE* tempFlashAddress;
BYTE temp;
WORD sizeX, sizeY;
WORD x,y;
BYTE stretchX,stretchY;
WORD pallete[2];
BYTE mask;

    // Move pointer to size information
    flashAddress = bitmap + 2;

    // Set start address
#if (DISP_ORIENTATION == 0)
    address = (long)LINE_MEM_PITCH*top + left;
#else
    address = (long)LINE_MEM_PITCH*left+ top;
#endif

    // Read image size
    sizeY = *((FLASH_WORD*)flashAddress);
    flashAddress += 2;
    sizeX = *((FLASH_WORD*)flashAddress);
    flashAddress += 2;
    pallete[0] = *((FLASH_WORD*)flashAddress);
    flashAddress += 2;
    pallete[1] = *((FLASH_WORD*)flashAddress);
    flashAddress += 2;

    for(y=0; y<sizeY; y++){
        tempFlashAddress = flashAddress;
        for(stretchY = 0; stretchY<stretch; stretchY++){
            flashAddress = tempFlashAddress;
            //SetAddress(address);// likw
            mask = 0;
            for(x=0; x<sizeX; x++){

                // Read 8 pixels from flash
                if(mask == 0){
                    temp = *flashAddress;
                    flashAddress++;
                    mask = 0x80;
                }
                
                // Set color
                if(mask&temp){
                    SetColor(pallete[1]);
                }else{
                    SetColor(pallete[0]);
                }

                // Write pixel to screen
                for(stretchX=0; stretchX<stretch; stretchX++){
                    //LCD_WriteRAM(_color);// likw
                }

                // Shift to the next pixel
                mask >>= 1;
           }
#if (DISP_ORIENTATION == 0)
		   address += LINE_MEM_PITCH; 
#else
           address -= 1;
#endif
        }
    }
}

/*********************************************************************
* Function: void PutImage4BPP(SHORT left, SHORT top, FLASH_BYTE* bitmap, BYTE stretch)
*
* PreCondition: none
*
* Input: left,top - left top image corner, bitmap - image pointer,
*        stretch - image stretch factor
*
* Output: none
*
* Side Effects: none
*
* Overview: outputs 16 color image starting from left,top coordinates
*
* Note: image must be located in flash
*
********************************************************************/
void PutImage4BPP(SHORT left, SHORT top, FLASH_BYTE* bitmap, BYTE stretch){
register DWORD address;
register FLASH_BYTE* flashAddress;
register FLASH_BYTE* tempFlashAddress;
WORD sizeX, sizeY;
register WORD x,y;
BYTE temp;
register BYTE stretchX,stretchY;
WORD pallete[16];
WORD counter;

    // Move pointer to size information
    flashAddress = bitmap + 2;

    // Set start address
#if (DISP_ORIENTATION == 0)
    address = (long)LINE_MEM_PITCH*top + left;
#else
    address = (long)LINE_MEM_PITCH*left+ top;
#endif

    // Read image size
    sizeY = *((FLASH_WORD*)flashAddress);
    flashAddress += 2;
    sizeX = *((FLASH_WORD*)flashAddress);
    flashAddress += 2;

    // Read pallete
    for(counter=0;counter<16;counter++){
        pallete[counter] = *((FLASH_WORD*)flashAddress);
        flashAddress += 2;
    }
    for(y=0; y<sizeY; y++){
        tempFlashAddress = flashAddress;
        for(stretchY = 0; stretchY<stretch; stretchY++){
            flashAddress = tempFlashAddress;
            //SetAddress(address);// likw
            for(x=0; x<sizeX; x++){
                // Read 2 pixels from flash
                if(x&0x0001){
                    // second pixel in byte
                    SetColor(pallete[temp>>4]);
                }else{
                    temp = *flashAddress;
                    flashAddress++;
                    // first pixel in byte
                    SetColor(pallete[temp&0x0f]);
                }

                // Write pixel to screen       
                for(stretchX=0; stretchX<stretch; stretchX++){
                   //LCD_WriteRAM(_color);// likw
                }

                // Shift to the next pixel
                //temp >>= 4;
            }
#if (DISP_ORIENTATION == 0)
		   address += LINE_MEM_PITCH; 
#else
           address -= 1;
#endif
        }
    }
}

/*********************************************************************
* Function: void PutImage8BPP(SHORT left, SHORT top, FLASH_BYTE* bitmap, BYTE stretch)
*
* PreCondition: none
*
* Input: left,top - left top image corner, bitmap - image pointer,
*        stretch - image stretch factor
*
* Output: none
*
* Side Effects: none
*
* Overview: outputs 256 color image starting from left,top coordinates
*
* Note: image must be located in flash
*
********************************************************************/
void PutImage8BPP(SHORT left, SHORT top, FLASH_BYTE* bitmap, BYTE stretch){
register DWORD address;
register FLASH_BYTE* flashAddress;
register FLASH_BYTE* tempFlashAddress;
WORD sizeX, sizeY;
WORD x,y;
BYTE temp;
BYTE stretchX, stretchY;
WORD pallete[256];
WORD counter;

    // Move pointer to size information
    flashAddress = bitmap + 2;

    // Set start address
#if (DISP_ORIENTATION == 0)
    address = (long)LINE_MEM_PITCH*top + left;
#else
    address = (long)LINE_MEM_PITCH*left+ top;
#endif

    // Read image size
    sizeY = *((FLASH_WORD*)flashAddress);
    flashAddress += 2;
    sizeX = *((FLASH_WORD*)flashAddress);
    flashAddress += 2;

    // Read pallete
    for(counter=0;counter<256;counter++){
        pallete[counter] = *((FLASH_WORD*)flashAddress);
        flashAddress += 2;
    }

      
    for(y=0; y<sizeY; y++){
        tempFlashAddress = flashAddress;
        for(stretchY = 0; stretchY<stretch; stretchY++){
            flashAddress = tempFlashAddress;
            //SetAddress(address);// likw
            for(x=0; x<sizeX; x++){
                // Read pixels from flash
                temp = *flashAddress;
                flashAddress++;

                // Set color
                SetColor(pallete[temp]);

                // Write pixel to screen       
                for(stretchX=0; stretchX<stretch; stretchX++){
                    //LCD_WriteRAM(_color);// likw
                }
            }
#if (DISP_ORIENTATION == 0)
		   address += LINE_MEM_PITCH; 
#else
           address -= 1;
#endif
        }
    }
}

/*********************************************************************
* Function: void PutImage16BPP(SHORT left, SHORT top, FLASH_BYTE* bitmap, BYTE stretch)
*
* PreCondition: none
*
* Input: left,top - left top image corner, bitmap - image pointer,
*        stretch - image stretch factor
*
* Output: none
*
* Side Effects: none
*
* Overview: outputs hicolor image starting from left,top coordinates
*
* Note: image must be located in flash
*
********************************************************************/
void PutImage16BPP(SHORT left, SHORT top, FLASH_BYTE* bitmap, BYTE stretch){
register DWORD address;
register FLASH_WORD* flashAddress;
register FLASH_WORD* tempFlashAddress;
WORD sizeX, sizeY;
register WORD x,y;
WORD temp;
register BYTE stretchX,stretchY;

    // Move pointer to size information
    flashAddress = (FLASH_WORD*)bitmap + 1;

    // Set start address
#if (DISP_ORIENTATION == 0)
    address = (long)LINE_MEM_PITCH*top + left;
#else
    address = (long)LINE_MEM_PITCH*left+ top;
#endif

    // Read image size
    sizeY = *flashAddress;
    flashAddress++;
    sizeX = *flashAddress;
    flashAddress++;
 
    for(y=0; y<sizeY; y++){
        tempFlashAddress = flashAddress;
        for(stretchY = 0; stretchY<stretch; stretchY++){
            flashAddress = tempFlashAddress;
            //SetAddress(address);// likw
            for(x=0; x<sizeX; x++){
                // Read pixels from flash
                temp = *flashAddress;
                flashAddress++;

                // Set color
                SetColor(temp);

                // Write pixel to screen       
                for(stretchX=0; stretchX<stretch; stretchX++){
                    //LCD_WriteRAM(_color);// likw
                }
            }
#if (DISP_ORIENTATION == 0)
		   address += LINE_MEM_PITCH; 
#else
           address -= 1;
#endif
        }
    }
}
#endif

#ifdef USE_BITMAP_EXTERNAL
/*********************************************************************
* Function: void PutImage1BPPExt(SHORT left, SHORT top, void* bitmap, BYTE stretch)
*
* PreCondition: none
*
* Input: left,top - left top image corner, bitmap - image pointer,
*        stretch - image stretch factor
*
* Output: none
*
* Side Effects: none
*
* Overview: outputs monochrome image starting from left,top coordinates
*
* Note: image must be located in external memory
*
********************************************************************/
void PutImage1BPPExt(SHORT left, SHORT top, void* bitmap, BYTE stretch){
register DWORD  address;
register DWORD      memOffset;
BITMAP_HEADER       bmp;
WORD                pallete[2];
BYTE                lineBuffer[((GetMaxX()+1)/8)+1];
BYTE*               pData; 
SHORT               byteWidth;

BYTE                temp;
BYTE                mask;
WORD                sizeX, sizeY;
WORD                x,y;
BYTE                stretchX, stretchY;

    // Set start address
#if (DISP_ORIENTATION == 0)
    address = (long)LINE_MEM_PITCH*top + left;
#else
    address = (long)LINE_MEM_PITCH*left+ top;
#endif

    // Get bitmap header
    ExternalMemoryCallback(bitmap, 0, sizeof(BITMAP_HEADER), &bmp);

    // Get pallete (2 entries)
    ExternalMemoryCallback(bitmap, sizeof(BITMAP_HEADER), 2*sizeof(WORD), pallete);

    // Set offset to the image data
    memOffset = sizeof(BITMAP_HEADER) + 2*sizeof(WORD);

    // Line width in bytes
    byteWidth = bmp.width>>3;
    if(bmp.width&0x0007)
        byteWidth++;

    // Get size
    sizeX = bmp.width;
    sizeY = bmp.height;

    for(y=0; y<sizeY; y++){

        // Get line
        ExternalMemoryCallback(bitmap, memOffset, byteWidth, lineBuffer);
        memOffset += byteWidth;
  
        for(stretchY = 0; stretchY<stretch; stretchY++){
            pData = lineBuffer;
            SetAddress(address);
            mask = 0;
            for(x=0; x<sizeX; x++){

                // Read 8 pixels from flash
                if(mask == 0){
                    temp = *pData++;
                    mask = 0x80;
                }
                
                // Set color
                if(mask&temp){
                    SetColor(pallete[1]);
                }else{
                    SetColor(pallete[0]);
                }

                // Write pixel to screen
                for(stretchX=0; stretchX<stretch; stretchX++){
                    LCD_WriteRAM(_color);
                }

                // Shift to the next pixel
                mask >>= 1;
           }
#if (DISP_ORIENTATION == 0)
		   address += LINE_MEM_PITCH; 
#else
           address -= 1;
#endif
        }
    }
}

/*********************************************************************
* Function: void PutImage4BPPExt(SHORT left, SHORT top, void* bitmap, BYTE stretch)
*
* PreCondition: none
*
* Input: left,top - left top image corner, bitmap - image pointer,
*        stretch - image stretch factor
*
* Output: none
*
* Side Effects: none
*
* Overview: outputs monochrome image starting from left,top coordinates
*
* Note: image must be located in external memory
*
********************************************************************/
void PutImage4BPPExt(SHORT left, SHORT top, void* bitmap, BYTE stretch){
register DWORD  address;
register DWORD      memOffset;
BITMAP_HEADER       bmp;
WORD                pallete[16];
BYTE                lineBuffer[((GetMaxX()+1)/2)+1];
BYTE*               pData; 
SHORT               byteWidth;

BYTE                temp;
WORD                sizeX, sizeY;
WORD                x,y;
BYTE                stretchX, stretchY;

    // Set start address
#if (DISP_ORIENTATION == 0)
    address = (long)LINE_MEM_PITCH*top + left;
#else
    address = (long)LINE_MEM_PITCH*left+ top;
#endif

    // Get bitmap header
    ExternalMemoryCallback(bitmap, 0, sizeof(BITMAP_HEADER), &bmp);

    // Get pallete (16 entries)
    ExternalMemoryCallback(bitmap, sizeof(BITMAP_HEADER), 16*sizeof(WORD), pallete);

    // Set offset to the image data
    memOffset = sizeof(BITMAP_HEADER) + 16*sizeof(WORD);

    // Line width in bytes
    byteWidth = bmp.width>>1;
    if(bmp.width&0x0001)
        byteWidth++;

    // Get size
    sizeX = bmp.width;
    sizeY = bmp.height;

    for(y=0; y<sizeY; y++){

        // Get line
        ExternalMemoryCallback(bitmap, memOffset, byteWidth, lineBuffer);
        memOffset += byteWidth;
 
        for(stretchY = 0; stretchY<stretch; stretchY++){

            pData = lineBuffer;
            SetAddress(address);

            for(x=0; x<sizeX; x++){

                // Read 2 pixels from flash
                if(x&0x0001){
                    // second pixel in byte
                    SetColor(pallete[temp>>4]);
                }else{
                    temp = *pData++;
                    // first pixel in byte
                    SetColor(pallete[temp&0x0f]);
                }

                // Write pixel to screen       
                for(stretchX=0; stretchX<stretch; stretchX++){
                   LCD_WriteRAM(_color);
                }
           }
#if (DISP_ORIENTATION == 0)
		   address += LINE_MEM_PITCH; 
#else
           address -= 1;
#endif
        }
    }
}

/*********************************************************************
* Function: void PutImage8BPPExt(SHORT left, SHORT top, void* bitmap, BYTE stretch)
*
* PreCondition: none
*
* Input: left,top - left top image corner, bitmap - image pointer,
*        stretch - image stretch factor
*
* Output: none
*
* Side Effects: none
*
* Overview: outputs monochrome image starting from left,top coordinates
*
* Note: image must be located in external memory
*
********************************************************************/
void PutImage8BPPExt(SHORT left, SHORT top, void* bitmap, BYTE stretch){
register DWORD  address;
register DWORD      memOffset;
BITMAP_HEADER       bmp;
WORD                pallete[256];
BYTE                lineBuffer[(GetMaxX()+1)];
BYTE*               pData; 

BYTE                temp;
WORD                sizeX, sizeY;
WORD                x,y;
BYTE                stretchX, stretchY;

    // Set start address
#if (DISP_ORIENTATION == 0)
    address = (long)LINE_MEM_PITCH*top + left;
#else
    address = (long)LINE_MEM_PITCH*left+ top;
#endif

    // Get bitmap header
    ExternalMemoryCallback(bitmap, 0, sizeof(BITMAP_HEADER), &bmp);

    // Get pallete (256 entries)
    ExternalMemoryCallback(bitmap, sizeof(BITMAP_HEADER), 256*sizeof(WORD), pallete);

    // Set offset to the image data
    memOffset = sizeof(BITMAP_HEADER) + 256*sizeof(WORD);

    // Get size
    sizeX = bmp.width;
    sizeY = bmp.height;

    for(y=0; y<sizeY; y++){

        // Get line
        ExternalMemoryCallback(bitmap, memOffset, sizeX, lineBuffer);
        memOffset += sizeX;
        for(stretchY = 0; stretchY<stretch; stretchY++){

            pData = lineBuffer;
            SetAddress(address);

            for(x=0; x<sizeX; x++){

                temp = *pData++;
                SetColor(pallete[temp]);                    

                // Write pixel to screen       
                for(stretchX=0; stretchX<stretch; stretchX++){
                    LCD_WriteRAM(_color);
                }
           }
#if (DISP_ORIENTATION == 0)
		   address += LINE_MEM_PITCH; 
#else
           address -= 1;
#endif
        }
    }
}

/*********************************************************************
* Function: void PutImage16BPPExt(SHORT left, SHORT top, void* bitmap, BYTE stretch)
*
* PreCondition: none
*
* Input: left,top - left top image corner, bitmap - image pointer,
*        stretch - image stretch factor
*
* Output: none
*
* Side Effects: none
*
* Overview: outputs monochrome image starting from left,top coordinates
*
* Note: image must be located in external memory
*
********************************************************************/
void PutImage16BPPExt(SHORT left, SHORT top, void* bitmap, BYTE stretch){
register DWORD  address;
register DWORD      memOffset;
BITMAP_HEADER       bmp;
WORD                lineBuffer[(GetMaxX()+1)];
WORD*               pData; 
WORD                byteWidth;

WORD                temp;
WORD                sizeX, sizeY;
WORD                x,y;
BYTE                stretchX, stretchY;

    // Set start address
#if (DISP_ORIENTATION == 0)
    address = (long)LINE_MEM_PITCH*top + left;
#else
    address = (long)LINE_MEM_PITCH*left+ top;
#endif

    // Get bitmap header
    ExternalMemoryCallback(bitmap, 0, sizeof(BITMAP_HEADER), &bmp);

    // Set offset to the image data
    memOffset = sizeof(BITMAP_HEADER);

    // Get size
    sizeX = bmp.width;
    sizeY = bmp.height;

    byteWidth = sizeX<<1;

    for(y=0; y<sizeY; y++){

        // Get line
        ExternalMemoryCallback(bitmap, memOffset, byteWidth, lineBuffer);
        memOffset += byteWidth;
        for(stretchY = 0; stretchY<stretch; stretchY++){

            pData = lineBuffer;
            SetAddress(address);

            for(x=0; x<sizeX; x++){

                temp = *pData++;
                SetColor(temp);                    

                // Write pixel to screen       
                for(stretchX=0; stretchX<stretch; stretchX++){
                    LCD_WriteRAM(_color);
                }

           }
#if (DISP_ORIENTATION == 0)
		   address += LINE_MEM_PITCH; 
#else
           address -= 1;
#endif
        }
    }
}
#endif // USE_DRV_PUTIMAGE

#endif
