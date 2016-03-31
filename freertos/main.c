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

//extern xSemaphoreHandle xBinarySemaphore[NUMBEROFSERVANT];  // the network topology
extern xTaskHandle xTaskOfHandle[NUMBEROFSERVANT+1];         // record the handle of all S-Servant, the last one is for debugging R-Servant
//extern portBASE_TYPE xRelation[NUMBEROFSERVANT][NUMBEROFSERVANT]; // record the relationship among servants excluding R-Servant
//extern portTickType xLetOfServant[NUMBEROFSERVANT];  // ms


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

    vSemaphoreInitialise();
    //vPrintString("semaphore initialise\n\r");
    vParameterInitialise();
    //vPrintString("parametere initialise\n\r");

    xTaskCreate( vR_Servant, "R-Servant", 512, NULL, tskIDLE_PRIORITY + 1, &xTaskOfHandle[NUMBEROFSERVANT]);

    // task 1, 25ms, including s_0, s_1, s_5
///*
    xTaskCreate( vSensor, "Sensor s_0", SERVANT_STACK_SIZE, (void *)&pvParameters[0], tskIDLE_PRIORITY + 10, &xTaskOfHandle[0]);

    xTaskCreate( vServant, "Servant s_1", SERVANT_STACK_SIZE, (void *)&pvParameters[1], tskIDLE_PRIORITY + 10, &xTaskOfHandle[1]);

    xTaskCreate( vActuator, "Actuator s_5", SERVANT_STACK_SIZE, (void *)&pvParameters[5], tskIDLE_PRIORITY + 10,&xTaskOfHandle[5]);
 // */

    // task 2, 50ms, including s_6, s_7, s_2, s_4, s_3

///*
    xTaskCreate( vSensor, "Sensor s_6", SERVANT_STACK_SIZE, (void *)&pvParameters[6], tskIDLE_PRIORITY + 8, &xTaskOfHandle[6]);

    xTaskCreate( vServant, "Servant s_7", SERVANT_STACK_SIZE, (void *)&pvParameters[7], tskIDLE_PRIORITY + 8, &xTaskOfHandle[7]);
    xTaskCreate( vServant, "Servant s_2", SERVANT_STACK_SIZE, (void *)&pvParameters[2], tskIDLE_PRIORITY + 8, &xTaskOfHandle[2]);
    xTaskCreate( vServant, "Servant s_4", SERVANT_STACK_SIZE, (void *)&pvParameters[4], tskIDLE_PRIORITY + 8, &xTaskOfHandle[4]);

    xTaskCreate( vActuator, "Actuator s_3", SERVANT_STACK_SIZE, (void *)&pvParameters[3], tskIDLE_PRIORITY + 8,&xTaskOfHandle[3]);

 //*/
    // task 3, 250ms, including s_8, s_9, s_10, s_11

///*
    xTaskCreate( vSensor, "Sensor s_8", SERVANT_STACK_SIZE, (void *)&pvParameters[8], tskIDLE_PRIORITY + 4, &xTaskOfHandle[8]);

    xTaskCreate( vServant, "Servant s_9", SERVANT_STACK_SIZE, (void *)&pvParameters[9], tskIDLE_PRIORITY + 4, &xTaskOfHandle[9]);
    xTaskCreate( vServant, "Servant s_10", SERVANT_STACK_SIZE, (void *)&pvParameters[10], tskIDLE_PRIORITY + 4, &xTaskOfHandle[10]);

    xTaskCreate( vActuator, "Actuator s_11", SERVANT_STACK_SIZE, (void *)&pvParameters[11], tskIDLE_PRIORITY + 4,&xTaskOfHandle[11]);
 // */
    // task 4, 100ms, including s_12, s_13

///*
    xTaskCreate( vSensor, "Sensor s_12", SERVANT_STACK_SIZE, (void *)&pvParameters[12], tskIDLE_PRIORITY + 6, &xTaskOfHandle[12]);

    xTaskCreate( vActuator, "Actuator -s_13", SERVANT_STACK_SIZE, (void *)&pvParameters[13], tskIDLE_PRIORITY + 6,&xTaskOfHandle[13]);
    
 //*/

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

