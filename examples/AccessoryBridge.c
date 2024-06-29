//
//  AccessoryBridge.c
//  resol-vbus-c
//
//
//  Copyright (C) 2024, Daniel Wippermann. All rights reserved.
//

#include <stdarg.h>
#include <string.h>


#include "ResolVBus.h"


#include "../src/Debug.h"



//---------------------------------------------------------------------------
// PRIVATE DEFINES
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// PRIVATE TYPEDEFS
//---------------------------------------------------------------------------

typedef enum __PHASE {
    __PHASE_UNKNOWN,
    __PHASE_WAITFORFREEBUS,
    __PHASE_WAITFORANYDATA,
    __PHASE_GETCHANGESETID,
    __PHASE_SENDPACKET1,
    __PHASE_SENDPACKET2,
    __PHASE_SENDPACKET3,
    __PHASE_RELEASEBUS,
} __PHASE;


typedef struct __VBUSVPACKETPAYLOAD __VBUSVPACKETPAYLOAD;
typedef struct __ACCESSORYBRIDGE __ACCESSORYBRIDGE;



//---------------------------------------------------------------------------
// PRIVATE DATA TYPES
//---------------------------------------------------------------------------

struct __VBUSVPACKETPAYLOAD {
    int32_t TotalActivePower;
    int32_t TotalActivePowerInv;
};


struct __ACCESSORYBRIDGE {
    // physical VBus
    RESOLVBUS_LIVETRANSCEIVER VBusPLiveTransceiver;
    uint8_t VBusPLiveDecoderBuffer [128];
    uint8_t VBusPLiveEncoderBuffer [128];
    __PHASE VBusPPhase;
    uint16_t VBusPPeerAddress;
    uint8_t VBusPPvsSeqNr;
    bool VBusPPvsInitialized;

    // virrtual VBus
    RESOLVBUS_LIVEENCODER VBusVLiveEncoder;
    uint8_t VBusVLiveEncoderBytes [1024];
    __VBUSVPACKETPAYLOAD VBusVPacketPayload;
};



//---------------------------------------------------------------------------
// PRIVATE METHOD DECLARATIONS
//---------------------------------------------------------------------------

static RESOLVBUS_RESULT __PerformVBusPPhaseAction(__ACCESSORYBRIDGE *Bridge, __PHASE Phase);



//---------------------------------------------------------------------------
// CONSTANTS
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// VARIABLES
//---------------------------------------------------------------------------

static const RESOLVBUS_LIVETRANSCEIVEROPTIONS __GetChangesetIdOptions = {
    .Tries = 1,
    .InitialTimeout = 100000,
    .TimeoutIncrement = 0,
};


static const RESOLVBUS_LIVETRANSCEIVEROPTIONS __VBusPSendPacketXOptions = {
    .Tries = 1,
    .InitialTimeout = 400000,
    .TimeoutIncrement = 0,
};



//---------------------------------------------------------------------------
// NON-PUBLIC METHODS
//---------------------------------------------------------------------------

