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
#define MAXOUTDEGREE 10   // network max in degree of every S-servant
#define MAXINDEGREE 10  // network max out degree of every s-servant


xSemaphoreHandle xBinarySemaphore[NUMBEROFSERVANT];  // the network topology
xTaskHandle xTaskOfHandle[NUMBEROFSERVANT+1];         // record the handle of all S-Servant
xList * pxCurrentReadyList;         // record the xEventReadyList that R-Servant transit event just now

/*
* It is used to record the topology of S-Servant
* */
struct xParam
{
    portBASE_TYPE xMyFlag;     // the flag of current servant
    portBASE_TYPE xNumOfIn;    // the number of source servants
    portBASE_TYPE xNumOfOut;   // the number of destination servants
    portBASE_TYPE xInFlag[MAXINDEGREE];  // the flags of source servants
    portBASE_TYPE xOutFlag[MAXOUTDEGREE]; // the flags of destination servants
};

struct xParam pvParameters[NUMBEROFSERVANT];

// pending for waiting all semaphores
/*
static void xSemaphoreTakeAll( void * pvParameter )
{
    portBASE_TYPE i ;
    portBASE_TYPE NUM = ((struct xParam *) pvParameter)->xNumOfIn;
    portBASE_TYPE Flags[NUM]; 

    for( i = 0; i < NUM; i++ )
    {
        // get all the in flag
        Flags[i] = ((struct xParam *) pvParameter)->xInFlag[i];
        xSemaphoreTake( xBinarySemaphore[Flags[i]], portMAX_DELAY );
    }
}
*/

/*
 * This function is used in normal S-Servant or actuator, while shouldn't be in sensor.
* pending for waiting all the source servant finishing execution.
* Then receive the events from xEventReadyList.
*
* @param pvParameter is the parameter from programmer
* @param pxEvent is an inout parameter which is used to transit the event from source servant
* */

// receive all Events from sources servant, and return them through the point
static void vEventReceiveAll( void * pvParameter, xEventHandle *pxEvent )
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


/* delete all the events which are received from source servants*/ 
static void vEventDeleteAll( void * pvParameter, xEventHandle * pxEvent )
{
    portBASE_TYPE i ;
    portBASE_TYPE NUM = ((struct xParam *) pvParameter)->xNumOfIn;

    for( i = 0; i < NUM; ++i )
    {
        vEventDelete( pxEvent[i] );
    }
}

/*
* This function is used in Sensor or normal S-Servant, while shouldn't be in Actuator.
* create all events which are used to transit information for destination servants.
*
* @param pvParamter is the parameter from programmer, which can be used to know the topology of task.
* @param xDatas are the data which will be add to the events for destination servant.
*
* */

