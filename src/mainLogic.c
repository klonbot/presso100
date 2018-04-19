// Главный обработчик логики
// mainLogic.c
// Рыбаков Д.Н.
//------------------------------------------------------------------------------

#include "mainLogic.h"
#include "tasks.h"
#include "types.h"
#include "ledHandler.h"
#include "report.h"
#include "solenoidHendler.h"
#include "pressure.h"

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* Library includes. */
#include "stm32f10x_gpio.h"

//------------------------------------------------------------------------------

#define TIME_OF_CYKLE 2 // время цикла
#define SECOND_COEFF (1000/TIME_OF_CYKLE)
#define SECOND_TO_CYKLE(x) (x*SECOND_COEFF)

//------------------------------------------------------------------------------

typedef enum
{
    cm_readyuse = 0,    // Готов к использованию
    cm_error,           // ошибка
    cm_pumping,         // Накачка
    cm_working,         // работа режима
} current_mode_t;       // текущий режим

//------------------------------------------------------------------------------

volatile unsigned int PowerLevel = 0;   // уровень мощности

current_mode_t m_current_mode = cm_readyuse;

u32 m_pumpState = 0;
u32 m_minivalveState = 0;
u32 m_reliefvalveState = 0;
u32 m_mode = 1;                 // режим процедуры

u32 m_cycles_cnt = 0;
u32 m_pumping_cnt = 0;          // счетчик накачки
u32 m_work_cycle = 0;           // цикл работы
u32 m_pulse_mode_cnt = 0;       // счетчик импульсного режима


//------------------------------------------------------------------------------
/**
 * Вывод уровня мощности
 * @param PowerLavel - уровень мощности
 */
void setLedsPowerLavel(u32 PowerLavel)
{
    setLedState((1), led_level1);
    setLedState((PowerLavel > 0), led_level2);
    setLedState((PowerLavel > 1), led_level3);
    setLedState((PowerLavel > 2), led_level4);
    setLedState((PowerLavel > 3), led_level5);
}

//------------------------------------------------------------------------------
/**
 * Проверка что рабочие режимы
 * @return
 */
u32 isWorkingMode(void)
{
    u32 isWrk = 0;
    if((m_current_mode == cm_pumping)||(m_current_mode == cm_working))
        isWrk = 1;
    return isWrk;
}

//------------------------------------------------------------------------------
/**
 * Нажатие кнопки повышения
 */
void PowerLavelUp (void)
{
    if (isWorkingMode())
    {
        if (PowerLevel < 4)
            PowerLevel++;
        setLedsPowerLavel(PowerLevel);
    }
    else
    {
        m_mode = 1;
    }
}

//------------------------------------------------------------------------------
/**
 * Нажатие кнопки понижения
 */
void PowerLavelDown (void)
{
    if (isWorkingMode())
    {
        if (PowerLevel > 0 )
            PowerLevel--;
        setLedsPowerLavel(PowerLevel);
    }
    else
    {
        m_mode = 0;
    }
}

//------------------------------------------------------------------------------
/**
 * Нажатие кнопки запуска/остановки
 */
void StartStop (void)
{
    m_cycles_cnt = 0;
    PowerLevel = 0;
    switch(m_current_mode)
    {
        case cm_readyuse:
            m_current_mode = cm_pumping;
            break;
        case cm_error:
            m_current_mode = cm_readyuse;
            break;
        case cm_pumping:
            m_current_mode = cm_readyuse;
            break;
        case cm_working:
            m_current_mode = cm_readyuse;
            break;
        default:
            // внештатная ситуация
            m_current_mode = cm_readyuse;
            break;
    }
}

//------------------------------------------------------------------------------
/**
 * Получение максимального значения давления
 * @return максимальное значение давления
 */
u32 getMaxPressure(void)
{
    u32 maxPressure = 0;
    switch(PowerLevel)
    {
        case 0:
            maxPressure = (0 == m_mode) ? 140 : 140;
            break;
        case 1:
            maxPressure = (0 == m_mode) ? 160 : 150;
            break;
        case 2:
            maxPressure = (0 == m_mode) ? 180 : 160;
            break;
        case 3:
            maxPressure = (0 == m_mode) ? 200 : 170;
            break;
        case 4:
            maxPressure = (0 == m_mode) ? 220 : 180;
            break;
    };
    return maxPressure;
}

//------------------------------------------------------------------------------
/**
 * Обработчик накачки
 */
void pumping_handler(void)
{
    u32 pressure = getPressure();
    u32 maxPressure = getMaxPressure();
    if(pressure < maxPressure)
    {
        m_pumpState = 1;        // включение помпы
        m_minivalveState = 0;   // открытие минираспределение на накачку
        m_reliefvalveState = 1; // закрытие клапана сброса
        m_pumping_cnt ++;
    }
    else
    {
        m_current_mode = cm_working;
        m_cycles_cnt = 0;
        m_work_cycle = 1; // начинает с режима накачки
        //m_pumping_cnt = 0;
    }
    // TODO: добавить ограничение по времени накачки 40 секунд
    if (m_cycles_cnt > SECOND_TO_CYKLE(40)) // 40 секунд
    {
        m_current_mode = cm_error;
        m_cycles_cnt = 0;
    }
}

//------------------------------------------------------------------------------
/**
 * Обработчик рабочих режимов
 */
