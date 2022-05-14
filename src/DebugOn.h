//
//  DebugOn.h
//  resol-vbus
//
//
//  Copyright (C) 2022, Daniel Wippermann. All rights reserved.
//


//---------------------------------------------------------------------------
// PUBLIC INCLUDES
//---------------------------------------------------------------------------

#include "DebugReset.h"



//---------------------------------------------------------------------------
// PUBLIC DEFINES
//---------------------------------------------------------------------------

#undef __FAIL
#define __FAIL(__ErrorSuffix__) \
    if (Result == RESOLVBUS_OK) { \
        Result = RESOLVBUS_ERROR_##__ErrorSuffix__; \
        ResolVBus_ResetBacktrace(#__ErrorSuffix__, #__ErrorSuffix__, __FILE__, __LINE__, __func__); \
    } \


#undef __WRAP
#define __WRAP(__Expression__) \
    if (Result == RESOLVBUS_OK) { \
        Result = (__Expression__); \
        if (Result != RESOLVBUS_OK) { \
            ResolVBus_AddBacktrace(#__Expression__, __FILE__, __LINE__, __func__); \
        } \
    } \


#undef __DLOG
#define __DLOG(...) \
    if (Result == RESOLVBUS_OK) { \
        ResolVBus_DebugLog(__FILE__, __LINE__, __func__, __VA_ARGS__); \
    } \


#undef __ASSERT_WITH
#define __ASSERT_WITH(__ErrorSuffix__, __Expression__) \
    if (Result == RESOLVBUS_OK) { \
        if (!(__Expression__)) { \
            Result = RESOLVBUS_ERROR_##__ErrorSuffix__; \
            ResolVBus_ResetBacktrace("Assertion failed: " #__Expression__, #__Expression__, __FILE__, __LINE__, __func__); \
        } \
    } \



//---------------------------------------------------------------------------
// PUBLIC TYPEDEFS
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// PUBLIC CALLBACKS
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// PUBLIC DATA TYPES
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// PUBLIC CONSTANTS
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// PUBLIC VARIABLES
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// PUBLIC METHODS
//---------------------------------------------------------------------------
