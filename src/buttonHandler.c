// Обработчик кнопок
// buttonHandler.c
// Рыбаков Д.Н.
//------------------------------------------------------------------------------

#include "buttonHandler.h"
#include "tasks.h"
#include "types.h"

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* Library includes. */
#include "stm32f10x_gpio.h"

//------------------------------------------------------------------------------

#define PORT_BUTTON_UP      GPIOC
#define PIN_BUTTON_UP       GPIO_Pin_6

#define PORT_BUTTON_DOWN    GPIOC
#define PIN_BUTTON_DOWN     GPIO_Pin_8

#define PORT_BUTTON_START   GPIOA
#define PIN_BUTTON_START    GPIO_Pin_8

//------------------------------------------------------------------------------

#define MAX_NUM_BUTTONS 10
#define QUANT_POLL_BUTTON_ON_CLICK 10

//------------------------------------------------------------------------------

typedef struct
{
    GPIO_TypeDef* Port;     // Порт кнопки
    u16 Pin;                // Пин кнопки
    u32 Ix;                 // Индекс кнопки
    u32 State;              // Состояние кнопки
    u32 Cnt;                // счетчик состояния
    u32 Clck;               // Наличие события клика
} button_t;

//------------------------------------------------------------------------------

void PowerLavelUp (void);   // TODO: Пееренести в модуль логики
void PowerLavelDown (void); // TODO: Пееренести в модуль логики
void StartStop (void);

button_t m_buttons[MAX_NUM_BUTTONS];
u32 m_button_cnt = 0;

//------------------------------------------------------------------------------

/**
 * Инициализация кнопки
 * @param GPIOx
 * @param GPIO_Pin
 */
void buttonInit(GPIO_TypeDef* GPIOx, u16 GPIO_Pin)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin;
    GPIO_Init(GPIOx, &GPIO_InitStructure);
}

/**
 * Событие нажатия кнопки
 * @param ind - индекс кнопки
 */
void button_click(u32 ind)
{
    switch (ind)
    {
        case 0: // up
            PowerLavelUp();
            break;
        case 1: // down
            PowerLavelDown();
            break;
        case 2: // start
            StartStop();
            break;
        default:
            break;
    }
}

/**
 * Проверяльщик состояния кнопки
 * @param ind - индекс кнопки
 */
void button_state_ctrl(u32 ind)
{
    button_t *p_bt = &m_buttons[ind];
    u32 c_state = !GPIO_ReadInputDataBit(p_bt->Port, p_bt->Pin);
    if (c_state != p_bt->State)
    {   // Состояние сменилось
        p_bt->Cnt = 0;
        p_bt->Clck = 0;
    }
    else // состояние не меняется считает сколько держиться в текущем состоянии
        ++p_bt->Cnt;
    p_bt->State = c_state;
    if (p_bt->Cnt > QUANT_POLL_BUTTON_ON_CLICK)
    {   // состояние не менялось время превышающее порог (антидребезг)
        if (c_state)
        {   // текущее состояние нажато
            if (!p_bt->Clck)
            {   // клика еще не было
                p_bt->Clck = 1;
                button_click(ind); // вызов обработчика клика
            }
        }
    }
}

/**
 * Задача обработчика кнопок
 * @param pvParameters
 */
void buttonsTask( void *pvParameters )
{
    NO_USE_PARAM(pvParameters);

    while(1)
    {
        u32 ind = 0;
        for (ind = 0; ind < m_button_cnt; ind ++)
        {   // Опрашивает все созданные кнопки
            button_state_ctrl(ind);
        }
        vTaskDelay(1);
    }
}

/**
 * создание кнопки (записи о кнопке)
 * @param GPIOx - порт кнопки
 * @param GPIO_Pin - вывод кнопки
 */
void createButton(GPIO_TypeDef* GPIOx, u16 GPIO_Pin)
{
    buttonInit(GPIOx, GPIO_Pin);
    button_t bt;
    bt.Port = GPIOx;
    bt.Pin = GPIO_Pin;
    bt.State = 0;
    bt.Ix = m_button_cnt;

    if (m_button_cnt < MAX_NUM_BUTTONS)
    {
        m_buttons[m_button_cnt] = bt; // против переполнения массива
        m_button_cnt++;
    }
}


void buttonHandler(void)
{
    // инициализация кнопок
    createButton(PORT_BUTTON_UP, PIN_BUTTON_UP);
    createButton(PORT_BUTTON_DOWN, PIN_BUTTON_DOWN);
    createButton(PORT_BUTTON_START, PIN_BUTTON_START);

    // инициализация задачи
    xTaskCreate( buttonsTask, "buttons", configMINIMAL_STACK_SIZE, NULL, BUTTONS_PRIORITY, NULL );


}


//------------------------------------------------------------------------------
