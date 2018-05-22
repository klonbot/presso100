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
    phaseRotation_3,
    phaseRotation_4,
    phaseRotation_num
} phaseRotation_t;

typedef enum
{
    profile_lowRight,
    profile_lowLeft,
    profile_mediumRight,
    profile_mediumLeft,
    profile_hightRight,
    profile_hightLeft,
    profile_num
} profile_t;

//------------------------------------------------------------------------------

enum {timePhase_s = 6};

typedef enum {directionRotation_right, directionRotation_left} directionRotation_t;

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
static directionRotation_t m_directionRotation = directionRotation_right;
static const programmProfile_t programmProfile[profile_num] =
{
    [profile_lowRight] =
    {
        .phaseChannelState =
        {
            [phaseRotation_1] =
            {
                [pneumoCnl_1] = channelState_PumpOn,
                [pneumoCnl_2] = channelState_PumpOut,
                [pneumoCnl_3] = channelState_PumpOut,
                [pneumoCnl_4] = channelState_Hold
            },
            [phaseRotation_2] =
            {
                [pneumoCnl_1] = channelState_Hold,
                [pneumoCnl_2] = channelState_PumpOn,
                [pneumoCnl_3] = channelState_PumpOut,
                [pneumoCnl_4] = channelState_PumpOut
            },
            [phaseRotation_3] =
            {
                [pneumoCnl_1] = channelState_PumpOut,
                [pneumoCnl_2] = channelState_Hold,
                [pneumoCnl_3] = channelState_PumpOn,
                [pneumoCnl_4] = channelState_PumpOut
            },
            [phaseRotation_4] =
            {
                [pneumoCnl_1] = channelState_PumpOut,
                [pneumoCnl_2] = channelState_PumpOut,
                [pneumoCnl_3] = channelState_Hold,
                [pneumoCnl_4] = channelState_PumpOn
            }
        }
    },
    [profile_lowLeft] =
    {
        .phaseChannelState =
        {
            [phaseRotation_1] =
            {
                [pneumoCnl_1] = channelState_PumpOn,
                [pneumoCnl_2] = channelState_Hold,
                [pneumoCnl_3] = channelState_PumpOut,
                [pneumoCnl_4] = channelState_PumpOut
            },
            [phaseRotation_2] =
            {
                [pneumoCnl_1] = channelState_Hold,
                [pneumoCnl_2] = channelState_PumpOut,
                [pneumoCnl_3] = channelState_PumpOut,
                [pneumoCnl_4] = channelState_PumpOn
            },
            [phaseRotation_3] =
            {
                [pneumoCnl_1] = channelState_PumpOut,
                [pneumoCnl_2] = channelState_PumpOut,
                [pneumoCnl_3] = channelState_PumpOn,
                [pneumoCnl_4] = channelState_Hold
            },
            [phaseRotation_4] =
            {
                [pneumoCnl_1] = channelState_PumpOut,
                [pneumoCnl_2] = channelState_PumpOn,
                [pneumoCnl_3] = channelState_Hold,
                [pneumoCnl_4] = channelState_PumpOut
            }
        }
    },
    [profile_mediumRight] =
    {
        .phaseChannelState =
        {
            [phaseRotation_1] =
            {
                [pneumoCnl_1] = channelState_PumpOn,
                [pneumoCnl_2] = channelState_PumpOn,
                [pneumoCnl_3] = channelState_PumpOut,
                [pneumoCnl_4] = channelState_Hold
            },
            [phaseRotation_2] =
            {
                [pneumoCnl_1] = channelState_Hold,
                [pneumoCnl_2] = channelState_PumpOn,
                [pneumoCnl_3] = channelState_PumpOn,
                [pneumoCnl_4] = channelState_PumpOut
            },
            [phaseRotation_3] =
            {
                [pneumoCnl_1] = channelState_PumpOut,
                [pneumoCnl_2] = channelState_Hold,
                [pneumoCnl_3] = channelState_PumpOn,
                [pneumoCnl_4] = channelState_PumpOn
            },
            [phaseRotation_4] =
            {
                [pneumoCnl_1] = channelState_PumpOn,
                [pneumoCnl_2] = channelState_PumpOut,
                [pneumoCnl_3] = channelState_Hold,
                [pneumoCnl_4] = channelState_PumpOn
            }
        }
    }
};


//------------------------------------------------------------------------------
/**
 * Переключает направление вращения
 */
void switchRotation(void)
{

    if (directionRotation_right == m_directionRotation)
    {
        m_directionRotation = directionRotation_left;
    }
	else
    if (directionRotation_left == m_directionRotation)
    {
        m_directionRotation = directionRotation_right;
    }
}

//------------------------------------------------------------------------------
/**
 * Переходит на следующий круг
 */
void nextRound(void)
{
    ++m_round_cnt;
    if (m_round_cnt > 2)
    {
        m_round_cnt = 0;
        switchRotation();
    }
}

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
        nextRound();
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
    profile_t profile = profile_lowRight;
    if (directionRotation_right == m_directionRotation)
    {
        profile = profile_mediumRight;
    }
    if (directionRotation_left == m_directionRotation)
    {
        profile = profile_mediumRight;
    }
    pneumoCnl_t pneumoCnl = pneumoCnl_1;
    for (; pneumoCnl< pneumoCnl_num; pneumoCnl++)
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
