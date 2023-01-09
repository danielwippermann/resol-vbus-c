//
//  ResolVBusLiveTransceiver.c
//  resol-vbus-c
//
//
//  Copyright (C) 2022, Daniel Wippermann. All rights reserved.
//

#include "ResolVBus.h"


#include "Debug.h"



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

static const RESOLVBUS_LIVETRANSCEIVEROPTIONS __DefaultOptions = {
    .Tries = 3,
    .InitialTimeout = 500000,
    .TimeoutIncrement = 500000,
};


static const RESOLVBUS_LIVETRANSCEIVEROPTIONS __WaitForFreeBusOptions = {
    .Tries = 1,
    .InitialTimeout = 20000000,
    .TimeoutIncrement = 0,
};


static const RESOLVBUS_LIVETRANSCEIVEROPTIONS __ReleaseBusOptions = {
    .Tries = 2,
    .InitialTimeout = 1500000,
    .TimeoutIncrement = 0,
};



//---------------------------------------------------------------------------
// NON-PUBLIC METHODS
//---------------------------------------------------------------------------

static RESOLVBUS_RESULT __Reset(RESOLVBUS_LIVETRANSCEIVER *Transceiver)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    Transceiver->ActionSet = false;
    Transceiver->ActionHandler = NULL;
    Transceiver->ActionTries = 0;
    Transceiver->ActionNextTimeout = 0;
    Transceiver->ActionTimeoutIncr = 0;
    Transceiver->ActionTimeout = 0;
    Transceiver->ActionCommitHandler = NULL;

    Transceiver->ActionPeerAddress = 0;
    Transceiver->ActionCommand = -1;
    Transceiver->ActionParam16 = -1;
    Transceiver->ActionParam32 = -1;
    Transceiver->ActionExpectedCommand1 = -1;
    Transceiver->ActionExpectedCommand2 = -1;
    Transceiver->ActionExpectedCommand3 = -1;
    Transceiver->ActionHasExpectedParam16 = false;
    Transceiver->ActionHasExpectedParam32 = false;

    return Result;
}


static RESOLVBUS_RESULT __SendActionEvent(RESOLVBUS_LIVETRANSCEIVER *Transceiver)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    if (Result == RESOLVBUS_OK) {
        Transceiver->ActionTimeout = Transceiver->ActionNextTimeout;
        Transceiver->ActionNextTimeout += Transceiver->ActionTimeoutIncr;

        RESOLVBUS_LIVETRANSCEIVEREVENT Event = {
            .EventType = RESOLVBUS_LIVETRANSCEIVEREVENTTYPE_ACTION,
        };

        Result = Transceiver->ActionHandler(Transceiver, &Event);
    }

    return Result;
}


static RESOLVBUS_RESULT __SetAction(RESOLVBUS_LIVETRANSCEIVER *Transceiver, RESOLVBUS_LIVETRANSCEIVERHANDLER Handler, const RESOLVBUS_LIVETRANSCEIVEROPTIONS *Options, RESOLVBUS_LIVETRANSCEIVERHANDLER CommitHandler)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    __ASSERT_WITH(INVALIDSTATE, !Transceiver->ActionSet);

    if (Result == RESOLVBUS_OK) {
        Transceiver->ActionSet = true;
        Transceiver->ActionHandler = Handler;
        Transceiver->ActionTries = Options->Tries;
        Transceiver->ActionNextTimeout = Options->InitialTimeout;
        Transceiver->ActionTimeoutIncr = Options->TimeoutIncrement;
        Transceiver->ActionCommitHandler = CommitHandler;

        Result = __SendActionEvent(Transceiver);
    }

    return Result;
}


static RESOLVBUS_RESULT __CommitAction(RESOLVBUS_LIVETRANSCEIVER *Transceiver, const RESOLVBUS_LIVETRANSCEIVEREVENT *Event)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    RESOLVBUS_LIVETRANSCEIVERHANDLER CommitHandler = Transceiver->ActionCommitHandler;

    __WRAP(__Reset(Transceiver));

    if (CommitHandler) {
        __WRAP(CommitHandler(Transceiver, Event));
    }

    return Result;
}


