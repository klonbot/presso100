// Обработчик кнопок
// solenoidHandler.c
// Рыбаков Д.Н.
//------------------------------------------------------------------------------

#include "solenoidHendler.h"
#include "tasks.h"
#include "types.h"
#include "outline.h"

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* Library includes. */
#include "stm32f10x_gpio.h"

/*void outlineInit(GPIO_TypeDef* GPIOx, u16 GPIO_Pin)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin;
    GPIO_Init(GPIOx, &GPIO_InitStructure);
}*/
//
#define PORT_CHANNEL1     GPIOB
#define PIN_CHANNEL1      GPIO_Pin_15
//
#define PORT_CHANNEL2     GPIOB
#define PIN_CHANNEL2      GPIO_Pin_14
//
#define PORT_CHANNEL3     GPIOB
#define PIN_CHANNEL3      GPIO_Pin_13
//
#define PORT_CHANNEL4     GPIOB
#define PIN_CHANNEL4      GPIO_Pin_12

//------------------------------------------------------------------------------

#define MAX_NUM_SOLENOIDS 4

//------------------------------------------------------------------------------

typedef outline_t solenoid_t;

solenoid_t m_solenoids[MAX_NUM_SOLENOIDS];
u32 m_solenoid_cnt = 0;

//------------------------------------------------------------------------------
/**
 * Задача обработчика светодиодов
 * @param pvParameters
 */
void solenoidTask( void *pvParameters )
{
    NO_USE_PARAM(pvParameters);

    while(1)
    {
        u32 ind = 0;
        for (ind = 0; ind < m_solenoid_cnt; ind ++)
        {
            solenoid_t *p_solenoid = &m_solenoids[ind];
            if (p_solenoid->isNew)
            {
                BitAction ba_set = (p_solenoid->invertLogic) ? Bit_RESET : Bit_SET;
                BitAction ba_reset = (p_solenoid->invertLogic) ? Bit_SET : Bit_RESET;
                BitAction ba = (p_solenoid->State) ? ba_set : ba_reset;
                GPIO_WriteBit(p_solenoid->Port, p_solenoid->Pin, ba);
                p_solenoid->isNew = 0;
            }
        }

        vTaskDelay(8);
    }
}

/**
 * создание соленоида (записи о соленоиде)
 * @param GPIOx - порт
 * @param GPIO_Pin - вывод
 */
void createSolenoid(GPIO_TypeDef* GPIOx, u16 GPIO_Pin, solenoide_t solenoide, bool invertLogic)
{
    outlineInit(GPIOx, GPIO_Pin);
    BitAction initState = (invertLogic) ? Bit_SET : Bit_RESET;
    GPIO_WriteBit(GPIOx, GPIO_Pin, initState);

    solenoid_t solenoid;
    solenoid.Port = GPIOx;
    solenoid.Pin = GPIO_Pin;
    solenoid.State = 0;
    solenoid.isNew = 0;
    solenoid.Ix = solenoide;
    solenoid.invertLogic = invertLogic;

    if (m_solenoid_cnt < MAX_NUM_SOLENOIDS)
    {
        m_solenoids[m_solenoid_cnt] = solenoid; // против переполнения массива
        m_solenoid_cnt++;
    }
}

void solenoidHandler(void)
{
    createSolenoid(PORT_CHANNEL1, PIN_CHANNEL1, sol_chnl1, Bit_RESET);  // реле красный
    createSolenoid(PORT_CHANNEL2, PIN_CHANNEL2, sol_chnl2, Bit_SET);  // реле зеленый
    createSolenoid(PORT_CHANNEL3, PIN_CHANNEL3, sol_chnl3, Bit_SET);    // помпа
    createSolenoid(PORT_CHANNEL4, PIN_CHANNEL4, sol_chnl4, Bit_SET);    // минираспределитель

    xTaskCreate( solenoidTask, "buttons", configMINIMAL_STACK_SIZE, NULL, SOLENOID_PRIORITY, NULL );
}

/**
 * Установка состояния соленоида
 * @param newState
 * @param lede
 */
void setSolenoidState (u32 newState, solenoide_t solenoide)
{
    u32 ind = 0;
    for(ind = 0; ind < m_solenoid_cnt; ind++)
    {
        solenoid_t *p_sol = &m_solenoids[ind];
        if (p_sol->Ix == solenoide)
        {
            p_sol->State = newState;
            p_sol->isNew = 1;
            break;
        }
    }
}
