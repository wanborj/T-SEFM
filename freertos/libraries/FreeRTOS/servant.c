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
#include "eventlist.h"
#include "servant.h"

xList * pxCurrentReadyList;         // record the xEventReadyList that R-Servant transit event just now

extern xSemaphoreHandle xBinarySemaphore[NUMBEROFSERVANT];  // the network topology
extern xTaskHandle xTaskOfHandle[NUMBEROFSERVANT+1];         // record the handle of all S-Servant, the last one is for debugging R-Servant 

void vSemaphoreInitialise()
{
    portBASE_TYPE i;

    for( i = 0; i < NUMBEROFSERVANT; ++ i )
    {
        vSemaphoreCreateBinary(xBinarySemaphore[i]);
        /* when created, it is initialised to 1. So, we take it away.*/
        xSemaphoreTake(xBinarySemaphore[i], portMAX_DELAY);
    }

}

void vTaskDelayLET()
{
    xTaskHandle pxTaskCurrentTCBLocal = xTaskGetCurrentTaskHandle();
    portTickType xStartTime = xTaskGetxStartTime( pxTaskCurrentTCBLocal );
    portTickType xOutputTime = xTaskGetxLet( pxTaskCurrentTCBLocal ) + xStartTime;
    portTickType xCurrentTime = xTaskGetTickCount();

    while( xCurrentTime <  xOutputTime)
    {
        xCurrentTime = xTaskGetTickCount();
    }
}

// receive all Events from sources servant, and return them through the point
void vEventReceiveAll( void * pvParameter, xEventHandle *pxEvent )
{
    portBASE_TYPE i ;

    portBASE_TYPE NUM = ((struct xParam *) pvParameter)->xNumOfIn;
    portBASE_TYPE xFlags[NUM]; 
    portBASE_TYPE xMyFlag = ((struct xParam *) pvParameter)->xMyFlag;
    portTickType xCurrentTime;
    
    for( i = 0; i < NUM; i++ )
    {
        // waiting for all semaphores. Here, the number of Semaphore are equal to the number of source servant
        xSemaphoreTake( xBinarySemaphore[xMyFlag],portMAX_DELAY );
    }

    xCurrentTime = xTaskGetTickCount(); 
    vTaskSetxStartTime( xTaskOfHandle[xMyFlag], xCurrentTime );

    for( i = 0; i < NUM; i ++ )
    {
        // get all the in flag
        xFlags[i] = ((struct xParam *) pvParameter)->xInFlag[i];
        // receive all events which are created by source servants and return 
        // them back to current Servant through the inout point
        vEventReceive( &pxEvent[i], xTaskOfHandle[xFlags[i]], pxCurrentReadyList );
    }
}

void vEventDeleteAll( void * pvParameter, xEventHandle * pxEvent )
{
    portBASE_TYPE i ;
    portBASE_TYPE NUM = ((struct xParam *) pvParameter)->xNumOfIn;

    for( i = 0; i < NUM; ++i )
    {
        vEventDelete( pxEvent[i] );
    }
}


void vEventCreateAll( void * pvParameter, struct eventData *xDatas )
{
    portBASE_TYPE i ;
    portBASE_TYPE NUM = ((struct xParam *) pvParameter)->xNumOfOut;
    portBASE_TYPE xFlags[NUM];

    for( i = 0; i < NUM; i ++ )
    {
        // get all flags of destination servants
        xFlags[i] = ((struct xParam *) pvParameter)->xOutFlag[i]; 
        // create events which would be sent to destination servants.
        vEventCreate(xTaskOfHandle[xFlags[i]], xDatas[i]) ;
    }
}

/*
* Sensor servant, which is different from normal S-Servant and actuator, is implemeted 
* in a different way as a result. Sensor here is implemented as a periodic servant.
*
* @param pvParameter is parameter from programmer.
*
* */
void vSensor( void * pvParameter )
{
    portTickType xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();
    portTickType xCurrentTime;
    portBASE_TYPE i;

    /* store the paramter into stack of servant */
    void * pvMyParameter = pvParameter;

    /* get the number of destination servants */
    portBASE_TYPE NUM = ((struct xParam *) pvMyParameter)->xNumOfOut;

    /* get the flag of current servant*/
    portBASE_TYPE xMyFlag = ((struct xParam *) pvMyParameter)->xMyFlag;

    /* get the LET of current servant */
    portTickType xLet = ((struct xParam *) pvMyParameter)->xLet;
    /* set the LET of current servant */
    vTaskSetxLet(xTaskOfHandle[xMyFlag], xLet);
    
    /* create data for destination servants and initialise them */
    struct eventData xDatas[NUM];
    for( i = 0; i < NUM; i ++ )
    {
        xDatas[i].xData = 0.0;
    }

    while(1)
    {
        xCurrentTime = xTaskGetTickCount();
        vTaskSetxStartTime( xTaskOfHandle[xMyFlag], xCurrentTime );

        /* Here are processing code used to init the environment data*/
        // coding...

        // create events for all destination servants of this sensor.
        vEventCreateAll( pvMyParameter, xDatas );

        vPrintString( "External Event!!!!!!!!!!!!!!!!!!!!!!\n\r");

        vTaskDelayLET();

        // sleep for 2 sec, which means that period of this task is 2 sec.
        vTaskDelayUntil(&xLastWakeTime, 2000/portTICK_RATE_MS);
    }
}

