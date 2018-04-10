/*
 * File:   tasks.h
 * Author: DiNastja
 *
 * Created on 24 августа 2015 г., 19:00
 */

#ifndef TASKS_H
#define	TASKS_H

#ifdef	__cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

#define BUTTONS_PRIORITY            ( tskIDLE_PRIORITY + 2 )
#define LED_PRIORITY                ( tskIDLE_PRIORITY + 2 )
#define TERMINAL_PRIORITY           ( tskIDLE_PRIORITY + 2 )
#define MAINLOGIC_PRIORITY          ( tskIDLE_PRIORITY + 2 )
#define PRESSURE_PRIORITY           ( tskIDLE_PRIORITY + 2 )
#define SOLENOID_PRIORITY           ( tskIDLE_PRIORITY + 2 )

//------------------------------------------------------------------------------

#ifdef	__cplusplus
}
#endif

#endif	/* TASKS_H */

