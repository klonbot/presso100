/*
 * File:   types.h
 * Author: DiNastja
 *
 * Created on 26 августа 2015 г., 15:16
 */

#ifndef TYPES_H
#define	TYPES_H

#ifdef	__cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

/* Library includes. */
#include "stm32f10x_gpio.h"
#include <stdint.h>

//------------------------------------------------------------------------------

#define NO_USE_PARAM(x) (x=x)
#define nop() asm("nop");



//------------------------------------------------------------------------------

#ifdef	__cplusplus
}
#endif

#endif	/* TYPES_H */

