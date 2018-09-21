/*
===============================================================================
 Name        : FRTOS.c
 Author      : $(author)
 Version     :
 Copyright   : $(copyright)
 Description : main definition
===============================================================================
*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//Includes
#include "chip.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include <cr_section_macros.h>
#include <stdlib.h>

//SD
#include "fat32.h"
#include "spi.h"
#include "stdutils.h"
#include "delay.h"
#include "sdcard.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//Defines
#define PORT(x) 	((uint8_t) x)
#define PIN(x)		((uint8_t) x)
/*
#define OUTPUT		((uint8_t) 1)
#define INPUT		((uint8_t) 0)
#define ON			((uint8_t) 1)
#define OFF			((uint8_t) 0)
*/
#define DEBUGOUT(...) //printf(__VA_ARGS__)

#define LED_STICK	PORT(0),PIN(22)

//SD
#define FILE_READ         '1'
#define FILE_WRITE        '2'
#define FILE_COPY         '3'
#define FILE_DELETE       '4'
#define FILE_LIST         '5'
#define MEMORY_STATICS    '6'


///////////////////////////////////////////////////////////////////////////////////////////////////////////
/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

SemaphoreHandle_t Semaforo_RTC;

QueueHandle_t Cola_SD;


/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

static volatile bool fIntervalReached;
static volatile bool fAlarmTimeMatched;
static volatile bool On0, On1;



/*****************************************************************************
 * Private functions
 ****************************************************************************/

/* Gets and shows the current time and date */
static void showTime(RTC_TIME_T *pTime)
{
	DEBUGOUT("Time: %.2d:%.2d %.2d/%.2d/%.4d\r\n",
			 pTime->time[RTC_TIMETYPE_HOUR],
			 pTime->time[RTC_TIMETYPE_MINUTE],
			 pTime->time[RTC_TIMETYPE_DAYOFMONTH],
			 pTime->time[RTC_TIMETYPE_MONTH],
			 pTime->time[RTC_TIMETYPE_YEAR]);
}





