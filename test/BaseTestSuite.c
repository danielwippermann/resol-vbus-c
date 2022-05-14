//
//  ResolVBusBaseTest.c
//  resol-vbus
//
//
//  Copyright (C) 2022, Daniel Wippermann. All rights reserved.
//

#include "Testing.h"



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

static RESOLVBUS_RESULT __TestReadUInt16LE(void)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    const uint8_t Bytes [4] = { 0x78, 0x56, 0x34, 0x12 };

    __ASSERT_EQL(ResolVBus_ReadUInt16LE(Bytes), 0x5678);

    return Result;
}


static RESOLVBUS_RESULT __TestWriteUInt16LE(void)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    uint8_t Bytes [4] = { 0 };

    ResolVBus_WriteUInt16LE(Bytes, 0x5678);

    __ASSERT_EQL(Bytes [0], 0x78);
    __ASSERT_EQL(Bytes [1], 0x56);

    return Result;
}


static RESOLVBUS_RESULT __TestReadUInt32LE(void)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    const uint8_t Bytes [4] = { 0x78, 0x56, 0x34, 0x12 };

    __ASSERT_EQL(ResolVBus_ReadUInt32LE(Bytes), 0x12345678);

    return Result;
}


static RESOLVBUS_RESULT __TestWriteUInt32LE(void)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    uint8_t Bytes [4] = { 0 };

    ResolVBus_WriteUInt32LE(Bytes, 0x12345678);

    __ASSERT_EQL(Bytes [0], 0x78);
    __ASSERT_EQL(Bytes [1], 0x56);
    __ASSERT_EQL(Bytes [2], 0x34);
    __ASSERT_EQL(Bytes [3], 0x12);

    return Result;
}


static RESOLVBUS_RESULT __TestCalcCrcV0(void)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    uint8_t Crc = 0;

    __WRAP(ResolVBus_CalcCrcV0(TestData_Live1 + 1, 0, &Crc));
    __ASSERT(Crc == 0x7F);

    __WRAP(ResolVBus_CalcCrcV0(TestData_Live1 + 1, 8, &Crc));
    __ASSERT(Crc == 0x34);

    __WRAP(ResolVBus_CalcCrcV0(TestData_Live1 + 1, 9, &Crc));
    __ASSERT(Crc == 0x00);

    return Result;
}


static RESOLVBUS_RESULT __TestCalcCrc(void)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    uint8_t Crc = 0;

    __WRAP(ResolVBus_CalcCrc(0x10, TestData_Live1 + 1, 9, &Crc));
    __ASSERT(Crc == 0x00);

    __ASSERT_RESULT_EQL(UNSUPPORTEDPROTOCOL, NULL, ResolVBus_CalcCrc(0x11, TestData_Live1 + 1, 9, &Crc));

    return Result;
}


static RESOLVBUS_RESULT __TestCalcAndSetCrc(void)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    uint8_t Buffer [] = { 0x10, 0x00, 0x11, 0x7e, 0x10, 0x00, 0x01, 0x1b, 0x00 };

    __WRAP(ResolVBus_CalcAndSetCrc(0x10, Buffer, sizeof (Buffer)));

    __ASSERT(Buffer [8] == 0x34);

    return Result;
}


static RESOLVBUS_RESULT __TestCalcAndCompareCrc(void)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    bool CrcIsValid = false;
    __WRAP(ResolVBus_CalcAndCompareCrc(0x10, TestData_Live1 + 1, 9, &CrcIsValid));

    __ASSERT(CrcIsValid);

    return Result;
}


static RESOLVBUS_RESULT __TestInjectSeptett(void)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    const uint8_t Source [] = { 0x38, 0x22, 0x38, 0x22, 0x05 };
    uint8_t Destination [4] = { 0 };
    __WRAP(ResolVBus_InjectSeptett(Source, Destination, 4));

    __ASSERT(Destination [0] == 0xb8);
    __ASSERT(Destination [1] == 0x22);
    __ASSERT(Destination [2] == 0xb8);
    __ASSERT(Destination [3] == 0x22);

    return Result;
}


static RESOLVBUS_RESULT __TestExtractSeptett(void)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    const uint8_t Source [] = { 0xb8, 0x22, 0xb8, 0x22 };
    uint8_t Destination [5] = { 0 };
    __WRAP(ResolVBus_ExtractSeptett(Source, Destination, 4));

    __ASSERT(Destination [0] == 0x38);
    __ASSERT(Destination [1] == 0x22);
    __ASSERT(Destination [2] == 0x38);
    __ASSERT(Destination [3] == 0x22);
    __ASSERT(Destination [4] == 0x05);

    return Result;
}



//---------------------------------------------------------------------------
// PUBLIC METHODS
//---------------------------------------------------------------------------

RESOLVBUS_RESULT RunTestSuite_Base(void)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    __WRAP(__TestReadUInt16LE());
    __WRAP(__TestWriteUInt16LE());
    __WRAP(__TestReadUInt32LE());
    __WRAP(__TestWriteUInt32LE());

    __WRAP(__TestCalcCrcV0());
    __WRAP(__TestCalcCrc());
    __WRAP(__TestCalcAndSetCrc());
    __WRAP(__TestCalcAndCompareCrc());
    __WRAP(__TestInjectSeptett());
    __WRAP(__TestExtractSeptett());

    return Result;
}

