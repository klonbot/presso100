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

#include "workModeRotator.h"

//------------------------------------------------------------------------------


//------------------------------------------------------------------------------

typedef enum
{
    cm_readyuse = 0,    // Готов к использованию
    cm_error,           // ошибка
    cm_pumping,         // Накачка
    cm_working,         // работа режима
} current_mode_t;       // текущий режим

typedef enum
{
    workMode_presso = 0,
    workMode_vibro = 1,
    workMode_vibroTime = 2,
    workMode_rotator = 3,

    workMode_num
} workMode_t;

typedef enum
{
    vibroTimeStep_vibroPumping,
    vibroTimeStep_relief,

    vibroTimeStep_num
} vibroTimeStep_t;

//------------------------------------------------------------------------------

volatile unsigned int PowerLevel = 3;   // уровень мощности

current_mode_t m_current_mode = cm_working;

u32 m_pumpLine[pneumoCnl_num];
u32 m_pumpValveLine[pneumoCnl_num];
u32 m_reliefValveLine[pneumoCnl_num];

workMode_t m_workMode = workMode_rotator;
static u32 m_cycles_cnt = 0;
u32 m_pumping_cnt = 0;          // счетчик накачки
u32 m_work_cycle = 0;           // цикл работы
u32 m_pulse_mode_cnt = 0;       // счетчик импульсного режима

vibroTimeStep_t m_vibroTimeStep = vibroTimeStep_relief;

//------------------------------------------------------------------------------
/**
 * Управление состоянием помпы
 * @param cnl
 * @param pumpState
 */
void setPumpState(pneumoCnl_t cnl, pumpState_t pumpState)
{
    m_pumpLine[cnl] = (u32)pumpState;
}

//------------------------------------------------------------------------------
/**
 * Управление состоянием клапаном накачки
 * @param cnl
 * @param pumpState
 */
void setPumpValveState(pneumoCnl_t cnl, pumpValveState_t pumpValveState)
{
    m_pumpValveLine[cnl] = (u32)pumpValveState;
}

//------------------------------------------------------------------------------
/**
 * Управление состоянием клапаном сброса
 * @param cnl
 * @param reliefValveState
 */
void setReliefValveState(pneumoCnl_t cnl, reliefValveState_t reliefValveState)
{
    m_reliefValveLine[cnl] = (u32)reliefValveState;
}

//------------------------------------------------------------------------------
/**
 * Установка состояния сразу всего пневматического канала
 * @param cnl
 * @param pumpState
 * @param pumpValveState
 * @param reliefValveState
 */
void setPneumoChannelState(pneumoCnl_t cnl, pumpState_t pumpState,
    pumpValveState_t pumpValveState, reliefValveState_t reliefValveState)
{
    setPumpState(cnl, pumpState);
    setPumpValveState(cnl, pumpValveState);
    setReliefValveState(cnl, reliefValveState);
}

//------------------------------------------------------------------------------
/**
 * Простая накачка
 * @param cnl
 */
void setPneumoChannelPumpOn(pneumoCnl_t cnl)
{
    setPneumoChannelState(cnl, pumpState_On,
        pumpValveState_Open, reliefValveState_Close); // надувается
}

//------------------------------------------------------------------------------
/**
 * Простой сдув
 * @param cnl
 */
void setPneumoChannelPumpOut(pneumoCnl_t cnl)
{
    setPneumoChannelState(cnl, pumpState_Off,
        pumpValveState_Close, reliefValveState_Open); // спускается
}

//------------------------------------------------------------------------------
/**
 * Удержание давления
 * @param cnl
 */
void setPneumoChannelHold(pneumoCnl_t cnl)
{
    setPneumoChannelState(cnl, pumpState_Off,
        pumpValveState_Open, reliefValveState_Close); // удерживается
}

//------------------------------------------------------------------------------
/**
 * Вибрационное наполнение
 * @param cnl
 */
