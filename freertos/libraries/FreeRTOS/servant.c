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
#include "app.h"

xList * pxCurrentReadyList;         // record the xEventReadyList that R-Servant transit event just now
struct xParam pvParameters[NUMBEROFSERVANT];

extern xSemaphoreHandle xBinarySemaphore[NUMBEROFSERVANT];  // the semaphores which are used to trigger new servant to execute
extern xTaskHandle xTaskOfHandle[NUMBEROFSERVANT+1];         // record the handle of all S-Servant, the last one is for debugging R-Servant 
// record the relationship among servants excluding R-Servant
//extern portBASE_TYPE xRelation[NUMBEROFSERVANT][NUMBEROFSERVANT] ;
// the new xRelation table which is implemeted with sparse matrix
extern struct xRelationship xRelations; 

// the LET of all S-Servant
extern portTickType xLetOfServant[NUMBEROFSERVANT] ;
// In app.c, this is used to sepcify the function of Servant
extern pvServantFunType xServantTable[NUMBEROFSERVANT];


/* create all semaphores which are used to triggered s-servant */
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

/*
 * Set all the parameters of S-Servants.
 *
 * */
void vParameterInitialise()
{
    portBASE_TYPE i;
    portBASE_TYPE xSource, xDest;
       
    // initialise the member of pvParameters
    for( i = 0; i < NUMBEROFSERVANT; ++ i )
    {
        pvParameters[i].xMyFlag = i;
        pvParameters[i].xNumOfIn = 0;
        pvParameters[i].xNumOfOut = 0;
        pvParameters[i].xLet = xLetOfServant[i]/portTICK_RATE_MS;
        pvParameters[i].xFp = xServantTable[i];
    }

    // set the xOut/InFlag and xNumOfOut/In of pvParameters according to the relation table
    // old edition
    /*
    for( i = 0; i < NUMBEROFSERVANT; ++ i )
    {
        for( j = 0; j < NUMBEROFSERVANT; ++ j )
        {
            if( xRelation[i][j] == 1 )
            {
                pvParameters[i].xOutFlag[pvParameters[i].xNumOfOut] = j;
                pvParameters[i].xNumOfOut ++;

                pvParameters[j].xInFlag[pvParameters[j].xNumOfIn] = i;
                pvParameters[j].xNumOfIn ++;
            }
        }
    }
    */
    // new edition with sparse matrix relation table
    for( i = 0; i < xRelations.xNumOfRelation; ++ i )
    {
        xSource = xRelations.xRelation[i].xInFlag;
        xDest   = xRelations.xRelation[i].xOutFlag;

        pvParameters[xSource].xOutFlag[pvParameters[xSource].xNumOfOut] = xDest;
        pvParameters[xSource].xNumOfOut ++;

        pvParameters[xDest].xInFlag[pvParameters[xDest].xNumOfIn] = xSource;
        pvParameters[xDest].xNumOfIn ++;
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
    portBASE_TYPE xInFlag[NUM]; 
    portBASE_TYPE xMyFlag = ((struct xParam *) pvParameter)->xMyFlag;
    portTickType xCurrentTime;
   
    xSemaphoreTake( xBinarySemaphore[xMyFlag],portMAX_DELAY );

    xCurrentTime = xTaskGetTickCount(); 
    vTaskSetxStartTime( xTaskOfHandle[xMyFlag], xCurrentTime );

    for( i = 0; i < NUM; i ++ )
    {
        // get all the in flag
        xInFlag[i] = ((struct xParam *) pvParameter)->xInFlag[i];
        // receive all events which are created by source servants and return 
        // them back to current Servant through the inout point
        vEventReceive( &pxEvent[i], xTaskOfHandle[xInFlag[i]], pxCurrentReadyList );
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

    portBASE_TYPE NUM = ((struct xParam *) pvMyParameter)->xNumOfOut;
    portBASE_TYPE xMyFlag = ((struct xParam *) pvMyParameter)->xMyFlag;
    portTickType xLet = ((struct xParam *) pvMyParameter)->xLet;
    pvServantFunType xMyFun = ((struct xParam *) pvMyParameter)->xFp;

    /* set the LET of Servant */
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

        // create events for all destination servants of this sensor.
        vEventCreateAll( pvMyParameter, xDatas );

        xMyFun( NULL, 0, xDatas, NUM);

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
    pvServantFunType xMyFun = ((struct xParam *) pvMyParameter)->xFp;

    xEventHandle pxEvent[NUM];

    //struct eventData xData[];

    /* get the LET of current servant */
    portTickType xLet = ((struct xParam *) pvMyParameter)->xLet;
    /* set the LET of current servant */
    vTaskSetxLet(xTaskOfHandle[xMyFlag], xLet);

    while(1)
    {
        vEventReceiveAll( pvMyParameter, pxEvent );

        xMyFun( pxEvent, NUM, NULL, 0 );

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
    pvServantFunType xMyFun = ((struct xParam *) pvMyParameter)->xFp;

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

        xMyFun(pxEvent, xNumOfIn, xDatas, xNumOfOut);
        
        vEventDeleteAll( pvMyParameter, pxEvent );        

        vEventCreateAll( pvMyParameter, xDatas );
        vTaskDelayLET();
    }
}

void vR_Servant( void * pvParameter)
{
    portBASE_TYPE i, j;
    portBASE_TYPE xSource, xDest;
    portBASE_TYPE HAVE_TO_SEND_SEMAPHORE; // could the semaphore be sent? 1 means yes , 0 means no

    xListItem * pxEventListItem;
    xTaskHandle destinationTCB, sourceTCB;

    while(1)
    {
        /*
         * transit the event item, whose timestamp equals to current Tick Count, 
         * from xEventList to the idlest xEvestReadyList
         *
         * There is a bug here. Under the multicore platform, the "pxCurrentReadyList" is an exclusive 
         * variable which should be used in critical section.
         * */
        vEventListTransit( &pxEventListItem, &pxCurrentReadyList);

        if( pxEventListItem == NULL && pxCurrentReadyList == NULL )
        {
            vTaskDelay(20/portTICK_RATE_MS);
            continue;
        }

        destinationTCB = xEventGetpxDestination( pxEventListItem->pvOwner);
        sourceTCB = xEventGetpxSource( pxEventListItem->pvOwner );
        HAVE_TO_SEND_SEMAPHORE = 1;


      //  vPrintString("semaphore check\n\r");

        for( i = 0; i < xRelations.xNumOfRelation; ++ i )
        {
            xSource = xRelations.xRelation[i].xInFlag;
            xDest   = xRelations.xRelation[i].xOutFlag;

            if( destinationTCB == xTaskOfHandle[xDest] )
            {
                // find the right relation
                if( sourceTCB == xTaskOfHandle[xSource] )
                {
                    if( xRelations.xRelation[i].xFlag == 2 )
                    {
                        vPrintString("Error: This event has arrived!!\n\r") ;
                        vEventDelete( (xEventHandle) pxEventListItem->pvOwner );
                    }
                    else
                    {
                        xRelations.xRelation[i].xFlag = 2;
                    }

                }
                // find other relation which is relative to destinationtcb
                else
                {
                    // waiting for an events that is not arriving yet
                    if( xRelations.xRelation[i].xFlag == 1 )
                    {
                        HAVE_TO_SEND_SEMAPHORE = 0;
                    }
                }
            }
        }

        if( HAVE_TO_SEND_SEMAPHORE )
        {
            vPrintString("sending semaphore to targetServantTCB------------\n\r");
            // set all the relations whose destination S-Servant is xTaskOfHandle[i] to 1.
            for( i = 0; i < xRelations.xNumOfRelation; ++ i )
            {
                xDest = xRelations.xRelation[i].xOutFlag;
                if( destinationTCB == xTaskOfHandle[xDest] )
                {
                    xRelations.xRelation[i].xFlag = 1;
                    // record the number of destinationtcb in xTaskOfHandle array.
                    j = xDest;
                }
            }
            // send semaphore to destinationtcb
            xSemaphoreGive( xBinarySemaphore[j] );
        }

        /*
        // find the handler of destination S-Servant
        for( i = 0; i < NUMBEROFSERVANT; ++i )
        {
            // the xTaskOfHandle[i] is the destination S-Servant
            if( destinationTCB == xTaskOfHandle[i])
            {
                break;
            }
        }

        // set the corresponding relation into 2.
        for( j = 0; j < pvParameters[i].xNumOfIn; ++ j )
        {
            // choose the right source TCB
            xInFlag = pvParameters[i].xInFlag[j];
            if( xTaskOfHandle[ xInFlag ] == sourceTCB ) 
            {
                // old edition
                if( xRelation[ xInFlag ][i] == 2 )
                {
                    vPrintString(" Error! There is already an event which is not proceed yet!\n\r");
                    break;
                }
                else
                {
                    xRelation[ xInFlag ][i] = 2;
                }
            }

            // counting the numbers of 2.
            if( xRelation[ xInFlag ][i] == 2 )
            {
                xCount ++;
            }
        }

        // if the numbers of 2 equals to the xNumofIn of destination S-Servant, then the destination S-servant 
        // should be triggered to execute.
        if( xCount == pvParameters[i].xNumOfIn )
        {
            vPrintString("sending semaphore to targetServantTCB------------\n\r");
            // set all the relations whose destination S-Servant is xTaskOfHandle[i] to 1.
            for( j = 0; j < pvParameters[i].xNumOfIn; ++ j )
            {
                xInFlag = pvParameters[i].xInFlag[j];
                xRelation[ xInFlag ][i] = 1;
            }

            xSemaphoreGive( xBinarySemaphore[i] );
        }
        */
    }
}
