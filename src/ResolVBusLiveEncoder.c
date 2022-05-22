//
//  ResolVBusLiveEncoder.c
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

static bool __IsIdle(RESOLVBUS_LIVEENCODER *Encoder)
{
    return ((Encoder->Phase == RESOLVBUS_LIVEENCODERPHASE_IDLE) && (Encoder->BufferWriteIndex == 0) && !Encoder->SuspendRequested && !Encoder->SuspendWithTimeoutRequested);
}


static RESOLVBUS_RESULT __EnsureNotSuspended(RESOLVBUS_LIVEENCODER *Encoder)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    if ((Encoder->Phase == RESOLVBUS_LIVEENCODERPHASE_SUSPENDED) || (Encoder->Phase == RESOLVBUS_LIVEENCODERPHASE_SUSPENDEDWITHTIMEOUT) || Encoder->SuspendRequested || Encoder->SuspendWithTimeoutRequested) {
        __FAIL(SUSPENDED);
    }

    return Result;
}



//---------------------------------------------------------------------------
// PUBLIC METHODS
//---------------------------------------------------------------------------

RESOLVBUS_RESULT ResolVBus_LiveEncoder_Initialize(RESOLVBUS_LIVEENCODER *Encoder, uint8_t *Buffer, size_t Length, RESOLVBUS_LIVEENCODERHANDLER Handler)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    __ASSERT_WITH(NULLPOINTER, Encoder && Buffer && Handler);

    if (Result == RESOLVBUS_OK) {
        *Encoder = (RESOLVBUS_LIVEENCODER) {
            .Buffer = Buffer,
            .BufferLength = Length,
            .Handler = Handler,
            .Baudrate = 9600,
        };
    }

    return Result;
}


RESOLVBUS_RESULT ResolVBus_LiveEncoder_GetTimeout(RESOLVBUS_LIVEENCODER *Encoder, uint32_t *pMicroseconds)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    __ASSERT_WITH(NULLPOINTER, Encoder && pMicroseconds);

    if (Result == RESOLVBUS_OK) {
        uint32_t Timeout = 0;
        switch (Encoder->Phase) {
        case RESOLVBUS_LIVEENCODERPHASE_IDLE:
            Timeout = 0;
            break;
        case RESOLVBUS_LIVEENCODERPHASE_TRANSMITTING:
        case RESOLVBUS_LIVEENCODERPHASE_GAININGENERGY:
        case RESOLVBUS_LIVEENCODERPHASE_SUSPENDEDWITHTIMEOUT:
            Timeout = Encoder->PhaseTimeoutUs;
            break;
        case RESOLVBUS_LIVEENCODERPHASE_SUSPENDED:
            Timeout = UINT32_MAX;
            break;
        default:
            __FAIL(NYI);
            break;
        }

        *pMicroseconds = Timeout;
    }

    return Result;
}



