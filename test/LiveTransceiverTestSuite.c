//
//  LiveTransceiverTestSuite.c
//  resol-vbus-c
//
//
//  Copyright (C) 2022, Daniel Wippermann. All rights reserved.
//

#include "Testing.h"



//---------------------------------------------------------------------------
// PRIVATE DEFINES
//---------------------------------------------------------------------------

#define __PREAMBLE() \
    RESOLVBUS_LIVETRANSCEIVER TransceiverLocal = RESOLVBUS_LIVETRANSCEIVER_INITIALIZER; \
    RESOLVBUS_LIVETRANSCEIVER *Transceiver = &TransceiverLocal; \
    uint8_t EncoderBuffer [512]; \
    uint8_t DecoderBuffer [512]; \
    __WRAP(ResolVBus_LiveTransceiver_Initialize(Transceiver, EncoderBuffer, sizeof (EncoderBuffer), DecoderBuffer, sizeof (DecoderBuffer), __GlobalHandler)); \
    __HandlerLog [0] = 0; \
    __AllGlobalEvents = false; \


#define __APPEND_LOG(...) \
    if (Result == RESOLVBUS_OK) { \
        AppendToLog(__HandlerLog, sizeof (__HandlerLog), __VA_ARGS__); \
    } \


#define __ASSERT_LOG_EQL(__Expected__) \
    __WRAP_SHORT(AssertStringEql(__Expected__, "Expected", __HandlerLog, "  Actual")); \
    if (Result == RESOLVBUS_OK) { \
        __HandlerLog [0] = 0; \
    } \


#define __ASSERT_TIMEOUT_EQL(__Expected__) \
    if (Result == RESOLVBUS_OK) { \
        uint32_t Microseconds = 0; \
        __WRAP(ResolVBus_LiveTransceiver_GetTimeout(Transceiver, &Microseconds)); \
        __ASSERT_EQL(Microseconds, __Expected__); \
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

static char __HandlerLog [4096];

static bool __AllGlobalEvents;


static const uint8_t __DatagramBytes_0000_7E11_0500_0000_00000000 [] = {
	0xAA, 0x00, 0x00, 0x11, 0x7E, 0x20, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4B,
};

static const uint8_t __DatagramBytes_0020_7E11_0100_0000_12345678 [] = {
	0xAA, 0x20, 0x00, 0x11, 0x7E, 0x20, 0x00, 0x01, 0x00, 0x00, 0x78, 0x56, 0x34, 0x12, 0x00, 0x1B,
};

static const uint8_t __DatagramBytes_0020_7E11_0100_1234_0000007B [] = {
	0xAA, 0x20, 0x00, 0x11, 0x7E, 0x20, 0x00, 0x01, 0x34, 0x12, 0x7B, 0x00, 0x00, 0x00, 0x00, 0x6E,
};

static const uint8_t __DatagramBytes_0020_7E11_1101_1234_12345678 [] = {
	0xAA, 0x20, 0x00, 0x11, 0x7E, 0x20, 0x01, 0x11, 0x34, 0x12, 0x78, 0x56, 0x34, 0x12, 0x00, 0x44,
};


//---------------------------------------------------------------------------
// NON-PUBLIC METHODS
//---------------------------------------------------------------------------

static RESOLVBUS_RESULT __GlobalHandler(RESOLVBUS_LIVETRANSCEIVER *Transceiver, const RESOLVBUS_LIVETRANSCEIVEREVENT *Event)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    if (Event->EventType == RESOLVBUS_LIVETRANSCEIVEREVENTTYPE_ACTION) {
        __APPEND_LOG("ActionEvent (g)\n");
    } else if (Event->EventType == RESOLVBUS_LIVETRANSCEIVEREVENTTYPE_TIMEOUT) {
        if (__AllGlobalEvents) {
            __APPEND_LOG("TimeoutEvent (g)\n");
        }
    } else if (Event->EventType == RESOLVBUS_LIVETRANSCEIVEREVENTTYPE_ENCODER) {
        const char *EventName;

        switch (Event->EncoderEvent->EventType) {
        case RESOLVBUS_LIVEENCODEREVENTTYPE_IDLE: EventName = "IDLE"; break;
        case RESOLVBUS_LIVEENCODEREVENTTYPE_TRANSMIT: EventName = "TRANSMIT"; break;
        default: EventName = NULL; break;
        }

        if (EventName) {
            __APPEND_LOG("EncoderEvent = %s\n", EventName);
        } else {
            __APPEND_LOG("EncoderEvent = ??? (%d)\n", Event->EventType);
        }
    } else if (Event->EventType == RESOLVBUS_LIVETRANSCEIVEREVENTTYPE_DECODER) {
        if (__AllGlobalEvents) {
            const char *EventName;

            switch (Event->DecoderEvent->EventType) {
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
                __APPEND_LOG("DecoderEvent (g) = %s\n", EventName);
            } else {
                __APPEND_LOG("DecoderEvent (g) = ??? (%d)\n", Event->EventType);
            }
        }
    } else {
        __APPEND_LOG("UnknownEvent\n");
    }

    return Result;
}


