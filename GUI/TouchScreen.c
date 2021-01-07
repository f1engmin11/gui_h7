/*****************************************************************************
 *
 * Simple 4 wire touch screen driver
 *
 *****************************************************************************
 * FileName:        TouchScreen.c
 * Dependencies:    Beep.h, TouchScreen.h
 * Processor:       PIC24, PIC32, dsPIC, PIC24H
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
 * SOFTWARE AND DOCUMENTATION ARE PROVIDED 揂S IS?WITHOUT WARRANTY OF ANY
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
 * Anton Alkhimenok		01/08/07	...
 * Anton Alkhimenok		06/06/07	Basic calibration and GOL messaging are added
 * Anton Alkhimenok     02/05/08    new PICtail support, portrait orientation is added
 * Sean Justice         02/07/08    PIC32 support
 * Anton Alkhimenok     05/27/08    More robust algorithm
 * Anton Alkhimenok     01/07/09    Graphics PICtail Version 3 support is added
 * Jayanth Murthy       06/25/09    dsPIC & PIC24H support 
 * PAT					06/29/09	Added event EVENT_STILLPRESS to support continuous press
 *****************************************************************************/
#include "sys.h"
//#include "ads7843drv.h" // touch IC
#include "touch.h" 
#include "TouchScreen.h"

//////////////////////// LOCAL PROTOTYPES ////////////////////////////
void TouchGetCalPoints(void);
//extern  matrix mat ;// likw
//extern  coordinate  display ;

/*********************************************************************
* Function: Timer3 ISR
*
* PreCondition: none
*
* Input: none
*
* Output: none
*
* Side Effects: none
*
* Overview: Timer3 ISR
*
* Note: none
*
********************************************************************/


/*********************************************************************
* Function: void TouchInit(void)
*
* PreCondition: none
*
* Input: none
*
* Output: none
*
* Side Effects: none
*
* Overview: sets ADC 
*
* Note: none
*
********************************************************************/
void TouchInit(void){
TP_Init();// likw
//setCalibrationMatrix(&DisplaySample[0],&ScreenSample[0],&mat ) ;// likw
}

/*********************************************************************
* Function: SHORT TouchGetX()
*
* PreCondition: none
*
* Input: none
*
* Output: x coordinate
*
* Side Effects: none
*
* Overview: returns x coordinate if touch screen is pressed
*           and -1 if not
*
* Note: none
*
********************************************************************/


/*********************************************************************
* Function: SHORT TouchGetY()
*
* PreCondition: none
*
* Input: none
*
* Output: y coordinate
*
* Side Effects: none
*
* Overview: returns y coordinate if touch screen is pressed
*           and -1 if not
*
* Note: none
*
********************************************************************/

/*********************************************************************
* Function: void TouchGetMsg(GOL_MSG* pMsg)
*
* PreCondition: none
*
* Input: pointer to the message structure to be populated
*
* Output: none
*
* Side Effects: none
*
* Overview: populates GOL message structure
*
* Note: none
*
********************************************************************/
void TouchGetMsg(GOL_MSG* pMsg){
static SHORT prevX = -1;
static SHORT prevY = -1;

SHORT x,y;
unsigned char result;
//1 代表不成功
    result = GT9271_Scan();
    x = tp_dev.x[0];  y = tp_dev.y[0];
    
    pMsg->type    = TYPE_TOUCHSCREEN;
    pMsg->uiEvent = EVENT_INVALID;

    if( result == 1 ){// 无触摸按下
        x = -1;
		y = -1;
		printf("result == 1\r\n");
    }
	else
	{
		printf("x:%d,y:%d\r\n",x,y);
	}

    if( (prevX == x) && (prevY == y) && (result != 1) ) {
        pMsg->uiEvent = EVENT_STILLPRESS;/////////3
        pMsg->param1 = x;
        pMsg->param2 = y;
        return;
    }    

    if( (prevX != -1) || (prevY != -1) ){

        if( (x != -1) && (y != -1) ){
            // Move
            pMsg->uiEvent = EVENT_MOVE;
        }else{
            // Released
            pMsg->uiEvent = EVENT_RELEASE;/////////4
            pMsg->param1 = prevX;
            pMsg->param2 = prevY;
            prevX = x;
            prevY = y;
            return;
        }

    }else{

        if( (x != -1) && (y != -1) ){
            // Pressed
            pMsg->uiEvent = EVENT_PRESS;/////////2
        }else{
            // No message
            pMsg->uiEvent = EVENT_INVALID;
        }

    }

    pMsg->param1 = x;
    pMsg->param2 = y;
    prevX = x;
    prevY = y;
}

/*********************************************************************
* Function: void TouchStoreCalibration(void)
*
* PreCondition: EEPROMInit() must be called before
*
* Input: none
*
* Output: none
*
* Side Effects: none
*
* Overview: stores calibration parameters into EEPROM
*
* Note: none
*
********************************************************************/

/*********************************************************************
* Function: void TouchLoadCalibration(void)
*
* PreCondition: EEPROMInit() must be called before
*
* Input: none
*
* Output: none
*
* Side Effects: none
*
* Overview: loads calibration parameters from EEPROM
*
* Note: none
*
********************************************************************/


/*********************************************************************
* Function:  void TouchCalibration()
*
* PreCondition: InitGraph() must be called before
*
* Input: none
*
* Output: none
*
* Side Effects: none
*
* Overview: calibrates touch screen
*
* Note: none
*
********************************************************************/


/*********************************************************************
* Function: void TouchGetCalPoints(void)
*
* PreCondition: InitGraph() must be called before
*
* Input: none
*
* Output: none
*
* Side Effects: none
*
* Overview: gets values for 3 touches
*
* Note: none
*
********************************************************************/
