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
    sol_chnl1, sol_chnl2, sol_chnl3, sol_chnl4
} solenoide_t;

#define MINIVALVE sol_chnl4
#define PUMP sol_chnl3
#define RELIEFVALVE sol_chnl2

//------------------------------------------------------------------------------

void solenoidHandler(void);
void setSolenoidState (u32 newState, solenoide_t solenoide);

//------------------------------------------------------------------------------

#ifdef	__cplusplus
}
#endif

#endif	/* SOLENOIDHENDLER_H */

