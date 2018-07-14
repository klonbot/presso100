// Обработчик терминалки
// ledHandler.c
// Рыбаков Д.Н.
//------------------------------------------------------------------------------

#include "terminal.h"
#include "types.h"
#include "tasks.h"

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "semphr.h"

/* Library includes. */
#include "stm32f10x_lib.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"

#include "serial.h"



//------------------------------------------------------------------------------

xComPortHandle m_serial;
signed char text_start[] = "start!\n\r";

//------------------------------------------------------------------------------
/**
 * Задача отправки данных
 * @param pvParameters
 */
void terminalTxTask( void *pvParameters )
{
    NO_USE_PARAM(pvParameters);

    vSerialPutString(m_serial, text_start, 8 );
    u32 cnt = 0;

    while(1)
    {
        if (!cnt)
            vTaskDelay(1000);

        (void)xSerialPutChar( m_serial, '>', 100 );
        (void)xSerialPutChar( m_serial, cnt+'1', 100 );
        (void)xSerialPutChar( m_serial, '\n', 100 );
        (void)xSerialPutChar( m_serial, '\r', 100 );

        cnt++;
        cnt = cnt%9;

        vTaskDelay(100);
    }
}

signed char text_rx[] = "Rx\n\r";
//------------------------------------------------------------------------------
/**
 * Задача приема данных
 * @param pvParameters
 */
void terminalRxTask( void *pvParameters )
{
    NO_USE_PARAM(pvParameters);

    while(1)
    {
        //vSerialPutString(m_serial, text_rx, 4 );
        signed char RxedChar;
        if (xSerialGetChar(m_serial, &RxedChar, 100))
        {
            xSerialPutChar( m_serial, RxedChar, 100);
        }
		vTaskDelay(1);
    }

}

//Структуры для инициализации GPIOA и USART1
GPIO_InitTypeDef    GPIO_InitStructure;
USART_InitTypeDef   USART_InitStructure;
NVIC_InitTypeDef NVIC_InitStructure;

void serialInit(void)
{
    m_serial = xSerialPortInitMinimal( 9600, 1000 );
}

void terminalHandler(void)
{
    // Инициализация соединения
    serialInit();

    //xTaskCreate( terminalTxTask, "terminalTx", configMINIMAL_STACK_SIZE, NULL, TERMINAL_PRIORITY, NULL );
    xTaskCreate( terminalRxTask, "terminalRx", configMINIMAL_STACK_SIZE, NULL, TERMINAL_PRIORITY, NULL );
}


//------------------------------------------------------------------------------