RESOLVBUS_RESULT ResolVBus_LiveEncoder_HandleTimer(RESOLVBUS_LIVEENCODER *Encoder, uint32_t Microseconds)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    __ASSERT_WITH(NULLPOINTER, Encoder);

    uint32_t RemainingMicroseconds = Microseconds;
    bool EmittedIdle = false;

    int Loops = 0;
    while (Result == RESOLVBUS_OK) {
        if (Loops < 10) {
            Loops += 1;
        } else {
            __FAIL(NYI);
        }

        if (Encoder->PhaseTimeoutUs > RemainingMicroseconds) {
            Encoder->PhaseTimeoutUs -= RemainingMicroseconds;
            RemainingMicroseconds = 0;
        } else {
            RemainingMicroseconds -= Encoder->PhaseTimeoutUs;
            Encoder->PhaseTimeoutUs = 0;
        }

        bool PhaseIsTimedOut = (Encoder->PhaseTimeoutUs == 0);

        if (Encoder->Phase == RESOLVBUS_LIVEENCODERPHASE_IDLE) {
            if (Encoder->BufferWriteIndex > 0) {
                size_t MaxBufferLength;
                if (Encoder->BufferWriteIndex > Encoder->BufferReadIndex) {
                    MaxBufferLength = Encoder->BufferWriteIndex - Encoder->BufferReadIndex;
                } else {
                    MaxBufferLength = 0;
                }

                size_t MaxEnergyLength = 0;
                if (!Encoder->MaxEnergy || !Encoder->EnergyLossPerByte) {
                    MaxEnergyLength = MaxBufferLength;
                } else {
                    MaxEnergyLength = Encoder->MaxEnergy / Encoder->EnergyLossPerByte;
                }

                size_t MaxLength = (MaxBufferLength <= MaxEnergyLength) ? MaxBufferLength : MaxEnergyLength;

                RESOLVBUS_LIVEENCODEREVENT Event = {
                    .EventType = RESOLVBUS_LIVEENCODEREVENTTYPE_TRANSMIT,
                    .TransmitBytes = Encoder->Buffer + Encoder->BufferReadIndex,
                    .TransmitLength = MaxLength,
                };

                __WRAP(Encoder->Handler(Encoder, &Event));

                if (Result == RESOLVBUS_OK) {
                    Encoder->BufferReadIndex += MaxLength;

                    Encoder->CurrentEnergy += Encoder->EnergyLossPerByte * MaxLength;

                    uint32_t TimeoutUs = (MaxLength * 10000000 + Encoder->Baudrate - 1) / Encoder->Baudrate;

                    Encoder->Phase = RESOLVBUS_LIVEENCODERPHASE_TRANSMITTING;
                    Encoder->PhaseTimeoutUs = TimeoutUs;

                    RemainingMicroseconds = 0;
                } else if (Result == RESOLVBUS_WOULDBLOCK) {
                    Result = RESOLVBUS_OK;
                    break;
                }
            } else if (Encoder->SuspendRequested) {
                Encoder->SuspendRequested = false;
                Encoder->Phase = RESOLVBUS_LIVEENCODERPHASE_SUSPENDED;
            } else if (Encoder->SuspendWithTimeoutRequested) {
                Encoder->SuspendWithTimeoutRequested = false;

                uint32_t TimeoutUs = Encoder->SuspendTimeoutUs;
                Encoder->SuspendTimeoutUs = 0;

                Encoder->Phase = RESOLVBUS_LIVEENCODERPHASE_SUSPENDEDWITHTIMEOUT;
                Encoder->PhaseTimeoutUs = TimeoutUs;
            } else if (!EmittedIdle) {
                EmittedIdle = true;

                RESOLVBUS_LIVEENCODEREVENT Event = {
                    .EventType = RESOLVBUS_LIVEENCODEREVENTTYPE_IDLE,
                };

                __WRAP(Encoder->Handler(Encoder, &Event));
            } else {
                // nothing to do
                break;
            }
        } else if (Encoder->Phase == RESOLVBUS_LIVEENCODERPHASE_TRANSMITTING) {
            if (PhaseIsTimedOut) {
                if (Encoder->BufferReadIndex >= Encoder->BufferWriteIndex) {
                    Encoder->BufferReadIndex = 0;
                    Encoder->BufferWriteIndex = 0;
                }

                uint32_t TimeoutUs;
                if (!Encoder->MaxEnergy || !Encoder->EnergyGainPerUs) {
                    TimeoutUs = 0;
                } else {
                    TimeoutUs = (Encoder->CurrentEnergy + Encoder->EnergyGainPerUs - 1) / Encoder->EnergyGainPerUs;
                }

                if (TimeoutUs > 0) {
                    Encoder->Phase = RESOLVBUS_LIVEENCODERPHASE_GAININGENERGY;
                    Encoder->PhaseTimeoutUs = TimeoutUs;
                } else {
                    Encoder->Phase = RESOLVBUS_LIVEENCODERPHASE_IDLE;
                }
            } else {
                // wait for transmission to complete
                break;
            }
        } else if (Encoder->Phase == RESOLVBUS_LIVEENCODERPHASE_GAININGENERGY) {
            if (PhaseIsTimedOut) {
                Encoder->CurrentEnergy = 0;

                Encoder->Phase = RESOLVBUS_LIVEENCODERPHASE_IDLE;
            } else {
                // wait for energy regain
                break;
            }
        } else if (Encoder->Phase == RESOLVBUS_LIVEENCODERPHASE_SUSPENDED) {
            // wait for manual resume
            break;
        } else if (Encoder->Phase == RESOLVBUS_LIVEENCODERPHASE_SUSPENDEDWITHTIMEOUT) {
            if (PhaseIsTimedOut) {
                Encoder->Phase = RESOLVBUS_LIVEENCODERPHASE_IDLE;
            } else {
                // wait for timeout or manual resume
                break;
            }
        } else {
            __FAIL(NYI);
        }
    }

    return Result;
}


