//
//  ResolVBusBase.c
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



//---------------------------------------------------------------------------
// PUBLIC METHODS
//---------------------------------------------------------------------------

uint16_t ResolVBus_ReadUInt16LE(const uint8_t *Buffer)
{
    uint16_t Value = Buffer [0];
    Value |= (((uint16_t) Buffer [1]) << 8);
    return Value;
}


void ResolVBus_WriteUInt16LE(uint8_t *Buffer, uint16_t Value)
{
    Buffer [0] = Value;
    Buffer [1] = (Value >> 8);
}


uint32_t ResolVBus_ReadUInt32LE(const uint8_t *Buffer)
{
    uint32_t Value = Buffer [0];
    Value |= (((uint32_t) Buffer [1]) << 8);
    Value |= (((uint32_t) Buffer [2]) << 16);
    Value |= (((uint32_t) Buffer [3]) << 24);
    return Value;
}


void ResolVBus_WriteUInt32LE(uint8_t *Buffer, uint32_t Value)
{
    Buffer [0] = Value;
    Buffer [1] = (Value >> 8);
    Buffer [2] = (Value >> 16);
    Buffer [3] = (Value >> 24);
}


RESOLVBUS_RESULT ResolVBus_CalcCrcV0(const uint8_t *Buffer, size_t Length, uint8_t *pCrc)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    __ASSERT_WITH(NULLPOINTER, Buffer);

    uint8_t Crc = 0x00;
    if (Result == RESOLVBUS_OK) {
        for (size_t Index = 0; Index < Length; Index++) {
            Crc = (Crc + Buffer [Index]) & 0x7F;
        }
    }

    if (pCrc) {
        *pCrc = 0x7F - Crc;
    }

    return Result;
}


RESOLVBUS_RESULT ResolVBus_CalcCrc(uint8_t ProtocolVersion, const uint8_t *Buffer, size_t Length, uint8_t *pCrc)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    if (Result == RESOLVBUS_OK) {
        uint8_t MinorVersion = (ProtocolVersion & 0x0F);
        if (MinorVersion == 0x00) {
            __WRAP(ResolVBus_CalcCrcV0(Buffer, Length, pCrc));
        } else {
            __FAIL(UNSUPPORTEDPROTOCOL);
        }
    }

    return Result;
}


RESOLVBUS_RESULT ResolVBus_CalcAndSetCrc(uint8_t ProtocolVersion, uint8_t *Buffer, size_t Length)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    __ASSERT_WITH(NULLPOINTER, Buffer);
    __ASSERT_WITH(INVALIDARGUMENT, Length >= 1);

    uint8_t Crc = 0;
    __WRAP(ResolVBus_CalcCrc(ProtocolVersion, Buffer, Length - 1, &Crc));

    if (Result == RESOLVBUS_OK) {
        Buffer [Length - 1] = Crc;
    }

    return Result;
}


RESOLVBUS_RESULT ResolVBus_CalcAndCompareCrc(uint8_t ProtocolVersion, const uint8_t *Buffer, size_t Length, bool *pCrcIsValid)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    __ASSERT_WITH(NULLPOINTER, Buffer);
    __ASSERT_WITH(INVALIDARGUMENT, Length >= 1);

    uint8_t Crc = 0;
    __WRAP(ResolVBus_CalcCrc(ProtocolVersion, Buffer, Length - 1, &Crc));

    bool CrcIsValid = false;
    if (Result == RESOLVBUS_OK) {
        CrcIsValid = (Buffer [Length - 1] == Crc);
    }

    if (pCrcIsValid) {
        *pCrcIsValid = CrcIsValid;
    }

    return Result;
}


RESOLVBUS_RESULT ResolVBus_InjectSeptett(const uint8_t *Source, uint8_t *Destination, size_t Length)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    __ASSERT_WITH(NULLPOINTER, Source && Destination);
    __ASSERT_WITH(INVALIDARGUMENT, Length <= 7);

    if (Result == RESOLVBUS_OK) {
        uint8_t Septett = Source [Length];
        for (size_t Index = 0; Index < Length; Index++) {
            uint8_t Mask = (Septett & (1 << Index)) ? 0x80 : 0x00;
            Destination [Index] = Source [Index] | Mask;
        }
    }

    return Result;
}


RESOLVBUS_RESULT ResolVBus_ExtractSeptett(const uint8_t *Source, uint8_t *Destination, size_t Length)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    __ASSERT_WITH(NULLPOINTER, Source && Destination);
    __ASSERT_WITH(INVALIDARGUMENT, Length <= 7);

    if (Result == RESOLVBUS_OK) {
        uint8_t Septett = 0;
        for (size_t Index = 0; Index < Length; Index++) {
            Destination [Index] = Source [Index] & 0x7F;
            if (Source [Index] & 0x80) {
                Septett |= (1 << Index);
            }
        }
        Destination [Length] = Septett;
    }

    return Result;
}
