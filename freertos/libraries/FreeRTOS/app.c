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
/* the include file of PapaBench */
#include "link_autopilot.h"
#include "servo.h"
#include "link_fbw.h"
#include "gps.h"
#include "autopilot.h"

xSemaphoreHandle xBinarySemaphore[NUMBEROFSERVANT];  // the semaphores which are used to trigger new servant to execute
xTaskHandle xTaskOfHandle[NUMBEROFSERVANT];         // record the handle of all S-Servant, the last one is for debugging R-Servant 
//portBASE_TYPE xTaskComplete[NUMBEROFTASK];  // record whether specified task completes execution
portTickType xPeriodOfTask[NUMBEROFTASK] =
{
    100,
    200,
    400,
    1000
};

// the LET of all S-Servant (ms)
portTickType xLetOfServant[NUMBEROFSERVANT] = 
{ 
    3,    // sensor_0
    14,    // s_1
    4,    // s_2
    7,    // s_3 
    2,    // actuator_4 
    1,    // Sensor_5
    9,    // s_6
    2,    // s_7
    2,    // s_8
    6,    // s_9
    6,    // s_10
    2,    // Actuator_11
    1,    // Sensor_12
    4,    // s_13
    2,    // Actuator_14
    1,    // Sensor_15
    2,    // s_16
    4,    // s_17
    2,    // s_18
    2,    // s_19
    1,    // actuator_20
    1    // R-Servant 
};

portBASE_TYPE xTaskOfServant[NUMBEROFSERVANT] =
{
    0, // task 0 consist of 5 servant
    0,
    0,
    0,
    0,
    1, // task 1 consist of 7 servant 
    1,
    1,
    1,
    1,
    1,
    1,
    2, // task 2 consist of 3 servant
    2,
    2,
    3, // task 3 consist of 5 servant
    3,
    3,
    3,
    3,
    3,
    0, 
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
    17,
    {
        {0, 1, 1}, // task 1
        {1, 2, 1},
        {2, 3, 1},
        {3, 4, 1},
        {5, 6, 1}, // task 2
        {6, 7, 1},
        {7, 8, 1},
        {8, 9, 1},
        {9,10, 1},
        {10,11,1}, 
        {12,13,1}, // task 3
        {13,14,1},
        {15,16,1}, // task 4
        {16,17,1},
        {17,18,1},
        {18,19,1},
        {19,20,1}
    }
};

/* the extern function of PapaBench */
extern void  navigation_update();
extern void  send_nav_values();
extern void  course_run();
extern void  altitude_control_task();
extern void  climb_control_task();
       
extern void  send_boot();
extern void  send_attitude();
extern void  send_adc();
extern void  send_settings();
extern void  send_desired();
extern void  send_bat();
extern void  send_climb();
extern void  send_mode();
extern void  send_debug();
extern void  send_nav_ref();

#define SUNNYBEIKE 1
#ifdef SUNNYBEIKE
/*
 * Task 1, including:
 * 1) sensor 0
 * 2) s_1
 * 3) s_2
 * 4) s_3
 * 5) actuator 4
 * */
void s_0(xEventHandle * pxEventArray, portBASE_TYPE NumOfEvent, struct eventData * pxDataArray, portBASE_TYPE NumOfData) 
{
    //vPrintString("Task 1 start ###############\n\r");
}
void s_1(xEventHandle * pxEventArray, portBASE_TYPE NumOfEvent, struct eventData * pxDataArray, portBASE_TYPE NumOfData) 
{
    //vPrintString("s_0\n\r");
    test_ppm_task(); // link_autopilot.h
}
void s_2(xEventHandle * pxEventArray, portBASE_TYPE NumOfEvent, struct eventData * pxDataArray, portBASE_TYPE NumOfData) 
{

    //vPrintString("s_1\n\r");
    send_data_to_autopilot_task(); // link_autopilot.h
}

void s_3(xEventHandle * pxEventArray, portBASE_TYPE NumOfEvent, struct eventData * pxDataArray, portBASE_TYPE NumOfData) 
{

    //vPrintString("s_3\n\r");
    //if( link_fbw_receive_complete ) // link_fbw.h
    {
        //vPrintString("hello,world s_3 \n\r");
        //link_fbw_receive_complete = FALSE;
        radio_control_task(); //autopilot.h
    }
}
void s_4(xEventHandle * pxEventArray, portBASE_TYPE NumOfEvent, struct eventData * pxDataArray, portBASE_TYPE NumOfData) 
{
    //vPrintString("Task 1 end ###############\n\r");
}

/*
 * Task 2, including:
 * 1) sensor 5
 * 2) s_6
 * 3) s_7
 * 4) s_8
 * 5) s_9
 * 6) s_10
 * 7) actuator 11
 * */
