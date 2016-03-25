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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Defining MPU_WRAPPERS_INCLUDED_FROM_API_FILE prevents task.h from redefining
all the API functions to use the MPU wrappers.  That should only be done when
task.h is included from an application file. */
#define MPU_WRAPPERS_INCLUDED_FROM_API_FILE

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "StackMacros.h"
#include "eventlist.h"

#undef MPU_WRAPPERS_INCLUDED_FROM_API_FILE

static  portBASE_TYPE IS_FIRST_EVENT = (portBASE_TYPE)1; /* A flag to know whether the event Lists are initialised. */

/* the struct of event item*/
typedef struct eveEventControlBlock
{
    xTaskHandle pxSource;         /*< the S-Servant where the event item from >*/
    xTaskHandle pxDestination;    /*< the S-Servant where the event item to >*/
    struct timeStamp xTimeStamp;    /*< the time stamp used to sort the event item in the event list >*/
    xListItem  xEventListItem;       /*< connect the eveECB to the xEventList by struct list>*/
    struct eventData xData;
}eveECB;


/* Event lists must be initialised before the first time to create an event. */

PRIVILEGED_DATA static  xList xEventList;                            /*< Event List is used to store the event item in a specific order which sended or received by S-Servant.>*/
PRIVILEGED_DATA static xList xEventReadyList[configCPU_NUMBER];                       /*< Event list is used to store the ready to be received events. the configCPU_NUMBER is defined in freeRTOSConfig.h>*/
static volatile unsigned portBASE_TYPE xEventSerialNumber  = (portBASE_TYPE)0;       /* used to set the level of timestamp in event */


/* initialise the event lists including xEventList and xEventReadyList[configCPU_NUMBER]
*
* no param and no return value
* */
static void prvInitialiseEventLists(void ) PRIVILEGED_FUNCTION; 

/* insert new event item into xEventList. */
static void prvEventListGenericInsert( xListItem * pxNewListItem) PRIVILEGED_FUNCTION;


/* 
 * compare the two timestamp with a specified sort algorithm.
 *
 * @param1: timestamp one
 * @param2: timestamp two
 * */
static portBASE_TYPE xCompareFunction( const struct timeStamp t1, const struct timeStamp t2 );


/*
 * initialise the eventItem and ready for the insertion. Set the pvOwner and Container which is the member of List Item.
 * **/
static void vListIntialiseEventItem( xEventHandle pvOwner, xListItem * pxNewEventItem);

static void vEventSetxData( xEventHandle pxEvent, struct eventData xData);

static void vEventSetxTimeStamp( xEventHandle pxNewEvent );

static xList * pxGetReadyList( void );

static void prvInitialiseEventLists(void )
{
    volatile portBASE_TYPE xCPU;

    vListInitialise( ( xList * ) &xEventList );

    // init the xEventReadyList[configCPU_NUMBER].
    for ( xCPU = 0; xCPU < configCPU_NUMBER; xCPU ++ )
    {
        vListInitialise( (xList * ) & xEventReadyList[xCPU] );
    }
}


static portBASE_TYPE xCompareFunction( const struct timeStamp t1, const struct timeStamp t2 )
{
    if( t1.xTime < t2.xTime )
    {
        return pdTRUE;
    }
    else if ( t1.xTime == t2.xTime)
    {
        if( t1.xLevel < t2.xLevel )
        {
            return pdTRUE;
        }
    }

    return pdFALSE;
}


/*
static portBASE_TYPE xCompareFunction(const struct timeStamp t1, const struct timeStamp t2)
{
    return pdTRUE;
}
*/

xTaskHandle xEventGetpxSource( xEventHandle pxEvent )
{
    return ((eveECB *)pxEvent)->pxSource;
}

xTaskHandle xEventGetpxDestination( xEventHandle pxEvent)
{
    return ((eveECB *) pxEvent)->pxDestination;
}

struct timeStamp xEventGetxTimeStamp( xEventHandle pxEvent)
{
    return ((eveECB *) pxEvent)->xTimeStamp;
}

struct eventData xEventGetxData( xEventHandle pxEvent)
{
    return ((eveECB *) pxEvent)->xData;
}

static void vEventSetxTimeStamp( xEventHandle pxNewEvent )
{
    eveECB * pxEvent =(eveECB *) pxNewEvent;
    /* get the current task TCB handler*/
    xTaskHandle pxCurrentTCBLocal = xTaskGetCurrentTaskHandle();

    /*set the time of this event to be processed */
    pxEvent->xTimeStamp.xTime = xTaskGetxStartTime(pxCurrentTCBLocal) + xTaskGetxLet(pxCurrentTCBLocal) + configMAX_RSERVANT_EXETIME;

    /*the microstep is not used now*/

    /* set the level of timestamp according to the topology sort result*/
    pxEvent->xTimeStamp.xLevel = xEventSerialNumber;

    xEventSerialNumber++;
}

static void vEventSetxData( xEventHandle pxEvent, struct eventData xData)
{
    ((eveECB *) pxEvent)->xData = xData;
}