void setPneumoChannelVibro(pneumoCnl_t cnl)
{
    ++m_pulse_mode_cnt;
    m_pulse_mode_cnt = m_pulse_mode_cnt%5;
    pumpValveState_t  pumpValveState = pumpValveState_Close;
    if (m_pulse_mode_cnt > 1)
    { // 3
        // открытие минираспределение на накачку
        pumpValveState = pumpValveState_Open;
    }
    else
    { // 2
        // закрытие минираспределение на накачку
        pumpValveState = pumpValveState_Close;
    }
    // включение помпы
    // закрытие клапана сброса
    setPneumoChannelState(cnl, pumpState_On,
        pumpValveState, reliefValveState_Close);
}

//------------------------------------------------------------------------------
/**
 * Установка Режима работы канала
 * @param cnl
 * @param state
 */
void setPneumoChannelStateTogether(pneumoCnl_t cnl, channelState_t state)
{
    switch (state)
    {
    case channelState_PumpOut:
        setPneumoChannelPumpOut(cnl);
        break;
    case channelState_PumpOn:
        setPneumoChannelPumpOn(cnl);
        break;
    case channelState_Hold:
        setPneumoChannelHold(cnl);
        break;
    case channelState_Vibro:
        setPneumoChannelVibro(cnl);
        break;
    }
}

//------------------------------------------------------------------------------
/**
 * Вывод уровня мощности
 * @param PowerLavel - уровень мощности
 */
void setLedsPowerLavel(u32 PowerLavel)
{
/*
    setLedState((1), led_level1);
    setLedState((PowerLavel > 0), led_level2);
    setLedState((PowerLavel > 1), led_level3);
    setLedState((PowerLavel > 2), led_level4);
    setLedState((PowerLavel > 3), led_level5);
*/
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
        if (m_workMode < (workMode_t)((u32)workMode_num-1))
        {
            m_workMode = (workMode_t)((u32)(m_workMode)+1);
        }
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
        if (m_workMode != (workMode_t)0)
        {
            m_workMode = (workMode_t)((u32)m_workMode-1);
        }
    }
}

void Start(void)
{
    if (m_current_mode == cm_readyuse)
    {
        StartStop ();
    }
}