/*
* Actuator servant, which is different from normal servant and sensor.
* Actuator only receive events from event ready list without creating events.
* Actuator is used for driving machines not computing.
*
* @param pvParamter is parameter from programmer
*
* */

void vActuator( void * pvParameter )
{
    void * pvMyParameter = pvParameter;
    portBASE_TYPE NUM = ((struct xParam *) pvMyParameter)->xNumOfIn;
    portBASE_TYPE xMyFlag = ((struct xParam *) pvMyParameter)->xMyFlag;

    xEventHandle pxEvent[NUM];

    struct eventData xData;

    /* get the LET of current servant */
    portTickType xLet = ((struct xParam *) pvMyParameter)->xLet;
    /* set the LET of current servant */
    vTaskSetxLet(xTaskOfHandle[xMyFlag], xLet);

    while(1)
    {
        vEventReceiveAll( pvMyParameter, pxEvent );

        /* here are the processing code with pxEvent */
        // coding...
        vPrintString( " This is the Actuator!----------------------\n\r" );

        vEventDeleteAll( pvMyParameter, pxEvent );
        vTaskDelayLET();
    }
}

/*
*  Normal S-Servant is used to process the data of sensor and send them to actuator.
*
*  @param pvParameter is parameter from programmer
*
* */

void vServant( void * pvParameter )
{
    portBASE_TYPE i;
    void * pvMyParameter = pvParameter;
    
    portBASE_TYPE xNumOfIn = ((struct xParam *) pvMyParameter)->xNumOfIn;
    portBASE_TYPE xNumOfOut = ((struct xParam *) pvMyParameter)->xNumOfOut;
    portBASE_TYPE xMyFlag = ((struct xParam *) pvMyParameter)->xMyFlag;

    xEventHandle pxEvent[xNumOfIn];
    struct eventData xDatas[xNumOfOut];

    /* get the LET of current servant */
    portTickType xLet = ((struct xParam *) pvMyParameter)->xLet;
    /* set the LET of current servant */
    vTaskSetxLet(xTaskOfHandle[xMyFlag], xLet);


    while(1)
    {
        vEventReceiveAll( pvMyParameter, pxEvent );

        /* Here are coding for processing data of events */
        // coding...
        for( i = 0; i < xNumOfOut; i ++ )
        {
            xDatas[i].xData = (portDOUBLE)xMyFlag;
        }

        vPrintString( "This is S-Servant!################\n\r" );
        
        vEventDeleteAll( pvMyParameter, pxEvent );        

        vEventCreateAll( pvMyParameter, xDatas );
        vTaskDelayLET();
    }
}

void vR_Servant( void * pvParameter)
{
    portBASE_TYPE i;

    xListItem * pxEventListItem;
    xTaskHandle targetServantTCB;

    while(1)
    {
        /*transit the highest event item from xEventList to the idlest xEvestReadyList*/
        vEventListTransit( &pxEventListItem, &pxCurrentReadyList);

        if( pxEventListItem == NULL && pxCurrentReadyList == NULL )
        {
            vTaskDelay(20/portTICK_RATE_MS);
            continue;
        }

        targetServantTCB = xEventGetpxDestination( pxEventListItem->pvOwner);
        vPrintString("enter R-Servant again #######\n\r");

        for( i = 1; i < NUMBEROFSERVANT; ++i )
        {
            /*the point has some problem here, these two points can be never same */
            if( targetServantTCB == xTaskOfHandle[i])
            {
                vPrintString("sending semaphore to targetServantTCB------------\n\r");
                xSemaphoreGive( xBinarySemaphore[i] );
                break;
            }
        }
    }
}
