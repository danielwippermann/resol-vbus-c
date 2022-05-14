//
//  ResolVBusLiveDecoder.c
//  resol-vbus
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



//---------------------------------------------------------------------------
// NON-PUBLIC METHODS
//---------------------------------------------------------------------------

static RESOLVBUS_RESULT __SendEvent(RESOLVBUS_LIVEDECODER *Decoder, RESOLVBUS_LIVEDECODEREVENTTYPE EventType)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    if (Result == RESOLVBUS_OK) {
        Decoder->Event.EventType = EventType;

        __WRAP(Decoder->Handler(Decoder, &Decoder->Event));
    }

    return Result;
}



//---------------------------------------------------------------------------
// PUBLIC METHODS
//---------------------------------------------------------------------------

RESOLVBUS_RESULT ResolVBus_LiveDecoder_Initialize(RESOLVBUS_LIVEDECODER *Decoder, void *FrameDataBuffer, size_t FrameDataBufferLength, RESOLVBUS_LIVEDECODERHANDLER Handler)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    __ASSERT_WITH(NULLPOINTER, Decoder && Handler);

    if (Result == RESOLVBUS_OK) {
        *Decoder = (RESOLVBUS_LIVEDECODER) {
            .Handler = Handler,
            .Event = {
                .FrameDataBuffer = FrameDataBuffer,
                .FrameDataBufferLength = FrameDataBufferLength,
            },
        };
    }

    return Result;
}


