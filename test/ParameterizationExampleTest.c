//
//  ParameterizationExampleTest.c
//  resol-vbus-c
//
//
//  Copyright (C) 2023, Daniel Wippermann. All rights reserved.
//

#define __IGNORE_MAIN_FOR_TESTING


#include "../examples/Parameterization.c"


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


#define __ASSERT_TIMEOUT_EQL(__Expected__) \
    __WRAP_SHORT(__AssertTimeoutEql(__Expected__)); \



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

static __PARAMETERIZATION __Parameterization;
static char __HandlerLog [1024];



//---------------------------------------------------------------------------
// NON-PUBLIC METHODS
//---------------------------------------------------------------------------

static RESOLVBUS_RESULT __AssertTimeoutEql(uint32_t Expected)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    uint32_t Actual = 0;
    __WRAP(ResolVBus_LiveTransceiver_GetTimeout(&__Parameterization.Transceiver, &Actual));

    __WRAP(AssertEql(Expected, "Expected", Actual, "Actual"));

    return Result;
}


static RESOLVBUS_RESULT __TestTransceiverHandler(RESOLVBUS_LIVETRANSCEIVER *Transceiver, const RESOLVBUS_LIVETRANSCEIVEREVENT *Event)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    if (Event->EventType == RESOLVBUS_LIVETRANSCEIVEREVENTTYPE_DECODER) {
        const RESOLVBUS_LIVEDECODEREVENT *DecoderEvent = Event->DecoderEvent;
        if (DecoderEvent->EventType == RESOLVBUS_LIVEDECODEREVENTTYPE_PACKETHEADER) {
            // nop
        } else if (DecoderEvent->EventType == RESOLVBUS_LIVEDECODEREVENTTYPE_PACKETFRAME) {
            // nop
        } else if (DecoderEvent->EventType == RESOLVBUS_LIVEDECODEREVENTTYPE_PACKETEND) {
            __APPEND_LOG("Decoder: PACKETEND\n");
        } else if (DecoderEvent->EventType == RESOLVBUS_LIVEDECODEREVENTTYPE_DATAGRAM) {
            __APPEND_LOG("Decoder: DATAGRAM\n");
        } else {
            __APPEND_LOG("Decoder: ??? (%d)", DecoderEvent->EventType);
        }
    } else if (Event->EventType == RESOLVBUS_LIVETRANSCEIVEREVENTTYPE_ENCODER) {
        const RESOLVBUS_LIVEENCODEREVENT *EncoderEvent = Event->EncoderEvent;
        if (EncoderEvent->EventType == RESOLVBUS_LIVEENCODEREVENTTYPE_TRANSMIT) {
            __APPEND_LOG("Encoder: TRANSMIT");
            const uint8_t *Bytes = EncoderEvent->TransmitBytes;
            for (size_t Index = 0; Index < EncoderEvent->TransmitLength; Index++) {
                __APPEND_LOG(" %02X", Bytes [Index]);
            }
            __APPEND_LOG("\n");
        } else {
            __APPEND_LOG("Encoder: ??? (%d)\n", EncoderEvent->EventType);
        }
    } else {
        __APPEND_LOG("???");
    }

    __WRAP(__TransceiverHandler(Transceiver, Event));

    return Result;
}



//---------------------------------------------------------------------------
// PUBLIC METHODS
//---------------------------------------------------------------------------

