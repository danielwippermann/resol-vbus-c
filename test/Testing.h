#ifndef __TESTING_H__
#define __TESTING_H__

//
//  Testing.h
//  resol-vbus
//
//
//  Copyright (C) 2022, Daniel Wippermann. All rights reserved.
//


//---------------------------------------------------------------------------
// PUBLIC INCLUDES
//---------------------------------------------------------------------------

#include "ResolVBus.h"


#include "../src/DebugOn.h"



//---------------------------------------------------------------------------
// PUBLIC DEFINES
//---------------------------------------------------------------------------

#define __ASSERT(__Expression__) __ASSERT_WITH(UNKNOWN, __Expression__)


#define __WRAP_SHORT(__Expression__) \
    if (Result == RESOLVBUS_OK) { \
        Result = (__Expression__); \
        if (Result != RESOLVBUS_OK) { \
            ResolVBus_AddBacktrace("<see above>", __FILE__, __LINE__, __func__); \
        } \
    } \


#define __ASSERT_EQL(__Left__, __Right__) \
    __WRAP_SHORT(AssertEql(__Left__, #__Left__, __Right__, #__Right__)); \


#define __ASSERT_RESULT_EQL(__ErrorSuffix__, __ErrorMessage__, __Result__) \
    __WRAP_SHORT(AssertResultEql(RESOLVBUS_ERROR_##__ErrorSuffix__, __ErrorMessage__, __Result__)); \



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

extern const uint8_t TestData_Live1 [368];
extern const uint8_t TestData_Live2 [17];



//---------------------------------------------------------------------------
// PUBLIC METHODS
//---------------------------------------------------------------------------

void AppendToLog(char *Log, size_t Length, const char *Format, ...);
RESOLVBUS_RESULT AssertEql(int64_t LeftValue, const char *LeftExpr, int64_t RightValue, const char *RightExpr);
RESOLVBUS_RESULT AssertStringEql(const char *LeftValue, const char *LeftExpr, const char *RightValue, const char *RightExpr);
RESOLVBUS_RESULT AssertResultEql(RESOLVBUS_RESULT ExpectedResult, const char *ExpectedMessage, RESOLVBUS_RESULT ActualResult);


RESOLVBUS_RESULT RunTestSuite_Base(void);
RESOLVBUS_RESULT RunTestSuite_LiveDecoder(void);
RESOLVBUS_RESULT RunTestSuite_LiveEncoder(void);


RESOLVBUS_RESULT RunTest_MasterExample(void);
RESOLVBUS_RESULT RunTest_MinionExample(void);



#endif // __TESTING_H__
