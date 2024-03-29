//
//  MasterExampleTest.c
//  resol-vbus
//
//
//  Copyright (C) 2022, Daniel Wippermann. All rights reserved.
//

#define __IGNORE_MAIN_FOR_TESTING


#include "../examples/Master.c"


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


static RESOLVBUS_RESULT __TestDecoderHandler(RESOLVBUS_LIVEDECODER *Decoder, const RESOLVBUS_LIVEDECODEREVENT *Event)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    const char *EventName;
    switch (Event->EventType) {
    case RESOLVBUS_LIVEDECODEREVENTTYPE_PACKETHEADER: EventName = "PACKETHEADER"; break;
    case RESOLVBUS_LIVEDECODEREVENTTYPE_PACKETFRAME: EventName = "PACKETFRAME"; break;
    case RESOLVBUS_LIVEDECODEREVENTTYPE_PACKETEND: EventName = "PACKETEND"; break;
    case RESOLVBUS_LIVEDECODEREVENTTYPE_DATAGRAM: EventName = "DATAGRAM"; break;
    case RESOLVBUS_LIVEDECODEREVENTTYPE_TELEGRAMHEADER: EventName = "TELEGRAMHEADER"; break;
    case RESOLVBUS_LIVEDECODEREVENTTYPE_TELEGRAMFRAME: EventName = "TELEGRAMFRAME"; break;
    case RESOLVBUS_LIVEDECODEREVENTTYPE_TELEGRAMEND: EventName = "TELEGRAMEND"; break;
    default: EventName = NULL; break;
    }

    if (EventName) {
        __APPEND_LOG("Event = %s\n", EventName);
    } else {
        __APPEND_LOG("Event = ??? (%d)\n", Event->EventType);
    }

    __WRAP(__DecoderHandler(Decoder, Event));

    return Result;
}



//---------------------------------------------------------------------------
// PUBLIC METHODS
//---------------------------------------------------------------------------

