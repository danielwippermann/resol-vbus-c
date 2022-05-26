//
//  Master.c
//  resol-vbus
//
//
//  Copyright (C) 2022, Daniel Wippermann. All rights reserved.
//

#include "ResolVBus.h"



//---------------------------------------------------------------------------
// PRIVATE DEFINES
//---------------------------------------------------------------------------

#define __SELFADDRESS 0x1234



//---------------------------------------------------------------------------
// PRIVATE TYPEDEFS
//---------------------------------------------------------------------------

typedef struct __REQUESTPAYLOAD1 __REQUESTPAYLOAD1;
typedef struct __REQUESTPAYLOAD2 __REQUESTPAYLOAD2;
typedef union __REPLYPAYLOAD __REPLYPAYLOAD;
typedef struct __MASTER __MASTER;



//---------------------------------------------------------------------------
// PRIVATE DATA TYPES
//---------------------------------------------------------------------------

struct __REQUESTPAYLOAD1 {
    int16_t Temperature1;
    int16_t Temperature2;
    int16_t Temperature3;
};


struct __REQUESTPAYLOAD2 {
    uint8_t Length1;
    uint8_t Kind1;
    uint8_t Padding1a, Padding1b;
    int16_t Temperature1;
    int16_t Temperature2;
    int8_t PumpSpeed;
};


union __REPLYPAYLOAD {
    struct {
        int16_t Temperature1;
        int16_t Temperature2;
        int16_t Temperature3;
    } Reply3;
};


struct __MASTER {
    __REPLYPAYLOAD ReplyPayload;
    RESOLVBUS_LIVEDECODER Decoder;

    uint8_t EncoderBuffer [128];
    RESOLVBUS_LIVEENCODER Encoder;

    int Phase;
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

static RESOLVBUS_RESULT __HandleNextPhase(__MASTER *Master)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    RESOLVBUS_LIVEENCODER *Encoder = &Master->Encoder;

    if (Master->Phase == 1) {
        __REQUESTPAYLOAD1 Payload = { 0 };

        // TODO: fill payload

        if (Result == RESOLVBUS_OK) {
            Result = ResolVBus_LiveEncoder_QueuePacketHeader(Encoder, 0x0010, __SELFADDRESS, 0x10, 0x0100, RESOLVBUS_PACKETFRAMECOUNTOF(Payload));
        }

        if (Result == RESOLVBUS_OK) {
            Result = ResolVBus_LiveEncoder_QueuePacketFrames(Encoder, &Payload, sizeof (Payload));
        }
    } else if (Master->Phase == 2) {
        __REQUESTPAYLOAD2 Payload = { 0 };

        // TODO: fill payload

        if (Result == RESOLVBUS_OK) {
            Result = ResolVBus_LiveEncoder_QueuePacketHeader(Encoder, 0x0015, __SELFADDRESS, 0x10, 0x0100, RESOLVBUS_PACKETFRAMECOUNTOF(Payload));
        }

        if (Result == RESOLVBUS_OK) {
            Result = ResolVBus_LiveEncoder_QueuePacketFrames(Encoder, &Payload, sizeof (Payload));
        }
    } else if (Master->Phase == 3) {
        if (Result == RESOLVBUS_OK) {
            Result = ResolVBus_LiveEncoder_QueuePacketHeader(Encoder, 0x2345, __SELFADDRESS, 0x10, 0x0300, 0);
        }

        if (Result == RESOLVBUS_OK) {
            Result = ResolVBus_LiveEncoder_SuspendWithTimeout(Encoder, 200000);
        }
    } else if (Master->Phase == 4) {
        if (Result == RESOLVBUS_OK) {
            Result = ResolVBus_LiveEncoder_QueueDatagram(Encoder, 0x0000, __SELFADDRESS, 0x20, 0x0500, 0, 0);
        }

        if (Result == RESOLVBUS_OK) {
            Result = ResolVBus_LiveEncoder_SuspendWithTimeout(Encoder, 400000);
        }
    } else {
        // wait for next external phase change again
        Master->Phase = 0;

        if (Result == RESOLVBUS_OK) {
            Result = ResolVBus_LiveEncoder_Suspend(Encoder);
        }
    }

    if (Result == RESOLVBUS_OK) {
        Result = ResolVBus_LiveEncoder_HandleTimer(Encoder, 0);
    }

    return Result;
}


