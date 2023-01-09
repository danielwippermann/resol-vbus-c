#if RESOLVBUS_DEBUG > 0

//
//  ResolVBusDebug.c
//  resol-vbus
//
//
//  Copyright (C) 2022, Daniel Wippermann. All rights reserved.
//

#include <stdarg.h>
#include <stdio.h>
#include <string.h>


#include "ResolVBus.h"


#include "Debug.h"



//---------------------------------------------------------------------------
// PRIVATE DEFINES
//---------------------------------------------------------------------------

#ifndef RESOLVBUS_DEBUG_BACKTRACELENGTH
#define RESOLVBUS_DEBUG_BACKTRACELENGTH 1024
#endif



//---------------------------------------------------------------------------
// PRIVATE TYPEDEFS
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// PRIVATE DATA TYPES
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// PRIVATE METHOD DECLARATIONS
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// CONSTANTS
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// VARIABLES
//---------------------------------------------------------------------------

static char __Backtrace [RESOLVBUS_DEBUG_BACKTRACELENGTH] = { 0 };



//---------------------------------------------------------------------------
// NON-PUBLIC METHODS
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// PUBLIC METHODS
//---------------------------------------------------------------------------

void ResolVBus_ResetBacktrace(const char *Message, const char *Expression, const char *File, int Line, const char *Func)
{
    snprintf(__Backtrace, sizeof (__Backtrace), "%s\n\nBacktrace:\n", Message);
    __Backtrace [sizeof (__Backtrace) - 1] = 0;

    ResolVBus_AddBacktrace(Expression, File, Line, Func);
}


void ResolVBus_AddBacktrace(const char *Expression, const char *File, int Line, const char *Func)
{
    size_t Index = strlen(__Backtrace);
    if (Index < sizeof (__Backtrace)) {
        snprintf(__Backtrace + Index, sizeof (__Backtrace) - Index, "- %s (%s @ %s:%d)\n", Expression, Func, File, Line);
        __Backtrace [sizeof (__Backtrace) - 1] = 0;
    }
}


const char *ResolVBus_GetBacktrace(void)
{
    return __Backtrace;
}


void ResolVBus_PrintBacktrace(void)
{
    if (__Backtrace [0]) {
        printf("%s\n", __Backtrace);
    }
}


void ResolVBus_DebugLog(const char *File, int Line, const char *Func, const char *Format, ...)
{
    va_list Arguments;
    va_start(Arguments, Format);
    vprintf(Format, Arguments);
    printf("  [%s (%s:%d)]\n", Func, File, Line);
    va_end(Arguments);
}


#endif  // RESOLVBUS_DEBUG