static RESOLVBUS_RESULT __VBusPLiveTransceiverActionHandler(RESOLVBUS_LIVETRANSCEIVER *Transceiver, const RESOLVBUS_LIVETRANSCEIVEREVENT *Event)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    __ACCESSORYBRIDGE *Bridge = RESOLVBUS_CONTAINEROF(Transceiver, __ACCESSORYBRIDGE, VBusPLiveTransceiver);

    __PHASE Phase = Bridge->VBusPPhase;

    bool Handled = false;
    if (Phase == __PHASE_WAITFORFREEBUS) {
        // wait for bus offer
        if (Event->EventType == RESOLVBUS_LIVETRANSCEIVEREVENTTYPE_TIMEOUT) {
            Handled = true;
            __WRAP(__PerformVBusPPhaseAction(Bridge, __PHASE_WAITFORFREEBUS));
        } else if ((Event->EventType == RESOLVBUS_LIVETRANSCEIVEREVENTTYPE_DECODER) && (Event->DecoderEvent->EventType == RESOLVBUS_LIVEDECODEREVENTTYPE_DATAGRAM)) {
            Handled = true;
            Bridge->VBusPPeerAddress = Event->DecoderEvent->SourceAddress;
            __WRAP(__PerformVBusPPhaseAction(Bridge, __PHASE_WAITFORANYDATA));
        }
    } else if (Phase == __PHASE_WAITFORANYDATA) {
        // received 0x0500 datagram, waiting for 100 ms whether other bus participant start communicating
        if (Event->EventType == RESOLVBUS_LIVETRANSCEIVEREVENTTYPE_TIMEOUT) {
            Handled = true;
            __WRAP(__PerformVBusPPhaseAction(Bridge, __PHASE_GETCHANGESETID));
        } else {
            Handled = true;
            __WRAP(__PerformVBusPPhaseAction(Bridge, __PHASE_WAITFORFREEBUS));
        }
    } else if (Phase == __PHASE_GETCHANGESETID) {
        // get changeset ID to start longer timeout in master
        if (Event->EventType == RESOLVBUS_LIVETRANSCEIVEREVENTTYPE_TIMEOUT) {
            Handled = true;
            __WRAP(__PerformVBusPPhaseAction(Bridge, __PHASE_WAITFORFREEBUS));
        } else if ((Event->EventType == RESOLVBUS_LIVETRANSCEIVEREVENTTYPE_DECODER) && (Event->DecoderEvent->EventType == RESOLVBUS_LIVEDECODEREVENTTYPE_DATAGRAM)) {
            Handled = true;
            if (Bridge->VBusPPvsInitialized) {
                __WRAP(__PerformVBusPPhaseAction(Bridge, __PHASE_SENDPACKET3));
            } else {
                __WRAP(__PerformVBusPPhaseAction(Bridge, __PHASE_SENDPACKET1));
            }
        }
    } else if (Phase == __PHASE_SENDPACKET1) {
        // send "request info" packet to PVS
        if (Event->EventType == RESOLVBUS_LIVETRANSCEIVEREVENTTYPE_TIMEOUT) {
            Handled = true;
            __WRAP(__PerformVBusPPhaseAction(Bridge, __PHASE_RELEASEBUS));
        } else if ((Event->EventType == RESOLVBUS_LIVETRANSCEIVEREVENTTYPE_DECODER) && (Event->DecoderEvent->EventType == RESOLVBUS_LIVEDECODEREVENTTYPE_PACKETEND)) {
            Handled = true;
            __WRAP(__PerformVBusPPhaseAction(Bridge, __PHASE_SENDPACKET2));
        }
    } else if (Phase == __PHASE_SENDPACKET2) {
        // send "request config" packet to PVS
        if (Event->EventType == RESOLVBUS_LIVETRANSCEIVEREVENTTYPE_TIMEOUT) {
            Handled = true;
            __WRAP(__PerformVBusPPhaseAction(Bridge, __PHASE_RELEASEBUS));
        } else if ((Event->EventType == RESOLVBUS_LIVETRANSCEIVEREVENTTYPE_DECODER) && (Event->DecoderEvent->EventType == RESOLVBUS_LIVEDECODEREVENTTYPE_PACKETEND)) {
            Handled = true;
            __WRAP(__PerformVBusPPhaseAction(Bridge, __PHASE_SENDPACKET3));
        }
    } else if (Phase == __PHASE_SENDPACKET3) {
        // send "send control info" packet to PVS
        if (Event->EventType == RESOLVBUS_LIVETRANSCEIVEREVENTTYPE_TIMEOUT) {
            Handled = true;
            Bridge->VBusPPvsInitialized = false;
            __WRAP(__PerformVBusPPhaseAction(Bridge, __PHASE_RELEASEBUS));
        } else if ((Event->EventType == RESOLVBUS_LIVETRANSCEIVEREVENTTYPE_DECODER) && (Event->DecoderEvent->EventType == RESOLVBUS_LIVEDECODEREVENTTYPE_PACKETEND)) {
            Handled = true;
            Bridge->VBusPPvsInitialized = true;
            __WRAP(__PerformVBusPPhaseAction(Bridge, __PHASE_RELEASEBUS));
        }
    } else if (Phase == __PHASE_RELEASEBUS) {
        // release bus
        if (Event->EventType == RESOLVBUS_LIVETRANSCEIVEREVENTTYPE_TIMEOUT) {
            Handled = true;
            __WRAP(__PerformVBusPPhaseAction(Bridge, __PHASE_WAITFORFREEBUS));
        } else {
            Handled = true;
            __WRAP(__PerformVBusPPhaseAction(Bridge, __PHASE_WAITFORFREEBUS));
        }
    }

    if (!Handled) {
        __WRAP(__PerformVBusPPhaseAction(Bridge, __PHASE_WAITFORFREEBUS));
    }

    return Result;
}