void Stop(void)
{
    if (m_current_mode != cm_readyuse)
    {
        StartStop ();
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
    m_vibroTimeStep = vibroTimeStep_vibroPumping;
    switch(m_current_mode)
    {
        case cm_readyuse:
            switch (m_workMode)
            {
            case workMode_presso:
            case workMode_vibro:
                m_current_mode = cm_pumping;
                break;
            case workMode_vibroTime:
            case workMode_rotator:
                m_current_mode = cm_working;
                break;

                
            }
            break;
        case cm_error:
        case cm_pumping:
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
            maxPressure = (0 == m_workMode) ? 120 : 120;
            break;
        case 1:
            maxPressure = (0 == m_workMode) ? 125 : 125;
            break;
        case 2:
            maxPressure = (0 == m_workMode) ? 130 : 130;
            break;
        case 3:
            maxPressure = (0 == m_workMode) ? 135 : 135;
            break;
        case 4:
            maxPressure = (0 == m_workMode) ? 140 : 140;
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
        // включение помпы
        // открытие минираспределение на накачку
        // закрытие клапана сброса
        setPneumoChannelState(pneumoCnl_1, pumpState_On,
            pumpValveState_Open, reliefValveState_Close);
        setPneumoChannelState(pneumoCnl_2, pumpState_On,
            pumpValveState_Open, reliefValveState_Close);
        setPneumoChannelState(pneumoCnl_3, pumpState_On,
            pumpValveState_Open, reliefValveState_Close);
        setPneumoChannelState(pneumoCnl_4, pumpState_On,
            pumpValveState_Open, reliefValveState_Close);

        m_pumping_cnt ++;
    }
    else
    {
        m_current_mode = cm_working;
        m_cycles_cnt = 0;
        m_work_cycle = 1; // начинает с режима накачки
        //m_pumping_cnt = 0;
    }
    // TODO: добавить ограничение по времени накачки
    if (m_cycles_cnt > SECOND_TO_CYKLE(120))
    {
        m_current_mode = cm_error;
        m_cycles_cnt = 0;
    }
}

//------------------------------------------------------------------------------
/**
 * Обработчик импульсной накачки
 */
void pulsePumping_handler(void)
{
    ++m_pulse_mode_cnt;
    m_pulse_mode_cnt = m_pulse_mode_cnt%5;
    pumpValveState_t  pumpValveState = pumpValveState_Close;
    if (m_pulse_mode_cnt > 1)
    { // 3
        // открытие минираспределение на накачку
        pumpValveState = pumpValveState_Open;
    }
    else
    { // 2
        // закрытие минираспределение на накачку
        pumpValveState = pumpValveState_Close;
    }
    // включение помпы
    // закрытие клапана сброса
    setPneumoChannelState(pneumoCnl_1, pumpState_On,
        pumpValveState, reliefValveState_Close);
    setPneumoChannelState(pneumoCnl_2, pumpState_On,
        pumpValveState, reliefValveState_Close);
    setPneumoChannelState(pneumoCnl_3, pumpState_On,
        pumpValveState, reliefValveState_Close);
    setPneumoChannelState(pneumoCnl_4, pumpState_On,
        pumpValveState, reliefValveState_Close);
}

//------------------------------------------------------------------------------
/**
 * Выставление состояние соленоидов на безопасное в начале каждого цикла обработки
 */
void solenoiduUnset(void)
{
    // выключение помпы
    // открытие клапана накачки
    // открытие клапана сброса на сброс

    setPneumoChannelState(pneumoCnl_1, pumpState_Off,
        pumpValveState_Open, reliefValveState_Open);
    setPneumoChannelState(pneumoCnl_2, pumpState_Off,
        pumpValveState_Open, reliefValveState_Open);
    setPneumoChannelState(pneumoCnl_3, pumpState_Off,
        pumpValveState_Open, reliefValveState_Open);
    setPneumoChannelState(pneumoCnl_4, pumpState_Off,
        pumpValveState_Open, reliefValveState_Open);
}

//------------------------------------------------------------------------------
/**
 * Рукоять режима сброса
 */
void releive_handler(void)
{
    solenoiduUnset();
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
            if (0 == m_workMode)
            {   // Обычный режим накачки
                // включение помпы
                // открытие минираспределение на накачку
                // закрытие клапана сброса

                setPneumoChannelState(pneumoCnl_1, pumpState_On,
                    pumpValveState_Open, reliefValveState_Close);
                setPneumoChannelState(pneumoCnl_2, pumpState_On,
                    pumpValveState_Open, reliefValveState_Close);
                setPneumoChannelState(pneumoCnl_3, pumpState_On,
                    pumpValveState_Open, reliefValveState_Close);
                setPneumoChannelState(pneumoCnl_4, pumpState_On,
                    pumpValveState_Open, reliefValveState_Close);
            }
            if (1 == m_workMode)
            {   // Импульсный режим накачки
                // накачка сменяется сбросом
                pulsePumping_handler();
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
        if (pressure > maxPressure)
        {
            releive_handler();
        }
        else
        {
            m_work_cycle = !m_work_cycle;
            m_cycles_cnt = 0;
            m_pumping_cnt = 0;
        }
    }

    u32 second_limit = (0 == m_workMode) ?  100 : 200;
    if (m_cycles_cnt > SECOND_TO_CYKLE(second_limit))
    {
        m_current_mode = cm_error;
        m_cycles_cnt = 0;
    }
}

//------------------------------------------------------------------------------
/**
 * Возвращает время накачки
 * @return
 */
u32 getMaxTimeVibroPumping(void)
{
    u32 maxTime = 0;
    switch(PowerLevel)
    {
        case 0:
            maxTime = 5;
            break;
        case 1:
            maxTime = 7;
            break;
        case 2:
            maxTime = 10;
            break;
        case 3:
            maxTime = 13;
            break;
        case 4:
            maxTime = 17;
            break;
    };

    return maxTime;
}

//------------------------------------------------------------------------------
/**
 * Возвращает время накачки
 * @return
 */
u32 getMaxTimeVibroRelief(void)
{
    u32 maxTime = 1;
#if 1
    switch(PowerLevel)
    {
        case 0:
            maxTime = 3;
            break;
        case 1:
            maxTime = 2;
            break;
        case 2:
            maxTime = 2;
            break;
        case 3:
            maxTime = 1;
            break;
        case 4:
            maxTime = 1;
            break;
    };
#endif
    return maxTime;
}

//------------------------------------------------------------------------------
/**
 * Режим вибрации только по времени (без контроля давления)
 */
void vibroTime_handler(void)
{
    u32 second_limit = 0;

    switch(m_vibroTimeStep)
    {
    case vibroTimeStep_vibroPumping:
        second_limit = getMaxTimeVibroPumping();
        pulsePumping_handler();
        break;
    case vibroTimeStep_relief:
        second_limit = getMaxTimeVibroRelief();
        releive_handler();
        break;
    }

    if(m_cycles_cnt > SECOND_TO_CYKLE(second_limit))
    {
        m_vibroTimeStep = (vibroTimeStep_t)((u32)m_vibroTimeStep + 1);
        m_vibroTimeStep = (vibroTimeStep_t)((u32)m_vibroTimeStep%(u32)vibroTimeStep_num);
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
        switch (m_workMode)
        {
        case workMode_presso:
        case workMode_vibro:
            pumping_handler();
            break;
        }
        break;
    case cm_working:
        switch (m_workMode)
        {
         case workMode_rotator:
            workModeRotatorHandler();
            break;
        case workMode_vibroTime:
            vibroTime_handler();
            break;
        case workMode_presso:
        case workMode_vibro:
            working_handler();
            break;
        };
        break;
    };
}

//------------------------------------------------------------------------------
/**
 * Управление солиноидами
 */
void solenoidControl(void)
{
    setSolenoidState (m_pumpValveLine[pneumoCnl_1], MINIVALVE_1);
    setSolenoidState (m_reliefValveLine[pneumoCnl_1], RELIEFVALVE_1);
    setSolenoidState (m_pumpLine[pneumoCnl_1], PUMP_1);

    setSolenoidState (m_pumpValveLine[pneumoCnl_2], MINIVALVE_2);
    setSolenoidState (m_reliefValveLine[pneumoCnl_2], RELIEFVALVE_2);
    setSolenoidState (m_pumpLine[pneumoCnl_2], PUMP_2);

    setSolenoidState (m_pumpValveLine[pneumoCnl_3], MINIVALVE_3);
    setSolenoidState (m_reliefValveLine[pneumoCnl_3], RELIEFVALVE_3);
    setSolenoidState (m_pumpLine[pneumoCnl_3], PUMP_3);

    setSolenoidState (m_pumpValveLine[pneumoCnl_4], MINIVALVE_4);
    setSolenoidState (m_reliefValveLine[pneumoCnl_4], RELIEFVALVE_4);
    setSolenoidState (m_pumpLine[pneumoCnl_4], PUMP_4);
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

/*
    setLedState(0, led_level1);
    setLedState(0, led_level2);
    setLedState(0, led_level3);
    setLedState(0, led_level4);
    setLedState(0, led_level5);

    setLedState(state, (lede_t)((u32)led_level1 + ledInd));
*/

    blink_cnt++;
    blink_cnt = blink_cnt%60;
    if (!blink_cnt)
        state = !state;
}

//------------------------------------------------------------------------------
/**
 * Мигатель  светодиода D2
 */
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
/*
    setLedState (led_start_state, led_start);
    setLedState (led_warning_state, led_warning);
*/

    if(isWorkingMode())
        setLedsPowerLavel(PowerLevel);
    else
        blinkLed(m_workMode); // мигание светодиода выбранного режима

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

/*
    setLedState (0, led_level1);
    setLedState (0, led_level2);
    setLedState (0, led_level3);
    setLedState (0, led_level4);
    setLedState (0, led_level5);
    setLedState (0, led_warning);
    setLedState (0, led_start);
*/

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