static RESOLVBUS_RESULT __Handler(RESOLVBUS_LIVETRANSCEIVER *Transceiver, const RESOLVBUS_LIVETRANSCEIVEREVENT *Event)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    if (Event->EventType == RESOLVBUS_LIVETRANSCEIVEREVENTTYPE_ACTION) {
        __APPEND_LOG("ActionEvent\n");
    } else if (Event->EventType == RESOLVBUS_LIVETRANSCEIVEREVENTTYPE_TIMEOUT) {
        __APPEND_LOG("TimeoutEvent\n");
    } else if (Event->EventType == RESOLVBUS_LIVETRANSCEIVEREVENTTYPE_ENCODER) {
        const char *EventName;

        switch (Event->EncoderEvent->EventType) {
        case RESOLVBUS_LIVEENCODEREVENTTYPE_IDLE: EventName = "IDLE"; break;
        case RESOLVBUS_LIVEENCODEREVENTTYPE_TRANSMIT: EventName = "TRANSMIT"; break;
        default: EventName = NULL; break;
        }

        if (EventName) {
            __APPEND_LOG("EncoderEvent (l) = %s\n", EventName);
        } else {
            __APPEND_LOG("EncoderEvent (l) = ??? (%d)\n", Event->EventType);
        }
    } else if (Event->EventType == RESOLVBUS_LIVETRANSCEIVEREVENTTYPE_DECODER) {
        const char *EventName;

        switch (Event->DecoderEvent->EventType) {
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
            __APPEND_LOG("DecoderEvent = %s\n", EventName);
        } else {
            __APPEND_LOG("DecoderEvent = ??? (%d)\n", Event->EventType);
        }
    } else {
        __APPEND_LOG("UnknownEvent\n");
    }

    return Result;
}


static RESOLVBUS_RESULT __TestInitialize(void)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    RESOLVBUS_LIVETRANSCEIVER TransceiverLocal = RESOLVBUS_LIVETRANSCEIVER_INITIALIZER;
    RESOLVBUS_LIVETRANSCEIVER *Transceiver = &TransceiverLocal;
    uint8_t EncoderBuffer [512];
    uint8_t DecoderBuffer [512];
    __WRAP(ResolVBus_LiveTransceiver_Initialize(Transceiver, EncoderBuffer, sizeof (EncoderBuffer), DecoderBuffer, sizeof (DecoderBuffer), __Handler));

    __ASSERT_POINTER_EQL(Transceiver->Encoder.Buffer, EncoderBuffer);
    __ASSERT_EQL(Transceiver->Encoder.BufferLength, sizeof (EncoderBuffer));
    __ASSERT_POINTER_EQL(Transceiver->Decoder.Event.FrameDataBuffer, DecoderBuffer);
    __ASSERT_EQL(Transceiver->Decoder.Event.FrameDataBufferLength, sizeof (DecoderBuffer));
    __ASSERT_POINTER_EQL((const void *) Transceiver->Handler, (const void *) __Handler);
    __ASSERT_EQL(Transceiver->ActionSet, false);
    __ASSERT_EQL(Transceiver->ActionTries, 0);
    __ASSERT_EQL(Transceiver->ActionNextTimeout, 0);
    __ASSERT_EQL(Transceiver->ActionTimeoutIncr, 0);
    __ASSERT_POINTER_EQL((const void *) Transceiver->ActionHandler, NULL);

    __ASSERT_LOG_EQL("");

    return Result;
}