static RESOLVBUS_RESULT __DecoderHandler(RESOLVBUS_LIVEDECODER *Decoder, const RESOLVBUS_LIVEDECODEREVENT *Event)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    // __MASTER *Master = RESOLVBUS_CONTAINEROF(Decoder, __MASTER, Decoder);

    if (Event->EventType == RESOLVBUS_LIVEDECODEREVENTTYPE_PACKETEND) {
        if (Event->DestinationAddress != __SELFADDRESS) {
            // ignore
        } else if (Event->SourceAddress == 0x2345) {
            if (Event->Command != 0x0100) {
                // ignore
            } else {
                // TODO: transfer data from reply
            }
        } else {
            // ignore
        }
    }

    return Result;
}


static RESOLVBUS_RESULT __EncoderHandler(RESOLVBUS_LIVEENCODER *Encoder, const RESOLVBUS_LIVEENCODEREVENT *Event)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    __MASTER *Master = RESOLVBUS_CONTAINEROF(Encoder, __MASTER, Encoder);

    if (Event->EventType == RESOLVBUS_LIVEENCODEREVENTTYPE_TRANSMIT) {
        if (Result == RESOLVBUS_OK) {
            // TODO: transmit `Data` of length `Length` over serial port
        }
    } else if (Event->EventType == RESOLVBUS_LIVEENCODEREVENTTYPE_IDLE) {
        if (Result == RESOLVBUS_OK) {
            if (Master->Phase > 0) {
                Master->Phase += 1;

                Result = __HandleNextPhase(Master);
            }
        }
    }

    return Result;
}


static RESOLVBUS_RESULT __Initialize(__MASTER *Master)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    if (Result == RESOLVBUS_OK) {
        Result = ResolVBus_LiveDecoder_Initialize(&Master->Decoder, &Master->ReplyPayload, sizeof (Master->ReplyPayload), __DecoderHandler);
    }

    if (Result == RESOLVBUS_OK) {
        Result = ResolVBus_LiveEncoder_Initialize(&Master->Encoder, Master->EncoderBuffer, sizeof (Master->EncoderBuffer), __EncoderHandler);
    }

    if (Result == RESOLVBUS_OK) {
        Result = ResolVBus_LiveEncoder_Suspend(&Master->Encoder);
    }

    return Result;
}


static RESOLVBUS_RESULT __HandleLoopCycle(__MASTER *Master, bool ShouldStartIfIdle, uint32_t TimePassedUs, const uint8_t *ReadBytes, size_t ReadLength)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    if ((Result == RESOLVBUS_OK) && ShouldStartIfIdle) {
        if (Master->Phase != 0) {
            // ignore, because previos transmission is still in progress
        } else {
            Master->Phase = 1;

            if (Result == RESOLVBUS_OK) {
                Result = ResolVBus_LiveEncoder_Resume(&Master->Encoder);
            }

            if (Result == RESOLVBUS_OK) {
                Result = __HandleNextPhase(Master);
            }
        }
    }

    if (Result == RESOLVBUS_OK) {
        Result = ResolVBus_LiveEncoder_HandleTimer(&Master->Encoder, TimePassedUs);
    }

    if ((Result == RESOLVBUS_OK) && ReadBytes) {
        Result = ResolVBus_LiveDecoder_Decode(&Master->Decoder, ReadBytes, ReadLength);
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

    __MASTER Master = {};

    if (Result == RESOLVBUS_OK) {
        Result = __Initialize(&Master);
    }

    if (Result == RESOLVBUS_OK) {
        // TODO: open serial port / socket / ...
    }

    while (Result == RESOLVBUS_OK) {
        uint8_t ReadBytes [128] = { 0 };
        size_t ReadLength = 0;
        if (Result == RESOLVBUS_OK) {
            // TODO: read from serial port / socket / ... with short timeout / non-blocking
        }

        uint32_t TimePassedUs = 0;
        if (Result == RESOLVBUS_OK) {
            // TODO: calculate time passed since last loop cycle and store it in `TimePassedUs`
        }

        bool ShouldStartIfIdle = false;
        if (Result == RESOLVBUS_OK) {
            // TODO: determine whether now would be a good time to start transmitting if the encoder is idle (e.g. once per second)
        }

        if (Result == RESOLVBUS_OK) {
            Result = __HandleLoopCycle(&Master, ShouldStartIfIdle, TimePassedUs, ReadBytes, ReadLength);
        }
    }

    // TODO: close serial port / socket / ...

    return 0;
}
#endif // __IGNORE_MAIN_FOR_TESTING