void s_5(xEventHandle * pxEventArray, portBASE_TYPE NumOfEvent, struct eventData * pxDataArray, portBASE_TYPE NumOfData) 
{
    //vPrintString("Task 2 start+++++++++++++++++++\n\r");
}
void s_6(xEventHandle * pxEventArray, portBASE_TYPE NumOfEvent, struct eventData * pxDataArray, portBASE_TYPE NumOfData) 
{
    //vPrintString("s_2\n\r");
    stabilisation_task(); //autopilot.h
}
void s_7(xEventHandle * pxEventArray, portBASE_TYPE NumOfEvent, struct eventData * pxDataArray, portBASE_TYPE NumOfData) 
{

    //vPrintString("s_7\n\r");
    link_fbw_send(); // link_fbw.h
}
void s_8(xEventHandle * pxEventArray, portBASE_TYPE NumOfEvent, struct eventData * pxDataArray, portBASE_TYPE NumOfData) 
{
    check_mega128_values_task(); // link_autopilot.h
}
void s_9(xEventHandle * pxEventArray, portBASE_TYPE NumOfEvent, struct eventData * pxDataArray, portBASE_TYPE NumOfData)
{
    
    //vPrintString("s_4\n\r");
    check_failsafe_task(); // link_autopilot.h
}
void s_10(xEventHandle * pxEventArray, portBASE_TYPE NumOfEvent, struct eventData * pxDataArray, portBASE_TYPE NumOfData) 
{

    //vPrintString("s_3\n\r");
    servo_transmit();  // servo.h
}
void s_11(xEventHandle * pxEventArray, portBASE_TYPE NumOfEvent, struct eventData * pxDataArray, portBASE_TYPE NumOfData) 
{
    //vPrintString("Task 2 end +++++++++++++++++++\n\r");
}


/*
 * Task 3, including:
 * 1) sensor 12
 * 2) s_13
 * 3) actuator 14
 * */
void s_12(xEventHandle * pxEventArray, portBASE_TYPE NumOfEvent, struct eventData * pxDataArray, portBASE_TYPE NumOfData) 
{
    //vPrintString("Task 3 start---------------------\n\r");
}
void s_13(xEventHandle * pxEventArray, portBASE_TYPE NumOfEvent, struct eventData * pxDataArray, portBASE_TYPE NumOfData) 
{
    
    //vPrintString("s_12\n\r");
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
void s_14(xEventHandle * pxEventArray, portBASE_TYPE NumOfEvent, struct eventData * pxDataArray, portBASE_TYPE NumOfData) 
{
    //vPrintString("Task 3 end ----------------------\n\r");
}


/*
 * Task 4, including:
 * 1) sensor 15
 * 2) s_16
 * 3) s_17
 * 4) s_18
 * 5) s_19
 * 6) actuator 20
 * */
void s_15(xEventHandle * pxEventArray, portBASE_TYPE NumOfEvent, struct eventData * pxDataArray, portBASE_TYPE NumOfData) 
{
    //vPrintString("Task 4 start********************\n\r");
}
void s_16(xEventHandle * pxEventArray, portBASE_TYPE NumOfEvent, struct eventData * pxDataArray, portBASE_TYPE NumOfData) 
{
    
    //vPrintString("s_8\n\r");
    //if ( gps_msg_received )  // gps.h
    {
        parse_gps_msg(); // gps.h
        send_gps_pos(); // autopilot.h
        send_radIR(); // autopilot.h
        send_takeOff(); // autopilot.h
    }
}
void s_17(xEventHandle * pxEventArray, portBASE_TYPE NumOfEvent, struct eventData * pxDataArray, portBASE_TYPE NumOfData) 
{
    //vPrintString("s_9\n\r");
    // have to create a file whose name is main.h
    navigation_update(); // main.c 
    send_nav_values(); // main.c
    course_run();  // main.c
}
void s_18(xEventHandle * pxEventArray, portBASE_TYPE NumOfEvent, struct eventData * pxDataArray, portBASE_TYPE NumOfData) 
{
    //vPrintString("s_10\n\r");
    altitude_control_task(); // main.c
}
void s_19(xEventHandle * pxEventArray, portBASE_TYPE NumOfEvent, struct eventData * pxDataArray, portBASE_TYPE NumOfData)
{
    //vPrintString("s_11\n\r");
    climb_control_task(); // main.c
}
void s_20(xEventHandle * pxEventArray, portBASE_TYPE NumOfEvent, struct eventData * pxDataArray, portBASE_TYPE NumOfData) 
{
    //vPrintString("Task 4 end *********************\n\r");
}
#endif

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
    &s_13,
    &s_14,
    &s_15,
    &s_16,
    &s_17,
    &s_18,
    &s_19,
    &s_20,
    NULL 
};