static RESOLVBUS_RESULT __VBusPSendPacket1Handler(RESOLVBUS_LIVETRANSCEIVER *Transceiver, const RESOLVBUS_LIVETRANSCEIVEREVENT *Event)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    if (Event->EventType == RESOLVBUS_LIVETRANSCEIVEREVENTTYPE_ACTION) {
        RESOLVBUS_LIVEENCODER *Encoder = NULL;
        __WRAP(ResolVBus_LiveTransceiver_GetEncoder(Transceiver, &Encoder));

        __WRAP(ResolVBus_LiveEncoder_Resume(Encoder));

        __WRAP(ResolVBus_LiveEncoder_QueuePacketHeader(Encoder, 0x1116, Transceiver->SelfAddress, 0, 0x0310, 0));

        __WRAP(ResolVBus_LiveEncoder_Suspend(Encoder));
    } else if (Event->EventType == RESOLVBUS_LIVETRANSCEIVEREVENTTYPE_DECODER) {
        const RESOLVBUS_LIVEDECODEREVENT *DecoderEvent = Event->DecoderEvent;

        if (DecoderEvent->EventType != RESOLVBUS_LIVEDECODEREVENTTYPE_PACKETEND) {
            // nop
        } else if (DecoderEvent->DestinationAddress != Transceiver->SelfAddress) {
            // nop
        } else if (DecoderEvent->SourceAddress != 0x1116) {
            // nop
        } else if (DecoderEvent->Command != 0x0110) {
            // nop
        } else if (DecoderEvent->FrameCount < 1) {
            // nop
        } else {
            const uint8_t *FrameData = DecoderEvent->FrameDataBuffer;
            if (FrameData [0] != 2) {
                // nop
            } else {
                __WRAP(ResolVBus_LiveTransceiver_CommitAction(Transceiver, Event));
            }
        }
    }

    return Result;
}


static RESOLVBUS_RESULT __VBusPSendPacket2Handler(RESOLVBUS_LIVETRANSCEIVER *Transceiver, const RESOLVBUS_LIVETRANSCEIVEREVENT *Event)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    if (Event->EventType == RESOLVBUS_LIVETRANSCEIVEREVENTTYPE_ACTION) {
        __ACCESSORYBRIDGE *Bridge = RESOLVBUS_CONTAINEROF(Transceiver, __ACCESSORYBRIDGE, VBusPLiveTransceiver);

        RESOLVBUS_LIVEENCODER *Encoder = NULL;
        __WRAP(ResolVBus_LiveTransceiver_GetEncoder(Transceiver, &Encoder));

        __WRAP(ResolVBus_LiveEncoder_Resume(Encoder));

        if (Result == RESOLVBUS_OK) {
            Bridge->VBusPPvsSeqNr += 1;
        }

        uint8_t FrameData [12] = { 0 };
        FrameData [0] = 2;  // Header-Version
        FrameData [1] = Bridge->VBusPPvsSeqNr;  // Header-SeqNr
        FrameData [2] = 0;  // Header-Zustand
        FrameData [4] = 100;
        FrameData [6] = 30;
        FrameData [8] = 100;
        FrameData [10] = 30;

        __WRAP(ResolVBus_LiveEncoder_QueuePacketHeader(Encoder, 0x1116, Transceiver->SelfAddress, 0, 0x0220, 3));
        __WRAP(ResolVBus_LiveEncoder_QueuePacketFrames(Encoder, FrameData, sizeof (FrameData)));

        __WRAP(ResolVBus_LiveEncoder_Suspend(Encoder));
    } else if (Event->EventType == RESOLVBUS_LIVETRANSCEIVEREVENTTYPE_DECODER) {
        const RESOLVBUS_LIVEDECODEREVENT *DecoderEvent = Event->DecoderEvent;

        if (DecoderEvent->EventType != RESOLVBUS_LIVEDECODEREVENTTYPE_PACKETEND) {
            // nop
        } else if (DecoderEvent->DestinationAddress != Transceiver->SelfAddress) {
            // nop
        } else if (DecoderEvent->SourceAddress != 0x1116) {
            // nop
        } else if (DecoderEvent->Command != 0x0120) {
            // nop
        } else {
            __WRAP(ResolVBus_LiveTransceiver_CommitAction(Transceiver, Event));
        }
    }

    return Result;
}