RESOLVBUS_RESULT ResolVBus_LiveEncoder_Suspend(RESOLVBUS_LIVEENCODER *Encoder)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    __ASSERT_WITH(NULLPOINTER, Encoder);

    if (Result == RESOLVBUS_OK) {
        if (Encoder->Phase == RESOLVBUS_LIVEENCODERPHASE_SUSPENDED) {
            // nop
        } else if (Encoder->Phase == RESOLVBUS_LIVEENCODERPHASE_SUSPENDEDWITHTIMEOUT) {
            Encoder->Phase = RESOLVBUS_LIVEENCODERPHASE_SUSPENDED;
        } else {
            bool WasIdle = __IsIdle(Encoder);

            Encoder->SuspendRequested = true;
            Encoder->SuspendWithTimeoutRequested = false;

            if (WasIdle) {
                __WRAP(ResolVBus_LiveEncoder_HandleTimer(Encoder, 0));
            }
        }
    }

    return Result;
}


RESOLVBUS_RESULT ResolVBus_LiveEncoder_SuspendWithTimeout(RESOLVBUS_LIVEENCODER *Encoder, uint32_t Microseconds)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    __ASSERT_WITH(NULLPOINTER, Encoder);

    if (Result == RESOLVBUS_OK) {
        if (Encoder->Phase == RESOLVBUS_LIVEENCODERPHASE_SUSPENDEDWITHTIMEOUT) {
            if (Encoder->PhaseTimeoutUs < Microseconds) {
                Encoder->PhaseTimeoutUs = Microseconds;
            }
        } else if ((Encoder->Phase == RESOLVBUS_LIVEENCODERPHASE_SUSPENDED) || Encoder->SuspendRequested) {
            __FAIL(SUSPENDED);
        } else {
            bool WasIdle = __IsIdle(Encoder);

            Encoder->SuspendWithTimeoutRequested = true;
            if (Encoder->SuspendTimeoutUs < Microseconds) {
                Encoder->SuspendTimeoutUs = Microseconds;
            }

            if (WasIdle) {
                __WRAP(ResolVBus_LiveEncoder_HandleTimer(Encoder, 0));
            }
        }
    }

    return Result;
}


RESOLVBUS_RESULT ResolVBus_LiveEncoder_Resume(RESOLVBUS_LIVEENCODER *Encoder)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    __ASSERT_WITH(NULLPOINTER, Encoder);

    if (Result == RESOLVBUS_OK) {
        Encoder->SuspendRequested = false;
        Encoder->SuspendWithTimeoutRequested = false;
        Encoder->SuspendTimeoutUs = 0;

        if ((Encoder->Phase == RESOLVBUS_LIVEENCODERPHASE_SUSPENDED) || (Encoder->Phase == RESOLVBUS_LIVEENCODERPHASE_SUSPENDEDWITHTIMEOUT)) {
            Encoder->Phase = RESOLVBUS_LIVEENCODERPHASE_IDLE;
            Encoder->PhaseTimeoutUs = 0;
        }
    }

    return Result;
}


RESOLVBUS_RESULT ResolVBus_LiveEncoder_QueuePacketHeader(RESOLVBUS_LIVEENCODER *Encoder, uint16_t DestinationAddress, uint16_t SourceAddress, uint8_t MinorVersion, uint16_t Command, uint8_t FrameCount)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    __ASSERT_WITH(NULLPOINTER, Encoder);
    __ASSERT_WITH(INDEXOUTOFBOUNDS, (Encoder->BufferWriteIndex + 10) <= Encoder->BufferLength);
    __WRAP(__EnsureNotSuspended(Encoder));

    if (Result == RESOLVBUS_OK) {
        uint8_t *Buffer = Encoder->Buffer + Encoder->BufferWriteIndex;
        Encoder->BufferWriteIndex += 10;

        Encoder->ProtocolVersion = 0x10 | (MinorVersion & 0x0F);

        Buffer [0] = 0xAA;
        ResolVBus_WriteUInt16LE(Buffer + 1, DestinationAddress);
        ResolVBus_WriteUInt16LE(Buffer + 3, SourceAddress);
        Buffer [5] = Encoder->ProtocolVersion;
        ResolVBus_WriteUInt16LE(Buffer + 6, Command);
        Buffer [8] = FrameCount;

        __WRAP(ResolVBus_CalcAndSetCrc(Encoder->ProtocolVersion, Buffer + 1, 9));
    }

    return Result;
}


