//
//  Parameterization.c
//  resol-vbus-c
//
//
//  Copyright (C) 2023, Daniel Wippermann. All rights reserved.
//

#include <stdlib.h>
#include <time.h>


#include "ResolVBus.h"


#include "../src/Debug.h"



//---------------------------------------------------------------------------
// PRIVATE DEFINES
//---------------------------------------------------------------------------

// for RESOL DeltaSol BX Plus
#define __PEERADDRESS 0x7112

// for RESOL DeltaSol MX
// #define __PEERADDRESS 0x7E11

// Changeset for version 2.08
#define __PEERCHANGESET 0x2734DABC

#define __VALUEID_SYSTEMDATUM 0x0039
#define __VALUEID_SYSTEMZEIT 0x003A



//---------------------------------------------------------------------------
// PRIVATE TYPEDEFS
//---------------------------------------------------------------------------

typedef struct __PARAMETERIZATION __PARAMETERIZATION;



//---------------------------------------------------------------------------
// PRIVATE DATA TYPES
//---------------------------------------------------------------------------

struct __PARAMETERIZATION {
    uint8_t DecoderBuffer [128];
    uint8_t EncoderBuffer [128];
    RESOLVBUS_LIVETRANSCEIVER Transceiver;

    uint32_t ExpectedTimestamp;
    bool ShouldCorrectTimestamp;
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

static RESOLVBUS_RESULT __ReleaseBusHandler(RESOLVBUS_LIVETRANSCEIVER *Transceiver, const RESOLVBUS_LIVETRANSCEIVEREVENT *Event)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    // nop

    return Result;
}


static RESOLVBUS_RESULT __SetSystemzeitHandler(RESOLVBUS_LIVETRANSCEIVER *Transceiver, const RESOLVBUS_LIVETRANSCEIVEREVENT *Event)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    __PARAMETERIZATION *Parameterization = RESOLVBUS_CONTAINEROF(Transceiver, __PARAMETERIZATION, Transceiver);

    if (Event->EventType == RESOLVBUS_LIVETRANSCEIVEREVENTTYPE_DECODER) {
        if (Event->DecoderEvent->EventType == RESOLVBUS_LIVEDECODEREVENTTYPE_DATAGRAM) {
            Parameterization->ShouldCorrectTimestamp = false;

            __WRAP(ResolVBus_LiveTransceiver_ReleaseBus(&Parameterization->Transceiver, __PEERADDRESS, NULL, __ReleaseBusHandler));
        }
    }

    return Result;
}


static RESOLVBUS_RESULT __SetSystemdatumHandler(RESOLVBUS_LIVETRANSCEIVER *Transceiver, const RESOLVBUS_LIVETRANSCEIVEREVENT *Event)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    __PARAMETERIZATION *Parameterization = RESOLVBUS_CONTAINEROF(Transceiver, __PARAMETERIZATION, Transceiver);

    if (Event->EventType == RESOLVBUS_LIVETRANSCEIVEREVENTTYPE_DECODER) {
        if (Event->DecoderEvent->EventType == RESOLVBUS_LIVEDECODEREVENTTYPE_DATAGRAM) {
            uint32_t Systemzeit = (Parameterization->ExpectedTimestamp / 60) % 1440;

            __WRAP(ResolVBus_LiveTransceiver_SetValueById(&Parameterization->Transceiver, __PEERADDRESS, __VALUEID_SYSTEMZEIT, 0, Systemzeit, NULL, __SetSystemzeitHandler));
        }
    }

    return Result;
}


static RESOLVBUS_RESULT __GetChangesetIdHandler(RESOLVBUS_LIVETRANSCEIVER *Transceiver, const RESOLVBUS_LIVETRANSCEIVEREVENT *Event)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    __PARAMETERIZATION *Parameterization = RESOLVBUS_CONTAINEROF(Transceiver, __PARAMETERIZATION, Transceiver);

    if (Event->EventType == RESOLVBUS_LIVETRANSCEIVEREVENTTYPE_DECODER) {
        if (Event->DecoderEvent->EventType == RESOLVBUS_LIVEDECODEREVENTTYPE_DATAGRAM) {
            if (Event->DecoderEvent->Param32 == __PEERCHANGESET) {
                uint32_t Systemdatum = Parameterization->ExpectedTimestamp;

                __WRAP(ResolVBus_LiveTransceiver_SetValueById(&Parameterization->Transceiver, __PEERADDRESS, __VALUEID_SYSTEMDATUM, 0, Systemdatum, NULL, __SetSystemdatumHandler));
            }
        }
    }

    return Result;
}


