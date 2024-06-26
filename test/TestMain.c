//
//  TestMain.c
//  resol-vbus
//
//
//  Copyright (C) 2022, Daniel Wippermann. All rights reserved.
//

#include <stdarg.h>
#include <stdio.h>
#include <string.h>


#include "Testing.h"



//---------------------------------------------------------------------------
// PRIVATE DEFINES
//---------------------------------------------------------------------------

#ifdef __APPLE__
#define __FORMAT_D64 "%lld"
#define __FORMAT_X64 "0x%llX"
#else
#define __FORMAT_D64 "%ld"
#define __FORMAT_X64 "0x%lX"
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



//---------------------------------------------------------------------------
// NON-PUBLIC METHODS
//---------------------------------------------------------------------------

static RESOLVBUS_RESULT __TestAssertEql(void)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    __WRAP(AssertEql(123, "LeftValue123", 123, "RightValue123"));

    __ASSERT_RESULT_EQL(UNKNOWN, "Expected values to equal\n  Left:  LeftValue123 = 123 (0x7B)\n  Right: RightValue234 = 234 (0xEA)\n\nBacktrace:\n", AssertEql(123, "LeftValue123", 234, "RightValue234"));

    return Result;
}


static RESOLVBUS_RESULT __TestAssertNotEql(void)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    __WRAP(AssertNotEql(123, "LeftValue123", 234, "RightValue234"));

    __ASSERT_RESULT_EQL(UNKNOWN, "Expected values not to equal\n  Left:  LeftValue123 = 123 (0x7B)\n  Right: RightValue123 = 123 (0x7B)\n\nBacktrace:\n", AssertNotEql(123, "LeftValue123", 123, "RightValue123"));

    return Result;
}


static RESOLVBUS_RESULT __TestAssertPointerEql(void)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    __WRAP(AssertPointerEql((const void *) 123, "LeftValue123", (const void *) 123, "RightValue123"));

    __ASSERT_RESULT_EQL(UNKNOWN, "Expected pointers to equal\n  Left:  LeftValue123 = 0x7b\n  Right: RightValue234 = 0xea\n\nBacktrace:\n", AssertPointerEql((const void *) 123, "LeftValue123", (const void *) 234, "RightValue234"));

    return Result;
}


static RESOLVBUS_RESULT __TestAssertStringEql(void)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    __WRAP(AssertStringEql("123", "LeftValue123", "123", "RightValue123"));

    __ASSERT_RESULT_EQL(UNKNOWN, "Expected strings to equal\n  Left:  LeftValue123 = \"123\"\n  Right: RightValue234 = \"234\"\n\nBacktrace:\n", AssertStringEql("123", "LeftValue123", "234", "RightValue234"));

    return Result;
}


static RESOLVBUS_RESULT __TestAssertResultEql(void)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    ResolVBus_ResetBacktrace("Message", "Expression", "File", 123, "Func");

    __ASSERT_RESULT_EQL(UNKNOWN, "Message\n\nBacktrace:\n- Expression (Func @ File:123)\n", RESOLVBUS_ERROR_UNKNOWN);

    const char *Expected =
        "Expected strings to equal\n"
        "  Left:    Actual = \"Message\n"
        "\n"
        "Backtrace:\n"
        "- Expression (Func @ File:123)\n"
        "\"\n"
        "  Right: Expected = \"UnexpectedMessage\"\n"
        "\n"
        "Backtrace:\n";

    __ASSERT_RESULT_EQL(UNKNOWN, Expected, AssertResultEql(0, "UnexpectedMessage", 0));

    return Result;
}


static RESOLVBUS_RESULT __RunTestSuite_Testing(void)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    __WRAP(__TestAssertEql());
    __WRAP(__TestAssertNotEql());
    __WRAP(__TestAssertPointerEql());
    __WRAP(__TestAssertStringEql());
    __WRAP(__TestAssertResultEql());

    return Result;
}


static RESOLVBUS_RESULT __TestContainerOf(void)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    struct __TESTSTRUCT1 {
        int Member1;
        int Member2;
        int Member3;
    };

    struct __TESTSTRUCT2 {
        struct __TESTSTRUCT1 Member1;
    };

    struct __TESTSTRUCT3 {
        struct __TESTSTRUCT2 Member1;
    };

    struct __TESTSTRUCT3 TestStruct = {
        .Member1.Member1.Member3 = 1234,
    };

    int *pMember3 = &TestStruct.Member1.Member1.Member3;

    __ASSERT_POINTER_EQL(&TestStruct, RESOLVBUS_CONTAINEROF(pMember3, struct __TESTSTRUCT3, Member1.Member1.Member3));

    return Result;
}


static RESOLVBUS_RESULT __RunTestSuite_Macros(void)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    __WRAP(__TestContainerOf());

    return Result;
}



//---------------------------------------------------------------------------
// PUBLIC METHODS
//---------------------------------------------------------------------------