static RESOLVBUS_RESULT __ApplyOptions(const RESOLVBUS_LIVETRANSCEIVEROPTIONS *DefaultOptions, const RESOLVBUS_LIVETRANSCEIVEROPTIONS *CustomOptions, RESOLVBUS_LIVETRANSCEIVEROPTIONS *Options)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    Options->Tries = (CustomOptions && CustomOptions->Tries) ? CustomOptions->Tries : DefaultOptions->Tries;
    Options->InitialTimeout = (CustomOptions && CustomOptions->InitialTimeout) ? CustomOptions->InitialTimeout : DefaultOptions->InitialTimeout;
    Options->TimeoutIncrement = (CustomOptions && CustomOptions->TimeoutIncrement) ? CustomOptions->TimeoutIncrement : DefaultOptions->TimeoutIncrement;

    return Result;
}


static RESOLVBUS_RESULT __WaitForFreeBusHandler(RESOLVBUS_LIVETRANSCEIVER *Transceiver, const RESOLVBUS_LIVETRANSCEIVEREVENT *Event)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    if (Event->EventType == RESOLVBUS_LIVETRANSCEIVEREVENTTYPE_ACTION) {
        __WRAP(ResolVBus_LiveEncoder_Resume(&Transceiver->Encoder));

        // nop

        __WRAP(ResolVBus_LiveEncoder_Suspend(&Transceiver->Encoder));
    } else if (Event->EventType == RESOLVBUS_LIVETRANSCEIVEREVENTTYPE_DECODER) {
        if (Event->DecoderEvent->EventType != RESOLVBUS_LIVEDECODEREVENTTYPE_DATAGRAM) {
            // nop
        } else if (Event->DecoderEvent->Command != 0x0500) {
            // nop
        } else {
            __WRAP(__CommitAction(Transceiver, Event));
        }
    }

    return Result;
}


static RESOLVBUS_RESULT __ReleaseBusHandler(RESOLVBUS_LIVETRANSCEIVER *Transceiver, const RESOLVBUS_LIVETRANSCEIVEREVENT *Event)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    if (Event->EventType == RESOLVBUS_LIVETRANSCEIVEREVENTTYPE_ACTION) {
        __WRAP(ResolVBus_LiveEncoder_Resume(&Transceiver->Encoder));

        __WRAP(ResolVBus_LiveEncoder_QueueDatagram(&Transceiver->Encoder, Transceiver->ActionPeerAddress, Transceiver->SelfAddress, 0, 0x0600, 0, 0));

        __WRAP(ResolVBus_LiveEncoder_Suspend(&Transceiver->Encoder));
    } else if (Event->EventType == RESOLVBUS_LIVETRANSCEIVEREVENTTYPE_DECODER) {
        if (Event->DecoderEvent->EventType != RESOLVBUS_LIVEDECODEREVENTTYPE_PACKETHEADER) {
            // nop
        } else {
            __WRAP(__CommitAction(Transceiver, Event));
        }
    }

    return Result;
}


static RESOLVBUS_RESULT __DatagramHandler(RESOLVBUS_LIVETRANSCEIVER *Transceiver, const RESOLVBUS_LIVETRANSCEIVEREVENT *Event)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    if (Event->EventType == RESOLVBUS_LIVETRANSCEIVEREVENTTYPE_ACTION) {
        __WRAP(ResolVBus_LiveEncoder_Resume(&Transceiver->Encoder));

        __WRAP(ResolVBus_LiveEncoder_QueueDatagram(&Transceiver->Encoder, Transceiver->ActionPeerAddress, Transceiver->SelfAddress, 0, Transceiver->ActionCommand, Transceiver->ActionParam16, Transceiver->ActionParam32));

        __WRAP(ResolVBus_LiveEncoder_Suspend(&Transceiver->Encoder));
    } else if (Event->EventType == RESOLVBUS_LIVETRANSCEIVEREVENTTYPE_DECODER) {
        if (Event->DecoderEvent->EventType != RESOLVBUS_LIVEDECODEREVENTTYPE_DATAGRAM) {
            // nop
        } else if (Event->DecoderEvent->DestinationAddress != Transceiver->SelfAddress) {
            // nop
        } else if (Event->DecoderEvent->SourceAddress != Transceiver->ActionPeerAddress) {
            // nop
        } else if ((Event->DecoderEvent->Command != Transceiver->ActionExpectedCommand1) && (Event->DecoderEvent->Command != Transceiver->ActionExpectedCommand2) && (Event->DecoderEvent->Command != Transceiver->ActionExpectedCommand3)) {
            // nop
        } else if (Transceiver->ActionHasExpectedParam16 && (Event->DecoderEvent->Param16 != Transceiver->ActionExpectedParam16)) {
            // nop
        } else if (Transceiver->ActionHasExpectedParam32 && (Event->DecoderEvent->Param32 != Transceiver->ActionExpectedParam32)) {
            // nop
        } else {
            __WRAP(__CommitAction(Transceiver, Event));
        }
    }

    return Result;
}