/*
* Get the event ready list according to the multicore scheduling algorithm.
*
* @return the address of the event ready list.
* */
static xList * pxGetReadyList( void )
{
    return &xEventReadyList[0];
}

/* insert the event item to the xEventList according the sort algorithm.*/
/*
static void prvEventListGenericInsert( xListItem *pxNewListItem)
{
    vListInsertEnd( &xEventList, pxNewListItem);
}

static void prvEventListGenericInsert( xListItem *pxNewListItem)
{
    volatile xListItem * pxIterator;
    struct timeStamp xTimeStampOfInsertion;
    xList * pxList = (xList *)pxNewListItem->pvContainer;
    
    portBASE_TYPE flag = 0;

    xTimeStampOfInsertion = xEventGetxTimeStamp( pxNewListItem->pvOwner );
    //xTimeStampOfInsertion = ( (eveECB *) pxNewListItem->pvOwner)->xTimeStamp;

    //dose not take the time overflow into consideration yet.
    pxIterator = ( xListItem * ) &( pxList->xListEnd.pxNext );


    //bug here is a big problem. the point will be a mess
    if( listLIST_IS_EMPTY( pxList ))
    {
        // there is no event in the event list now, then insert the newEventItem into the end of the xEventList. 
    }
    else
    {
        //do nothing, just find the approperiate position
        for ( ; xCompareFunction( ( (xEventHandle) pxIterator->pvOwner)->xTimeStamp, xTimeStampOfInsertion); pxIterator = pxIterator->pxNext)
        {
            //if flag == pxList->uxNumberOfItems, then the timestamp of newListItem is the biggest. It should be inserted into the end of the xEventList.
            flag ++;
            if ( flag == pxList->uxNumberOfItems )
            {
                pxIterator = pxIterator->pxNext;
                break;
            }
        }
    }

    pxNewListItem->pxNext = pxIterator;
    pxNewListItem->pxPrevious = pxIterator->pxPrevious;
    pxNewListItem->pxPrevious->pxNext = (volatile xListItem *) pxNewListItem;
    pxIterator->pxPrevious= (volatile xListItem *) pxNewListItem;

    ( pxList->uxNumberOfItems ) ++;

    pxList->pxIndex = pxList->xListEnd.pxNext ;
}

*/
static void prvEventListGenericInsert( xListItem *pxNewListItem )
{
    volatile xListItem *pxIterator;
    struct timeStamp xTimeStampOfInsertion;
    xList * pxList = &xEventList;

    /* Insert the new list item into the list, sorted in ulListItem order. */
    xTimeStampOfInsertion = xEventGetxTimeStamp(pxNewListItem->pvOwner);

    /* If the list already contains a list item with the same item value then
    the new list item should be placed after it.  This ensures that TCB's which
    are stored in ready lists (all of which have the same ulListItem value)
    get an equal share of the CPU.  However, if the xItemValue is the same as
    the back marker the iteration loop below will not end.  This means we need
    to guard against this by checking the value first and modifying the
    algorithm slightly if necessary. */
    if( xTimeStampOfInsertion.xTime == portMAX_DELAY )
    {
        pxIterator = pxList->xListEnd.pxPrevious;
    }
    else
    {
        /* *** NOTE ***********************************************************
        If you find your application is crashing here then likely causes are:
        1) Stack overflow -
        see http://www.freertos.org/Stacks-and-stack-overflow-checking.html
        2) Incorrect interrupt priority assignment, especially on Cortex-M3
        parts where numerically high priority values denote low actual
        interrupt priories, which can seem counter intuitive.  See
        configMAX_SYSCALL_INTERRUPT_PRIORITY on http://www.freertos.org/a00110.html
        3) Calling an API function from within a critical section or when
        the scheduler is suspended.
        4) Using a queue or semaphore before it has been initialised or
        before the scheduler has been started (are interrupts firing
        before vTaskStartScheduler() has been called?).
        See http://www.freertos.org/FAQHelp.html for more tips.
        **********************************************************************/

        taskENTER_CRITICAL();
        for( pxIterator = ( xListItem * ) &( pxList->xListEnd ); xCompareFunction( xEventGetxTimeStamp( pxIterator->pxNext->pvOwner ), xTimeStampOfInsertion ); pxIterator = pxIterator->pxNext ) 
        //for( pxIterator = ( xListItem * ) &( pxList->xListEnd ); pxIterator->pxNext->xItemValue <= xTimeStampOfInsertion; pxIterator = pxIterator->pxNext )
        {
            /* There is nothing to do here, we are just iterating to the
            wanted insertion position. */
        }
        taskEXIT_CRITICAL();
    }

    pxNewListItem->pxNext = pxIterator->pxNext;
    pxNewListItem->pxNext->pxPrevious = ( volatile xListItem * ) pxNewListItem;
    pxNewListItem->pxPrevious = pxIterator;
    pxIterator->pxNext = ( volatile xListItem * ) pxNewListItem;

    /* Remember which list the item is in.  This allows fast removal of the
    item later. */
    pxNewListItem->pvContainer = ( void * ) pxList;

    ( pxList->uxNumberOfItems )++;
}




