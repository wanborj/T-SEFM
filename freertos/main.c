/*
* this application is used to try to exam the EventList mechanism.
*
* what this application can do is to print the "hello world! I am from China" in a periodic way.
* The task to print the words is finished by one task which is composed of five S-servant include S-1,
* S-2, S-3, S-4, S-5. They print the words in a specified order and in a collaborative way in a finite 
* time duration.
* */


#define USE_STDPERIPH_DRIVER
#include "stm32f10x.h"

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "eventlist.h"

static void setup_hardware( void );

#define NUMBEROFSERVANT 6 

xSemaphoreHandle xBinarySemaphore[NUMBEROFSERVANT];  // the network topology
xTaskHandle xTaskOfHandle[NUMBEROFSERVANT];         // record the handle of all S-Servant
xList * pxCurrentReadyList;         // record the xEventReadyList that R-Servant transit event just now

struct xParam
{
    portBASE_TYPE xInFlag;
    portBASE_TYPE xOutFlag;
};

struct xParam pvParameters[NUMBEROFSERVANT];


void led_flash_task( void *pvParameter )
{
    while(1) {
        /* Toggle the LED. */
        GPIOC->ODR = GPIOC->ODR ^ 0x00001000;

        /* Wait one second. */
        vTaskDelay(100);
    }
}



void vPrintString( const char * string)
{
    int i = 0;
    while(string[i] != '\0')
    {
        send_byte(string[i]);
        i++;
    }
}

void vPrintNumber( const portBASE_TYPE num)
{
    vPrintString("the number is :");
    send_byte(num+'0');
    send_byte('\n');
    send_byte('\r');
}

static void vServant( void * pvParameter )
{
    portBASE_TYPE xInFlag = ((struct xParam *) pvParameter)->xInFlag;
    portBASE_TYPE xOutFlag = ((struct xParam *) pvParameter)->xOutFlag;
    xEventHandle pxEvent;
    struct eventData sData;

    while(1)
    {
        xSemaphoreTake( xBinarySemaphore[xInFlag+1], portMAX_DELAY );
        /* get event whose pxSource equals to xTaskOfHandle[xInFlag] from specified xEventReadyList */
        
        vPrintString("enter S-Servant+++++++++++++\n\r");
        //vPrintNumber(xInFlag+1);
        // receive the event whose source Servant is xTaskOfHandle[xInFlag]
        vEventReceive( &pxEvent, xTaskOfHandle[xInFlag], pxCurrentReadyList );

        if( pxEvent == NULL )
        {
            vPrintString("pxEvent == NULL, there isn't right event in the xEventReadyList\n\r");
            vTaskDelay(15/portTICK_RATE_MS);
            continue;
        }
       /* read data */ 
        sData= xEventGetxData(pxEvent);
        sData.xData = sData.xData+ 1;
        //vPrintNumber(sData.xData);

        /* delete the old event*/
        vEventDelete(pxEvent);
        /* create a new event whose destination is xTaskOfHandle[xOutFlag] and insert it into the xEventList*/
        vEventCreate(xTaskOfHandle[xOutFlag], sData);
    }
}

static void vActuator( void * pvParameter)
{
    portBASE_TYPE xInFlag = ((struct xParam *) pvParameter)->xInFlag;
    xEventHandle pxEvent = NULL;
    struct eventData xData;

    while(1)
    {

        xSemaphoreTake(xBinarySemaphore[xInFlag+1], portMAX_DELAY);

        vEventReceive( &pxEvent, xTaskOfHandle[xInFlag], pxCurrentReadyList);

        if( pxEvent == NULL )
        {
            vTaskDelay(20/portTICK_RATE_MS);
            continue;
        }
        xData = xEventGetxData( pxEvent );

        vEventDelete( pxEvent );
    }
}

/*
* function as the init Task in system. vPeriodicTask have the highest priority. It creates the first event to 
* xEventList in system. After that, it Delay until another period.
* */

static void vSensor( void * pvParameter )
{
    portTickType xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();

    /* create a data and initialise it */
    struct eventData xData;
    xData.xData = 1.0;

    /* network parameter*/
    portBASE_TYPE xOutFlag = ((struct xParam *) pvParameter)->xOutFlag;

    while(1)
    {
        /* create the the first event item to trigger xTaskOfHandle[xOutFlag] and insert it into the xEventList*/
        //vEventCreate(xTaskOfHandle[xOutFlag], xData);
        vEventCreate(xTaskOfHandle[xOutFlag], xData);

        vPrintString( "External Event!!!!!!!!!!!!!!!!!!!!!!\n\r");

        vTaskDelayUntil(&xLastWakeTime, 2000/portTICK_RATE_MS);
        xData.xData ++;
    }
}