static RESOLVBUS_RESULT __EncoderHandler(RESOLVBUS_LIVEENCODER *Encoder, const RESOLVBUS_LIVEENCODEREVENT *EncoderEvent)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    RESOLVBUS_LIVETRANSCEIVER *Transceiver = RESOLVBUS_CONTAINEROF(Encoder, RESOLVBUS_LIVETRANSCEIVER, Encoder);

    if (Transceiver->Handler) {
        RESOLVBUS_LIVETRANSCEIVEREVENT Event = {
            .EventType = RESOLVBUS_LIVETRANSCEIVEREVENTTYPE_ENCODER,
            .EncoderEvent = EncoderEvent,
        };

        __WRAP(Transceiver->Handler(Transceiver, &Event));
    }

    return Result;
}


static RESOLVBUS_RESULT __DecoderHandler(RESOLVBUS_LIVEDECODER *Decoder, const RESOLVBUS_LIVEDECODEREVENT *DecoderEvent)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    RESOLVBUS_LIVETRANSCEIVER *Transceiver = RESOLVBUS_CONTAINEROF(Decoder, RESOLVBUS_LIVETRANSCEIVER, Decoder);

    if (Transceiver->ActionHandler) {
        RESOLVBUS_LIVETRANSCEIVEREVENT Event = {
            .EventType = RESOLVBUS_LIVETRANSCEIVEREVENTTYPE_DECODER,
            .DecoderEvent = DecoderEvent,
        };

        __WRAP(Transceiver->ActionHandler(Transceiver, &Event));
    }

    if (Transceiver->Handler) {
        RESOLVBUS_LIVETRANSCEIVEREVENT Event = {
            .EventType = RESOLVBUS_LIVETRANSCEIVEREVENTTYPE_DECODER,
            .DecoderEvent = DecoderEvent,
        };

        __WRAP(Transceiver->Handler(Transceiver, &Event));
    }

    return Result;
}



//---------------------------------------------------------------------------
// PUBLIC METHODS
//---------------------------------------------------------------------------

RESOLVBUS_RESULT ResolVBus_LiveTransceiver_Initialize(RESOLVBUS_LIVETRANSCEIVER *Transceiver, uint8_t *EncoderBuffer, size_t EncoderBufferLength, void *DecoderBuffer, size_t DecoderBufferLength, RESOLVBUS_LIVETRANSCEIVERHANDLER Handler)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    __ASSERT_WITH(NULLPOINTER, Transceiver);

    if (Result == RESOLVBUS_OK) {
        *Transceiver = (RESOLVBUS_LIVETRANSCEIVER) RESOLVBUS_LIVETRANSCEIVER_INITIALIZER;
    }

    __WRAP(ResolVBus_LiveEncoder_Initialize(&Transceiver->Encoder, EncoderBuffer, EncoderBufferLength, __EncoderHandler));

    __WRAP(ResolVBus_LiveEncoder_Suspend(&Transceiver->Encoder));

    __WRAP(ResolVBus_LiveDecoder_Initialize(&Transceiver->Decoder, DecoderBuffer, DecoderBufferLength, __DecoderHandler));

    if (Result == RESOLVBUS_OK) {
        Transceiver->Handler = Handler;
        Transceiver->SelfAddress = 0x0020;
    }

    __WRAP(__Reset(Transceiver));

    return Result;
}


RESOLVBUS_RESULT ResolVBus_LiveTransceiver_GetTimeout(RESOLVBUS_LIVETRANSCEIVER *Transceiver, uint32_t *pMicroseconds)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    __ASSERT_WITH(NULLPOINTER, Transceiver && pMicroseconds);

    uint32_t EncoderTimeout = 0;
    __WRAP(ResolVBus_LiveEncoder_GetTimeout(&Transceiver->Encoder, &EncoderTimeout));

    if (Result == RESOLVBUS_OK) {
        uint32_t ActionTimeout = (Transceiver->ActionTries > 0) ? Transceiver->ActionTimeout : UINT32_MAX;

        uint32_t Microseconds = 0;
        if (EncoderTimeout < ActionTimeout) {
            Microseconds = EncoderTimeout;
        } else {
            Microseconds = ActionTimeout;
        }

        *pMicroseconds = Microseconds;
    }

    return Result;
}


