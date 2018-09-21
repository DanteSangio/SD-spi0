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

#define FREC_TIMER	4	//Frec del timer es la cuarta parte del clock: 96000000/4 - NO CAMBIAR
#define FREC_MATCH0	200	//Cant de veces que interrumpe por seg el match0
#define FREC_MATCH1	100	//Cant de veces que interrumpe por seg el match1
#define TICKRATE_HZ1 (FREC_TIMER*FREC_MATCH0)	// 4: cte de frec - 2 ticks por segundo (cada 500ms)
#define TICKRATE_HZ2 (FREC_TIMER*FREC_MATCH1)	// 4: cte de frec - 1 tick por segundo	(cada 1000ms)


//Placa Infotronic
#define LED_STICK	PORT(0),PIN(22)
#define	BUZZER		PORT(0),PIN(28)
#define	SW1			PORT(2),PIN(10)
#define SW2			PORT(0),PIN(18)
#define	SW3			PORT(0),PIN(11)
#define SW4			PORT(2),PIN(13)
#define SW5			PORT(1),PIN(26)
#define	LED1		PORT(2),PIN(0)
#define	LED2		PORT(0),PIN(23)
#define	LED3		PORT(0),PIN(21)
#define	LED4		PORT(0),PIN(27)
#define	RGBB		PORT(2),PIN(1)
#define	RGBG		PORT(2),PIN(2)
#define	RGBR		PORT(2),PIN(3)
#define EXPANSION0	PORT(2),PIN(7)
#define EXPANSION1	PORT(1),PIN(29)
#define EXPANSION2	PORT(4),PIN(28)
#define EXPANSION3	PORT(1),PIN(23)
#define EXPANSION4	PORT(1),PIN(20)
#define EXPANSION5	PORT(0),PIN(19)
#define EXPANSION6	PORT(3),PIN(26)
#define EXPANSION7	PORT(1),PIN(25)
#define EXPANSION8	PORT(1),PIN(22)
#define EXPANSION9	PORT(1),PIN(19)
#define EXPANSION10	PORT(0),PIN(20)
#define EXPANSION11	PORT(3),PIN(25)
#define EXPANSION12	PORT(1),PIN(27)
#define EXPANSION13	PORT(1),PIN(24)
#define EXPANSION14	PORT(1),PIN(21)
#define EXPANSION15	PORT(1),PIN(18)
#define EXPANSION16	PORT(2),PIN(8)
#define EXPANSION17	PORT(2),PIN(12)
#define EXPANSION18	PORT(0),PIN(16) //RX1 No se puede usar
#define EXPANSION19	PORT(0),PIN(15) //TX1 No se puede usar
#define EXPANSION20	PORT(0),PIN(22) //RTS1
#define EXPANSION21	PORT(0),PIN(17) //CTS1
/*
#define EXPANSION22		PORT
#define	EXPANSION23		PORT
#define EXPANSION24		PORT
*/
#define ENTRADA_ANALOG0 PORT(1),PIN(31)
#define ENTRADA_DIG1	PORT(4),PIN(29)
#define ENTRADA_DIG2	PORT(1),PIN(26)
#define MOSI1			PORT(0),PIN(9)
#define MISO1 			PORT(0),PIN(8)
#define SCK1			PORT(0),PIN(7)
#define SSEL1			PORT(0),PIN(6)

//UART
#define UART_SELECTION 	LPC_UART1
#define IRQ_SELECTION 	UART1_IRQn
#define HANDLER_NAME 	UART1_IRQHandler
#define TXD1	0,15	//TX UART1
#define	RXD1	0,16	//RX UART1

#define UART_SRB_SIZE 32	//S:Send - Transmit ring buffer size
#define UART_RRB_SIZE 1024	//R:Receive - Receive ring buffer size


//SD
#define FILE_READ         '1'
#define FILE_WRITE        '2'
#define FILE_COPY         '3'
#define FILE_DELETE       '4'
#define FILE_LIST         '5'
#define MEMORY_STATICS    '6'


///////////////////////////////////////////////////////////////////////////////////////////////////////////
//Variables globales

SemaphoreHandle_t Semaforo_1;
//SemaphoreHandle_t Semaforo_2;
QueueHandle_t Cola_SD;
QueueHandle_t Cola_2;