RESOLVBUS_RESULT RunTest_ParameterizationExample(void)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    __Parameterization = (__PARAMETERIZATION ) {
        .Transceiver = RESOLVBUS_LIVETRANSCEIVER_INITIALIZER,
    };

    __PARAMETERIZATION *Parameterization = &__Parameterization;

    __WRAP(__Initialize(Parameterization));

    if (Result == RESOLVBUS_OK) {
        Parameterization->Transceiver.Handler = __TestTransceiverHandler;
    }

    __WRAP(__HandleLoopCycle(Parameterization, 0x12345678, 0, NULL, 0));

    __ASSERT_LOG_EQL("");
    __ASSERT_TIMEOUT_EQL(UINT32_MAX);

    const uint8_t Bytes1 [] = {
        0xAA, 0x10, 0x00, 0x12, 0x71, 0x10, 0x00, 0x01, 0x0D, 0x4E,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x7F,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x7F,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x7F,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x7F,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x7F,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x7F,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x7F,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x7F,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x7F,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x7F,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x7F,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x7F,
        0x78, 0x56, 0x34, 0x12, 0x00, 0x6B,
    };

    __WRAP(__HandleLoopCycle(Parameterization, 0x12345678, 0, Bytes1, sizeof (Bytes1)));

    __ASSERT_LOG_EQL("Decoder: PACKETEND\n");
    __ASSERT_TIMEOUT_EQL(UINT32_MAX);
    __ASSERT_EQL(Parameterization->ShouldCorrectTimestamp, false);

    const uint8_t Bytes2 [] = {
        0xAA, 0x00, 0x00, 0x12, 0x71, 0x20, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x57,
    };

    __WRAP(__HandleLoopCycle(Parameterization, 0x12345678, 0, Bytes2, sizeof (Bytes2)));

    __ASSERT_LOG_EQL("Decoder: DATAGRAM\n");
    __ASSERT_TIMEOUT_EQL(UINT32_MAX);

    __WRAP(__HandleLoopCycle(Parameterization, 725097110, 0, Bytes1, sizeof (Bytes1)));

    __ASSERT_LOG_EQL("Decoder: PACKETEND\n");
    __ASSERT_TIMEOUT_EQL(UINT32_MAX);
    __ASSERT_EQL(Parameterization->ShouldCorrectTimestamp, true);

    __WRAP(__HandleLoopCycle(Parameterization, 725097110, 0, Bytes2, sizeof (Bytes2)));

    __ASSERT_LOG_EQL("Decoder: DATAGRAM\n");
    __ASSERT_TIMEOUT_EQL(UINT32_MAX);

    __WRAP(__HandleLoopCycle(Parameterization, 725097120, 0, Bytes1, sizeof (Bytes1)));

    __ASSERT_LOG_EQL("Decoder: PACKETEND\n");
    __ASSERT_TIMEOUT_EQL(UINT32_MAX);
    __ASSERT_EQL(Parameterization->ShouldCorrectTimestamp, true);

    __WRAP(__HandleLoopCycle(Parameterization, 725097120, 0, Bytes2, sizeof (Bytes2)));

    __ASSERT_LOG_EQL(
        "Decoder: DATAGRAM\n"
        "Encoder: TRANSMIT AA 12 71 20 00 20 00 03 00 00 00 00 00 00 00 39\n"
    );
    __ASSERT_TIMEOUT_EQL(16667);

    __WRAP(__HandleLoopCycle(Parameterization, 725097120, 16667, NULL, 0));

    __ASSERT_LOG_EQL("");
    __ASSERT_TIMEOUT_EQL(483333);

    const uint8_t Bytes3 [] = {
        0xaa, 0x20, 0x00, 0x12, 0x71, 0x20, 0x00, 0x01, 0x00, 0x00, 0x3c, 0x5a, 0x34, 0x27, 0x0c, 0x3e,
    };

    __WRAP(__HandleLoopCycle(Parameterization, 725097120, 0, Bytes3, sizeof (Bytes3)));

    __ASSERT_LOG_EQL(
        "Decoder: DATAGRAM\n"
        "Encoder: TRANSMIT AA 12 71 20 00 20 00 02 39 00 20 1A 38 2B 04 60\n"
    );
    __ASSERT_TIMEOUT_EQL(16667);

    __WRAP(__HandleLoopCycle(Parameterization, 725097120, 16667, NULL, 0));

    __ASSERT_LOG_EQL("");
    __ASSERT_TIMEOUT_EQL(483333);

    const uint8_t Bytes4 [] = {
        0xaa, 0x20, 0x00, 0x12, 0x71, 0x20, 0x00, 0x01, 0x39, 0x00, 0x20, 0x1a, 0x38, 0x2b, 0x04, 0x61,
    };

    __WRAP(__HandleLoopCycle(Parameterization, 725097120, 0, Bytes4, sizeof (Bytes4)));

    __ASSERT_LOG_EQL(
        "Decoder: DATAGRAM\n"
        "Encoder: TRANSMIT AA 12 71 20 00 20 00 02 3A 00 58 01 00 00 04 23\n"
    );
    __ASSERT_TIMEOUT_EQL(16667);

    __WRAP(__HandleLoopCycle(Parameterization, 725097120, 16667, NULL, 0));

    __ASSERT_LOG_EQL("");
    __ASSERT_TIMEOUT_EQL(483333);
    __ASSERT_EQL(Parameterization->ShouldCorrectTimestamp, true);

    const uint8_t Bytes5 [] = {
        0xaa, 0x20, 0x00, 0x12, 0x71, 0x20, 0x00, 0x01, 0x3a, 0x00, 0x58, 0x01, 0x00, 0x00, 0x04, 0x24,
    };

    __WRAP(__HandleLoopCycle(Parameterization, 725097120, 0, Bytes5, sizeof (Bytes5)));

    __ASSERT_LOG_EQL(
        "Decoder: DATAGRAM\n"
        "Encoder: TRANSMIT AA 12 71 20 00 20 00 06 00 00 00 00 00 00 00 36\n"
    );
    __ASSERT_TIMEOUT_EQL(16667);
    __ASSERT_EQL(Parameterization->ShouldCorrectTimestamp, false);

    __WRAP(__HandleLoopCycle(Parameterization, 725097120, 16667, NULL, 0));

    __ASSERT_LOG_EQL("");
    __ASSERT_TIMEOUT_EQL(1483333);

    __WRAP(__HandleLoopCycle(Parameterization, 0x12345678, 0, Bytes1, sizeof (Bytes1)));

    __ASSERT_LOG_EQL("Decoder: PACKETEND\n");
    __ASSERT_TIMEOUT_EQL(UINT32_MAX);

    return Result;
}