RESOLVBUS_RESULT ResolVBus_LiveTransceiver_HandleTimer(RESOLVBUS_LIVETRANSCEIVER *Transceiver, uint32_t Microseconds)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    __ASSERT_WITH(NULLPOINTER, Transceiver);

    if (Result == RESOLVBUS_OK) {
        if (Transceiver->ActionTries > 0) {
            if (Transceiver->ActionTimeout >= Microseconds) {
                Transceiver->ActionTimeout -= Microseconds;
            } else {
                Transceiver->ActionTimeout = 0;
            }

            if (Transceiver->ActionTimeout == 0) {
                Transceiver->ActionTries -= 1;

                if (Transceiver->ActionTries > 0) {
                    __WRAP(__SendActionEvent(Transceiver));
                } else {
                    RESOLVBUS_LIVETRANSCEIVEREVENT Event = {
                        .EventType = RESOLVBUS_LIVETRANSCEIVEREVENTTYPE_TIMEOUT,
                    };

                    __WRAP(__CommitAction(Transceiver, &Event));
                }
            }
        }
    }

    __WRAP(ResolVBus_LiveEncoder_HandleTimer(&Transceiver->Encoder, Microseconds));

    return Result;
}


// RESOLVBUS_RESULT ResolVBus_LiveTransceiver_GetEncoder(RESOLVBUS_LIVETRANSCEIVER *Transceiver, RESOLVBUS_LIVEENCODER **pEncoder);


RESOLVBUS_RESULT ResolVBus_LiveTransceiver_Decode(RESOLVBUS_LIVETRANSCEIVER *Transceiver, const uint8_t *Bytes, size_t Length)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    __ASSERT_WITH(NULLPOINTER, Transceiver);

    __WRAP(ResolVBus_LiveDecoder_Decode(&Transceiver->Decoder, Bytes, Length));

    return Result;
}


RESOLVBUS_RESULT ResolVBus_LiveTransceiver_WaitForFreeBus(RESOLVBUS_LIVETRANSCEIVER *Transceiver, const RESOLVBUS_LIVETRANSCEIVEROPTIONS *CustomOptions, RESOLVBUS_LIVETRANSCEIVERHANDLER Handler)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    RESOLVBUS_LIVETRANSCEIVEROPTIONS Options = RESOLVBUS_LIVETRANSCEIVEROPTIONS_INITIALIZER;
    if (Result == RESOLVBUS_OK) {
        Result = __ApplyOptions(&__WaitForFreeBusOptions, CustomOptions, &Options);
    }

    if (Result == RESOLVBUS_OK) {
        Result = __SetAction(Transceiver, __WaitForFreeBusHandler, &Options, Handler);
    }

    return Result;
}


RESOLVBUS_RESULT ResolVBus_LiveTransceiver_ReleaseBus(RESOLVBUS_LIVETRANSCEIVER *Transceiver, uint16_t PeerAddress, const RESOLVBUS_LIVETRANSCEIVEROPTIONS *CustomOptions, RESOLVBUS_LIVETRANSCEIVERHANDLER Handler)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    RESOLVBUS_LIVETRANSCEIVEROPTIONS Options = RESOLVBUS_LIVETRANSCEIVEROPTIONS_INITIALIZER;
    if (Result == RESOLVBUS_OK) {
        Result = __ApplyOptions(&__ReleaseBusOptions, CustomOptions, &Options);
    }

    if (Result == RESOLVBUS_OK) {
        Transceiver->ActionPeerAddress = PeerAddress;

        Result = __SetAction(Transceiver, __ReleaseBusHandler, &Options, Handler);
    }

    return Result;
}


