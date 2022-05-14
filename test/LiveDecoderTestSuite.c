//
//  LiveDecoderTestSuite.c
//  resol-vbus
//
//
//  Copyright (C) 2022, Daniel Wippermann. All rights reserved.
//

#include "Testing.h"



//---------------------------------------------------------------------------
// PRIVATE DEFINES
//---------------------------------------------------------------------------

#define __PREAMBLE() \
    RESOLVBUS_LIVEDECODER DecoderLocal = { 0 }; \
    RESOLVBUS_LIVEDECODER *Decoder = &DecoderLocal; \
    uint8_t FrameDataBufferLocal [512] = { 0 }; \
    __WRAP(ResolVBus_LiveDecoder_Initialize(Decoder, FrameDataBufferLocal, sizeof (FrameDataBufferLocal), __Handler)); \
    __HandlerLog [0] = 0; \


#define __APPEND_LOG(...) \
    if (Result == RESOLVBUS_OK) { \
        AppendToLog(__HandlerLog, sizeof (__HandlerLog), __VA_ARGS__); \
    } \


#define __ASSERT_LOG_EQL(__Expected__) \
    __WRAP_SHORT(AssertStringEql(__Expected__, "Expected", __HandlerLog, "  Actual")); \
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

static RESOLVBUS_RESULT __Handler(RESOLVBUS_LIVEDECODER *Decoder, const RESOLVBUS_LIVEDECODEREVENT *Event)
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

    return Result;
}


