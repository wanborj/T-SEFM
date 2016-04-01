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

// the LET of all S-Servant (ms)
portTickType xLetOfServant[NUMBEROFSERVANT] = 
{ 
    20,    // s_0
    20,    // s_1
    20,    // s_2
    20,    // s_3 
    20,    // s_4 
    20,    // s_5
    20,    // s_6
    20,    // s_7
    20,    // s_8
    20,    // s_9
    20,    // s_10
    20,    // s_11
    20,    // s_12
    20,    // s_13
};
// record the relationship among servants excluding R-Servant
/*
struct sparseRelation
{
    portBASE_TYPE xInFlag;  // source servant
    portBASE_TYPE xOutFlag; // destination servant
    portBASE_TYPE xFlag;  // 1 means relation exist but without event, 2 means that there is a event
};
struct xRelationship
{
    portBASE_TYPE xNumOfRelation;   // the real number of relations
    struct sparseRelation xRelation[MAXRELATION];  // the number of effective relations among servant
};
*/
struct xRelationship xRelations = 
{
    10,
    {
        {0, 1, 1},
        {1, 5, 1},
        {2, 4, 1},
        {4, 3, 1},
        {6, 7, 1},
        {7, 2, 1},
        {8, 9, 1},
        {9,10, 1},
        {10,11,1},
        {12,13,1}
    }
};

// T1, test_ppm_task, 
void s_0(xEventHandle * pxEventArray, portBASE_TYPE NumOfEvent, struct eventData * pxDataArray, portBASE_TYPE NumOfData) 
{
    vPrintString("s_0\n\r");
    test_ppm_task(); // link_autopilot.h
}

// T2, send_data_to_autopilot_task
void s_1(xEventHandle * pxEventArray, portBASE_TYPE NumOfEvent, struct eventData * pxDataArray, portBASE_TYPE NumOfData) 
{

    vPrintString("s_1\n\r");
    send_data_to_autopilot_task(); // link_autopilot.h
}

// T3, check_mega128_values_task
void s_2(xEventHandle * pxEventArray, portBASE_TYPE NumOfEvent, struct eventData * pxDataArray, portBASE_TYPE NumOfData) 
{
    
    vPrintString("s_2\n\r");
    check_mega128_values_task(); // link_autopilot.h
}

// T4, servo_transmit
void s_3(xEventHandle * pxEventArray, portBASE_TYPE NumOfEvent, struct eventData * pxDataArray, portBASE_TYPE NumOfData) 
{

    vPrintString("s_3\n\r");
    servo_transmit();  // servo.h
}

// T5, check_failsafe_task
void s_4(xEventHandle * pxEventArray, portBASE_TYPE NumOfEvent, struct eventData * pxDataArray, portBASE_TYPE NumOfData)
{
    
    vPrintString("s_4\n\r");
    check_failsafe_task(); // link_autopilot.h
}

// T6, radio_control_task
void s_5(xEventHandle * pxEventArray, portBASE_TYPE NumOfEvent, struct eventData * pxDataArray, portBASE_TYPE NumOfData) 
{

    vPrintString("s_5\n\r");
    if( link_fbw_receive_complete ) // link_fbw.h
    {
        link_fbw_receive_complete = FALSE;
        radio_control_task(); //autopilot.h
    }
}


// T7, stablization_task
void s_6(xEventHandle * pxEventArray, portBASE_TYPE NumOfEvent, struct eventData * pxDataArray, portBASE_TYPE NumOfData) 
{
    
    vPrintString("s_6\n\r");
    stabilisation_task(); //autopilot.h
}

// T8, link_fbw_send
void s_7(xEventHandle * pxEventArray, portBASE_TYPE NumOfEvent, struct eventData * pxDataArray, portBASE_TYPE NumOfData) 
{

    vPrintString("s_7\n\r");
    link_fbw_send(); // link_fbw.h
}

// T9, receive_gps_data_task
void s_8(xEventHandle * pxEventArray, portBASE_TYPE NumOfEvent, struct eventData * pxDataArray, portBASE_TYPE NumOfData) 
{
    
    vPrintString("s_8\n\r");
    if ( gps_msg_received )  // gps.h
    {
        parse_gps_msg(); // gps.h
        send_gps_pos(); // autopilot.h
        send_radIR(); // autopilot.h
        send_takeOff(); // autopilot.h
    }

}

// T10, navigation_task
void s_9(xEventHandle * pxEventArray, portBASE_TYPE NumOfEvent, struct eventData * pxDataArray, portBASE_TYPE NumOfData) 
{

    vPrintString("s_9\n\r");
    // have to create a file whose name is main.h
    navigation_update(); // main.c 
    send_nav_values(); // main.c
    course_run();  // main.c
}

// T11, altitude_control_task
void s_10(xEventHandle * pxEventArray, portBASE_TYPE NumOfEvent, struct eventData * pxDataArray, portBASE_TYPE NumOfData) 
{
    
    vPrintString("s_10\n\r");
    altitude_control_task(); // main.c

}

// T12, climb_control_task
void s_11(xEventHandle * pxEventArray, portBASE_TYPE NumOfEvent, struct eventData * pxDataArray, portBASE_TYPE NumOfData)
{

    vPrintString("s_11\n\r");
    climb_control_task(); // main.c
}

// T13, reporting_task
void s_12(xEventHandle * pxEventArray, portBASE_TYPE NumOfEvent, struct eventData * pxDataArray, portBASE_TYPE NumOfData) 
{
    
    vPrintString("s_12\n\r");
 // main.c
    send_boot();
    send_attitude();
    send_adc();
    send_settings();
    send_desired();
    send_bat();
    send_climb();
    send_mode();
    send_debug();
    send_nav_ref();
}

// T14, NULL, do as the actuator of task 4
void s_13(xEventHandle * pxEventArray, portBASE_TYPE NumOfEvent, struct eventData * pxDataArray, portBASE_TYPE NumOfData) 
{
    
    vPrintString("s_13\n\r");
}

// assigned the point of function into specified position of xServantTable.
pvServantFunType xServantTable[NUMBEROFSERVANT] = 
{
    &s_0, 
    &s_1,
    &s_2,
    &s_3,
    &s_4,
    &s_5,
    &s_6,
    &s_7,
    &s_8,
    &s_9,
    &s_10, 
    &s_11,
    &s_12,
    &s_13
};