static void vListIntialiseEventItem( xEventHandle pvOwner, xListItem * pxNewEventItem)
{
    /* set the pvOwner of the EventItem as a event*/
    listSET_LIST_ITEM_OWNER( pxNewEventItem, pvOwner );
    //pxNewEventItem->pvContainer = (void *) &xEventList;
}


void vEventGenericCreate( xTaskHandle pxDestination, struct eventData pdData)
{
    eveECB * pxNewEvent = NULL;

    /* using the pxCurrentTCB, current task should not be changed */
    taskENTER_CRITICAL();

    if ( IS_FIRST_EVENT == 1 )
    {
        IS_FIRST_EVENT = 0;
        prvInitialiseEventLists();
    }

    xTaskHandle pxCurrentTCBLocal = xTaskGetCurrentTaskHandle();
    pxNewEvent = (eveECB *)pvPortMalloc( sizeof( eveECB ));
    if ( pxNewEvent != NULL )
    {
        pxNewEvent->pxSource = pxCurrentTCBLocal;
        pxNewEvent->pxDestination = pxDestination;

        /* initialise the time stamp of an event item according to the sort algorithm.*/
        vEventSetxTimeStamp( pxNewEvent );

        vListIntialiseEventItem( pxNewEvent, (xListItem *) &pxNewEvent->xEventListItem );

        vEventSetxData( pxNewEvent, pdData );

        /*how to call this funciton: vEventListInsert( newListItem ). This function add the new item into the xEventList as default*/
        prvEventListGenericInsert( (xListItem *) &(pxNewEvent->xEventListItem));
    
    }
    taskEXIT_CRITICAL();

}


/* An API to transfer the Event Item from xEventList to one of the xEventReadyList*/
void vEventListGenericTransit( xListItem ** pxEventListItem, xList ** pxCurrentReadyList)
{
    taskENTER_CRITICAL();
    //send_num(xEventList.uxNumberOfItems);
    //helloworld();
    
    if( !listLIST_IS_EMPTY(&xEventList) ) 
    {
        *pxCurrentReadyList = pxGetReadyList();

        /* get the event from xEventList */
        //*test = (xListItem *) (xEventList.pxIndex);
        *pxEventListItem = (xListItem *)xEventList.xListEnd.pxNext;
        if((*pxEventListItem)->pvContainer == (void *)&xEventList)
        {
            //successful.
            //helloworld();
        }
        
        /* remove pxListItem from xEventList */ 
        vListRemove(*pxEventListItem);
        /* insert the pxListItem into the specified pxList */
        vListInsertEnd(*pxCurrentReadyList, *pxEventListItem);
        //send_num(listCURRENT_LIST_LENGTH(*pxCurrentReadyList));
    }
    else
    {
        *pxEventListItem = NULL;
        *pxCurrentReadyList = NULL;
    }

    taskEXIT_CRITICAL();
}

void vEventGenericReceive( xEventHandle * pxEvent, xTaskHandle pxSource, xList * pxList )
{
    xList * const pxConstList = pxList; 
    if (pxList == &xEventReadyList[0])
    {
        //successful.
        //helloworld();
    }

    if( listLIST_IS_EMPTY( pxList ) )
    {
        *pxEvent = NULL;
        return;
    }

    volatile xListItem * pxFlag = pxConstList->xListEnd.pxNext;


    taskENTER_CRITICAL();

    xTaskHandle pxCurrentTCBLocal = xTaskGetCurrentTaskHandle();

    /* look up in the specified xEventReadyList. */
    for(; pxFlag != (xListItem *) ( pxConstList->xListEnd.pxPrevious); pxFlag = pxFlag->pxNext)
    {
        if( xEventGetpxSource((xEventHandle) pxFlag->pvOwner) == pxSource && xEventGetpxDestination((xEventHandle) pxFlag->pvOwner) == pxCurrentTCBLocal )
        {
            *pxEvent = pxFlag->pvOwner;
            vListRemove((xListItem *) pxFlag);
            taskEXIT_CRITICAL();
            return;
        }
    }

    /* we can't forget the last one item of the list. */
    if( xEventGetpxSource((xEventHandle) pxFlag->pvOwner) == pxSource && xEventGetpxDestination((xEventHandle) pxFlag->pvOwner) == pxCurrentTCBLocal )
    {
        *pxEvent = pxFlag->pvOwner;
        vListRemove((xListItem *) pxFlag);
        taskEXIT_CRITICAL();
        return;
    }
    else
    {
        // cannot find the specified event.
        *pxEvent = NULL;
    }

    taskEXIT_CRITICAL();
}

void vEventGenericDelete( xEventHandle xEvent)
{
    taskENTER_CRITICAL();

    vPortFree( xEvent );

    taskEXIT_CRITICAL();
}