RESOLVBUS_RESULT RunTest_MasterExample(void)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    __MASTER Master = {
        .Decoder = RESOLVBUS_LIVEDECODER_INITIALIZER,
        .Encoder = RESOLVBUS_LIVEENCODER_INITIALIZER,
    };

    __WRAP(__Initialize(&Master));

    if (Result == RESOLVBUS_OK) {
        Master.Encoder.Handler = __TestEncoderHandler;
        Master.Decoder.Handler = __TestDecoderHandler;
    }

    __WRAP(__HandleLoopCycle(&Master, false, 0, NULL, 0));

    __ASSERT_LOG_EQL("");
    __ASSERT_EQL(Master.Encoder.Phase, RESOLVBUS_LIVEENCODERPHASE_SUSPENDED);
    __ASSERT_EQL(Master.Phase, 0);

    __WRAP(__HandleLoopCycle(&Master, true, 0, NULL, 0));

    __ASSERT_LOG_EQL("Event = TRANSMIT, Data = 0xAA 0x10 0x00 0x34 0x12 0x10 0x00 0x01 0x02 0x16 0x00 0x00 0x00 0x00 0x00 0x7F 0x00 0x00 0x7F 0x7F 0x0C 0x75\n");
    __ASSERT_EQL(Master.Encoder.Phase, RESOLVBUS_LIVEENCODERPHASE_TRANSMITTING);
    __ASSERT_EQL(Master.Encoder.PhaseTimeoutUs, 22917);
    __ASSERT_EQL(Master.Phase, 1);

    __WRAP(__HandleLoopCycle(&Master, true, 22917, NULL, 0));

    __ASSERT_LOG_EQL("Event = IDLE\nEvent = TRANSMIT, Data = 0xAA 0x15 0x00 0x34 0x12 0x10 0x00 0x01 0x03 0x10 0x00 0x00 0x00 0x00 0x00 0x7F 0x00 0x00 0x00 0x00 0x00 0x7F 0x00 0x00 0x7F 0x7F 0x0C 0x75\n");
    __ASSERT_EQL(Master.Encoder.Phase, RESOLVBUS_LIVEENCODERPHASE_TRANSMITTING);
    __ASSERT_EQL(Master.Encoder.PhaseTimeoutUs, 29167);
    __ASSERT_EQL(Master.Phase, 2);

    __WRAP(__HandleLoopCycle(&Master, false, 29167, NULL, 0));

    __ASSERT_LOG_EQL("Event = IDLE\nEvent = TRANSMIT, Data = 0xAA 0x45 0x23 0x34 0x12 0x10 0x00 0x03 0x00 0x3E\n");
    __ASSERT_EQL(Master.Encoder.Phase, RESOLVBUS_LIVEENCODERPHASE_TRANSMITTING);
    __ASSERT_EQL(Master.Encoder.PhaseTimeoutUs, 10417);
    __ASSERT_EQL(Master.Phase, 3);

    __WRAP(__HandleLoopCycle(&Master, false, 10417, NULL, 0));

    __ASSERT_LOG_EQL("");
    __ASSERT_EQL(Master.Encoder.Phase, RESOLVBUS_LIVEENCODERPHASE_SUSPENDEDWITHTIMEOUT);
    __ASSERT_EQL(Master.Encoder.PhaseTimeoutUs, 200000);
    __ASSERT_EQL(Master.Phase, 3);

    const uint8_t Bytes1 [] = {
        0xAA, 0x34, 0x12, 0x45, 0x23, 0x10, 0x00, 0x01, 0x00, 0x40,
    };

    __WRAP(__HandleLoopCycle(&Master, false, 0, Bytes1, sizeof (Bytes1)));

    __ASSERT_LOG_EQL("Event = PACKETHEADER\nEvent = PACKETEND\n");
    __ASSERT_EQL(Master.Encoder.Phase, RESOLVBUS_LIVEENCODERPHASE_SUSPENDEDWITHTIMEOUT);
    __ASSERT_EQL(Master.Encoder.PhaseTimeoutUs, 200000);
    __ASSERT_EQL(Master.Phase, 3);

    __WRAP(__HandleLoopCycle(&Master, false, 200000, NULL, 0));

    __ASSERT_LOG_EQL("Event = IDLE\nEvent = TRANSMIT, Data = 0xAA 0x00 0x00 0x34 0x12 0x20 0x00 0x05 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x14\n");
    __ASSERT_EQL(Master.Encoder.Phase, RESOLVBUS_LIVEENCODERPHASE_TRANSMITTING);
    __ASSERT_EQL(Master.Encoder.PhaseTimeoutUs, 16667);
    __ASSERT_EQL(Master.Phase, 4);

    __WRAP(__HandleLoopCycle(&Master, false, 16667, NULL, 0));

    __ASSERT_LOG_EQL("");
    __ASSERT_EQL(Master.Encoder.Phase, RESOLVBUS_LIVEENCODERPHASE_SUSPENDEDWITHTIMEOUT);
    __ASSERT_EQL(Master.Encoder.PhaseTimeoutUs, 400000);
    __ASSERT_EQL(Master.Phase, 4);

    __WRAP(__HandleLoopCycle(&Master, false, 400000, NULL, 0));

    __ASSERT_LOG_EQL("Event = IDLE\n");
    __ASSERT_EQL(Master.Encoder.Phase, RESOLVBUS_LIVEENCODERPHASE_SUSPENDED);
    __ASSERT_EQL(Master.Phase, 0);

    __WRAP(__HandleLoopCycle(&Master, true, 0, NULL, 0));

    __ASSERT_LOG_EQL("Event = TRANSMIT, Data = 0xAA 0x10 0x00 0x34 0x12 0x10 0x00 0x01 0x02 0x16 0x00 0x00 0x00 0x00 0x00 0x7F 0x00 0x00 0x7F 0x7F 0x0C 0x75\n");
    __ASSERT_EQL(Master.Encoder.Phase, RESOLVBUS_LIVEENCODERPHASE_TRANSMITTING);
    __ASSERT_EQL(Master.Encoder.PhaseTimeoutUs, 22917);
    __ASSERT_EQL(Master.Phase, 1);

    __WRAP(__HandleLoopCycle(&Master, true, 22917, NULL, 0));

    __ASSERT_LOG_EQL("Event = IDLE\nEvent = TRANSMIT, Data = 0xAA 0x15 0x00 0x34 0x12 0x10 0x00 0x01 0x03 0x10 0x00 0x00 0x00 0x00 0x00 0x7F 0x00 0x00 0x00 0x00 0x00 0x7F 0x00 0x00 0x7F 0x7F 0x0C 0x75\n");
    __ASSERT_EQL(Master.Encoder.Phase, RESOLVBUS_LIVEENCODERPHASE_TRANSMITTING);
    __ASSERT_EQL(Master.Encoder.PhaseTimeoutUs, 29167);
    __ASSERT_EQL(Master.Phase, 2);

    __WRAP(__HandleLoopCycle(&Master, false, 29167, NULL, 0));

    __ASSERT_LOG_EQL("Event = IDLE\nEvent = TRANSMIT, Data = 0xAA 0x45 0x23 0x34 0x12 0x10 0x00 0x03 0x00 0x3E\n");
    __ASSERT_EQL(Master.Encoder.Phase, RESOLVBUS_LIVEENCODERPHASE_TRANSMITTING);
    __ASSERT_EQL(Master.Encoder.PhaseTimeoutUs, 10417);
    __ASSERT_EQL(Master.Phase, 3);

    __WRAP(__HandleLoopCycle(&Master, false, 10417, NULL, 0));

    __ASSERT_LOG_EQL("");
    __ASSERT_EQL(Master.Encoder.Phase, RESOLVBUS_LIVEENCODERPHASE_SUSPENDEDWITHTIMEOUT);
    __ASSERT_EQL(Master.Encoder.PhaseTimeoutUs, 200000);
    __ASSERT_EQL(Master.Phase, 3);

    __WRAP(__HandleLoopCycle(&Master, false, 200000, NULL, 0));

    __ASSERT_LOG_EQL("Event = IDLE\nEvent = TRANSMIT, Data = 0xAA 0x00 0x00 0x34 0x12 0x20 0x00 0x05 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x14\n");
    __ASSERT_EQL(Master.Encoder.Phase, RESOLVBUS_LIVEENCODERPHASE_TRANSMITTING);
    __ASSERT_EQL(Master.Encoder.PhaseTimeoutUs, 16667);
    __ASSERT_EQL(Master.Phase, 4);

    __WRAP(__HandleLoopCycle(&Master, false, 16667, NULL, 0));

    __ASSERT_LOG_EQL("");
    __ASSERT_EQL(Master.Encoder.Phase, RESOLVBUS_LIVEENCODERPHASE_SUSPENDEDWITHTIMEOUT);
    __ASSERT_EQL(Master.Encoder.PhaseTimeoutUs, 400000);
    __ASSERT_EQL(Master.Phase, 4);

    const uint8_t Bytes2 [] = {
        0xaa, 0x34, 0x12, 0x20, 0x00, 0x20, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x76,
    };

    __WRAP(__HandleLoopCycle(&Master, false, 0, Bytes2, sizeof (Bytes2)));

    __ASSERT_LOG_EQL("Event = DATAGRAM\nEvent = TRANSMIT, Data = 0xAA 0x20 0x00 0x34 0x12 0x20 0x00 0x01 0x00 0x00 0x78 0x56 0x34 0x12 0x00 0x64\n");
    __ASSERT_EQL(Master.Encoder.Phase, RESOLVBUS_LIVEENCODERPHASE_TRANSMITTING);
    __ASSERT_EQL(Master.Encoder.PhaseTimeoutUs, 16667);
    __ASSERT_EQL(Master.Phase, 4);

    __WRAP(__HandleLoopCycle(&Master, false, 16667, NULL, 0));

    __ASSERT_LOG_EQL("");
    __ASSERT_EQL(Master.Encoder.Phase, RESOLVBUS_LIVEENCODERPHASE_SUSPENDEDWITHTIMEOUT);
    __ASSERT_EQL(Master.Encoder.PhaseTimeoutUs, 10000000);
    __ASSERT_EQL(Master.Phase, 4);

    const uint8_t Bytes3 [] = {
        0xaa, 0x34, 0x12, 0x20, 0x00, 0x20, 0x00, 0x03, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x75,
    };

    __WRAP(__HandleLoopCycle(&Master, false, 0, Bytes3, sizeof (Bytes3)));

    __ASSERT_LOG_EQL("Event = DATAGRAM\n");
    __ASSERT_EQL(Master.Encoder.Phase, RESOLVBUS_LIVEENCODERPHASE_SUSPENDEDWITHTIMEOUT);
    __ASSERT_EQL(Master.Encoder.PhaseTimeoutUs, 10000000);
    __ASSERT_EQL(Master.Phase, 4);

    const uint8_t Bytes4 [] = {
        0xaa, 0x34, 0x12, 0x20, 0x00, 0x20, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x73,
    };

    __WRAP(__HandleLoopCycle(&Master, false, 0, Bytes4, sizeof (Bytes4)));

    __ASSERT_LOG_EQL("Event = DATAGRAM\nEvent = IDLE\n");
    __ASSERT_EQL(Master.Encoder.Phase, RESOLVBUS_LIVEENCODERPHASE_SUSPENDED);
    __ASSERT_EQL(Master.Encoder.PhaseTimeoutUs, 0);
    __ASSERT_EQL(Master.Phase, 0);

    // __FAIL(NYI);

    return Result;
}