void working_handler(void)
{
    u32 pressure = getPressure();
    u32 maxPressure = getMaxPressure();
    if (m_work_cycle)
    {   // цикл накачки
        if(pressure < maxPressure)
        //if (m_pumping_cnt<10)
        {
            if (0 == m_mode)
            {   // Обычный режим накачки
                m_pumpState = 1;        // включение помпы
                m_minivalveState = 0;   // открытие минираспределение на накачку
                m_reliefvalveState = 1; // закрытие клапана сброса
            }
            if (1 == m_mode)
            {   // Импульсный режим накачки
                // накачка сменяется сбросом
                ++m_pulse_mode_cnt;
                m_pulse_mode_cnt = m_pulse_mode_cnt%5;
                if (m_pulse_mode_cnt > 1)
                { // 3
                    m_minivalveState = 0;   // открытие минираспределение на накачку
                }
                else
                { // 2
                    m_minivalveState = 1;   // закрытие минираспределение на накачку
                }
                m_pumpState = 1;        // включение помпы
                m_reliefvalveState = 1; // закрытие клапана сброса
            }
            ++m_pumping_cnt;        // считает сколько циклов качал, чтобы столько же сбрасывать
        }
        else
        {
            m_work_cycle = !m_work_cycle;
            m_cycles_cnt = 0;
        }
    }
    else
    { // цикл сброса
        u32 relax_decim = (0 == m_mode) ? 2 : 4;
        if (pressure > maxPressure)
        {
            m_pumpState = 0;        // выключение помпы
            m_minivalveState = 1;   // открытие минираспределение на сброс
            m_reliefvalveState = 0; // открытие клапана сброса на сброс
        }
        else
        {
            m_work_cycle = !m_work_cycle;
            m_cycles_cnt = 0;
            m_pumping_cnt = 0;
        }
    }

    u32 second_limit = (0 == m_mode) ?  25 : 50;
    if (m_cycles_cnt > SECOND_TO_CYKLE(second_limit)) // 25 секунд
    {
        m_current_mode = cm_error;
        m_cycles_cnt = 0;
    }
}

//------------------------------------------------------------------------------
/**
 * Коммутатор режимов
 */
void modeSwitch(void)
{
    switch (m_current_mode)
    {
        case cm_readyuse:
            break;
        case cm_error:
            break;
        case cm_pumping:
            pumping_handler();
            break;
        case cm_working:
            working_handler();
            break;
    };
}

//------------------------------------------------------------------------------
/**
 * Выставление состояние соленоидов на безопасное в начале каждого цикла обработки
 */
void solenoiduUnset(void)
{
    m_pumpState = 0;            // выключение помпы
    m_minivalveState = 1;       // закрытие минираспределение на накачку, открытие на сброс
    m_reliefvalveState = 0;     // открытие клапана сброса на сброс
}

//------------------------------------------------------------------------------
/**
 * Управление солиноидами
 */
void solenoidControl(void)
{
    setSolenoidState (m_minivalveState, MINIVALVE);
    setSolenoidState (m_reliefvalveState, RELIEFVALVE);
    setSolenoidState (0, sol_chnl1);
    setSolenoidState (m_pumpState, PUMP);
}

//------------------------------------------------------------------------------
/**
 * Мигание выбранным светодиодом
 * @param ledInd - индекс светодиода
 */
void blinkLed(u32 ledInd)
{
    static u32 blink_cnt = 0;
    static u32 state = 0;

    setLedState(0, led_level1);
    setLedState(0, led_level2);
    setLedState(0, led_level3);
    setLedState(0, led_level4);
    setLedState(0, led_level5);

    setLedState(state, (lede_t)((u32)led_level1 + ledInd));

    blink_cnt++;
    blink_cnt = blink_cnt%60;
    if (!blink_cnt)
        state = !state;
}

void ledD2Control(void)
{
    static u32 blink_cnt = 0;
    static u32 state = 0;

    setLedState(state, led_D2);

    blink_cnt++;
    blink_cnt = blink_cnt%60;
    if (!blink_cnt)
        state = !state;
}

//------------------------------------------------------------------------------
/**
 * Управление светодиодами
 */
void ledControl(void)
{
    u32 led_start_state = 0;
    u32 led_warning_state = 0;


    switch (m_current_mode)
    {
        case cm_readyuse:
            led_start_state = 0;
            break;
        case cm_error:
            led_warning_state = 1;
            break;
        case cm_pumping:
            led_start_state = 1;
            break;
        case cm_working:
            led_start_state = 1;
            break;
    };
    setLedState (led_start_state, led_start);
    setLedState (led_warning_state, led_warning);

    if(isWorkingMode())
        setLedsPowerLavel(PowerLevel);
    else
        blinkLed(m_mode); // мигание светодиода выбранного режима

    ledD2Control();
}

//------------------------------------------------------------------------------
/**
 * главная задача логики
 * @param pvParameters
 */
void mainLogicTask( void *pvParameters )
{
    NO_USE_PARAM(pvParameters);

    vTaskDelay(1000);

    setLedState (0, led_level1);
    setLedState (0, led_level2);
    setLedState (0, led_level3);
    setLedState (0, led_level4);
    setLedState (0, led_level5);
    setLedState (0, led_warning);
    setLedState (0, led_start);

    setLedsPowerLavel(PowerLevel);

    while(1)
    {
        // В выключеном состоянии переключение режима
        // В во включеном состоянии управление мощностью

        solenoiduUnset();
        modeSwitch();       // Коммутатор режимов
        solenoidControl();  // управление соленоидами
        ledControl();       // управление светодиодами

        ++m_cycles_cnt;          // счетчик циклов
        vTaskDelay(TIME_OF_CYKLE);
    }
}

//------------------------------------------------------------------------------

void mainLogicHandler(void)
{

    xTaskCreate( mainLogicTask, "buttons", configMINIMAL_STACK_SIZE, NULL, MAINLOGIC_PRIORITY, NULL );
}




//------------------------------------------------------------------------------