static void vR_Servant( void * pvParameter)
{
    xListItem * pxEventListItem;
    portBASE_TYPE i;
//    portBASE_TYPE targetServantNum;
    xTaskHandle targetServantTCB;

    while(1)
    {
        /*transit the highest event item from xEventList to the idlest xEvestReadyList*/
        vEventListTransit( &pxEventListItem, &pxCurrentReadyList);

        if ( pxEventListItem == NULL && pxCurrentReadyList == NULL)
        {
            vTaskDelay(300/portTICK_RATE_MS);
            continue;
        }

        targetServantTCB = xEventGetpxDestination( pxEventListItem->pvOwner);
        vPrintString("enter R-Servant again #######\n\r");

        for( i = 1; i < NUMBEROFSERVANT; ++i )
        {
            vTaskDelay(10/portTICK_RATE_MS);

            /*the point has some problem here, these two points can be never same */
            if( targetServantTCB == xTaskOfHandle[i])
            {
                vPrintString("sending semaphore to targetServantTCB------------\n\r");
                vPrintNumber(i);
                xSemaphoreGive( xBinarySemaphore[i] );
                break;
            }
        }

    }
}

int main(void)
{
    init_led();
    init_rs232();
    enable_rs232_interrupts();
    enable_rs232();

    portBASE_TYPE i;    

    for( i = 0; i < NUMBEROFSERVANT; i ++ )
    {
        // i == 0,  which is the external event
        if ( i == 0 )
            pvParameters[i].xInFlag = 0;
        else
            pvParameters[i].xInFlag = i-1;

        // i == NUMBEROFSERVANT - 1, which is the actuator
        if( i != (NUMBEROFSERVANT -1 ))
            pvParameters[i].xOutFlag = (i+1);
        else
            pvParameters[i].xOutFlag = i;

        //vPrintString("the in flag is:\n\r");
        //send_num(pvParameters[i].xInFlag);

        //vPrintString("the out flag is:\n\r");
        //send_num(pvParameters[i].xOutFlag);

        vSemaphoreCreateBinary(xBinarySemaphore[i]);

        /* when created, it is initialised to 1. So, we take it away.*/
        xSemaphoreTake(xBinarySemaphore[i], portMAX_DELAY);
    }

    xTaskCreate( vR_Servant, "R Servant", 512, NULL, tskIDLE_PRIORITY + 1, NULL);

    xTaskCreate( vSensor, "Sensor", 512, (void *)&pvParameters[0], tskIDLE_PRIORITY + 10, &xTaskOfHandle[0]);

    xTaskCreate( vServant, "Servant No.1", 512 /* stack size */, (void *)&pvParameters[1], tskIDLE_PRIORITY + 2, &xTaskOfHandle[1]);
    xTaskCreate( vServant, "Servant No.2", 512 /* stack size */, (void *)&pvParameters[2], tskIDLE_PRIORITY + 3, &xTaskOfHandle[2]);
    xTaskCreate( vServant, "Servant No.3", 512 /* stack size */, (void *)&pvParameters[3], tskIDLE_PRIORITY + 4, &xTaskOfHandle[3]);
    xTaskCreate( vServant, "Servant No.4", 512 /* stack size */, (void *)&pvParameters[4], tskIDLE_PRIORITY + 5, &xTaskOfHandle[4]);

    xTaskCreate( vActuator, "Actuator", 512 /* stack size */, (void *)&pvParameters[5], tskIDLE_PRIORITY + 6,&xTaskOfHandle[5]);

    /* Start running the task. */
    vTaskStartScheduler();

    return 0;
}

void myTraceCreate      (){
}

void myTraceSwitchedIn  (){
}

void myTraceSwitchedOut	(){
}

/*
inline float myTraceGetTick(){
	// 0xE000E014 -> Systick reload value
	// 0xE000E018 -> Systick current value
	return ((float)((*(unsigned long *)0xE000E014)-(*(unsigned long *)0xE000E018)))/(*(unsigned long *)0xE000E014);
}

inline unsigned long myTraceGetTimeMillisecond(){
	return (xTaskGetTickCountFromISR() + myTraceGetTick()) * 1000 / configTICK_RATE_HZ;
}
*/
void vApplicationTickHook( void )
{
}
