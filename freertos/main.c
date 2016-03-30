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

static void setup_hardware( void );

struct xParam pvParameters[NUMBEROFSERVANT];

xSemaphoreHandle xBinarySemaphore[NUMBEROFSERVANT];  // the network topology
xTaskHandle xTaskOfHandle[NUMBEROFSERVANT+1];         // record the handle of all S-Servant, the last one is for debugging R-Servant

// record the relationship among servants excluding R-Servant
portBASE_TYPE xRelation[NUMBEROFSERVANT][NUMBEROFSERVANT] = { 0, 1, 0, 0, 0, 0,
                                                              0, 0, 1, 1, 0, 0,
                                                              0, 0, 0, 0, 1, 0,
                                                              0, 0, 0, 0, 0, 1,
                                                              0, 0, 0, 0, 0, 1,
                                                              0, 0, 0, 0, 0, 0};

// the LET of all S-Servant
portTickType xLetOfServant[NUMBEROFSERVANT] = { 100, 100, 100, 100, 100, 100 };


/*
struct xParam
{
    portBASE_TYPE xMyFlag;     // the flag of current servant
    portBASE_TYPE xNumOfIn;    // the number of source servants
    portBASE_TYPE xNumOfOut;   // the number of destination servants
    portBASE_TYPE xInFlag[MAXINDEGREE];  // the flags of source servants
    portBASE_TYPE xOutFlag[MAXOUTDEGREE]; // the flags of destination servants
    portTickType xLet; // the xLet of current servant
}; */

int main(void)
{
    init_led();
    init_rs232();
    enable_rs232_interrupts();
    enable_rs232();

    vSemaphoreInitialise();
    vRelationInitialise();

    xTaskCreate( vR_Servant, "R Servant", 512, NULL, tskIDLE_PRIORITY + 1, &xTaskOfHandle[6]);

    xTaskCreate( vSensor, "Sensor Servant", 512, (void *)&pvParameters[0], tskIDLE_PRIORITY + 10, &xTaskOfHandle[0]);

    xTaskCreate( vServant, "Servant No.1", 512 /* stack size */, (void *)&pvParameters[1], tskIDLE_PRIORITY + 6, &xTaskOfHandle[1]);
    xTaskCreate( vServant, "Servant No.2", 512 /* stack size */, (void *)&pvParameters[2], tskIDLE_PRIORITY + 5, &xTaskOfHandle[2]);
    xTaskCreate( vServant, "Servant No.3", 512 /* stack size */, (void *)&pvParameters[3], tskIDLE_PRIORITY + 4, &xTaskOfHandle[3]);
    xTaskCreate( vServant, "Servant No.4", 512 /* stack size */, (void *)&pvParameters[4], tskIDLE_PRIORITY + 3, &xTaskOfHandle[4]);

    xTaskCreate( vActuator, "Actuator Servant", 512 /* stack size */, (void *)&pvParameters[5], tskIDLE_PRIORITY + 2,&xTaskOfHandle[5]);

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

