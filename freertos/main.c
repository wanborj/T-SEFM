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
#include "servant.h"
#include "app.h"

static void setup_hardware( void );

extern struct xParam pvParameters[NUMBEROFSERVANT];

extern xSemaphoreHandle xBinarySemaphore[NUMBEROFSERVANT];  // the network topology
extern xTaskHandle xTaskOfHandle[NUMBEROFSERVANT];         // record the handle of all S-Servant, the last one is for debugging R-Servant

extern portTickType xPeriodOfTask[NUMBEROFTASK];

extern xTaskComplete[NUMBEROFTASK];


/*
struct xParam
{
    portBASE_TYPE xMyFlag;     // the flag of current servant
    portBASE_TYPE xNumOfIn;    // the number of source servants
    portBASE_TYPE xNumOfOut;   // the number of destination servants
    portBASE_TYPE xInFlag[MAXINDEGREE];  // the flags of source servants
    portBASE_TYPE xOutFlag[MAXOUTDEGREE]; // the flags of destination servants
    portTickType xLet; // the xLet of current servant
    pvServantFunType xFp;  // the implementation of current Servant
}; */

#define SERVANT_STACK_SIZE 128 
int main(void)
{
    init_led();
    init_rs232();
    enable_rs232_interrupts();
    enable_rs232();

    vTaskCompleteInitialise();
    vSemaphoreInitialise();
    vParameterInitialise();

    xTaskCreate( vR_Servant, "R-Servant", SERVANT_STACK_SIZE, (void *)&pvParameters[NUMBEROFSERVANT-1],tskIDLE_PRIORITY + 1, &xTaskOfHandle[NUMBEROFSERVANT-1]);

    // task 1, 25ms,
    xTaskCreate( vSensor, "Sensor Of Task 1", SERVANT_STACK_SIZE, (void *)&pvParameters[0], tskIDLE_PRIORITY + 10, &xTaskOfHandle[0]);

    xTaskCreate( vServant, "Servant s_0", SERVANT_STACK_SIZE, (void *)&pvParameters[1], tskIDLE_PRIORITY + 10, &xTaskOfHandle[1]);
    xTaskCreate( vServant, "Servant s_1", SERVANT_STACK_SIZE, (void *)&pvParameters[2], tskIDLE_PRIORITY + 10, &xTaskOfHandle[2]);
    xTaskCreate( vServant, "Servant s_5", SERVANT_STACK_SIZE, (void *)&pvParameters[3], tskIDLE_PRIORITY + 10,&xTaskOfHandle[3]);

    xTaskCreate( vActuator, "Actuator Of Task 1", SERVANT_STACK_SIZE, (void *)&pvParameters[4], tskIDLE_PRIORITY + 10, &xTaskOfHandle[4]);

    // task 2, 50ms, 
    xTaskCreate( vSensor, "Sensor of Task 2", SERVANT_STACK_SIZE, (void *)&pvParameters[5], tskIDLE_PRIORITY + 8, &xTaskOfHandle[5]); 

    xTaskCreate( vServant, "Servant s_6", SERVANT_STACK_SIZE, (void *)&pvParameters[6], tskIDLE_PRIORITY + 8, &xTaskOfHandle[6]);
    xTaskCreate( vServant, "Servant s_7", SERVANT_STACK_SIZE, (void *)&pvParameters[7], tskIDLE_PRIORITY + 8, &xTaskOfHandle[7]);
    xTaskCreate( vServant, "Servant s_2", SERVANT_STACK_SIZE, (void *)&pvParameters[8], tskIDLE_PRIORITY + 8, &xTaskOfHandle[8]);
    xTaskCreate( vServant, "Servant s_4", SERVANT_STACK_SIZE, (void *)&pvParameters[9], tskIDLE_PRIORITY + 8, &xTaskOfHandle[9]);
    xTaskCreate( vServant, "Servant s_3", SERVANT_STACK_SIZE, (void *)&pvParameters[10], tskIDLE_PRIORITY + 8, &xTaskOfHandle[10]);

    xTaskCreate( vActuator, "Actuator of Task 2", SERVANT_STACK_SIZE, (void *)&pvParameters[11], tskIDLE_PRIORITY + 8, &xTaskOfHandle[11]); 

    // task 3, 100ms,
    xTaskCreate( vSensor, "Sensor of Task 4", SERVANT_STACK_SIZE, (void *)&pvParameters[12], tskIDLE_PRIORITY + 6, &xTaskOfHandle[12]); 

    xTaskCreate( vServant, "Servant s_12", SERVANT_STACK_SIZE, (void *)&pvParameters[13], tskIDLE_PRIORITY + 6, &xTaskOfHandle[13]);

    xTaskCreate( vActuator, "Actuator Of Task 4", SERVANT_STACK_SIZE, (void *)&pvParameters[14], tskIDLE_PRIORITY + 6, &xTaskOfHandle[14]);
    
    // task 4, 250ms,
    xTaskCreate( vSensor, "Sensor of Task 3", SERVANT_STACK_SIZE, (void *)&pvParameters[15], tskIDLE_PRIORITY + 4, &xTaskOfHandle[15]); 

    xTaskCreate( vServant, "Sensor s_8", SERVANT_STACK_SIZE, (void *)&pvParameters[16], tskIDLE_PRIORITY + 4, &xTaskOfHandle[16]);
    xTaskCreate( vServant, "Servant s_9", SERVANT_STACK_SIZE, (void *)&pvParameters[17], tskIDLE_PRIORITY + 4, &xTaskOfHandle[17]);
    xTaskCreate( vServant, "Servant s_10", SERVANT_STACK_SIZE, (void *)&pvParameters[18], tskIDLE_PRIORITY + 4,&xTaskOfHandle[18]);
    xTaskCreate( vServant, "Servant s_11", SERVANT_STACK_SIZE, (void *)&pvParameters[19], tskIDLE_PRIORITY + 4,&xTaskOfHandle[19]);

    xTaskCreate( vActuator, "Actuator of Task 3", SERVANT_STACK_SIZE, (void *)&pvParameters[20], tskIDLE_PRIORITY + 4, &xTaskOfHandle[20]); 

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

/* time tick hook which is used to triggered every sensor of corresponding task to execute at
 * specified time according to their period.
 *
 * if there is any task need to be triggered at this time,
 * tick hook function would send semaphore to them.
 * */
void vApplicationTickHook( void )
{
    portTickType xCurrentTime = xTaskGetTickCount();
    if( xCurrentTime < 200 )
    {
        return;
    }

    // xTaskComplete initialise to 1, and it will be set to 0 after sensor start executing and set
    // back to 1 after actuator complete execution.
    if(xTaskComplete[0] == 1 && xCurrentTime % xPeriodOfTask[0] == 0)
    {
        xSemaphoreGive(xBinarySemaphore[0]);
    }
    if(xTaskComplete[1] == 1 && xCurrentTime % xPeriodOfTask[1] == 0)
    {
        xSemaphoreGive(xBinarySemaphore[5]);
    }
    if(xTaskComplete[2] == 1 && xCurrentTime % xPeriodOfTask[2] == 0)
    {
        xSemaphoreGive(xBinarySemaphore[12]);
    }
    if(xTaskComplete[3] == 1 && xCurrentTime % xPeriodOfTask[3] == 0)
    {
        xSemaphoreGive(xBinarySemaphore[15]);
    }
}
