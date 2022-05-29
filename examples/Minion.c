//
//  Minion.c
//  resol-vbus
//
//
//  Copyright (C) 2022, Daniel Wippermann. All rights reserved.
//

#include "ResolVBus.h"



//---------------------------------------------------------------------------
// PRIVATE DEFINES
//---------------------------------------------------------------------------

#define __SELFADDRESS 0x2345



//---------------------------------------------------------------------------
// PRIVATE TYPEDEFS
//---------------------------------------------------------------------------

typedef struct __REQUESTPAYLOAD __REQUESTPAYLOAD;
typedef struct __REPLYPAYLOAD __REPLYPAYLOAD;
typedef struct __MINION __MINION;



//---------------------------------------------------------------------------
// PRIVATE DATA TYPES
//---------------------------------------------------------------------------

struct __REQUESTPAYLOAD {
    int16_t Value;
};

struct __REPLYPAYLOAD {
    int16_t Temperature1;
    int16_t Temperature2;
    int16_t Temperature3;
};


struct __MINION {
    __REQUESTPAYLOAD RequestPayload;
    RESOLVBUS_LIVEDECODER Decoder;

    uint8_t EncoderBuffer [128];
    RESOLVBUS_LIVEENCODER Encoder;
};



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

static RESOLVBUS_RESULT __DecoderHandler(RESOLVBUS_LIVEDECODER *Decoder, const RESOLVBUS_LIVEDECODEREVENT *Event)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    __MINION *Minion = RESOLVBUS_CONTAINEROF(Decoder, __MINION, Decoder);

    if (Event->EventType == RESOLVBUS_LIVEDECODEREVENTTYPE_PACKETEND) {
        if (Event->DestinationAddress != __SELFADDRESS) {
            // ignore
        } else if (Event->Command != 0x0300) {
            // ignore
        } else {
            __REPLYPAYLOAD ReplyPayload = { 0 };

            // TODO: fill reply payload

            if (Result == RESOLVBUS_OK) {
                Result = ResolVBus_LiveEncoder_Resume(&Minion->Encoder);
            }

            if (Result == RESOLVBUS_OK) {
                Result = ResolVBus_LiveEncoder_QueuePacketHeader(&Minion->Encoder, Event->SourceAddress, __SELFADDRESS, 0x10, 0x0100, RESOLVBUS_PACKETFRAMECOUNTOF(ReplyPayload));
            }

            if (Result == RESOLVBUS_OK) {
                Result = ResolVBus_LiveEncoder_QueuePacketFrames(&Minion->Encoder, &ReplyPayload, sizeof (ReplyPayload));
            }

            if (Result == RESOLVBUS_OK) {
                Result = ResolVBus_LiveEncoder_Suspend(&Minion->Encoder);
            }
        }
    }

    return Result;
}


static RESOLVBUS_RESULT __EncoderHandler(RESOLVBUS_LIVEENCODER *Encoder, const RESOLVBUS_LIVEENCODEREVENT *Event)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    if (Event->EventType == RESOLVBUS_LIVEENCODEREVENTTYPE_TRANSMIT) {
        if (Result == RESOLVBUS_OK) {
            // TODO: transmit `Data` of length `Length` over serial port
        }
    }

    return Result;
}


static RESOLVBUS_RESULT __Initialize(__MINION *Minion)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    if (Result == RESOLVBUS_OK) {
        Result = ResolVBus_LiveDecoder_Initialize(&Minion->Decoder, &Minion->RequestPayload, sizeof (Minion->RequestPayload), __DecoderHandler);
    }

    if (Result == RESOLVBUS_OK) {
        Result = ResolVBus_LiveEncoder_Initialize(&Minion->Encoder, Minion->EncoderBuffer, sizeof (Minion->EncoderBuffer), __EncoderHandler);
    }

    if (Result == RESOLVBUS_OK) {
        Result = ResolVBus_LiveEncoder_Suspend(&Minion->Encoder);
    }

    return Result;
}


static RESOLVBUS_RESULT __HandleLoopCycle(__MINION *Minion, uint32_t TimePassedUs, const uint8_t *ReadBytes, size_t ReadLength)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    if ((Result == RESOLVBUS_OK) && ReadBytes) {
        Result = ResolVBus_LiveDecoder_Decode(&Minion->Decoder, ReadBytes, ReadLength);
    }

    if (Result == RESOLVBUS_OK) {
        Result = ResolVBus_LiveEncoder_HandleTimer(&Minion->Encoder, TimePassedUs);
    }

    return Result;
}



//---------------------------------------------------------------------------
// PUBLIC METHODS
//---------------------------------------------------------------------------

#ifndef __IGNORE_MAIN_FOR_TESTING
int main(int argc, char **argv)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    __MINION Minion = {
        .Decoder = RESOLVBUS_LIVEDECODER_INITIALIZER,
        .Encoder = RESOLVBUS_LIVEENCODER_INITIALIZER,
    };

    if (Result == RESOLVBUS_OK) {
        Result = __Initialize(&Minion);
    }

    if (Result == RESOLVBUS_OK) {
        // TODO: open serial port / socket / ...
    }

    while (Result == RESOLVBUS_OK) {
        uint8_t ReadBytes [128] = {};
        size_t ReadLength = 0;
        if (Result == RESOLVBUS_OK) {
            // TODO: read from serial port / socket / ... with short timeout / non-blocking
        }

        uint32_t TimePassedUs = 0;
        if (Result == RESOLVBUS_OK) {
            // TODO: calculate time passed since last loop cycle and store it in `TimePassedUs`
        }

        if (Result == RESOLVBUS_OK) {
            Result = __HandleLoopCycle(&Minion, TimePassedUs, ReadBytes, ReadLength);
        }
    }

    // TODO: close serial port / socket / ...

    return 0;
}
#endif // __IGNORE_MAIN_FOR_TESTING
