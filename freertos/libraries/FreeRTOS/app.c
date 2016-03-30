/*
    FreeRTOS V7.1.1 - Copyright (C) 2012 Real Time Engineers Ltd.
	

    ***************************************************************************
     *                                                                       *
     *    FreeRTOS tutorial books are available in pdf and paperback.        *
     *    Complete, revised, and edited pdf reference manuals are also       *
     *    available.                                                         *
     *                                                                       *
     *    Purchasing FreeRTOS documentation will not only help you, by       *
     *    ensuring you get running as quickly as possible and with an        *
     *    in-depth knowledge of how to use FreeRTOS, it will also help       *
     *    the FreeRTOS project to continue with its mission of providing     *
     *    professional grade, cross platform, de facto standard solutions    *
     *    for microcontrollers - completely free of charge!                  *
     *                                                                       *
     *    >>> See http://www.FreeRTOS.org/Documentation for details. <<<     *
     *                                                                       *
     *    Thank you for using FreeRTOS, and thank you for your support!      *
     *                                                                       *
    ***************************************************************************


    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation AND MODIFIED BY the FreeRTOS exception.
    >>>NOTE<<< The modification to the GPL is included to allow you to
    distribute a combined work that includes FreeRTOS without being obliged to
    provide the source code for proprietary components outside of the FreeRTOS
    kernel.  FreeRTOS is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
    or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
    more details. You should have received a copy of the GNU General Public
    License and the FreeRTOS license exception along with FreeRTOS; if not it
    can be viewed here: http://www.freertos.org/a00114.html and also obtained
    by writing to Richard Barry, contact details for whom are available on the
    FreeRTOS WEB site.

    1 tab == 4 spaces!

    ***************************************************************************
     *                                                                       *
     *    Having a problem?  Start by reading the FAQ "My application does   *
     *    not run, what could be wrong?                                      *
     *                                                                       *
     *    http://www.FreeRTOS.org/FAQHelp.html                               *
     *                                                                       *
    ***************************************************************************


    http://www.FreeRTOS.org - Documentation, training, latest information,
    license and contact details.

    http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
    including FreeRTOS+Trace - an indispensable productivity tool.

    Real Time Engineers ltd license FreeRTOS to High Integrity Systems, who sell
    the code with commercial support, indemnification, and middleware, under
    the OpenRTOS brand: http://www.OpenRTOS.com.  High Integrity Systems also
    provide a safety engineered and independently SIL3 certified version under
    the SafeRTOS brand: http://www.SafeRTOS.com.
*/

#define USE_STDPERIPH_DRIVER
#include "stm32f10x.h"

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "list.h"
#include "queue.h"
#include "semphr.h"
#include "app.h"

xSemaphoreHandle xBinarySemaphore[NUMBEROFSERVANT];  // the semaphores which are used to trigger new servant to execute
xTaskHandle xTaskOfHandle[NUMBEROFSERVANT+1];         // record the handle of all S-Servant, the last one is for debugging R-Servant 

// the LET of all S-Servant
portTickType xLetOfServant[NUMBEROFSERVANT] = { 100, 100, 100, 100, 100, 100 };
// record the relationship among servants excluding R-Servant
portBASE_TYPE xRelation[NUMBEROFSERVANT][NUMBEROFSERVANT] = { 0, 1, 0, 0, 0, 0,
                                                              0, 0, 1, 1, 0, 0,
                                                              0, 0, 0, 0, 1, 0,
                                                              0, 0, 0, 0, 0, 1,
                                                              0, 0, 0, 0, 0, 1,
                                                              0, 0, 0, 0, 0, 0};


/* the function */
void s_0(xEventHandle * pxEventArray, portBASE_TYPE NumOfEvent, struct eventData * pxDataArray, portBASE_TYPE NumOfData)
{
    vPrintString("S-0\n\r");
}

void s_1(xEventHandle * pxEventArray, portBASE_TYPE NumOfEvent, struct eventData * pxDataArray, portBASE_TYPE NumOfData)
{
    vPrintString("S-1\n\r");
}

void s_2(xEventHandle * pxEventArray, portBASE_TYPE NumOfEvent, struct eventData * pxDataArray, portBASE_TYPE NumOfData)
{
    vPrintString("S-2\n\r");
}

void s_3(xEventHandle * pxEventArray, portBASE_TYPE NumOfEvent, struct eventData * pxDataArray, portBASE_TYPE NumOfData)
{
    vPrintString("S-3\n\r");
}

void s_4(xEventHandle * pxEventArray, portBASE_TYPE NumOfEvent, struct eventData * pxDataArray, portBASE_TYPE NumOfData)
{
    vPrintString("S-4\n\r");
}

void s_5(xEventHandle * pxEventArray, portBASE_TYPE NumOfEvent, struct eventData * pxDataArray, portBASE_TYPE NumOfData)
{
    vPrintString("S-5\n\r");
}

// assigned the point of function into specified position of xServantTable.
pvServantFunType xServantTable[NUMBEROFSERVANT] = {&s_0, &s_1, &s_2, &s_3, &s_4, &s_5};

