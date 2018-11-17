/*
 * File:   ledHandler.h
 * Author: DiNastja
 *
 * Created on 26 августа 2015 г., 15:08
 */

#ifndef LEDHANDLER_H
#define	LEDHANDLER_H

#ifdef	__cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

#include "types.h"

/**
 * Перечисление всех светодиодов
 */
typedef enum
{
    /*led_level1, led_level2, led_level3,
    led_level4, led_level5, led_warning, led_start,
    led_1, led_2, led_3,*/
    led_D2,
    led_num
} lede_t;

//------------------------------------------------------------------------------

void ledHandler(void);
void setLedState (u32 newState, lede_t lede);

//------------------------------------------------------------------------------


#ifdef	__cplusplus
}
#endif

#endif	/* LEDHANDLER_H */