static RESOLVBUS_RESULT __VBusPSendPacket3Handler(RESOLVBUS_LIVETRANSCEIVER *Transceiver, const RESOLVBUS_LIVETRANSCEIVEREVENT *Event)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    if (Event->EventType == RESOLVBUS_LIVETRANSCEIVEREVENTTYPE_ACTION) {
        __ACCESSORYBRIDGE *Bridge = RESOLVBUS_CONTAINEROF(Transceiver, __ACCESSORYBRIDGE, VBusPLiveTransceiver);

        RESOLVBUS_LIVEENCODER *Encoder = NULL;
        __WRAP(ResolVBus_LiveTransceiver_GetEncoder(Transceiver, &Encoder));

        __WRAP(ResolVBus_LiveEncoder_Resume(Encoder));

        uint8_t FrameData [28] = { 0 };
        FrameData [0] = 2;  // Header-Version
        FrameData [1] = Bridge->VBusPPvsSeqNr;  // Header-SeqNr
        FrameData [2] = 0;  // Header-Zustand
        FrameData [4] = 2;
        FrameData [5] = 2;
        FrameData [8] = 2;
        FrameData [9] = 2;
        FrameData [12] = 2;
        FrameData [13] = 2;
        FrameData [16] = 2;
        FrameData [17] = 2;

        __WRAP(ResolVBus_LiveEncoder_QueuePacketHeader(Encoder, 0x1116, Transceiver->SelfAddress, 0, 0x0230, 7));
        __WRAP(ResolVBus_LiveEncoder_QueuePacketFrames(Encoder, FrameData, sizeof (FrameData)));

        __WRAP(ResolVBus_LiveEncoder_Suspend(Encoder));
    } else if (Event->EventType == RESOLVBUS_LIVETRANSCEIVEREVENTTYPE_DECODER) {
        const RESOLVBUS_LIVEDECODEREVENT *DecoderEvent = Event->DecoderEvent;

        if (DecoderEvent->EventType != RESOLVBUS_LIVEDECODEREVENTTYPE_PACKETEND) {
            // nop
        } else if (DecoderEvent->DestinationAddress != Transceiver->SelfAddress) {
            // nop
        } else if (DecoderEvent->SourceAddress != 0x1116) {
            // nop
        } else if (DecoderEvent->Command != 0x0130) {
            // nop
        } else if (DecoderEvent->FrameCount < 2) {
            // nop
        } else {
            const uint8_t *FrameData = DecoderEvent->FrameDataBuffer;
            int64_t TotalActivePowerRaw = (int64_t) (int32_t) ResolVBus_ReadUInt32LE(FrameData + 4);
            int64_t TotalActivePowerMilliwatt = (TotalActivePowerRaw * 8) / 25;

            __ACCESSORYBRIDGE *Bridge = RESOLVBUS_CONTAINEROF(Transceiver, __ACCESSORYBRIDGE, VBusPLiveTransceiver);

            RESOLVBUS_LIVEENCODER *Encoder = &Bridge->VBusVLiveEncoder;

            __VBUSVPACKETPAYLOAD Payload = { 0 };
            Payload.TotalActivePower = TotalActivePowerMilliwatt;
            Payload.TotalActivePowerInv = -TotalActivePowerMilliwatt;

            __WRAP(ResolVBus_LiveEncoder_Resume(Encoder));

            __WRAP(ResolVBus_LiveEncoder_QueuePacketHeader(Encoder, 0x0010, 0x112E, 0, 0x0101, RESOLVBUS_PACKETFRAMECOUNTOF(Payload)));
            __WRAP(ResolVBus_LiveEncoder_QueuePacketFrames(Encoder, &Payload, sizeof (Payload)));

            __WRAP(ResolVBus_LiveEncoder_Suspend(Encoder));

            __WRAP(ResolVBus_LiveTransceiver_CommitAction(Transceiver, Event));
        }
    }

    return Result;
}


