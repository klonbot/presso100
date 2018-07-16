/*
 * File:   solenoidhendler.h
 * Author: DiNastja
 *
 * Created on 2 сентября 2015 г., 20:53
 */

#ifndef SOLENOIDHENDLER_H
#define	SOLENOIDHENDLER_H

#ifdef	__cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

#include "types.h"

//------------------------------------------------------------------------------

/**
 * Перечисление всех соленоидов
 */
typedef enum
{
    sol_chnl1, sol_chnl2/*, sol_chnl3,
    sol_chnl4, sol_chnl5, sol_chnl6,
    sol_chnl7, sol_chnl8, sol_chnl9,
    sol_chnl10, sol_chnl11, sol_chnl12*/
} solenoide_t;

#define MINIVALVE_1 sol_chnl2
#define PUMP_1 sol_chnl1

//#define RELIEFVALVE_1 sol_chnl10
//
//#define MINIVALVE_2 sol_chnl9
//#define PUMP_2 sol_chnl8
//#define RELIEFVALVE_2 sol_chnl7
//
//#define MINIVALVE_3 sol_chnl6
//#define PUMP_3 sol_chnl5
//#define RELIEFVALVE_3 sol_chnl4
//
//#define MINIVALVE_4 sol_chnl3
//#define PUMP_4 sol_chnl2
//#define RELIEFVALVE_4 sol_chnl1

//------------------------------------------------------------------------------

void solenoidHandler(void);
void setSolenoidState (u32 newState, solenoide_t solenoide);

//------------------------------------------------------------------------------

#ifdef	__cplusplus
}
#endif

#endif	/* SOLENOIDHENDLER_H */