///////////////////////////////////////////////////////////////////////////////////////////////////////////
/* uC_StartUp */
void uC_StartUp (void)
{
	Chip_GPIO_Init (LPC_GPIO);
	Chip_GPIO_SetDir (LPC_GPIO, LED_STICK, OUTPUT);
	Chip_IOCON_PinMux (LPC_IOCON, LED_STICK, IOCON_MODE_INACT, IOCON_FUNC0);
	Chip_GPIO_SetDir (LPC_GPIO, BUZZER, OUTPUT);
	Chip_IOCON_PinMux (LPC_IOCON, BUZZER, IOCON_MODE_INACT, IOCON_FUNC0);
	Chip_GPIO_SetDir (LPC_GPIO, RGBB, OUTPUT);
	Chip_IOCON_PinMux (LPC_IOCON, RGBB, IOCON_MODE_INACT, IOCON_FUNC0);
	Chip_GPIO_SetDir (LPC_GPIO, RGBG, OUTPUT);
	Chip_IOCON_PinMux (LPC_IOCON, RGBG, IOCON_MODE_INACT, IOCON_FUNC0);
	Chip_GPIO_SetDir (LPC_GPIO, RGBR, OUTPUT);
	Chip_IOCON_PinMux (LPC_IOCON, RGBR, IOCON_MODE_INACT, IOCON_FUNC0);
	Chip_GPIO_SetDir (LPC_GPIO, LED1, OUTPUT);
	Chip_IOCON_PinMux (LPC_IOCON, LED1, IOCON_MODE_INACT, IOCON_FUNC0);
	Chip_GPIO_SetDir (LPC_GPIO, LED2, OUTPUT);
	Chip_IOCON_PinMux (LPC_IOCON, LED2, IOCON_MODE_INACT, IOCON_FUNC0);
	Chip_GPIO_SetDir (LPC_GPIO, LED3, OUTPUT);
	Chip_IOCON_PinMux (LPC_IOCON, LED3, IOCON_MODE_INACT, IOCON_FUNC0);
	Chip_GPIO_SetDir (LPC_GPIO, LED4, OUTPUT);
	Chip_IOCON_PinMux (LPC_IOCON, LED4, IOCON_MODE_INACT, IOCON_FUNC0);
	Chip_GPIO_SetDir (LPC_GPIO, SW1, INPUT);
	Chip_IOCON_PinMux (LPC_IOCON, SW1, IOCON_MODE_PULLDOWN, IOCON_FUNC0);

	//Salidas apagadas
	Chip_GPIO_SetPinOutLow(LPC_GPIO, LED_STICK);
	Chip_GPIO_SetPinOutHigh(LPC_GPIO, BUZZER);
	Chip_GPIO_SetPinOutLow(LPC_GPIO, RGBR);
	Chip_GPIO_SetPinOutLow(LPC_GPIO, RGBG);
	Chip_GPIO_SetPinOutLow(LPC_GPIO, RGBB);
	Chip_GPIO_SetPinOutLow(LPC_GPIO, LED1);
	Chip_GPIO_SetPinOutLow(LPC_GPIO, LED2);
	Chip_GPIO_SetPinOutLow(LPC_GPIO, LED3);
	Chip_GPIO_SetPinOutLow(LPC_GPIO, LED4);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////
/* xTaskPruebaSD */
static void xTaskPruebaSD(void *pvParameters)
{
	uint8_t i=0;
    char str[]={"Hello World, Writing data to SD CARD using 1789"};

    for(i=0;i<4;i++)
	{
		xQueueSendToBack(Cola_SD, str, portMAX_DELAY);
		vTaskDelay(1000/portTICK_RATE_MS);			//Delay 1 seg
	}
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
    /* PARA ESCRIBIR ARCHIVO */
    do
    {
    srcFilePtr = FILE_Open("datalog.txt",WRITE,&returnStatus);
    }while(srcFilePtr == 0);

    while (1)
	{
		xQueueReceive(Cola_SD,&Receive,portMAX_DELAY);	//Para recibir los datos a guardar
    	while(Receive[i])
		{
		   FILE_PutCh(srcFilePtr,Receive[i++]);
		}
        FILE_PutCh(srcFilePtr,EOF);
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

	///////////////////////////////////////////////////////////////////
    /* PARA ESCRIBIR ARCHIVO */
    /*    do
    {
    srcFilePtr = FILE_Open("datalog.txt",WRITE,&returnStatus);
    }while(srcFilePtr == 0);
    if(srcFilePtr == 0)
    {
    	DEBUGOUT("\n\rFile Opening Failed");
    }
    else
    {
    	DEBUGOUT("\n\rEscribiendo la SD..");
    	while(str[i])
		{
		   FILE_PutCh(srcFilePtr,str[i++]);
		}
        FILE_PutCh(srcFilePtr,EOF);

        FILE_Close(srcFilePtr);
        DEBUGOUT("\n\rData saved to file, closing the file.");
    //}*/

	///////////////////////////////////////////////////////////////////


	vSemaphoreCreateBinary(Semaforo_1);			//Creamos el semaforo
	xSemaphoreTake(Semaforo_1, portMAX_DELAY);	//Tomamos el semaforo

	Cola_SD = xQueueCreate(4, sizeof(char) * 100);	//Creamos una cola para mandar una trama completa
	Cola_2 = xQueueCreate(1, sizeof(uint32_t));	//Creamos una cola de un elemento

	//Creamos las tareas

	xTaskCreate(xTaskPruebaSD, (char *) "xTaskPruebaSD",
				configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL),
				(xTaskHandle *) NULL);
	xTaskCreate(xTaskWriteSD, (char *) "xTaskWriteSD",
				configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 2UL),
				(xTaskHandle *) NULL);
	xTaskCreate(vTaskInicSD, (char *) "vTaskInicSD",
				configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 3UL),
				(xTaskHandle *) NULL);


	/* Start the scheduler */
	vTaskStartScheduler();

	/* Nunca debería arribar aquí */
    return 0;
}

