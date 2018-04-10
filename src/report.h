/*
 * File:   report.h
 * Author: DiNastja
 *
 * Created on 29 августа 2015 г., 11:19
 */

#ifndef REPORT_H
#define	REPORT_H

#ifdef	__cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

#include <stdio.h>

//------------------------------------------------------------------------------

//void Report_Debug(const char *report, ...);

#define COLOR_GREEN     "\033[1;32m"    // Светло зеленый
#define COLOR_RED       "\033[1;31m"    // Светло красный
#define COLOR_WHITE     "\033[0;37m"    // Белый
#define COLOR_YELLOW    "\033[1;33m"    // Желтый

#define Report_Debug(...) {printf (COLOR_WHITE); printf(__VA_ARGS__); printf ("\n\r");}
#define Report_Info(...) {printf (COLOR_GREEN); printf(__VA_ARGS__); printf ("\n\r");}
#define Report_Error(...) {printf (COLOR_RED); printf(__VA_ARGS__); printf ("\n\r");}
#define Report_Warning(...) {printf (COLOR_YELLOW); printf(__VA_ARGS__); printf ("\n\r");}

#define _Report_Debug(...) {};
#define _Report_Info(...) {};
#define _Report_Error(...) {};
#define _Report_Warning(...) {};

//------------------------------------------------------------------------------

#ifdef	__cplusplus
}
#endif

#endif	/* REPORT_H */