static RESOLVBUS_RESULT __PerformVBusPPhaseAction(__ACCESSORYBRIDGE *Bridge, __PHASE Phase)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    Bridge->VBusPPhase = Phase;

    RESOLVBUS_LIVETRANSCEIVER *Transceiver = &Bridge->VBusPLiveTransceiver;

    if (Phase == __PHASE_WAITFORFREEBUS) {
        // wait for bus offer
        Bridge->VBusPPeerAddress = 0;
        __WRAP(ResolVBus_LiveTransceiver_WaitForFreeBus(Transceiver, NULL, __VBusPLiveTransceiverActionHandler));
    } else if (Phase == __PHASE_WAITFORANYDATA) {
        // received 0x0500 datagram, waiting for 100 ms whether other bus participant start communicating
        __WRAP(ResolVBus_LiveTransceiver_WaitForAnyData(Transceiver, NULL, __VBusPLiveTransceiverActionHandler));
    } else if (Phase == __PHASE_GETCHANGESETID) {
        // get changeset ID to start longer timeout in master
        __WRAP(ResolVBus_LiveTransceiver_GetValueById(Transceiver, Bridge->VBusPPeerAddress, 0, 0, &__GetChangesetIdOptions, __VBusPLiveTransceiverActionHandler));
    } else if (Phase == __PHASE_SENDPACKET1) {
        // send "request info" packet to PVS
        __WRAP(ResolVBus_LiveTransceiver_SetAction(Transceiver, __VBusPSendPacket1Handler, &__VBusPSendPacketXOptions, __VBusPLiveTransceiverActionHandler));
    } else if (Phase == __PHASE_SENDPACKET2) {
        // send "request config" packet to PVS
        __WRAP(ResolVBus_LiveTransceiver_SetAction(Transceiver, __VBusPSendPacket2Handler, &__VBusPSendPacketXOptions, __VBusPLiveTransceiverActionHandler));
    } else if (Phase == __PHASE_SENDPACKET3) {
        // send "send control info" packet to PVS
        __WRAP(ResolVBus_LiveTransceiver_SetAction(Transceiver, __VBusPSendPacket3Handler, &__VBusPSendPacketXOptions, __VBusPLiveTransceiverActionHandler));
    } else if (Phase == __PHASE_RELEASEBUS) {
        // release bus
        __WRAP(ResolVBus_LiveTransceiver_ReleaseBus(Transceiver, Bridge->VBusPPeerAddress, NULL, __VBusPLiveTransceiverActionHandler));
    } else {
        __WRAP(__PerformVBusPPhaseAction(Bridge, __PHASE_WAITFORFREEBUS));
    }

    return Result;
}


static RESOLVBUS_RESULT __VBusPLiveTransceiverHandler(RESOLVBUS_LIVETRANSCEIVER *Transceiver, const RESOLVBUS_LIVETRANSCEIVEREVENT *Event)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    // __ACCESSORYBRIDGE *Bridge = RESOLVBUS_CONTAINEROF(Transceiver, __ACCESSORYBRIDGE, VBusPLiveTransceiver);

    if (Event->EventType == RESOLVBUS_LIVETRANSCEIVEREVENTTYPE_ENCODER) {
        const RESOLVBUS_LIVEENCODEREVENT *EncoderEvent = Event->EncoderEvent;

        if (EncoderEvent->EventType == RESOLVBUS_LIVEENCODEREVENTTYPE_TRANSMIT) {
            // TODO: send `EncoderEvent->TransmitBytes` of length `EncoderEvent->TransmitLength` to physical VBus
        }
    }

    return Result;
}


static RESOLVBUS_RESULT __VBusVLiveEncoderHandler(RESOLVBUS_LIVEENCODER *Encoder, const RESOLVBUS_LIVEENCODEREVENT *Event)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    // __ACCESSORYBRIDGE *Bridge = RESOLVBUS_CONTAINEROF(Encoder, __ACCESSORYBRIDGE, VBusVLiveEncoder);

    if (Event->EventType == RESOLVBUS_LIVEENCODEREVENTTYPE_TRANSMIT) {
        // TODO: send `Event->TransmitBytes` of length `Event->TransmitLength` to virtual VBus
    }

    return Result;
}


