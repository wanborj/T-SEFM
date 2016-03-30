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


#ifndef SERVANT_H
#define SERVANT_H
#include "stm32f10x.h"

#include "queue.h"
#include "semphr.h"
#include "task.h"
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"

#define NUMBEROFSERVANT 6 
#define MAXOUTDEGREE 10   // network max in degree of every S-servant
#define MAXINDEGREE 10  // network max out degree of every s-servant

//portBASE_TYPE NUMBEROFSERVANT = 6;

/*
* It is used to record the topology and basic time information of servant
* */
struct xParam
{
    /* the topology of system */
    portBASE_TYPE xMyFlag;     // the flag of current servant
    portBASE_TYPE xNumOfIn;    // the number of source servants
    portBASE_TYPE xNumOfOut;   // the number of destination servants
    portBASE_TYPE xInFlag[MAXINDEGREE];  // the flags of source servants
    portBASE_TYPE xOutFlag[MAXOUTDEGREE]; // the flags of destination servants
    /* the basic time of servant */
    portTickType xLet;
};


/*
 * create semaphores for every S-Servant. 
 * Each S-Servant are pending for specified semaphores. Once corresponding 
 * semaphores occurs, the S-Servant will be triggered to execute.
 * */
void vSemaphoreInitialise();

/*
 * Initialise the paramter which will be send to each S-Servant. And
 * initialise the topology of all the S-Servant in system according to the relation table of S-Servant.
 * */
void vRelationInitialise();

/* Occupied CPU until the output time of current Servant for keep LET semantics. */
void vTaskDelayLET();

/*
 * This function is used in normal S-Servant or actuator, while shouldn't be in sensor.
* pending for waiting all the source servant finishing execution.
* Then receive the events from xEventReadyList.
*
* @param pvParameter is the parameter from programmer
* @param pxEvent is an inout parameter which is used to transit the event from source servant
* */
void vEventReceiveAll( void * pvParameter, xEventHandle * pxEvent );

/* delete all the events which are received from source servants*/ 
void vEventDeleteAll( void * pvParameter, xEventHandle * pxEvent );

/*
* This function is used in Sensor or normal S-Servant, while shouldn't be in Actuator.
* create all events which are used to transit information for destination servants.
*
* @param pvParamter is the parameter from programmer, which can be used to know the topology of task.
* @param xDatas are the data which will be add to the events for destination servant.
*
* */
void vEventCreateAll( void * pvParameter, struct eventData * xDatas );

void vSensor( void * pvParameter );

void vActuator( void * pvParameter );

void vServant( void * pvParameter );

void vR_Servant( void * pvParameter );


#endif