static void vEventCreateAll( void * pvParameter, struct eventData *xDatas )
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
static void vSensor( void * pvParameter )
{
    portTickType xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();
    portTickType xCurrentTime;

    /* store the paramter into stack of servant */
    void * pvMyParameter = pvParameter;
    portBASE_TYPE i;

    /* get the number of destination servants */
    portBASE_TYPE NUM = ((struct xParam *) pvMyParameter)->xNumOfOut;

    /* get the flag of current servant*/
    portBASE_TYPE xMyFlag = ((struct xParam *) pvMyParameter)->xMyFlag;

    /* create data for destination servants and initialise them */
    struct eventData xDatas[NUM];
    for( i = 0; i < NUM; i ++ )
    {
        xDatas[i].xData = 0.0;
    }

    /* set the LET of current servant */
    vTaskSetxLet(xTaskOfHandle[xMyFlag], 10/portTICK_RATE_MS);

    while(1)
    {
        xCurrentTime = xTaskGetTickCount();
        vTaskSetxStartTime( xTaskOfHandle[xMyFlag], xCurrentTime );

        /* Here are processing code used to init the environment data*/
        // coding...

        // create events for all destination servants of this sensor.
        vEventCreateAll( pvMyParameter, xDatas );

        vPrintString( "External Event!!!!!!!!!!!!!!!!!!!!!!\n\r");

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

static void vActuator( void * pvParameter )
{
    void * pvMyParameter = pvParameter;
    portBASE_TYPE NUM = ((struct xParam *) pvMyParameter)->xNumOfIn;
    portBASE_TYPE xMyFlag = ((struct xParam *) pvMyParameter)->xMyFlag;

    xEventHandle pxEvent[NUM];

    struct eventData xData;

    /* set the LET of current servant */
    vTaskSetxLet(xTaskOfHandle[xMyFlag], 10/portTICK_RATE_MS);

    while(1)
    {

        vEventReceiveAll( pvMyParameter, pxEvent );

        /* here are the processing code with pxEvent */
        // coding...
        vPrintString( " This is the Actuator!----------------------\n\r" );

        vEventDeleteAll( pvMyParameter, pxEvent );
    }
}

/*
*  Normal S-Servant is used to process the data of sensor and send them to actuator.
*
*  @param pvParameter is parameter from programmer
*
* */

static void vServant( void * pvParameter )
{
    void * pvMyParameter = pvParameter;
    
    portBASE_TYPE xNumOfIn = ((struct xParam *) pvMyParameter)->xNumOfIn;
    portBASE_TYPE xNumOfOut = ((struct xParam *) pvMyParameter)->xNumOfOut;
    portBASE_TYPE xMyFlag = ((struct xParam *) pvMyParameter)->xMyFlag;
    portBASE_TYPE i;

    xEventHandle pxEvent[xNumOfIn];
    struct eventData xDatas[xNumOfOut];

    /* set the LET of current servant */
    vTaskSetxLet(xTaskOfHandle[xMyFlag], 20/portTICK_RATE_MS);


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
    }
}

static void vR_Servant( void * pvParameter)
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
            vTaskDelay(120/portTICK_RATE_MS);
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

/*
struct xParam
{
    portBASE_TYPE xMyFlag;     // the flag of current servant
    portBASE_TYPE xNumOfIn;    // the number of source servants
    portBASE_TYPE xNumOfOut;   // the number of destination servants
    portBASE_TYPE xInFlag[MAXINDEGREE];  // the flags of source servants
    portBASE_TYPE xOutFlag[MAXOUTDEGREE]; // the flags of destination servants
}; */

int main(void)
{
    init_led();
    init_rs232();
    enable_rs232_interrupts();
    enable_rs232();

    portBASE_TYPE i;    

    for( i = 0; i < NUMBEROFSERVANT; ++ i )
    {
        pvParameters[i].xMyFlag = i;

        vSemaphoreCreateBinary(xBinarySemaphore[i]);

        /* when created, it is initialised to 1. So, we take it away.*/
        xSemaphoreTake(xBinarySemaphore[i], portMAX_DELAY);
    }

    /* set the topology of a task */
    // sensor
    pvParameters[0].xNumOfIn = 0;
    pvParameters[0].xNumOfOut = 1;

    pvParameters[0].xOutFlag[0] = 1;

    // S-Servant S-1
    pvParameters[1].xNumOfIn = 1;
    pvParameters[1].xNumOfOut = 2;

    pvParameters[1].xInFlag[0] = 0;
    
    pvParameters[1].xOutFlag[0] = 2;
    pvParameters[1].xOutFlag[1] = 3;

    // S-Servant S-2
    pvParameters[2].xNumOfIn = 1;
    pvParameters[2].xNumOfOut = 1;

    pvParameters[2].xInFlag[0] = 1;
    
    pvParameters[2].xOutFlag[0] = 4;

    // S-Servant S-3
    pvParameters[3].xNumOfIn = 1;
    pvParameters[3].xNumOfOut = 1;

    pvParameters[3].xInFlag[0] = 1;
    
    pvParameters[3].xOutFlag[0] = 5;

    // S-Servant S-4
    pvParameters[4].xNumOfIn = 1;
    pvParameters[4].xNumOfOut = 1;

    pvParameters[4].xInFlag[0] = 2;
    
    pvParameters[4].xOutFlag[0] = 5;

    //actuator
    pvParameters[5].xNumOfIn = 2;
    pvParameters[5].xNumOfOut = 0;

    pvParameters[5].xInFlag[0] = 3;
    pvParameters[5].xInFlag[1] = 4;

    xTaskCreate( vR_Servant, "R Servant", 512, NULL, tskIDLE_PRIORITY + 1, &xTaskOfHandle[6]);

    xTaskCreate( vSensor, "Sensor", 512, (void *)&pvParameters[0], tskIDLE_PRIORITY + 10, &xTaskOfHandle[0]);

    xTaskCreate( vServant, "Servant No.1", 512 /* stack size */, (void *)&pvParameters[1], tskIDLE_PRIORITY + 6, &xTaskOfHandle[1]);
    xTaskCreate( vServant, "Servant No.2", 512 /* stack size */, (void *)&pvParameters[2], tskIDLE_PRIORITY + 5, &xTaskOfHandle[2]);
    xTaskCreate( vServant, "Servant No.3", 512 /* stack size */, (void *)&pvParameters[3], tskIDLE_PRIORITY + 4, &xTaskOfHandle[3]);
    xTaskCreate( vServant, "Servant No.4", 512 /* stack size */, (void *)&pvParameters[4], tskIDLE_PRIORITY + 3, &xTaskOfHandle[4]);

    xTaskCreate( vActuator, "Actuator", 512 /* stack size */, (void *)&pvParameters[5], tskIDLE_PRIORITY + 2,&xTaskOfHandle[5]);

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