static RESOLVBUS_RESULT __TestGetTimeout(void)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    __PREAMBLE();

    uint32_t Microseconds = 0;
    __WRAP(ResolVBus_LiveTransceiver_GetTimeout(Transceiver, &Microseconds));

    __ASSERT_EQL(Microseconds, UINT32_MAX);

    __WRAP(ResolVBus_LiveTransceiver_HandleTimer(Transceiver, 0));

    __ASSERT_LOG_EQL("");

    // FIXME(daniel): add more tests
    // __FAIL(NYI);

    return Result;
}


static RESOLVBUS_RESULT __TestHandleTimer(void)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    __PREAMBLE();

    uint32_t Microseconds = 0;
    __WRAP(ResolVBus_LiveTransceiver_GetTimeout(Transceiver, &Microseconds));

    __ASSERT_EQL(Microseconds, UINT32_MAX);

    __WRAP(ResolVBus_LiveTransceiver_HandleTimer(Transceiver, 0));

    __ASSERT_LOG_EQL("");

    __WRAP(ResolVBus_LiveTransceiver_GetTimeout(Transceiver, &Microseconds));

    __ASSERT_EQL(Microseconds, UINT32_MAX);

    __WRAP(ResolVBus_LiveTransceiver_GetValueById(Transceiver, 0x7E11, 0x1234, 0, NULL, __Handler));

    __ASSERT_EQL(Transceiver->ActionTries, 3);
    __ASSERT_EQL(Transceiver->ActionTimeout, 500000);
    __ASSERT_EQL(Transceiver->ActionNextTimeout, 1000000);
    __ASSERT_EQL(Transceiver->ActionTimeoutIncr, 500000);

    __ASSERT_LOG_EQL("");

    __WRAP(ResolVBus_LiveTransceiver_GetTimeout(Transceiver, &Microseconds));

    __ASSERT_EQL(Microseconds, 0);

    __WRAP(ResolVBus_LiveTransceiver_HandleTimer(Transceiver, 0));

    __ASSERT_LOG_EQL("EncoderEvent = TRANSMIT\n");

    __WRAP(ResolVBus_LiveTransceiver_GetTimeout(Transceiver, &Microseconds));

    __ASSERT_EQL(Microseconds, 16667);

    __WRAP(ResolVBus_LiveTransceiver_HandleTimer(Transceiver, 16667));

    __ASSERT_LOG_EQL("");

    __WRAP(ResolVBus_LiveTransceiver_GetTimeout(Transceiver, &Microseconds));

    __ASSERT_EQL(Microseconds, 483333);

    __WRAP(ResolVBus_LiveTransceiver_HandleTimer(Transceiver, 483333));

    __ASSERT_LOG_EQL("EncoderEvent = TRANSMIT\n");

    __WRAP(ResolVBus_LiveTransceiver_GetTimeout(Transceiver, &Microseconds));

    __ASSERT_EQL(Microseconds, 16667);

    __WRAP(ResolVBus_LiveTransceiver_HandleTimer(Transceiver, 16667));

    __ASSERT_LOG_EQL("");

    __WRAP(ResolVBus_LiveTransceiver_GetTimeout(Transceiver, &Microseconds));

    __ASSERT_EQL(Microseconds, 983333);

    __WRAP(ResolVBus_LiveTransceiver_HandleTimer(Transceiver, 983333));

    __ASSERT_LOG_EQL("EncoderEvent = TRANSMIT\n");

    __WRAP(ResolVBus_LiveTransceiver_GetTimeout(Transceiver, &Microseconds));

    __ASSERT_EQL(Microseconds, 16667);

    __WRAP(ResolVBus_LiveTransceiver_HandleTimer(Transceiver, 16667));

    __ASSERT_LOG_EQL("");

    __WRAP(ResolVBus_LiveTransceiver_GetTimeout(Transceiver, &Microseconds));

    __ASSERT_EQL(Microseconds, 1483333);

    __WRAP(ResolVBus_LiveTransceiver_HandleTimer(Transceiver, 1483333));

    __ASSERT_LOG_EQL("TimeoutEvent\n");

    __WRAP(ResolVBus_LiveTransceiver_GetTimeout(Transceiver, &Microseconds));

    __ASSERT_EQL(Microseconds, UINT32_MAX);

    return Result;
}


