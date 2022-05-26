//
//  MinionExampleTest.c
//  resol-vbus
//
//
//  Copyright (C) 2022, Daniel Wippermann. All rights reserved.
//

#define __IGNORE_MAIN_FOR_TESTING


#include "../examples/Minion.c"


#include "Testing.h"



//---------------------------------------------------------------------------
// PRIVATE DEFINES
//---------------------------------------------------------------------------

#define __APPEND_LOG(...) \
    if (Result == RESOLVBUS_OK) { \
        AppendToLog(__HandlerLog, sizeof (__HandlerLog), __VA_ARGS__); \
    } \


#define __ASSERT_LOG_EQL(__Expected__) \
    __WRAP_SHORT(AssertStringEql(__Expected__, "Expected", __HandlerLog, "__HandlerLog")); \
    if (Result == RESOLVBUS_OK) { \
        __HandlerLog [0] = 0; \
    } \



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

static char __HandlerLog [1024];



//---------------------------------------------------------------------------
// NON-PUBLIC METHODS
//---------------------------------------------------------------------------

static RESOLVBUS_RESULT __TestEncoderHandler(RESOLVBUS_LIVEENCODER *Encoder, const RESOLVBUS_LIVEENCODEREVENT *Event)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    if (Event->EventType == RESOLVBUS_LIVEENCODEREVENTTYPE_IDLE) {
        __APPEND_LOG("Event = IDLE\n");
    } else if (Event->EventType == RESOLVBUS_LIVEENCODEREVENTTYPE_TRANSMIT) {
        __APPEND_LOG("Event = TRANSMIT, Data =");
        for (size_t Index = 0; Index < Event->TransmitLength; Index++) {
            __APPEND_LOG(" 0x%02X", ((const uint8_t *) Event->TransmitBytes) [Index]);
        }
        __APPEND_LOG("\n");
    } else {
        __APPEND_LOG("Event = ??? (%d)", Event);
    }

    __WRAP(__EncoderHandler(Encoder, Event));

    return Result;
}



//---------------------------------------------------------------------------
// PUBLIC METHODS
//---------------------------------------------------------------------------

RESOLVBUS_RESULT RunTest_MinionExample(void)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    __MINION Minion = {
        .Decoder = RESOLVBUS_LIVEDECODER_INITIALIZER,
        .Encoder = RESOLVBUS_LIVEENCODER_INITIALIZER,
    };

    __WRAP(__Initialize(&Minion));

    if (Result == RESOLVBUS_OK) {
        Minion.Encoder.Handler = __TestEncoderHandler;
    }

    __WRAP(__HandleLoopCycle(&Minion, 0, NULL, 0));

    __ASSERT_LOG_EQL("");
    __ASSERT_EQL(Minion.Encoder.Phase, RESOLVBUS_LIVEENCODERPHASE_SUSPENDED);

    const uint8_t Bytes1 [] = {
        0xAA, 0x45, 0x23, 0x34, 0x12, 0x10, 0x00, 0x03, 0x00, 0x3E,
    };

    __WRAP(__HandleLoopCycle(&Minion, 0, Bytes1, sizeof (Bytes1)));

    __ASSERT_LOG_EQL("");
    __ASSERT_EQL(Minion.Encoder.Phase, RESOLVBUS_LIVEENCODERPHASE_IDLE);

    __WRAP(__HandleLoopCycle(&Minion, 0, NULL, 0));

    __ASSERT_LOG_EQL("Event = TRANSMIT, Data = 0xAA 0x34 0x12 0x45 0x23 0x10 0x00 0x01 0x02 0x3E 0x00 0x00 0x00 0x00 0x00 0x7F 0x00 0x00 0x7F 0x7F 0x0C 0x75\n");
    __ASSERT_EQL(Minion.Encoder.Phase, RESOLVBUS_LIVEENCODERPHASE_TRANSMITTING);
    __ASSERT_EQL(Minion.Encoder.PhaseTimeoutUs, 22917);

    __WRAP(__HandleLoopCycle(&Minion, 22917, NULL, 0));

    __ASSERT_LOG_EQL("");
    __ASSERT_EQL(Minion.Encoder.Phase, RESOLVBUS_LIVEENCODERPHASE_SUSPENDED);

    return Result;
}