///////////////////////////////////////////////////////////////////////////////////////////////////////////
/* uC_StartUp */
void uC_StartUp (void)
{
	Chip_GPIO_Init (LPC_GPIO);
	Chip_GPIO_SetDir (LPC_GPIO, LED_STICK, OUTPUT);
	Chip_IOCON_PinMux (LPC_IOCON, LED_STICK, IOCON_MODE_INACT, IOCON_FUNC0);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////
/* xTaskPruebaSD */
static void xTaskPruebaSD(void *pvParameters)
{
	uint8_t i=0;
    char str[]={"Hello World, Writing data to SD CARD using 1789\n\r"};

    for(i=0;i<4;i++)
	{
		xQueueSendToBack(Cola_SD, str, portMAX_DELAY);
		vTaskDelay(1000/portTICK_RATE_MS);			//Delay 1 seg
	}
    vTaskDelete(NULL);	//Borra la tarea
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
/* vTaskInicSD */
static void vTaskInicSD(void *pvParameters)
{
    uint8_t returnStatus,sdcardType;
///////////////////////////////////////////////////////////////////
	/* PARA COMPROBAR SI LA SD ESTA CONECTADA */

    do //if(returnStatus)
    {
        returnStatus = SD_Init(&sdcardType);
        if(returnStatus == SDCARD_NOT_DETECTED)
        {
        	DEBUGOUT("\n\r SD card not detected..");
        }
        else if(returnStatus == SDCARD_INIT_FAILED)
        {
        	DEBUGOUT("\n\r Card Initialization failed..");
        }
        else if(returnStatus == SDCARD_FAT_INVALID)
        {
        	DEBUGOUT("\n\r Invalid Fat filesystem");
        }
    }while(returnStatus!=0);

    DEBUGOUT("\n\rSD Card Detected!");

    vTaskDelete(NULL);	//Borra la tarea
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////
/* xTaskWriteSD*/
static void xTaskWriteSD(void *pvParameters)
{
    uint8_t returnStatus,i=0;
    fileConfig_st *srcFilePtr;
    char Receive[100];


    while (1)
	{

        /* PARA ESCRIBIR ARCHIVO */
        do
        {
        srcFilePtr = FILE_Open("datalog.txt",WRITE,&returnStatus);
        }while(srcFilePtr == 0);

		xQueueReceive(Cola_SD,&Receive,portMAX_DELAY);	//Para recibir los datos a guardar
    	for(i=0;Receive[i];)
		{
		   FILE_PutCh(srcFilePtr,Receive[i++]);
		}
        FILE_PutCh(srcFilePtr,EOF);
        FILE_Close(srcFilePtr);
        for(i=0;Receive[i];i++)//limpio el vector
        {
        	Receive[i]=0;
        }
	}
	vTaskDelete(NULL);	//Borra la tarea si sale del while 1
}


/**
 * @brief	RTC interrupt handler
 * @return	Nothing
 */
void RTC_IRQHandler(void)
{

	BaseType_t Testigo=pdFALSE;

	/* Interrupcion cada 1 minuto */
	if (Chip_RTC_GetIntPending(LPC_RTC, RTC_INT_COUNTER_INCREASE)) {
		/* Clear pending interrupt */
		Chip_RTC_ClearIntPending(LPC_RTC, RTC_INT_COUNTER_INCREASE);
		xSemaphoreGiveFromISR(Semaforo_RTC, &Testigo);	//Devuelve si una de las tareas bloqueadas tiene mayor prioridad que la actual
		portYIELD_FROM_ISR(Testigo);					//Si testigo es TRUE -> ejecuta el scheduler

	}
}

static void xTaskRTConfig(void *pvParameters)
{
	RTC_TIME_T FullTime;

	SystemCoreClockUpdate();

	DEBUGOUT("PRUEBA RTC..\n");	//Imprimo en la consola

	Chip_RTC_Init(LPC_RTC);

	/* Set current time for RTC 2:00:00PM, 2012-10-05 */

	FullTime.time[RTC_TIMETYPE_SECOND]  = 0;
	FullTime.time[RTC_TIMETYPE_MINUTE]  = 41;
	FullTime.time[RTC_TIMETYPE_HOUR]    = 17;
	FullTime.time[RTC_TIMETYPE_DAYOFMONTH]  = 21;
	FullTime.time[RTC_TIMETYPE_DAYOFWEEK]   = 5;
	FullTime.time[RTC_TIMETYPE_DAYOFYEAR]   = 207;
	FullTime.time[RTC_TIMETYPE_MONTH]   = 9;
	FullTime.time[RTC_TIMETYPE_YEAR]    = 2018;

	Chip_RTC_SetFullTime(LPC_RTC, &FullTime);
	//*/

	/* Set the RTC to generate an interrupt on each second */
	Chip_RTC_CntIncrIntConfig(LPC_RTC, RTC_AMR_CIIR_IMSEC, ENABLE);

	/* Clear interrupt pending */
	Chip_RTC_ClearIntPending(LPC_RTC, RTC_INT_COUNTER_INCREASE);

	/* Enable RTC interrupt in NVIC */
	NVIC_EnableIRQ((IRQn_Type) RTC_IRQn);

	/* Enable RTC (starts increase the tick counter and second counter register) */
	Chip_RTC_Enable(LPC_RTC, ENABLE);

//
	vTaskDelete(NULL);	//Borra la tarea
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
/* vTaskInicTimer */
static void vTaskRTC(void *pvParameters)
{
	RTC_TIME_T FullTime;
	char sec[2],min[2],hora[10],puntos[]=":",coma[]=",";
	while (1)
	{
		xSemaphoreTake(Semaforo_RTC, portMAX_DELAY);
		Chip_RTC_GetFullTime(LPC_RTC, &FullTime);
		//showTime(&FullTime);

		itoa(FullTime.time[RTC_TIMETYPE_SECOND],sec,10);
		itoa(FullTime.time[RTC_TIMETYPE_MINUTE],min,10);
		itoa(FullTime.time[RTC_TIMETYPE_HOUR],hora,10);
		strcat(hora,puntos);
		strcat(hora,min);
		strcat(hora,puntos);
		strcat(hora,sec);
		strcat(hora,coma);
		xQueueSendToBack(Cola_SD, hora, portMAX_DELAY);

	}
	vTaskDelete(NULL);	//Borra la tarea si sale del while
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////
/* main
*/
int main(void)
{
    uC_StartUp ();
    SystemCoreClockUpdate();
    SystemInit();


	Cola_SD = xQueueCreate(4, sizeof(char) * 100);	//Creamos una cola para mandar una trama completa
	vSemaphoreCreateBinary(Semaforo_RTC);			//Creamos el semaforo

	//Creamos las tareas
/*
	xTaskCreate(xTaskPruebaSD, (char *) "xTaskPruebaSD",
				configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL),
				(xTaskHandle *) NULL);*/
	xTaskCreate(xTaskWriteSD, (char *) "xTaskWriteSD",
				configMINIMAL_STACK_SIZE * 2, NULL, (tskIDLE_PRIORITY + 2UL),
				(xTaskHandle *) NULL);
	xTaskCreate(vTaskInicSD, (char *) "vTaskInicSD",
				configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 3UL),
				(xTaskHandle *) NULL);
	xTaskCreate(vTaskRTC, (char *) "vTaskRTC",
				configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL),
				(xTaskHandle *) NULL);
	xTaskCreate(xTaskRTConfig, (char *) "xTaskRTConfig",
				configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 3UL),
				(xTaskHandle *) NULL);
	/* Start the scheduler */
	vTaskStartScheduler();

	/* Nunca debería arribar aquí */
    return 0;
}