static RESOLVBUS_RESULT __TestDecode(void)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    __PREAMBLE();

    __AllGlobalEvents = true;

    __WRAP(ResolVBus_LiveTransceiver_Decode(Transceiver, TestData_Live1, 172));

    __ASSERT_LOG_EQL(
        "DecoderEvent (g) = PACKETHEADER\n"
        "DecoderEvent (g) = PACKETFRAME\n"
        "DecoderEvent (g) = PACKETFRAME\n"
        "DecoderEvent (g) = PACKETFRAME\n"
        "DecoderEvent (g) = PACKETFRAME\n"
        "DecoderEvent (g) = PACKETFRAME\n"
        "DecoderEvent (g) = PACKETFRAME\n"
        "DecoderEvent (g) = PACKETFRAME\n"
        "DecoderEvent (g) = PACKETFRAME\n"
        "DecoderEvent (g) = PACKETFRAME\n"
        "DecoderEvent (g) = PACKETFRAME\n"
        "DecoderEvent (g) = PACKETFRAME\n"
        "DecoderEvent (g) = PACKETFRAME\n"
        "DecoderEvent (g) = PACKETFRAME\n"
        "DecoderEvent (g) = PACKETFRAME\n"
        "DecoderEvent (g) = PACKETFRAME\n"
        "DecoderEvent (g) = PACKETFRAME\n"
        "DecoderEvent (g) = PACKETFRAME\n"
        "DecoderEvent (g) = PACKETFRAME\n"
        "DecoderEvent (g) = PACKETFRAME\n"
        "DecoderEvent (g) = PACKETFRAME\n"
        "DecoderEvent (g) = PACKETFRAME\n"
        "DecoderEvent (g) = PACKETFRAME\n"
        "DecoderEvent (g) = PACKETFRAME\n"
        "DecoderEvent (g) = PACKETFRAME\n"
        "DecoderEvent (g) = PACKETFRAME\n"
        "DecoderEvent (g) = PACKETFRAME\n"
        "DecoderEvent (g) = PACKETFRAME\n"
        "DecoderEvent (g) = PACKETEND\n"
    );

    return Result;
}


static RESOLVBUS_RESULT __TestWaitForFreeBus(void)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    __PREAMBLE();

    __WRAP(ResolVBus_LiveTransceiver_WaitForFreeBus(Transceiver, NULL, __Handler));

    __WRAP(ResolVBus_LiveTransceiver_Decode(Transceiver, __DatagramBytes_0000_7E11_0500_0000_00000000, sizeof (__DatagramBytes_0000_7E11_0500_0000_00000000)));

    __ASSERT_LOG_EQL("DecoderEvent = DATAGRAM\n");

    __WRAP(ResolVBus_LiveTransceiver_WaitForFreeBus(Transceiver, NULL, __Handler));

    __ASSERT_EQL(Transceiver->ActionTries, 1);
    __ASSERT_EQL(Transceiver->ActionNextTimeout, 20000000);
    __ASSERT_EQL(Transceiver->ActionTimeoutIncr, 0);
    __ASSERT_EQL(Transceiver->ActionTimeout, 20000000);

    __WRAP(ResolVBus_LiveTransceiver_HandleTimer(Transceiver, 20000000));

    __ASSERT_EQL(Transceiver->ActionTries, 0);
    __ASSERT_EQL(Transceiver->ActionNextTimeout, 0);
    __ASSERT_EQL(Transceiver->ActionTimeoutIncr, 0);
    __ASSERT_EQL(Transceiver->ActionTimeout, 0);

    __ASSERT_LOG_EQL("TimeoutEvent\n");

    return Result;
}


