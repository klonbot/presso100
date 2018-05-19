/*
 * File:   workModeRotator.cpp
 * Author: Дмитрий
 *
 * Created on 19 Май 2018 г., 16:06
 */
//------------------------------------------------------------------------------

#include "workModeRotator.h"
#include "mainLogic.h"
#include "types.h"

//------------------------------------------------------------------------------

typedef enum
{
    phaseRotation_1,
    phaseRotation_2,
    phaseRotation_3,
    phaseRotation_4,
    phaseRotation_num
} phaseRotation_t;

//------------------------------------------------------------------------------

enum {timePhase_s = 7};

//------------------------------------------------------------------------------

phaseRotation_t m_phaseRotation = phaseRotation_1;
static u32 m_cycles_cnt = 0;
u32 m_round_cnt = 0;

//------------------------------------------------------------------------------
/**
 * переключает на следующую фазу
 */
void nextPhase (void)
{
    m_phaseRotation = (phaseRotation_t)((u32)m_phaseRotation +1);
    if (m_phaseRotation >= phaseRotation_num)
    {
        m_phaseRotation = phaseRotation_1;
        ++m_round_cnt;
    }
}

//------------------------------------------------------------------------------
/**
 * Обработчик переключения на следующую фазу
 */
void nextPhaseHandler(void)
{
    if (m_cycles_cnt > SECOND_TO_CYKLE(timePhase_s))
    {
        nextPhase ();
        m_cycles_cnt = 0;
    }
}

//------------------------------------------------------------------------------
/**
 * Переключатель в зависимости от фазы
 */
void phaseSwitch(void)
{
    switch(m_phaseRotation)
    {
        case phaseRotation_1:
            setPneumoChannelPumpOn(pneumoCnl_1);    // надувается
            setPneumoChannelHold(pneumoCnl_4);      // удерживается
            setPneumoChannelPumpOut(pneumoCnl_3);   // спускается
            setPneumoChannelPumpOut(pneumoCnl_2);   // спускается - уже спущено
            break;
        case phaseRotation_2:
            setPneumoChannelPumpOn(pneumoCnl_2);    // надувается
            setPneumoChannelHold(pneumoCnl_1);      // удерживается
            setPneumoChannelPumpOut(pneumoCnl_4);   // спускается
            setPneumoChannelPumpOut(pneumoCnl_3);   // спускается - уже спущено
            break;
        case phaseRotation_3:
            setPneumoChannelPumpOn(pneumoCnl_3);    // надувается
            setPneumoChannelHold(pneumoCnl_2);      // удерживается
            setPneumoChannelPumpOut(pneumoCnl_1);   // спускается
            setPneumoChannelPumpOut(pneumoCnl_4);   // спускается - уже спущено
            break;
        case phaseRotation_4:
            setPneumoChannelPumpOn(pneumoCnl_4);    // надувается
            setPneumoChannelHold(pneumoCnl_3);      // удерживается
            setPneumoChannelPumpOut(pneumoCnl_2);   // спускается
            setPneumoChannelPumpOut(pneumoCnl_1);   // спускается - уже спущено
            break;
    }
}

//------------------------------------------------------------------------------
/**
 * Обработчик режима вращательного наполнения
 */
void workModeRotatorHandler(void)
{
    phaseSwitch();
    nextPhaseHandler();
}

//------------------------------------------------------------------------------
