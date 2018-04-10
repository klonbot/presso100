/*
 * File:   pressure.h
 * Author: DiNastja
 *
 * Created on 29 августа 2015 г., 14:30
 */

#ifndef PRESSUREHANDLER_H
#define	PRESSUREHANDLER_H

#ifdef	__cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

#include "types.h"

//------------------------------------------------------------------------------

void pressureHandler(void);
u32 getPressure(void);

//------------------------------------------------------------------------------

#ifdef	__cplusplus
}
#endif

#endif	/* PRESSUREHANDLER_H */

