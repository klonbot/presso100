// main.c

extern "C"
{
/* Standard includes. */
#include <stdio.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "types.h"
#include "buttonHandler.h"
#include "ledHandler.h"
#include "terminal.h"

#include "pressure.h"
#include "solenoidHendler.h"
#include "initRCC.h"
}

#include "mainLogic.h"

//------------------------------------------------------------------------------

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

    //buttonHandler();    // Обработчик кнопок
    ledHandler();       // Обработчик светодиодов
    //terminalHandler();  // Обработчик терминалки
    mainLogicHandler(); // Главный обработчик логики
    //pressureHandler();  // Обработчик давления
    solenoidHandler();  // Обработчик соленоидов

    // Start the scheduler.
    vTaskStartScheduler();

    // Will only get here if there was not enough heap space to create the
    // idle task.
    return 0;
}