RESOLVBUS_RESULT ResolVBus_LiveEncoder_QueuePacketFrame(RESOLVBUS_LIVEENCODER *Encoder, const uint8_t *FourBytes)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    __ASSERT_WITH(NULLPOINTER, Encoder && FourBytes);
    __ASSERT_WITH(INDEXOUTOFBOUNDS, (Encoder->BufferWriteIndex + 6) <= Encoder->BufferLength);

    __WRAP(__EnsureNotSuspended(Encoder));

    if (Result == RESOLVBUS_OK) {
        uint8_t *Buffer = Encoder->Buffer + Encoder->BufferWriteIndex;
        Encoder->BufferWriteIndex += 6;

        __WRAP(ResolVBus_ExtractSeptett(FourBytes, Buffer + 0, 4));
        __WRAP(ResolVBus_CalcAndSetCrc(Encoder->ProtocolVersion, Buffer + 0, 6));
    }

    return Result;
}


RESOLVBUS_RESULT ResolVBus_LiveEncoder_QueuePacketFrames(RESOLVBUS_LIVEENCODER *Encoder, const void *Bytes, size_t Length)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    __ASSERT_WITH(NULLPOINTER, Encoder && Bytes);

    __WRAP(__EnsureNotSuspended(Encoder));

    size_t SrcIndex = 0;
    while ((Result == RESOLVBUS_OK) && ((SrcIndex + 4) <= Length)) {
        __WRAP(ResolVBus_LiveEncoder_QueuePacketFrame(Encoder, ((const uint8_t *) Bytes) + SrcIndex));

        SrcIndex += 4;
    }

    if ((Result == RESOLVBUS_OK) && (SrcIndex < Length)) {
        uint8_t FourBytes [4] = { 0 };
        size_t DstIndex = 0;

        while ((DstIndex < 4) && (SrcIndex < Length)) {
            FourBytes [DstIndex] = ((const uint8_t *) Bytes) [SrcIndex];
            DstIndex += 1;
            SrcIndex += 1;
        }

        while (DstIndex < 4) {
            FourBytes [DstIndex] = 0xFF;
            DstIndex += 1;
        }

        __WRAP(ResolVBus_LiveEncoder_QueuePacketFrame(Encoder, FourBytes));
    }

    __ASSERT_WITH(INVALIDSTATE, SrcIndex == Length);

    return Result;
}


RESOLVBUS_RESULT ResolVBus_LiveEncoder_QueueDatagram(RESOLVBUS_LIVEENCODER *Encoder, uint16_t DestinationAddress, uint16_t SourceAddress, uint8_t MinorVersion, uint16_t Command, uint16_t Param16, uint32_t Param32)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    __ASSERT_WITH(NULLPOINTER, Encoder);
    __ASSERT_WITH(INDEXOUTOFBOUNDS, (Encoder->BufferWriteIndex + 16) <= Encoder->BufferLength);

    __WRAP(__EnsureNotSuspended(Encoder));

    if (Result == RESOLVBUS_OK) {
        uint8_t *Buffer = Encoder->Buffer + Encoder->BufferWriteIndex;
        Encoder->BufferWriteIndex += 16;

        Encoder->ProtocolVersion = 0x20 | (MinorVersion & 0x0F);

        uint8_t SixBytes [6] = { 0 };
        ResolVBus_WriteUInt16LE(SixBytes + 0, Param16);
        ResolVBus_WriteUInt32LE(SixBytes + 2, Param32);

        Buffer [0] = 0xAA;
        ResolVBus_WriteUInt16LE(Buffer + 1, DestinationAddress);
        ResolVBus_WriteUInt16LE(Buffer + 3, SourceAddress);
        Buffer [5] = Encoder->ProtocolVersion;
        ResolVBus_WriteUInt16LE(Buffer + 6, Command);

        __WRAP(ResolVBus_ExtractSeptett(SixBytes, Buffer + 8, 6));
        __WRAP(ResolVBus_CalcAndSetCrc(Encoder->ProtocolVersion, Buffer + 1, 15));
    }

    return Result;
}


