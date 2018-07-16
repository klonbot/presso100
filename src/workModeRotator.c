/*
 * File:   workModeRotator.c
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
/*
    phaseRotation_3,
    phaseRotation_4,

    phaseRotation_5,
    phaseRotation_6,

    phaseRotation_7,
    phaseRotation_8,
    phaseRotation_9,
    phaseRotation_10,

    phaseRotation_11,
    phaseRotation_12,
*/

    phaseRotation_num
} phaseRotation_t;

typedef enum
{
    profile_roundPillow,

    profile_num
} profile_t;

//------------------------------------------------------------------------------

enum {timePhase_s = 1};

//------------------------------------------------------------------------------
/**
 * Структура программного профиля
 */
typedef struct
{
    channelState_t phaseChannelState[phaseRotation_num][pneumoCnl_num];
} programmProfile_t;

//------------------------------------------------------------------------------

phaseRotation_t m_phaseRotation = phaseRotation_1;
static u32 m_cycles_cnt = 0;
static u32 m_round_cnt = 0;
static const programmProfile_t programmProfile[profile_num] =
{
    [profile_roundPillow] =
    {
        .phaseChannelState =
        {
            [phaseRotation_1] =
            {
                [pneumoCnl_1] = channelState_PumpOut,
            },
            [phaseRotation_2] =
            {
                [pneumoCnl_1] = channelState_Vibro,
            },
        }
    },
};

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
    }
}

//------------------------------------------------------------------------------
/**
 * Обработчик переключения на следующую фазу
 */
void nextPhaseHandler(void)
{
    ++m_cycles_cnt;
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
    profile_t profile = profile_roundPillow;
    pneumoCnl_t pneumoCnl = pneumoCnl_1;
    for (; pneumoCnl < pneumoCnl_num; pneumoCnl++)
    {
        setPneumoChannelStateTogether(pneumoCnl,
            programmProfile[profile].phaseChannelState[m_phaseRotation][pneumoCnl]);
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
