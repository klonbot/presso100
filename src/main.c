// main.c

/* Standard includes. */
#include <stdio.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* Library includes. */
#include "stm32f10x_it.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_map.h"

#include "types.h"
#include "buttonHandler.h"
#include "ledHandler.h"
#include "terminal.h"
#include "mainLogic.h"
#include "pressure.h"
#include "solenoidHendler.h"


#include "stm32f10x_rcc.h"
#include "stm32f10x_map.h"

#include "report.h"

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------

void initRCC (void)
{
    /* Start with the clocks in their expected state. */
    RCC_DeInit();

    /* Enable HSE (high speed external clock). */
    RCC_HSEConfig( RCC_HSE_ON );

    /* Wait till HSE is ready. */
    while( RCC_GetFlagStatus( RCC_FLAG_HSERDY ) == RESET )
    {
    }

    /* 2 wait states required on the flash. */
    *( ( unsigned long * ) 0x40022000 ) = 0x02;

    /* HCLK = SYSCLK */
    RCC_HCLKConfig( RCC_SYSCLK_Div1 );

    /* PCLK2 = HCLK */
    RCC_PCLK2Config( RCC_HCLK_Div1 );

    /* PCLK1 = HCLK/2 */
    RCC_PCLK1Config( RCC_HCLK_Div1 );

    /* PLLCLK = 8MHz * 9 = 72 MHz. */
    RCC_PLLConfig( RCC_PLLSource_HSE_Div1, RCC_PLLMul_9 );

    /* Enable PLL. */
    RCC_PLLCmd( ENABLE );

    /* Wait till PLL is ready. */
    while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
    {
    }

    /* Select PLL as system clock source. */
    RCC_SYSCLKConfig( RCC_SYSCLKSource_PLLCLK );

    /* Wait till PLL is used as system clock source. */
    while( RCC_GetSYSCLKSource() != 0x08 )
    {
    }

     // Enable GPIO clock and release reset.
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE);
    RCC_APB2PeriphResetCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, DISABLE);
}

static void prvSetupHardware( void )
{
    initRCC();
}

//------------------------------------------------------------------------------
int main( void )
{
#ifdef DEBUG
    debug();
#endif

    prvSetupHardware();

    buttonHandler();    // Обработчик кнопок
    ledHandler();       // Обработчик светодиодов
    terminalHandler();  // Обработчик терминалки
    mainLogicHandler(); // Главный обработчик логики
    pressureHandler();  // Обработчик давления
    solenoidHandler();  // Обработчик соленоидов

    // Start the scheduler.
    vTaskStartScheduler();

    // Will only get here if there was not enough heap space to create the
    // idle task.
    return 0;
}

//------------------------------------------------------------------------------
#ifdef  DEBUG
/* Keep the linker happy. */
void assert_failed( unsigned char* pcFile, unsigned long ulLine )
{
	for( ;; )
	{
	}
}
#endif