RESOLVBUS_RESULT ResolVBus_LiveDecoder_Decode(RESOLVBUS_LIVEDECODER *Decoder, const uint8_t *Bytes, size_t Length)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    __ASSERT_WITH(NULLPOINTER, Decoder && Bytes);

    for (size_t Index = 0; (Result == RESOLVBUS_OK) && (Index < Length); Index++) {
        uint8_t Byte = Bytes [Index];
        if (Byte == 0xAA) {
            Decoder->Buffer [0] = Byte;
            Decoder->BufferIndex = 1;
        } else if (Byte & 0x80) {
            Decoder->BufferIndex = 0;
        } else if (Decoder->BufferIndex == 0) {
            // nop
        } else if (Decoder->BufferIndex >= sizeof (Decoder->Buffer)) {
            Decoder->BufferIndex = 0;
        } else {
            Decoder->Buffer [Decoder->BufferIndex] = Byte;
            Decoder->BufferIndex += 1;

            RESOLVBUS_LIVEDECODEREVENT *Event = &Decoder->Event;

            if (Decoder->BufferIndex < 6) {
                // nop
            } else if (Decoder->BufferIndex == 6) {
                Event->DestinationAddress = ResolVBus_ReadUInt16LE(Decoder->Buffer + 1);
                Event->SourceAddress = ResolVBus_ReadUInt16LE(Decoder->Buffer + 3);
                Event->ProtocolVersion = Decoder->Buffer [5];
                Event->MajorVersion = Event->ProtocolVersion & 0xF0;
            } else if (Event->MajorVersion == 0x10) {
                if (Decoder->BufferIndex == 10) {
                    bool CrcIsValid = false;
                    __WRAP(ResolVBus_CalcAndCompareCrc(Event->ProtocolVersion, Decoder->Buffer + 1, 9, &CrcIsValid));

                    if (!CrcIsValid) {
                        __FAIL(INVALIDCHECKSUM);
                    }

                    if (Result == RESOLVBUS_OK) {
                        Event->Command = ResolVBus_ReadUInt16LE(Decoder->Buffer + 6);
                        Event->FrameCount = Decoder->Buffer [8];
                        Event->FrameIndex = 0;
                        Event->FrameDataLength = 4;

                        __WRAP(__SendEvent(Decoder, RESOLVBUS_LIVEDECODEREVENTTYPE_PACKETHEADER));

                        if (Result == RESOLVBUS_OK) {
                            if (Event->FrameCount == 0) {
                                __WRAP(__SendEvent(Decoder, RESOLVBUS_LIVEDECODEREVENTTYPE_PACKETEND));

                                Decoder->BufferIndex = 0;
                            }
                        }
                    }
                } else if (Decoder->BufferIndex == 16) {
                    bool CrcIsValid = false;
                    __WRAP(ResolVBus_CalcAndCompareCrc(Event->ProtocolVersion, Decoder->Buffer + 10, 6, &CrcIsValid));

                    if (!CrcIsValid) {
                        __FAIL(INVALIDCHECKSUM);
                    }

                    __WRAP(ResolVBus_InjectSeptett(Decoder->Buffer + 10, Event->FrameData, 4));

                    if (Result == RESOLVBUS_OK) {
                        size_t SrcIndex = 0;
                        size_t DstIndex = Event->FrameIndex * 4;
                        while ((SrcIndex < 4) && (DstIndex < Event->FrameDataBufferLength)) {
                            ((uint8_t *) Event->FrameDataBuffer) [DstIndex] = Event->FrameData [SrcIndex];
                            SrcIndex += 1;
                            DstIndex += 1;
                        }
                    }

                    __WRAP(__SendEvent(Decoder, RESOLVBUS_LIVEDECODEREVENTTYPE_PACKETFRAME));

                    if (Result == RESOLVBUS_OK) {
                        Event->FrameIndex += 1;

                        if (Event->FrameIndex == Event->FrameCount) {
                            __WRAP(__SendEvent(Decoder, RESOLVBUS_LIVEDECODEREVENTTYPE_PACKETEND));

                            Decoder->BufferIndex = 0;
                        } else {
                            Decoder->BufferIndex = 10;
                        }
                    }
                }
            } else if (Event->MajorVersion == 0x20) {
                if (Decoder->BufferIndex == 16) {
                    bool CrcIsValid = false;
                    __WRAP(ResolVBus_CalcAndCompareCrc(Event->ProtocolVersion, Decoder->Buffer + 1, 15, &CrcIsValid));

                    if (!CrcIsValid) {
                        __FAIL(INVALIDCHECKSUM);
                    }

                    if (Result == RESOLVBUS_OK) {
                        Event->Command = ResolVBus_ReadUInt16LE(Decoder->Buffer + 6);

                        __WRAP(ResolVBus_InjectSeptett(Decoder->Buffer + 8, Event->FrameData, 6));

                        if (Result == RESOLVBUS_OK) {
                            Event->Param16 = ResolVBus_ReadUInt16LE(Event->FrameData + 0);
                            Event->Param32 = ResolVBus_ReadUInt32LE(Event->FrameData + 2);
                        }

                        __WRAP(__SendEvent(Decoder, RESOLVBUS_LIVEDECODEREVENTTYPE_DATAGRAM));

                        Decoder->BufferIndex = 0;
                    }
                }
            } else if (Event->MajorVersion == 0x30) {
                if (Decoder->BufferIndex == 8) {
                    bool CrcIsValid = false;
                    __WRAP(ResolVBus_CalcAndCompareCrc(Event->ProtocolVersion, Decoder->Buffer + 1, 7, &CrcIsValid));

                    if (!CrcIsValid) {
                        __FAIL(INVALIDCHECKSUM);
                    }

                    if (Result == RESOLVBUS_OK) {
                        Event->Command = Decoder->Buffer [6] & 0x1F;
                        Event->FrameCount = Decoder->Buffer [6] >> 5;
                        Event->FrameIndex = 0;
                        Event->FrameDataLength = 7;

                        __WRAP(__SendEvent(Decoder, RESOLVBUS_LIVEDECODEREVENTTYPE_TELEGRAMHEADER));

                        if (Result == RESOLVBUS_OK) {
                            if (Event->FrameCount == 0) {
                                __WRAP(__SendEvent(Decoder, RESOLVBUS_LIVEDECODEREVENTTYPE_TELEGRAMEND));

                                Decoder->BufferIndex = 0;
                            }
                        }
                    }
                } else if (Decoder->BufferIndex == 17) {
                    bool CrcIsValid = false;
                    __WRAP(ResolVBus_CalcAndCompareCrc(Event->ProtocolVersion, Decoder->Buffer + 8, 9, &CrcIsValid));

                    if (!CrcIsValid) {
                        __FAIL(INVALIDCHECKSUM);
                    }

                    __WRAP(ResolVBus_InjectSeptett(Decoder->Buffer + 8, Event->FrameData, 7));

                    if (Result == RESOLVBUS_OK) {
                        size_t SrcIndex = 0;
                        size_t DstIndex = Event->FrameIndex * 7;
                        while ((SrcIndex < 7) && (DstIndex < Event->FrameDataBufferLength)) {
                            ((uint8_t *) Event->FrameDataBuffer) [DstIndex] = Event->FrameData [SrcIndex];
                            SrcIndex += 1;
                            DstIndex += 1;
                        }
                    }

                    __WRAP(__SendEvent(Decoder, RESOLVBUS_LIVEDECODEREVENTTYPE_TELEGRAMFRAME));

                    if (Result == RESOLVBUS_OK) {
                        Event->FrameIndex += 1;

                        if (Event->FrameIndex == Event->FrameCount) {
                            __WRAP(__SendEvent(Decoder, RESOLVBUS_LIVEDECODEREVENTTYPE_TELEGRAMEND));

                            Decoder->BufferIndex = 0;
                        } else {
                            Decoder->BufferIndex = 8;
                        }
                    }
                }
            } else {
                __FAIL(UNSUPPORTEDPROTOCOL);
            }
        }

        if (Result != RESOLVBUS_OK) {
            Decoder->BufferIndex = 0;
            Result = RESOLVBUS_OK;
        }
    }

    return Result;
}