RESOLVBUS_RESULT ResolVBus_LiveTransceiver_GetValueById(RESOLVBUS_LIVETRANSCEIVER *Transceiver, uint16_t PeerAddress, uint16_t ValueId, uint8_t ValueSubIndex, const RESOLVBUS_LIVETRANSCEIVEROPTIONS *CustomOptions, RESOLVBUS_LIVETRANSCEIVERHANDLER Handler)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    RESOLVBUS_LIVETRANSCEIVEROPTIONS Options = RESOLVBUS_LIVETRANSCEIVEROPTIONS_INITIALIZER;
    if (Result == RESOLVBUS_OK) {
        Result = __ApplyOptions(&__DefaultOptions, CustomOptions, &Options);
    }

    if (Result == RESOLVBUS_OK) {
        Transceiver->ActionPeerAddress = PeerAddress;
        Transceiver->ActionCommand = 0x0300 | ValueSubIndex;
        Transceiver->ActionParam16 = ValueId;
        Transceiver->ActionParam32 = 0;
        Transceiver->ActionExpectedCommand1 = 0x0100 | ValueSubIndex;
        Transceiver->ActionExpectedCommand2 = 0x4300 | ValueSubIndex;
        Transceiver->ActionExpectedCommand3 = -1;
        Transceiver->ActionHasExpectedParam16 = true;
        Transceiver->ActionExpectedParam16 = ValueId;
        Transceiver->ActionHasExpectedParam32 = false;

        Result = __SetAction(Transceiver, __DatagramHandler, &Options, Handler);
    }

    return Result;
}


RESOLVBUS_RESULT ResolVBus_LiveTransceiver_SetValueById(RESOLVBUS_LIVETRANSCEIVER *Transceiver, uint16_t PeerAddress, uint16_t ValueId, uint8_t ValueSubIndex, uint32_t Value, const RESOLVBUS_LIVETRANSCEIVEROPTIONS *CustomOptions, RESOLVBUS_LIVETRANSCEIVERHANDLER Handler)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    RESOLVBUS_LIVETRANSCEIVEROPTIONS Options = RESOLVBUS_LIVETRANSCEIVEROPTIONS_INITIALIZER;
    if (Result == RESOLVBUS_OK) {
        Result = __ApplyOptions(&__DefaultOptions, CustomOptions, &Options);
    }

    if (Result == RESOLVBUS_OK) {
        Transceiver->ActionPeerAddress = PeerAddress;
        Transceiver->ActionCommand = 0x0200 | ValueSubIndex;
        Transceiver->ActionParam16 = ValueId;
        Transceiver->ActionParam32 = Value;
        Transceiver->ActionExpectedCommand1 = 0x0100 | ValueSubIndex;
        Transceiver->ActionExpectedCommand2 = -1;
        Transceiver->ActionExpectedCommand3 = -1;
        Transceiver->ActionHasExpectedParam16 = true;
        Transceiver->ActionExpectedParam16 = ValueId;
        Transceiver->ActionHasExpectedParam32 = false;

        Result = __SetAction(Transceiver, __DatagramHandler, &Options, Handler);
    }

    return Result;
}


RESOLVBUS_RESULT ResolVBus_LiveTransceiver_GetValueIdByIdHash(RESOLVBUS_LIVETRANSCEIVER *Transceiver, uint16_t PeerAddress, uint32_t ValueIdHash, const RESOLVBUS_LIVETRANSCEIVEROPTIONS *CustomOptions, RESOLVBUS_LIVETRANSCEIVERHANDLER Handler)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    RESOLVBUS_LIVETRANSCEIVEROPTIONS Options = RESOLVBUS_LIVETRANSCEIVEROPTIONS_INITIALIZER;
    if (Result == RESOLVBUS_OK) {
        Result = __ApplyOptions(&__DefaultOptions, CustomOptions, &Options);
    }

    if (Result == RESOLVBUS_OK) {
        Transceiver->ActionPeerAddress = PeerAddress;
        Transceiver->ActionCommand = 0x1100;
        Transceiver->ActionParam16 = 0;
        Transceiver->ActionParam32 = ValueIdHash;
        Transceiver->ActionExpectedCommand1 = 0x0100;
        Transceiver->ActionExpectedCommand2 = 0x1101;
        Transceiver->ActionExpectedCommand3 = -1;
        Transceiver->ActionHasExpectedParam16 = false;
        Transceiver->ActionHasExpectedParam32 = true;
        Transceiver->ActionExpectedParam32 = ValueIdHash;

        Result = __SetAction(Transceiver, __DatagramHandler, &Options, Handler);
    }

    return Result;
}
