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

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//Defines
#define PORT(x) 	((uint8_t) x)
#define PIN(x)		((uint8_t) x)
#define OUTPUT		((uint8_t) 1)
#define INPUT		((uint8_t) 0)
#define ON			((uint8_t) 1)
#define OFF			((uint8_t) 0)
#define DEBUGOUT(...) printf(__VA_ARGS__)
#define TICKRATE_HZ1 (40000*2)	// 4: cte de frec - 2 ticks por segundo (cada 500ms)
#define TICKRATE_HZ2 (40000*1)	// 4: cte de frec - 1 tick por segundo	(cada 1000ms)

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

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//Variables globales

SemaphoreHandle_t Semaforo_1;
//SemaphoreHandle_t Semaforo_2;
QueueHandle_t Cola_1;
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
/* xTaskPWM
	Tarea que se encarga de togglear el led
*/
static void xTaskPWM(void *pvParameters)
{
	while (1)
	{
		xSemaphoreTake(Semaforo_1, portMAX_DELAY);
		Chip_GPIO_SetPinToggle(LPC_GPIO,LED3);
	}
	vTaskDelete(NULL);	//Borra la tarea si sale del while 1
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////
/* xTaskLedRGB
*/
static void vTaskLedRGB(void *pvParameters)
{
	uint8_t Receive=OFF;

	while (1)
	{
		xQueueReceive(Cola_2, &Receive, portMAX_DELAY);

		switch(Receive)
		{
			case 1:
				Chip_GPIO_SetPinToggle(LPC_GPIO,LED_STICK);
				Chip_GPIO_SetPinOutLow(LPC_GPIO, RGBG);
				Chip_GPIO_SetPinOutLow(LPC_GPIO, RGBB);
				Chip_GPIO_SetPinOutHigh(LPC_GPIO, RGBR);
			break;

			case 2:
				Chip_GPIO_SetPinToggle(LPC_GPIO,LED_STICK);
				Chip_GPIO_SetPinOutLow(LPC_GPIO, RGBR);
				Chip_GPIO_SetPinOutLow(LPC_GPIO, RGBB);
				Chip_GPIO_SetPinOutHigh(LPC_GPIO, RGBG);
			break;

			case 3:
				Chip_GPIO_SetPinToggle(LPC_GPIO,LED_STICK);
				Chip_GPIO_SetPinOutLow(LPC_GPIO, RGBR);
				Chip_GPIO_SetPinOutLow(LPC_GPIO, RGBB);
				Chip_GPIO_SetPinOutLow(LPC_GPIO, RGBG);
			break;

			default:
			break;
		}
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////
/* xTaskPulsadores */
static void xTaskPulsadores(void *pvParameters)
{
	uint8_t Send=OFF;

	while (1)
	{
		if(Chip_GPIO_GetPinState(LPC_GPIO, SW1)==OFF)	//Si se presiona el SW1
		{
			Send=1;	//Para incrementar
			xQueueSendToBack(Cola_1, &Send, portMAX_DELAY);
			vTaskDelay(1000/portTICK_RATE_MS);			//Delay 1 seg
		}
		if(Chip_GPIO_GetPinState(LPC_GPIO, SW2)==OFF)	//Si se presiona el SW2
		{
			Send=2;	//Para decrementar
			xQueueSendToBack(Cola_1, &Send, portMAX_DELAY);
			vTaskDelay(1000/portTICK_RATE_MS);			//Delay 1 seg
		}
		if(Chip_GPIO_GetPinState(LPC_GPIO, SW4)==OFF)	//Si se presiona el SW4
		{
			Send=1;	//Para led RGBR
			xQueueSendToBack(Cola_2, &Send, portMAX_DELAY);
			vTaskDelay(1000/portTICK_RATE_MS);			//Delay 1 seg
		}
		if(Chip_GPIO_GetPinState(LPC_GPIO, SW5)==OFF)	//Si se presiona el SW5
		{
			Send=2;	//Para led RGBG
			xQueueSendToBack(Cola_2, &Send, portMAX_DELAY);
			vTaskDelay(1000/portTICK_RATE_MS);			//Delay 1 seg
		}
		if(Chip_GPIO_GetPinState(LPC_GPIO, SW3)==OFF)	//Si se presiona el SW3
		{
			Send=3;	//Apaga led RGB
			xQueueSendToBack(Cola_2, &Send, portMAX_DELAY);
			vTaskDelay(1000/portTICK_RATE_MS);			//Delay 1 seg
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
/* vTaskInicTimer */
static void vTaskInicTimer(void *pvParameters)
{
	while (1)
	{
		uint32_t timerFreq;

		/* Enable timer 1 clock */
		Chip_TIMER_Init(LPC_TIMER0);									//Enciende el modulo
		/* Timer rate is system clock rate */
		timerFreq = Chip_Clock_GetSystemClockRate();					//Obtiene la frecuencia a la que esta corriendo el uC
		/* Timer setup for match and interrupt at TICKRATE_HZ */
		Chip_TIMER_Reset(LPC_TIMER0);									//Borra la cuenta

		//MATCH 0: NO RESETEA LA CUENTA
		Chip_TIMER_MatchEnableInt(LPC_TIMER0, 0);						//Habilita interrupcion del match 0 timer 0
		Chip_TIMER_SetMatch(LPC_TIMER0, 0, (timerFreq / TICKRATE_HZ1));	//Le asigna un valor al match - seteo la frec a la que quiero que el timer me interrumpa (Ej 500ms)
		Chip_TIMER_ResetOnMatchDisable(LPC_TIMER0, 0);					//Cada vez que llega al match NO resetea la cuenta

		//MATCH 1: SI RESETEA LA CUENTA
		Chip_TIMER_MatchEnableInt(LPC_TIMER0, 1);						//Habilita interrupcion del match 1 timer 0
		Chip_TIMER_SetMatch(LPC_TIMER0, 1, (timerFreq / TICKRATE_HZ2));	//Le asigna un valor al match - seteo la frec a la que quiero que el timer me interrumpa (Ej 1s)
		Chip_TIMER_ResetOnMatchEnable(LPC_TIMER0, 1);					//Cada vez que llega al match resetea la cuenta

		Chip_TIMER_Enable(LPC_TIMER0);		//Comienza a contar
		/* Enable timer interrupt */ 		//El NVIC asigna prioridades de las interrupciones (prioridad de 0 a inf)
		NVIC_ClearPendingIRQ(TIMER0_IRQn);
		NVIC_EnableIRQ(TIMER0_IRQn);		//Enciende la interrupcion que acabamos de configurar

		vTaskDelete(NULL);	//Borra la tarea, no necesitaria el while(1)
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////
/* xTaskMatch0:
	Tarea que se encarga de controlar el % de tiempo encendido o apagada la salida
*/
static void xTaskMatch0(void *pvParameters)
{
	uint32_t Ticks_Hz_Match0 = 80000;	//El PWM inicializa al 50%
	uint8_t Receive=0;
	uint32_t timerFreq;

	while (1)
	{
		xQueueReceive(Cola_1,&Receive,portMAX_DELAY);	//Para recibir de la Cola_1

		switch(Receive)
		{
			case 1:
				if(Ticks_Hz_Match0>=80000)
				{
					Ticks_Hz_Match0=Ticks_Hz_Match0-40000;	//Incrementa el duty, va a dar cualquier porcentaje
					timerFreq = Chip_Clock_GetSystemClockRate();	//Obtiene la frecuencia a la que esta corriendo el uC
					Chip_TIMER_SetMatch(LPC_TIMER0, 0, (timerFreq / Ticks_Hz_Match0));	//Le cambia el valor al match
					Receive=0;										//Reestablece la variable
				}
			break;

			case 2:
				if(Ticks_Hz_Match0<=1200000)
				{
					Ticks_Hz_Match0=Ticks_Hz_Match0+40000;	//Incrementa el duty, va a dar cualquier porcentaje
					timerFreq = Chip_Clock_GetSystemClockRate();	//Obtiene la frecuencia a la que esta corriendo el uC
					Chip_TIMER_SetMatch(LPC_TIMER0, 0, (timerFreq / Ticks_Hz_Match0));	//Le cambia el valor al match
					Receive=0;										//Reestablece la variable
				}
			break;

			default:
			break;
		}
	}
	vTaskDelete(NULL);	//Borra la tarea si sale del while 1
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////
/* TIMER0_IRQHandler
*/
void TIMER0_IRQHandler(void)
{
	BaseType_t Testigo=pdFALSE;

	if (Chip_TIMER_MatchPending(LPC_TIMER0, 0))
	{
		Chip_TIMER_ClearMatch(LPC_TIMER0, 0);			//Resetea match
		xSemaphoreGiveFromISR(Semaforo_1, &Testigo);	//Devuelve si una de las tareas bloqueadas tiene mayor prioridad que la actual
		portYIELD_FROM_ISR(Testigo);					//Si testigo es TRUE -> ejecuta el scheduler
	}

	if (Chip_TIMER_MatchPending(LPC_TIMER0, 1))
	{
		Chip_TIMER_ClearMatch(LPC_TIMER0, 1);			//Resetea match
		xSemaphoreGiveFromISR(Semaforo_1, &Testigo);	//Devuelve si una de las tareas bloqueadas tiene mayor prioridad que la actual
		portYIELD_FROM_ISR(Testigo);					//Si testigo es TRUE -> ejecuta el scheduler
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////
/* main
*/
int main(void)
{
	uC_StartUp ();

	SystemCoreClockUpdate();

	vSemaphoreCreateBinary(Semaforo_1);			//Creamos el semaforo
	//vSemaphoreCreateBinary(Semaforo_2);
	xSemaphoreTake(Semaforo_1, portMAX_DELAY);	//Tomamos el semaforo
	//xSemaphoreTake(Semaforo_2, portMAX_DELAY);

	Cola_1 = xQueueCreate(1, sizeof(uint32_t));	//Creamos una cola
	Cola_2 = xQueueCreate(1, sizeof(uint32_t));	//Creamos una cola
	//Creamos las tareas
	xTaskCreate(vTaskLedRGB, (char *) "vTaskLedRGB",
				configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL),
				(xTaskHandle *) NULL);
	xTaskCreate(xTaskPulsadores, (char *) "vTaskLedPulsadores",
				configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL),
				(xTaskHandle *) NULL);
	xTaskCreate(xTaskMatch0, (char *) "xTaskMatch0",
				configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 2UL),
				(xTaskHandle *) NULL);
	xTaskCreate(vTaskInicTimer, (char *) "vTaskInicTimer",
				configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 3UL),
				(xTaskHandle *) NULL);
	xTaskCreate(xTaskPWM, (char *) "xTaskPWM",
				configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL),
				(xTaskHandle *) NULL);

	/* Start the scheduler */
	vTaskStartScheduler();

	/* Nunca debería arribar aquí */
    return 0;
}

