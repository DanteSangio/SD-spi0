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
#define DEBUGOUT(...) printf(__VA_ARGS__)

#define LED_STICK	PORT(0),PIN(22)

//SD
#define FILE_READ         '1'
#define FILE_WRITE        '2'
#define FILE_COPY         '3'
#define FILE_DELETE       '4'
#define FILE_LIST         '5'
#define MEMORY_STATICS    '6'


///////////////////////////////////////////////////////////////////////////////////////////////////////////
//Variables globales

QueueHandle_t Cola_SD;

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



///////////////////////////////////////////////////////////////////////////////////////////////////////////
/* main
*/
int main(void)
{
    uC_StartUp ();
    SystemCoreClockUpdate();
    SystemInit();


	Cola_SD = xQueueCreate(4, sizeof(char) * 100);	//Creamos una cola para mandar una trama completa

	//Creamos las tareas

	xTaskCreate(xTaskPruebaSD, (char *) "xTaskPruebaSD",
				configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL),
				(xTaskHandle *) NULL);
	xTaskCreate(xTaskWriteSD, (char *) "xTaskWriteSD",
				configMINIMAL_STACK_SIZE * 2, NULL, (tskIDLE_PRIORITY + 2UL),
				(xTaskHandle *) NULL);
	xTaskCreate(vTaskInicSD, (char *) "vTaskInicSD",
				configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 3UL),
				(xTaskHandle *) NULL);


	/* Start the scheduler */
	vTaskStartScheduler();

	/* Nunca debería arribar aquí */
    return 0;
}