static RESOLVBUS_RESULT __TestReleaseBus(void)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    __PREAMBLE();

    __AllGlobalEvents = true;

    __WRAP(ResolVBus_LiveTransceiver_ReleaseBus(Transceiver, 0x7E11, NULL, __Handler));

    __ASSERT_LOG_EQL("");
    __ASSERT_TIMEOUT_EQL(0);

    __WRAP(ResolVBus_LiveTransceiver_HandleTimer(Transceiver, 0));

    __ASSERT_LOG_EQL("EncoderEvent = TRANSMIT\n");
    __ASSERT_TIMEOUT_EQL(16667);

    __WRAP(ResolVBus_LiveTransceiver_HandleTimer(Transceiver, 16667));

    __ASSERT_LOG_EQL("");
    __ASSERT_TIMEOUT_EQL(1483333);

    __WRAP(ResolVBus_LiveTransceiver_Decode(Transceiver, TestData_Live1, 16));

    __ASSERT_LOG_EQL(
        "DecoderEvent = PACKETHEADER\n"
        "DecoderEvent (g) = PACKETHEADER\n"
        "DecoderEvent (g) = PACKETFRAME\n"
    );

    return Result;
}


static RESOLVBUS_RESULT __TestGetValueById(void)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    __PREAMBLE();

    __WRAP(ResolVBus_LiveTransceiver_GetValueById(Transceiver, 0x7E11, 0x1234, 0, NULL, __Handler));

    __ASSERT_LOG_EQL("");

    __WRAP(ResolVBus_LiveTransceiver_HandleTimer(Transceiver, 0));

    __ASSERT_LOG_EQL("EncoderEvent = TRANSMIT\n");

    __WRAP(ResolVBus_LiveTransceiver_Decode(Transceiver, __DatagramBytes_0020_7E11_0100_1234_0000007B, sizeof (__DatagramBytes_0020_7E11_0100_1234_0000007B)));

    __ASSERT_LOG_EQL("DecoderEvent = DATAGRAM\n");

    return Result;
}


static RESOLVBUS_RESULT __TestSetValueById(void)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    __PREAMBLE();

    __WRAP(ResolVBus_LiveTransceiver_SetValueById(Transceiver, 0x7E11, 0x1234, 0, 123, NULL, __Handler));

    __ASSERT_LOG_EQL("");

    __WRAP(ResolVBus_LiveTransceiver_HandleTimer(Transceiver, 0));

    __ASSERT_LOG_EQL("EncoderEvent = TRANSMIT\n");

    __WRAP(ResolVBus_LiveTransceiver_Decode(Transceiver, __DatagramBytes_0020_7E11_0100_1234_0000007B, sizeof (__DatagramBytes_0020_7E11_0100_1234_0000007B)));

    __ASSERT_LOG_EQL("DecoderEvent = DATAGRAM\n");

    return Result;
}