void AppendToLog(char *Log, size_t Length, const char *Format, ...)
{
    va_list Arguments;
    va_start(Arguments, Format);
    size_t Index = strlen(Log);
    if (Index < Length) {
        vsnprintf(Log + Index, Length - Index, Format, Arguments);
        Log [Length - 1] = 0;
    }
    va_end(Arguments);
}


RESOLVBUS_RESULT AssertEql(int64_t LeftValue, const char *LeftExpr, int64_t RightValue, const char *RightExpr)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    if (LeftValue != RightValue) {
        static char Message [1024] = { 0 };
        snprintf(Message, sizeof (Message), "Expected values to equal\n  Left:  %s = " __FORMAT_D64 " (" __FORMAT_X64 ")\n  Right: %s = " __FORMAT_D64 " (" __FORMAT_X64 ")", LeftExpr, LeftValue, LeftValue, RightExpr, RightValue, RightValue);

        Result = RESOLVBUS_ERROR_UNKNOWN;
        ResolVBus_ResetBacktrace(Message, "<see above>", __FILE__, __LINE__, __func__);
    }

    return Result;
}

RESOLVBUS_RESULT AssertNotEql(int64_t LeftValue, const char *LeftExpr, int64_t RightValue, const char *RightExpr)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    if (LeftValue == RightValue) {
        static char Message [1024] = { 0 };
        snprintf(Message, sizeof (Message), "Expected values not to equal\n  Left:  %s = " __FORMAT_D64 " (" __FORMAT_X64 ")\n  Right: %s = " __FORMAT_D64 " (" __FORMAT_X64 ")", LeftExpr, LeftValue, LeftValue, RightExpr, RightValue, RightValue);

        Result = RESOLVBUS_ERROR_UNKNOWN;
        ResolVBus_ResetBacktrace(Message, "<see above>", __FILE__, __LINE__, __func__);
    }

    return Result;
}


RESOLVBUS_RESULT AssertPointerEql(const void *LeftValue, const char *LeftExpr, const void *RightValue, const char *RightExpr)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    if (LeftValue != RightValue) {
        static char Message [1024] = { 0 };
        snprintf(Message, sizeof (Message), "Expected pointers to equal\n  Left:  %s = %p\n  Right: %s = %p", LeftExpr, LeftValue, RightExpr, RightValue);

        Result = RESOLVBUS_ERROR_UNKNOWN;
        ResolVBus_ResetBacktrace(Message, "<see above>", __FILE__, __LINE__, __func__);
    }

    return Result;
}


RESOLVBUS_RESULT AssertStringEql(const char *LeftValue, const char *LeftExpr, const char *RightValue, const char *RightExpr)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    if (strcmp(LeftValue, RightValue) != 0) {
        static char Message [4096] = { 0 };
        snprintf(Message, sizeof (Message), "Expected strings to equal\n  Left:  %s = \"%s\"\n  Right: %s = \"%s\"", LeftExpr, LeftValue, RightExpr, RightValue);

        Result = RESOLVBUS_ERROR_UNKNOWN;
        ResolVBus_ResetBacktrace(Message, "<see above>", __FILE__, __LINE__, __func__);
    }

    return Result;
}


RESOLVBUS_RESULT AssertResultEql(RESOLVBUS_RESULT ExpectedResult, const char *ExpectedMessage, RESOLVBUS_RESULT ActualResult)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    __ASSERT_EQL(ExpectedResult, ActualResult);

    if (ExpectedMessage) {
        const char *ActualMessage = ResolVBus_GetBacktrace();
        if (strncmp(ActualMessage, ExpectedMessage, strlen(ExpectedMessage)) != 0) {
            __WRAP_SHORT(AssertStringEql(ActualMessage, "  Actual", ExpectedMessage, "Expected"));
        }
    }

    return Result;
}


int main(int argc, char **argv)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    ResolVBus_PrintBacktrace();  // NOTE(daniel): for coverage reasons

    __WRAP(__RunTestSuite_Testing());
    __WRAP(__RunTestSuite_Macros());

    __WRAP(RunTestSuite_Base());
    __WRAP(RunTestSuite_Debug());
    __WRAP(RunTestSuite_LiveDecoder());
    __WRAP(RunTestSuite_LiveEncoder());
    __WRAP(RunTestSuite_LiveTransceiver());

    __WRAP(RunTest_AccessoryBridgeExample());
    __WRAP(RunTest_MasterExample());
    __WRAP(RunTest_MinionExample());
    __WRAP(RunTest_ParameterizationExample());

    int ExitCode = 0;
    if (Result == RESOLVBUS_OK) {
        printf("All tests ran successfully!\n");
    } else {
        printf("A test failure occurred:\n");
        ResolVBus_PrintBacktrace();
        ExitCode = 1;
    }

    return ExitCode;
}
