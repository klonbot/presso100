// Обработчик кнопок
// ledHandler.c
// Рыбаков Д.Н.
//------------------------------------------------------------------------------

#include "ledHandler.h"
#include "tasks.h"
#include "types.h"
#include "outline.h"

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* Library includes. */
#include "stm32f10x_gpio.h"

//------------------------------------------------------------------------------

#define PORT_LED_LEVEL1     GPIOB
#define PIN_LED_LEVEL1      GPIO_Pin_10

#define PORT_LED_LEVEL2     GPIOB
#define PIN_LED_LEVEL2      GPIO_Pin_1

#define PORT_LED_LEVEL3     GPIOC
#define PIN_LED_LEVEL3      GPIO_Pin_5

#define PORT_LED_LEVEL4     GPIOA
#define PIN_LED_LEVEL4      GPIO_Pin_7

#define PORT_LED_LEVEL5     GPIOA
#define PIN_LED_LEVEL5      GPIO_Pin_5

#define PORT_LED_WARNING    GPIOB
#define PIN_LED_WARNING     GPIO_Pin_12

#define PORT_LED_START      GPIOB
#define PIN_LED_START       GPIO_Pin_14

#define PORT_LED_1          GPIOA // Level 5
#define PIN_LED_1           GPIO_Pin_4

#define PORT_LED_2          GPIOA
#define PIN_LED_2           GPIO_Pin_5

#define PORT_LED_3          GPIOA
#define PIN_LED_3           GPIO_Pin_6

#define PORT_LED_D2         GPIOC
#define PIN_LED_D2          GPIO_Pin_13

//------------------------------------------------------------------------------

#define MAX_NUM_LEDS 15

//------------------------------------------------------------------------------

typedef outline_t led_t;

led_t m_leds[MAX_NUM_LEDS];
u32 m_led_cnt = 0;

//------------------------------------------------------------------------------

/**
 * Инициализация светодиода
 * @param GPIOx
 * @param GPIO_Pin
 */
/*
void ledInit(GPIO_TypeDef* GPIOx, u16 GPIO_Pin)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin;
    GPIO_Init(GPIOx, &GPIO_InitStructure);
}
*/

/**
 * Задача обработчика светодиодов
 * @param pvParameters
 */
void ledTask( void *pvParameters )
{
    NO_USE_PARAM(pvParameters);

    while(1)
    {
        u32 ind = 0;
        for (ind = 0; ind < m_led_cnt; ind ++)
        {
            led_t *p_led = &m_leds[ind];
            if (p_led->isNew)
            {
                BitAction ba = (p_led->State) ? Bit_SET : Bit_RESET;
                GPIO_WriteBit(p_led->Port, p_led->Pin, ba);
                p_led->isNew = 0;
            }
        }

        vTaskDelay(20);
    }
}

/**
 * создание светодиода (записи о светодиоде)
 * @param GPIOx - порт
 * @param GPIO_Pin - вывод
 */
void createLed(GPIO_TypeDef* GPIOx, u16 GPIO_Pin, lede_t lede)
{
    outlineInit(GPIOx, GPIO_Pin);
    GPIO_WriteBit(GPIOx, GPIO_Pin, Bit_SET);

    led_t led;
    led.Port = GPIOx;
    led.Pin = GPIO_Pin;
    led.State = 0;
    led.isNew = 0;
    led.Ix = lede;

    if (m_led_cnt < MAX_NUM_LEDS)
    {
        m_leds[m_led_cnt] = led; // против переполнения массива
        m_led_cnt++;
    }
}

void ledHandler(void)
{
    // инициализация светодиодов
    createLed(PORT_LED_LEVEL1, PIN_LED_LEVEL1, led_level1);
    createLed(PORT_LED_LEVEL2, PIN_LED_LEVEL2, led_level2);
    createLed(PORT_LED_LEVEL3, PIN_LED_LEVEL3, led_level3);
    createLed(PORT_LED_LEVEL4, PIN_LED_LEVEL4, led_level4);
    createLed(PORT_LED_LEVEL5, PIN_LED_LEVEL5, led_level5);
    createLed(PORT_LED_WARNING, PIN_LED_WARNING, led_warning);
    createLed(PORT_LED_START, PIN_LED_START, led_start);

    createLed(PORT_LED_1, PIN_LED_1, led_1);
    createLed(PORT_LED_2, PIN_LED_2, led_2);
    createLed(PORT_LED_3, PIN_LED_3, led_3);

    createLed(PORT_LED_D2, PIN_LED_D2, led_D2);

    // инициализация светодиодов
    xTaskCreate( ledTask, "led", configMINIMAL_STACK_SIZE, NULL, LED_PRIORITY, NULL );
}

/**
 * Установка состояния светодиода
 * @param newState
 * @param lede
 */
void setLedState (u32 newState, lede_t lede)
{
    u32 ind = 0;
    for(ind = 0; ind < m_led_cnt; ind++)
    {
        led_t *p_led = &m_leds[ind];
        if (p_led->Ix == lede)
        {
            p_led->State = newState;
            p_led->isNew = 1;
            break;
        }
    }
}