RESOLVBUS_RESULT ResolVBus_LiveEncoder_QueueTelegramHeader(RESOLVBUS_LIVEENCODER *Encoder, uint16_t DestinationAddress, uint16_t SourceAddress, uint8_t MinorVersion, uint8_t Command, uint8_t FrameCount)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    __ASSERT_WITH(NULLPOINTER, Encoder);
    __ASSERT_WITH(INDEXOUTOFBOUNDS, (Encoder->BufferWriteIndex + 8) <= Encoder->BufferLength);

    __WRAP(__EnsureNotSuspended(Encoder));

    if (Result == RESOLVBUS_OK) {
        uint8_t *Buffer = Encoder->Buffer + Encoder->BufferWriteIndex;
        Encoder->BufferWriteIndex += 8;

        Encoder->ProtocolVersion = 0x30 | (MinorVersion & 0x0F);

        Buffer [0] = 0xAA;
        ResolVBus_WriteUInt16LE(Buffer + 1, DestinationAddress);
        ResolVBus_WriteUInt16LE(Buffer + 3, SourceAddress);
        Buffer [5] = Encoder->ProtocolVersion;
        Buffer [6] = ((FrameCount & 3) << 5) | (Command & 0x1F);

        __WRAP(ResolVBus_CalcAndSetCrc(Encoder->ProtocolVersion, Buffer + 1, 7));
    }

    return Result;
}


RESOLVBUS_RESULT ResolVBus_LiveEncoder_QueueTelegramFrame(RESOLVBUS_LIVEENCODER *Encoder, const uint8_t *SevenBytes)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    __ASSERT_WITH(NULLPOINTER, Encoder && SevenBytes);
    __ASSERT_WITH(INDEXOUTOFBOUNDS, (Encoder->BufferWriteIndex + 9) <= Encoder->BufferLength);

    __WRAP(__EnsureNotSuspended(Encoder));

    if (Result == RESOLVBUS_OK) {
        uint8_t *Buffer = Encoder->Buffer + Encoder->BufferWriteIndex;
        Encoder->BufferWriteIndex += 9;

        __WRAP(ResolVBus_ExtractSeptett(SevenBytes, Buffer + 0, 7));
        __WRAP(ResolVBus_CalcAndSetCrc(Encoder->ProtocolVersion, Buffer + 0, 9));
    }

    return Result;
}


RESOLVBUS_RESULT ResolVBus_LiveEncoder_QueueTelegramFrames(RESOLVBUS_LIVEENCODER *Encoder, const void *Bytes, size_t Length)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    __ASSERT_WITH(NULLPOINTER, Encoder && Bytes);

    __WRAP(__EnsureNotSuspended(Encoder));

    size_t SrcIndex = 0;
    while ((Result == RESOLVBUS_OK) && ((SrcIndex + 7) <= Length)) {
        __WRAP(ResolVBus_LiveEncoder_QueueTelegramFrame(Encoder, ((const uint8_t *) Bytes) + SrcIndex));

        SrcIndex += 7;
    }

    if ((Result == RESOLVBUS_OK) && (SrcIndex < Length)) {
        uint8_t SevenBytes [7] = { 0 };
        size_t DstIndex = 0;

        while ((DstIndex < 7) && (SrcIndex < Length)) {
            SevenBytes [DstIndex] = ((const uint8_t *) Bytes) [SrcIndex];
            DstIndex += 1;
            SrcIndex += 1;
        }

        while (DstIndex < 7) {
            SevenBytes [DstIndex] = 0xFF;
            DstIndex += 1;
        }

        __WRAP(ResolVBus_LiveEncoder_QueueTelegramFrame(Encoder, SevenBytes));
    }

    __ASSERT_WITH(INVALIDSTATE, SrcIndex == Length);

    return Result;
}