static RESOLVBUS_RESULT __TestDecode(void)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    __PREAMBLE();

    __WRAP(ResolVBus_LiveDecoder_Decode(Decoder, TestData_Live1 + 0, 9));

    __ASSERT_LOG_EQL("");

    __WRAP(ResolVBus_LiveDecoder_Decode(Decoder, TestData_Live1 + 9, 1));

    __ASSERT_LOG_EQL("Event = PACKETHEADER\n");
    __ASSERT_EQL(Decoder->BufferIndex, 10);
    __ASSERT_EQL(Decoder->Event.DestinationAddress, 0x0010);
    __ASSERT_EQL(Decoder->Event.SourceAddress, 0x7E11);
    __ASSERT_EQL(Decoder->Event.ProtocolVersion, 0x10);
    __ASSERT_EQL(Decoder->Event.Command, 0x0100);
    __ASSERT_EQL(Decoder->Event.FrameCount, 27);
    __ASSERT_EQL(Decoder->Event.FrameIndex, 0);
    __ASSERT_EQL(Decoder->Event.FrameDataLength, 4);

    __WRAP(ResolVBus_LiveDecoder_Decode(Decoder, TestData_Live1 + 10, 5));

    __ASSERT_LOG_EQL("");

    __WRAP(ResolVBus_LiveDecoder_Decode(Decoder, TestData_Live1 + 15, 1));

    __ASSERT_LOG_EQL("Event = PACKETFRAME\n");
    __ASSERT_EQL(Decoder->BufferIndex, 10);
    __ASSERT_EQL(Decoder->Event.FrameData [0], 0x37);
    __ASSERT_EQL(Decoder->Event.FrameData [1], 0x00);
    __ASSERT_EQL(Decoder->Event.FrameData [2], 0x1d);
    __ASSERT_EQL(Decoder->Event.FrameData [3], 0x01);

    __WRAP(ResolVBus_LiveDecoder_Decode(Decoder, TestData_Live1 + 16, 155));

    __ASSERT_LOG_EQL("Event = PACKETFRAME\nEvent = PACKETFRAME\nEvent = PACKETFRAME\nEvent = PACKETFRAME\nEvent = PACKETFRAME\nEvent = PACKETFRAME\nEvent = PACKETFRAME\nEvent = PACKETFRAME\nEvent = PACKETFRAME\nEvent = PACKETFRAME\nEvent = PACKETFRAME\nEvent = PACKETFRAME\nEvent = PACKETFRAME\nEvent = PACKETFRAME\nEvent = PACKETFRAME\nEvent = PACKETFRAME\nEvent = PACKETFRAME\nEvent = PACKETFRAME\nEvent = PACKETFRAME\nEvent = PACKETFRAME\nEvent = PACKETFRAME\nEvent = PACKETFRAME\nEvent = PACKETFRAME\nEvent = PACKETFRAME\nEvent = PACKETFRAME\n");

    __WRAP(ResolVBus_LiveDecoder_Decode(Decoder, TestData_Live1 + 171, 1));

    __ASSERT_LOG_EQL("Event = PACKETFRAME\nEvent = PACKETEND\n");
    __ASSERT_EQL(Decoder->BufferIndex, 0);

    __WRAP(ResolVBus_LiveDecoder_Decode(Decoder, TestData_Live1 + 352, 15));

    __ASSERT_LOG_EQL("");
    __ASSERT_EQL(Decoder->BufferIndex, 15);

    __WRAP(ResolVBus_LiveDecoder_Decode(Decoder, TestData_Live1 + 367, 1));

    __ASSERT_LOG_EQL("Event = DATAGRAM\n");
    __ASSERT_EQL(Decoder->BufferIndex, 0);
    __ASSERT_EQL(Decoder->Event.DestinationAddress, 0x0000);
    __ASSERT_EQL(Decoder->Event.SourceAddress, 0x7E11);
    __ASSERT_EQL(Decoder->Event.ProtocolVersion, 0x20);
    __ASSERT_EQL(Decoder->Event.Command, 0x0500);
    __ASSERT_EQL(Decoder->Event.Param16, 0);
    __ASSERT_EQL(Decoder->Event.Param32, 0);

    __WRAP(ResolVBus_LiveDecoder_Decode(Decoder, TestData_Live2 + 0, 7));

    __ASSERT_LOG_EQL("");
    __ASSERT_EQL(Decoder->BufferIndex, 7);

    __WRAP(ResolVBus_LiveDecoder_Decode(Decoder, TestData_Live2 + 7, 1));

    __ASSERT_LOG_EQL("Event = TELEGRAMHEADER\n");
    __ASSERT_EQL(Decoder->BufferIndex, 8);
    __ASSERT_EQL(Decoder->Event.DestinationAddress, 0x7771);
    __ASSERT_EQL(Decoder->Event.SourceAddress, 0x2011);
    __ASSERT_EQL(Decoder->Event.ProtocolVersion, 0x30);
    __ASSERT_EQL(Decoder->Event.Command, 0x05);
    __ASSERT_EQL(Decoder->Event.FrameCount, 1);
    __ASSERT_EQL(Decoder->Event.FrameIndex, 0);
    __ASSERT_EQL(Decoder->Event.FrameDataLength, 7);

    __WRAP(ResolVBus_LiveDecoder_Decode(Decoder, TestData_Live2 + 8, 8));

    __ASSERT_LOG_EQL("");
    __ASSERT_EQL(Decoder->BufferIndex, 16);

    __WRAP(ResolVBus_LiveDecoder_Decode(Decoder, TestData_Live2 + 16, 1));

    __ASSERT_LOG_EQL("Event = TELEGRAMFRAME\nEvent = TELEGRAMEND\n");
    __ASSERT_EQL(Decoder->BufferIndex, 0);
    __ASSERT_EQL(Decoder->Event.DestinationAddress, 0x7771);
    __ASSERT_EQL(Decoder->Event.SourceAddress, 0x2011);
    __ASSERT_EQL(Decoder->Event.ProtocolVersion, 0x30);
    __ASSERT_EQL(Decoder->Event.Command, 0x05);
    __ASSERT_EQL(Decoder->Event.FrameCount, 1);
    __ASSERT_EQL(Decoder->Event.FrameIndex, 1);

    const uint8_t CraftedBytes1 [] = { 0xAA, 0x00, 0x00, 0x00, 0x00, 0x70, 0x00 };

    __WRAP(ResolVBus_LiveDecoder_Decode(Decoder, CraftedBytes1 + 0, 6));

    __ASSERT_LOG_EQL("");
    __ASSERT_EQL(Decoder->BufferIndex, 6);

    __WRAP(ResolVBus_LiveDecoder_Decode(Decoder, CraftedBytes1 + 6, 1));

    __ASSERT_LOG_EQL("");
    __ASSERT_EQL(Decoder->BufferIndex, 0);

    const uint8_t CraftedBytes2 [] = { 0xAA, 0x00, 0x80 };

    __WRAP(ResolVBus_LiveDecoder_Decode(Decoder, CraftedBytes2, sizeof (CraftedBytes2)));

    __ASSERT_LOG_EQL("");
    __ASSERT_EQL(Decoder->BufferIndex, 0);

    const uint8_t CraftedBytes3 [] = { 0xAA, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00 };

    __WRAP(ResolVBus_LiveDecoder_Decode(Decoder, CraftedBytes3, sizeof (CraftedBytes3)));

    __ASSERT_LOG_EQL("");
    __ASSERT_EQL(Decoder->BufferIndex, 0);

    const uint8_t CraftedBytes4 [] = { 0xAA, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x6F };

    __WRAP(ResolVBus_LiveDecoder_Decode(Decoder, CraftedBytes4, sizeof (CraftedBytes4)));

    __ASSERT_LOG_EQL("Event = PACKETHEADER\nEvent = PACKETEND\n");
    __ASSERT_EQL(Decoder->BufferIndex, 0);

    const uint8_t CraftedBytes5 [] = { 0xAA, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x01, 0x6E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

    __WRAP(ResolVBus_LiveDecoder_Decode(Decoder, CraftedBytes5, sizeof (CraftedBytes5)));

    __ASSERT_LOG_EQL("Event = PACKETHEADER\n");
    __ASSERT_EQL(Decoder->BufferIndex, 0);

    const uint8_t CraftedBytes6 [] = { 0xAA, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

    __WRAP(ResolVBus_LiveDecoder_Decode(Decoder, CraftedBytes6, sizeof (CraftedBytes6)));

    __ASSERT_LOG_EQL("");
    __ASSERT_EQL(Decoder->BufferIndex, 0);

    const uint8_t CraftedBytes7 [] = { 0xAA, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00 };

    __WRAP(ResolVBus_LiveDecoder_Decode(Decoder, CraftedBytes7, sizeof (CraftedBytes7)));

    __ASSERT_LOG_EQL("");
    __ASSERT_EQL(Decoder->BufferIndex, 0);

    const uint8_t CraftedBytes8 [] = { 0xAA, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x4F };

    __WRAP(ResolVBus_LiveDecoder_Decode(Decoder, CraftedBytes8, sizeof (CraftedBytes8)));

    __ASSERT_LOG_EQL("Event = TELEGRAMHEADER\nEvent = TELEGRAMEND\n");
    __ASSERT_EQL(Decoder->BufferIndex, 0);

    const uint8_t CraftedBytes9 [] = {
        0xAA, 0x00, 0x00, 0x00, 0x00, 0x30, 0x20, 0x2F,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    };

    __WRAP(ResolVBus_LiveDecoder_Decode(Decoder, CraftedBytes9, sizeof (CraftedBytes9)));

    __ASSERT_LOG_EQL("Event = TELEGRAMHEADER\n");
    __ASSERT_EQL(Decoder->BufferIndex, 0);

    const uint8_t CraftedBytes10 [] = {
        0xAA, 0x00, 0x00, 0x00, 0x00, 0x30, 0x40, 0x0F,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7F,
    };

    __WRAP(ResolVBus_LiveDecoder_Decode(Decoder, CraftedBytes10, sizeof (CraftedBytes10)));

    __ASSERT_LOG_EQL("Event = TELEGRAMHEADER\nEvent = TELEGRAMFRAME\n");
    __ASSERT_EQL(Decoder->BufferIndex, 8);

    return Result;
}



//---------------------------------------------------------------------------
// PUBLIC METHODS
//---------------------------------------------------------------------------

RESOLVBUS_RESULT RunTestSuite_LiveDecoder(void)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    __WRAP(__TestDecode());

    return Result;
}