static RESOLVBUS_RESULT __Initialize(__ACCESSORYBRIDGE *Bridge)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    if (Result == RESOLVBUS_OK) {
        memset(Bridge, 0, sizeof (*Bridge));
    }

    __WRAP(ResolVBus_LiveTransceiver_Initialize(&Bridge->VBusPLiveTransceiver, Bridge->VBusPLiveEncoderBuffer, sizeof (Bridge->VBusPLiveEncoderBuffer), Bridge->VBusPLiveDecoderBuffer, sizeof (Bridge->VBusPLiveDecoderBuffer), __VBusPLiveTransceiverHandler));

    __WRAP(__PerformVBusPPhaseAction(Bridge, __PHASE_WAITFORFREEBUS));

    __WRAP(ResolVBus_LiveEncoder_Initialize(&Bridge->VBusVLiveEncoder, Bridge->VBusVLiveEncoderBytes, sizeof (Bridge->VBusVLiveEncoderBytes), __VBusVLiveEncoderHandler));

    __WRAP(ResolVBus_LiveEncoder_Suspend(&Bridge->VBusVLiveEncoder));

    return Result;
}


static RESOLVBUS_RESULT __GetTimeout(__ACCESSORYBRIDGE *Bridge, uint32_t *pTimeoutUs)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    uint32_t VBusPTimeout = UINT32_MAX;
    __WRAP(ResolVBus_LiveTransceiver_GetTimeout(&Bridge->VBusPLiveTransceiver, &VBusPTimeout));

    uint32_t VBusVTimeout = UINT32_MAX;
    __WRAP(ResolVBus_LiveEncoder_GetTimeout(&Bridge->VBusVLiveEncoder, &VBusVTimeout));

    if (Result == RESOLVBUS_OK) {
        *pTimeoutUs = (VBusPTimeout < VBusVTimeout) ? VBusPTimeout : VBusVTimeout;
    }

    return Result;
}


static RESOLVBUS_RESULT __HandleLoopCycle(__ACCESSORYBRIDGE *Bridge, uint32_t TimePassedUs, const uint8_t *ReadBytes, size_t ReadLength)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    __WRAP(ResolVBus_LiveTransceiver_HandleTimer(&Bridge->VBusPLiveTransceiver, TimePassedUs));

    if (ReadBytes) {
        __WRAP(ResolVBus_LiveTransceiver_Decode(&Bridge->VBusPLiveTransceiver, ReadBytes, ReadLength));

        __WRAP(ResolVBus_LiveTransceiver_HandleTimer(&Bridge->VBusPLiveTransceiver, 0));
    }

    __WRAP(ResolVBus_LiveEncoder_HandleTimer(&Bridge->VBusVLiveEncoder, TimePassedUs));

    return Result;
}


static RESOLVBUS_RESULT __Finalize(__ACCESSORYBRIDGE *Bridge)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    return Result;
}



//---------------------------------------------------------------------------
// PUBLIC METHODS
//---------------------------------------------------------------------------

#ifndef __IGNORE_MAIN_FOR_TESTING

int main(void)
{
    RESOLVBUS_RESULT Result = 0;

    __ACCESSORYBRIDGE Bridge = { 0 };
    __WRAP(__Initialize(&Bridge));

    if (Result == RESOLVBUS_OK) {
        // TODO: open serial ports / sockets / ...
    }

    while (Result == RESOLVBUS_OK) {
        uint32_t TimeoutUs = 0;
        __WRAP(__GetTimeout(&Bridge, &TimeoutUs));

        uint8_t ReadBytes [128] = {};
        size_t ReadLength = 0;
        if (Result == RESOLVBUS_OK) {
            // TODO: read from serial ports / sockets / ... with short timeout / non-blocking
            // `TimeoutUs` contains an indication, how long the timeout should be
        }

        uint32_t TimePassedUs = 0;
        if (Result == RESOLVBUS_OK) {
            // TODO: calculate time passed since last loop cycle and store it in `TimePassedUs`
        }

        __WRAP(__HandleLoopCycle(&Bridge, TimePassedUs, ReadBytes, ReadLength));
    }

    // TODO: close serial ports / sockets / ...

    __WRAP(__Finalize(&Bridge));

    if (Result != 0) {
        // fprintf(stderr, "%s", __Backtrace);
        return 1;
    } else {
        return 0;
    }
}

#endif // __IGNORE_MAIN_FOR_TESTING