static RESOLVBUS_RESULT __TestGetValueIdByIdHash(void)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    __PREAMBLE();

    __WRAP(ResolVBus_LiveTransceiver_GetValueIdByIdHash(Transceiver, 0x7E11, 0x12345678, NULL, __Handler));

    __ASSERT_LOG_EQL("");

    __WRAP(ResolVBus_LiveTransceiver_HandleTimer(Transceiver, 0));

    __ASSERT_LOG_EQL("EncoderEvent = TRANSMIT\n");

    __WRAP(ResolVBus_LiveTransceiver_Decode(Transceiver, __DatagramBytes_0020_7E11_1101_1234_12345678, sizeof (__DatagramBytes_0020_7E11_1101_1234_12345678)));

    __ASSERT_LOG_EQL("DecoderEvent = DATAGRAM\n");

    return Result;
}


static RESOLVBUS_RESULT __IntegrationTest(void)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    __PREAMBLE();

    // 1. wait for free bus
    __WRAP(ResolVBus_LiveTransceiver_WaitForFreeBus(Transceiver, NULL, __Handler));

    __ASSERT_LOG_EQL("");
    __ASSERT_TIMEOUT_EQL(20000000);

    __WRAP(ResolVBus_LiveTransceiver_HandleTimer(Transceiver, 0));

    __ASSERT_LOG_EQL("");
    __ASSERT_TIMEOUT_EQL(20000000);

    __WRAP(ResolVBus_LiveTransceiver_Decode(Transceiver, __DatagramBytes_0000_7E11_0500_0000_00000000, sizeof (__DatagramBytes_0000_7E11_0500_0000_00000000)));

    __ASSERT_LOG_EQL("DecoderEvent = DATAGRAM\n");
    __ASSERT_TIMEOUT_EQL(UINT32_MAX);

    // 2. get changeset ID
    __WRAP(ResolVBus_LiveTransceiver_GetValueById(Transceiver, 0x7E11, 0, 0, NULL, __Handler));

    __ASSERT_LOG_EQL("");
    __ASSERT_TIMEOUT_EQL(0);

    __WRAP(ResolVBus_LiveTransceiver_HandleTimer(Transceiver, 0));

    __ASSERT_LOG_EQL("EncoderEvent = TRANSMIT\n");
    __ASSERT_TIMEOUT_EQL(16667);

    __WRAP(ResolVBus_LiveTransceiver_HandleTimer(Transceiver, 16667));

    __ASSERT_LOG_EQL("");
    __ASSERT_TIMEOUT_EQL(483333);

    __WRAP(ResolVBus_LiveTransceiver_Decode(Transceiver, __DatagramBytes_0020_7E11_0100_0000_12345678, sizeof (__DatagramBytes_0020_7E11_0100_0000_12345678)));

    __ASSERT_LOG_EQL("DecoderEvent = DATAGRAM\n");
    __ASSERT_TIMEOUT_EQL(UINT32_MAX);

    // 3. get value ID by value ID hash
    __WRAP(ResolVBus_LiveTransceiver_GetValueIdByIdHash(Transceiver, 0x7E11, 0x12345678, NULL, __Handler));

    __ASSERT_LOG_EQL("");
    __ASSERT_TIMEOUT_EQL(0);

    __WRAP(ResolVBus_LiveTransceiver_HandleTimer(Transceiver, 0));

    __ASSERT_LOG_EQL("EncoderEvent = TRANSMIT\n");
    __ASSERT_TIMEOUT_EQL(16667);

    __WRAP(ResolVBus_LiveTransceiver_HandleTimer(Transceiver, 16667));

    __ASSERT_LOG_EQL("");
    __ASSERT_TIMEOUT_EQL(483333);

    __WRAP(ResolVBus_LiveTransceiver_Decode(Transceiver, __DatagramBytes_0020_7E11_1101_1234_12345678, sizeof (__DatagramBytes_0020_7E11_1101_1234_12345678)));

    __ASSERT_LOG_EQL("DecoderEvent = DATAGRAM\n");

    // 4. get changeset ID again to resync
    __WRAP(ResolVBus_LiveTransceiver_GetValueById(Transceiver, 0x7E11, 0, 0, NULL, __Handler));

    __ASSERT_LOG_EQL("");
    __ASSERT_TIMEOUT_EQL(0);

    __WRAP(ResolVBus_LiveTransceiver_HandleTimer(Transceiver, 0));

    __ASSERT_LOG_EQL("EncoderEvent = TRANSMIT\n");
    __ASSERT_TIMEOUT_EQL(16667);

    __WRAP(ResolVBus_LiveTransceiver_HandleTimer(Transceiver, 16667));

    __ASSERT_LOG_EQL("");
    __ASSERT_TIMEOUT_EQL(483333);

    __WRAP(ResolVBus_LiveTransceiver_Decode(Transceiver, __DatagramBytes_0020_7E11_0100_0000_12345678, sizeof (__DatagramBytes_0020_7E11_0100_0000_12345678)));

    __ASSERT_LOG_EQL("DecoderEvent = DATAGRAM\n");
    __ASSERT_TIMEOUT_EQL(UINT32_MAX);

    // 5. get value by ID
    __WRAP(ResolVBus_LiveTransceiver_GetValueById(Transceiver, 0x7E11, 0x1234, 0, NULL, __Handler));

    __ASSERT_LOG_EQL("");
    __ASSERT_TIMEOUT_EQL(0);

    __WRAP(ResolVBus_LiveTransceiver_HandleTimer(Transceiver, 0));

    __ASSERT_LOG_EQL("EncoderEvent = TRANSMIT\n");
    __ASSERT_TIMEOUT_EQL(16667);

    __WRAP(ResolVBus_LiveTransceiver_HandleTimer(Transceiver, 16667));

    __ASSERT_LOG_EQL("");
    __ASSERT_TIMEOUT_EQL(483333);

    __WRAP(ResolVBus_LiveTransceiver_Decode(Transceiver, __DatagramBytes_0020_7E11_0100_1234_0000007B, sizeof (__DatagramBytes_0020_7E11_0100_1234_0000007B)));

    __ASSERT_LOG_EQL("DecoderEvent = DATAGRAM\n");
    __ASSERT_TIMEOUT_EQL(UINT32_MAX);

    // 6. get changeset ID again to resync
    __WRAP(ResolVBus_LiveTransceiver_GetValueById(Transceiver, 0x7E11, 0, 0, NULL, __Handler));

    __ASSERT_LOG_EQL("");
    __ASSERT_TIMEOUT_EQL(0);

    __WRAP(ResolVBus_LiveTransceiver_HandleTimer(Transceiver, 0));

    __ASSERT_LOG_EQL("EncoderEvent = TRANSMIT\n");
    __ASSERT_TIMEOUT_EQL(16667);

    __WRAP(ResolVBus_LiveTransceiver_HandleTimer(Transceiver, 16667));

    __ASSERT_LOG_EQL("");
    __ASSERT_TIMEOUT_EQL(483333);

    __WRAP(ResolVBus_LiveTransceiver_Decode(Transceiver, __DatagramBytes_0020_7E11_0100_0000_12345678, sizeof (__DatagramBytes_0020_7E11_0100_0000_12345678)));

    __ASSERT_LOG_EQL("DecoderEvent = DATAGRAM\n");
    __ASSERT_TIMEOUT_EQL(UINT32_MAX);

    // 7. set value by ID
    __WRAP(ResolVBus_LiveTransceiver_SetValueById(Transceiver, 0x7E11, 0x1234, 0, 123, NULL, __Handler));

    __ASSERT_LOG_EQL("");
    __ASSERT_TIMEOUT_EQL(0);

    __WRAP(ResolVBus_LiveTransceiver_HandleTimer(Transceiver, 0));

    __ASSERT_LOG_EQL("EncoderEvent = TRANSMIT\n");
    __ASSERT_TIMEOUT_EQL(16667);

    __WRAP(ResolVBus_LiveTransceiver_HandleTimer(Transceiver, 16667));

    __ASSERT_LOG_EQL("");
    __ASSERT_TIMEOUT_EQL(483333);

    __WRAP(ResolVBus_LiveTransceiver_Decode(Transceiver, __DatagramBytes_0020_7E11_0100_1234_0000007B, sizeof (__DatagramBytes_0020_7E11_0100_1234_0000007B)));

    __ASSERT_LOG_EQL("DecoderEvent = DATAGRAM\n");
    __ASSERT_TIMEOUT_EQL(UINT32_MAX);

    // 8. release bus
    __WRAP(ResolVBus_LiveTransceiver_ReleaseBus(Transceiver, 0x7E11, NULL, __Handler));

    __ASSERT_LOG_EQL("");
    __ASSERT_TIMEOUT_EQL(0);

    __WRAP(ResolVBus_LiveTransceiver_HandleTimer(Transceiver, 0));

    __ASSERT_LOG_EQL("EncoderEvent = TRANSMIT\n");
    __ASSERT_TIMEOUT_EQL(16667);

    __WRAP(ResolVBus_LiveTransceiver_HandleTimer(Transceiver, 16667));

    __ASSERT_LOG_EQL("");
    __ASSERT_TIMEOUT_EQL(1483333);

    __AllGlobalEvents = true;

    __WRAP(ResolVBus_LiveTransceiver_Decode(Transceiver, TestData_Live1, 172));

    __ASSERT_LOG_EQL(
        "DecoderEvent = PACKETHEADER\n"
        "DecoderEvent (g) = PACKETHEADER\n"
        "DecoderEvent (g) = PACKETFRAME\n"
        "DecoderEvent (g) = PACKETFRAME\n"
        "DecoderEvent (g) = PACKETFRAME\n"
        "DecoderEvent (g) = PACKETFRAME\n"
        "DecoderEvent (g) = PACKETFRAME\n"
        "DecoderEvent (g) = PACKETFRAME\n"
        "DecoderEvent (g) = PACKETFRAME\n"
        "DecoderEvent (g) = PACKETFRAME\n"
        "DecoderEvent (g) = PACKETFRAME\n"
        "DecoderEvent (g) = PACKETFRAME\n"
        "DecoderEvent (g) = PACKETFRAME\n"
        "DecoderEvent (g) = PACKETFRAME\n"
        "DecoderEvent (g) = PACKETFRAME\n"
        "DecoderEvent (g) = PACKETFRAME\n"
        "DecoderEvent (g) = PACKETFRAME\n"
        "DecoderEvent (g) = PACKETFRAME\n"
        "DecoderEvent (g) = PACKETFRAME\n"
        "DecoderEvent (g) = PACKETFRAME\n"
        "DecoderEvent (g) = PACKETFRAME\n"
        "DecoderEvent (g) = PACKETFRAME\n"
        "DecoderEvent (g) = PACKETFRAME\n"
        "DecoderEvent (g) = PACKETFRAME\n"
        "DecoderEvent (g) = PACKETFRAME\n"
        "DecoderEvent (g) = PACKETFRAME\n"
        "DecoderEvent (g) = PACKETFRAME\n"
        "DecoderEvent (g) = PACKETFRAME\n"
        "DecoderEvent (g) = PACKETFRAME\n"
        "DecoderEvent (g) = PACKETEND\n"
    );
    __ASSERT_TIMEOUT_EQL(UINT32_MAX);

    return Result;
}



//---------------------------------------------------------------------------
// PUBLIC METHODS
//---------------------------------------------------------------------------

RESOLVBUS_RESULT RunTestSuite_LiveTransceiver(void)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    __WRAP(__TestInitialize());
    __WRAP(__TestGetTimeout());
    __WRAP(__TestHandleTimer());
    __WRAP(__TestDecode());
    __WRAP(__TestWaitForFreeBus());
    __WRAP(__TestReleaseBus());
    __WRAP(__TestGetValueById());
    __WRAP(__TestSetValueById());
    __WRAP(__TestGetValueIdByIdHash());
    __WRAP(__IntegrationTest());

    return Result;
}