static RESOLVBUS_RESULT __TransceiverHandler(RESOLVBUS_LIVETRANSCEIVER *Transceiver, const RESOLVBUS_LIVETRANSCEIVEREVENT *Event)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    __PARAMETERIZATION *Parameterization = RESOLVBUS_CONTAINEROF(Transceiver, __PARAMETERIZATION, Transceiver);

    if (Event->EventType == RESOLVBUS_LIVETRANSCEIVEREVENTTYPE_DECODER) {
        const RESOLVBUS_LIVEDECODEREVENT *DecoderEvent = Event->DecoderEvent;

        if (DecoderEvent->EventType == RESOLVBUS_LIVEDECODEREVENTTYPE_PACKETEND) {
            if ((DecoderEvent->DestinationAddress == 0x0010) && (DecoderEvent->SourceAddress == 0x7112) && (DecoderEvent->Command == 0x0100)) {
                uint32_t ActualTimestamp = ResolVBus_ReadUInt32LE(Parameterization->DecoderBuffer + 48);

                int32_t TimestampDiff = ActualTimestamp - Parameterization->ExpectedTimestamp;
                if ((TimestampDiff < -100) || (TimestampDiff > 100)) {
                    Parameterization->ShouldCorrectTimestamp = true;
                }
            } else if ((DecoderEvent->DestinationAddress == 0x0010) && (DecoderEvent->SourceAddress == 0x7E11) && (DecoderEvent->Command == 0x0100)) {
                uint32_t ActualTimestamp = ResolVBus_ReadUInt32LE(Parameterization->DecoderBuffer + 92);

                int32_t TimestampDiff = ActualTimestamp - Parameterization->ExpectedTimestamp;
                if ((TimestampDiff < -100) || (TimestampDiff > 100)) {
                    Parameterization->ShouldCorrectTimestamp = true;
                }
            }
        } else if (DecoderEvent->EventType == RESOLVBUS_LIVEDECODEREVENTTYPE_DATAGRAM) {
            if ((DecoderEvent->DestinationAddress == 0x0000) && (DecoderEvent->SourceAddress == __PEERADDRESS) && (DecoderEvent->Command == 0x0500)) {
                if (Parameterization->ShouldCorrectTimestamp) {
                    if ((Parameterization->ExpectedTimestamp % 60) < 10) {
                        __WRAP(ResolVBus_LiveTransceiver_GetValueById(&Parameterization->Transceiver, __PEERADDRESS, 0, 0, NULL, __GetChangesetIdHandler));
                    } else {
                        // NOTE(daniel): seconds will always reset to 0 on write and we are too late in the minute, so wait a bit...
                    }
                }
            }
        }
    } else if (Event->EventType == RESOLVBUS_LIVETRANSCEIVEREVENTTYPE_ENCODER) {
        const RESOLVBUS_LIVEENCODEREVENT *EncoderEvent = Event->EncoderEvent;

        if (EncoderEvent->EventType == RESOLVBUS_LIVEENCODEREVENTTYPE_TRANSMIT) {
            if (Result == RESOLVBUS_OK) {
                // TODO: transmit `Data` of length `Length` over serial port
            }
        }
    }

    return Result;
}


static RESOLVBUS_RESULT __Initialize(__PARAMETERIZATION *Parameterization)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    __WRAP(ResolVBus_LiveTransceiver_Initialize(&Parameterization->Transceiver, Parameterization->EncoderBuffer, sizeof (Parameterization->EncoderBuffer), Parameterization->DecoderBuffer, sizeof (Parameterization->DecoderBuffer), __TransceiverHandler));

    return Result;
}


static RESOLVBUS_RESULT __HandleLoopCycle(__PARAMETERIZATION *Parameterization, uint32_t ExpectedTimestamp, uint32_t TimePassedUs, const uint8_t *ReadBytes, size_t ReadLength)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    if (Result == RESOLVBUS_OK) {
        Parameterization->ExpectedTimestamp = ExpectedTimestamp;
    }

    if (ReadBytes) {
        __WRAP(ResolVBus_LiveTransceiver_Decode(&Parameterization->Transceiver, ReadBytes, ReadLength));
    }

    __WRAP(ResolVBus_LiveTransceiver_HandleTimer(&Parameterization->Transceiver, TimePassedUs));

    return Result;
}



//---------------------------------------------------------------------------
// PUBLIC METHODS
//---------------------------------------------------------------------------

#ifndef __IGNORE_MAIN_FOR_TESTING
int main(int argc, char **argv)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    __PARAMETERIZATION Parameterization = {
        .Transceiver = RESOLVBUS_LIVETRANSCEIVER_INITIALIZER,
    };

    __WRAP(__Initialize(&Parameterization));

    if (Result == RESOLVBUS_OK) {
        // TODO: open serial port / socket / ...
    }

    while (Result == RESOLVBUS_OK) {
        uint32_t TimeoutUs = 0;
        __WRAP(ResolVBus_LiveTransceiver_GetTimeout(&Parameterization.Transceiver, &TimeoutUs));

        uint8_t ReadBytes [128] = {};
        size_t ReadLength = 0;
        if (Result == RESOLVBUS_OK) {
            // TODO: read from serial port / socket / ... with short timeout / non-blocking
            // `TimeoutUs` contains an indication, how long the timeout should be
        }

        time_t HostTimestamp = 0;
        if (Result == RESOLVBUS_OK) {
            // TODO: get host timestamp
        }

        uint32_t ControllerTimestamp = 0;
        if (Result == RESOLVBUS_OK) {
            // TODO: convert HostTimestamp to ControllerTimestamp
            // ControllerTimestamp is defined as seconds since 2001-01-01T00:00:00 local time!
        }

        uint32_t TimePassedUs = 0;
        if (Result == RESOLVBUS_OK) {
            // TODO: calculate time passed since last loop cycle and store it in `TimePassedUs`
        }

        if (Result == RESOLVBUS_OK) {
            Result = __HandleLoopCycle(&Parameterization, ControllerTimestamp, TimePassedUs, ReadBytes, ReadLength);
        }
    }

    // TODO: close serial port / socket / ...

    return 0;
}
#endif // __IGNORE_MAIN_FOR_TESTING
