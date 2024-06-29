//
//  DebugTestSuite.c
//  resol-vbus-c
//
//
//  Copyright (C) 2024, Daniel Wippermann. All rights reserved.
//

#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>


#include "Testing.h"



//---------------------------------------------------------------------------
// PRIVATE DEFINES
//---------------------------------------------------------------------------



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

static RESOLVBUS_RESULT __TestGetBacktrace(void)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    const char *Backtrace = ResolVBus_GetBacktrace();

    __ASSERT(Backtrace != NULL);

    return Result;
}


static RESOLVBUS_RESULT __TestResetBacktrace(void)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    const char *Backtrace = ResolVBus_GetBacktrace();

    ResolVBus_ResetBacktrace("Message1", "Expression1", "File1", 111, "Func1");

    const char *Expected1 =
        "Message1\n"
        "\n"
        "Backtrace:\n"
        "- Expression1 (Func1 @ File1:111)\n";

    __ASSERT_STRING_EQL(Backtrace, Expected1);

    ResolVBus_ResetBacktrace("Message2", "Expression2", "File2", 222, "Func2");

    const char *Expected2 =
        "Message2\n"
        "\n"
        "Backtrace:\n"
        "- Expression2 (Func2 @ File2:222)\n";

    __ASSERT_STRING_EQL(Backtrace, Expected2);

    return Result;
}


static RESOLVBUS_RESULT __TestAddBacktrace(void)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    const char *Backtrace = ResolVBus_GetBacktrace();

    ResolVBus_ResetBacktrace("Message1", "Expression1", "File1", 111, "Func1");

    ResolVBus_AddBacktrace("Expression2", "File2", 222, "Func2");

    const char *Expected1 =
        "Message1\n"
        "\n"
        "Backtrace:\n"
        "- Expression1 (Func1 @ File1:111)\n"
        "- Expression2 (Func2 @ File2:222)\n";

    __ASSERT_STRING_EQL(Backtrace, Expected1);

    ResolVBus_AddBacktrace("Expression3", "File3", 333, "Func3");

    const char *Expected2 =
        "Message1\n"
        "\n"
        "Backtrace:\n"
        "- Expression1 (Func1 @ File1:111)\n"
        "- Expression2 (Func2 @ File2:222)\n"
        "- Expression3 (Func3 @ File3:333)\n";

    __ASSERT_STRING_EQL(Backtrace, Expected2);

    return Result;
}


static RESOLVBUS_RESULT __TestPrintBacktrace(void)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    int StdoutPipe [2];
    __ASSERT(pipe(StdoutPipe) == 0);

    int StdoutReader = StdoutPipe [0];
    int StdoutWriter = StdoutPipe [1];

    int Pid = fork();

    __ASSERT(Pid >= 0);

    if (Result == RESOLVBUS_OK) {
        if (Pid == 0) {
            // forked process
            close(StdoutReader);
            dup2(StdoutWriter, 1);
            close(StdoutWriter);

            ResolVBus_ResetBacktrace("Message1", "Expression1", "File1", 111, "Func1");
            ResolVBus_AddBacktrace("Expression2", "File2", 222, "Func2");
            ResolVBus_AddBacktrace("Expression3", "File3", 333, "Func3");

            ResolVBus_PrintBacktrace();

            exit(0);
            _exit(0);
        } else {
            // parent process
            close(StdoutWriter);

            static char Contents [1024];
            size_t Index = 0;

            while (Result == RESOLVBUS_OK) {
                int ReadLength = read(StdoutReader, Contents + Index, sizeof (Contents) - Index - 1);

                __ASSERT(ReadLength >= 0);

                if (ReadLength == 0) {
                    Contents [Index] = 0;
                    break;
                } else if (ReadLength > 0) {
                    Index += ReadLength;
                }
            }

            close(StdoutReader);

            int ChildStatus = 0;
            __ASSERT(waitpid(Pid, &ChildStatus, 0) == Pid);
            __ASSERT(WIFEXITED(ChildStatus));
            __ASSERT(WEXITSTATUS(ChildStatus) == 0);

            const char *Expected =
                "Message1\n"
                "\n"
                "Backtrace:\n"
                "- Expression1 (Func1 @ File1:111)\n"
                "- Expression2 (Func2 @ File2:222)\n"
                "- Expression3 (Func3 @ File3:333)\n"
                "\n";

            __ASSERT_STRING_EQL(Contents, Expected);
        }
    }

    return Result;
}


static RESOLVBUS_RESULT __TestDebugLog(void)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    int StdoutPipe [2];
    __ASSERT(pipe(StdoutPipe) == 0);

    int StdoutReader = StdoutPipe [0];
    int StdoutWriter = StdoutPipe [1];

    int Pid = fork();

    __ASSERT(Pid >= 0);

    if (Result == RESOLVBUS_OK) {
        if (Pid == 0) {
            // forked process
            close(StdoutReader);
            dup2(StdoutWriter, 1);
            close(StdoutWriter);

            ResolVBus_DebugLog("File", 111, "Func", "Message: %s", "Test");

            exit(0);
            _exit(0);
        } else {
            // parent process
            close(StdoutWriter);

            static char Contents [1024];
            size_t Index = 0;

            while (Result == RESOLVBUS_OK) {
                int ReadLength = read(StdoutReader, Contents + Index, sizeof (Contents) - Index - 1);

                __ASSERT(ReadLength >= 0);

                if (ReadLength == 0) {
                    Contents [Index] = 0;
                    break;
                } else if (ReadLength > 0) {
                    Index += ReadLength;
                }
            }

            close(StdoutReader);

            int ChildStatus = 0;
            __ASSERT(waitpid(Pid, &ChildStatus, 0) == Pid);
            __ASSERT(WIFEXITED(ChildStatus));
            __ASSERT(WEXITSTATUS(ChildStatus) == 0);

            const char *Expected = "Message: Test  [Func (File:111)]\n";

            __ASSERT_STRING_EQL(Contents, Expected);
        }
    }

    return Result;
}



//---------------------------------------------------------------------------
// PUBLIC METHODS
//---------------------------------------------------------------------------

RESOLVBUS_RESULT RunTestSuite_Debug(void)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    __WRAP(__TestGetBacktrace());
    __WRAP(__TestResetBacktrace());
    __WRAP(__TestAddBacktrace());
    __WRAP(__TestPrintBacktrace());
    __WRAP(__TestDebugLog());

    return Result;
}

