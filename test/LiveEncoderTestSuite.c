//
//  LiveEncoderTestSuite.c
//  resol-vbus
//
//
//  Copyright (C) 2022, Daniel Wippermann. All rights reserved.
//

#include <stdarg.h>
#include <stdio.h>
#include <string.h>


#include "Testing.h"



//---------------------------------------------------------------------------
// PRIVATE DEFINES
//---------------------------------------------------------------------------

#define __PREAMBLE() \
    RESOLVBUS_LIVEENCODER EncoderLocal = RESOLVBUS_LIVEENCODER_INITIALIZER; \
    RESOLVBUS_LIVEENCODER *Encoder = &EncoderLocal; \
    uint8_t Buffer [512] = { 0 }; \
    __WRAP(ResolVBus_LiveEncoder_Initialize(Encoder, Buffer, sizeof (Buffer), __Handler)); \
    Encoder->MaxEnergy = 10000; \
    Encoder->EnergyLossPerByte = 500; \
    Encoder->EnergyGainPerUs = 1; \
    __HandlerLog [0] = 0; \
    __HandlerResult = RESOLVBUS_OK; \


#define __APPEND_LOG(...) \
    if (Result == RESOLVBUS_OK) { \
        AppendToLog(__HandlerLog, sizeof (__HandlerLog), __VA_ARGS__); \
    } \


#define __ASSERT_LOG_EQL(__Expected__) \
    __WRAP_SHORT(AssertStringEql(__Expected__, "Expected", __HandlerLog, "__HandlerLog")); \
    if (Result == RESOLVBUS_OK) { \
        __HandlerLog [0] = 0; \
    } \



//---------------------------------------------------------------------------
// PRIVATE TYPEDEFS
//---------------------------------------------------------------------------

typedef struct __CALLBACKCONTEXT __CALLBACKCONTEXT;



//---------------------------------------------------------------------------
// PRIVATE DATA TYPES
//---------------------------------------------------------------------------

struct __CALLBACKCONTEXT {
    RESOLVBUS_LIVEENCODER *Encoder;
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

static char __HandlerLog [1024];
static RESOLVBUS_RESULT __HandlerResult;



//---------------------------------------------------------------------------
// NON-PUBLIC METHODS
//---------------------------------------------------------------------------

static RESOLVBUS_RESULT __Handler(RESOLVBUS_LIVEENCODER *Encoder, const RESOLVBUS_LIVEENCODEREVENT *Event)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    if (Event->EventType == RESOLVBUS_LIVEENCODEREVENTTYPE_IDLE) {
        __APPEND_LOG("Event = IDLE\n");
    } else if (Event->EventType == RESOLVBUS_LIVEENCODEREVENTTYPE_TRANSMIT) {
        __APPEND_LOG("Event = TRANSMIT, Data =");
        for (size_t Index = 0; Index < Event->TransmitLength; Index++) {
            __APPEND_LOG(" 0x%02X", ((const uint8_t *) Event->TransmitBytes) [Index]);
        }
        __APPEND_LOG("\n");
    } else {
        __APPEND_LOG("Event = ??? (%d)", Event);
    }

    // __FAIL(NYI); // FIXME(daniel): add tests here

    if (Result == RESOLVBUS_OK) {
        Result = __HandlerResult;
    }

    return Result;
}


static RESOLVBUS_RESULT __TestInitialize(void)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    RESOLVBUS_LIVEENCODER Encoder = { 0 };
    uint8_t Buffer [512] = { 0 };

    __WRAP(ResolVBus_LiveEncoder_Initialize(&Encoder, Buffer, sizeof (Buffer), __Handler));

    __ASSERT(Encoder.Buffer == Buffer);
    __ASSERT(Encoder.BufferLength == sizeof (Buffer));
    __ASSERT_EQL(Encoder.BufferWriteIndex, 0);
    __ASSERT_EQL(Encoder.BufferReadIndex, 0);
    __ASSERT_EQL(Encoder.Baudrate, 9600);
    __ASSERT_EQL(Encoder.MaxEnergy, 0);
    __ASSERT_EQL(Encoder.EnergyLossPerByte, 0);
    __ASSERT_EQL(Encoder.EnergyGainPerUs, 0);
    __ASSERT_EQL(Encoder.CurrentEnergy, 0);
    __ASSERT_EQL(Encoder.SuspendRequested, false);
    __ASSERT_EQL(Encoder.SuspendTimeoutUs, 0);
    __ASSERT_EQL(Encoder.Phase, RESOLVBUS_LIVEENCODERPHASE_IDLE);
    __ASSERT(Encoder.Handler == __Handler);

    return Result;
}


static RESOLVBUS_RESULT __TestHandleTimer(void)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    __PREAMBLE();

    __WRAP(ResolVBus_LiveEncoder_HandleTimer(Encoder, 0));

    return Result;
}


static RESOLVBUS_RESULT __TestQueueXxxWhileSuspended(void)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    __PREAMBLE();

    const uint8_t FourBytes [4] = { 0 };

    __WRAP(ResolVBus_LiveEncoder_QueuePacketFrame(Encoder, FourBytes));
    __WRAP(ResolVBus_LiveEncoder_Suspend(Encoder));

    __ASSERT_EQL(RESOLVBUS_LIVEENCODERPHASE_IDLE, Encoder->Phase);
    __ASSERT_EQL(true, Encoder->SuspendRequested);

    __ASSERT_RESULT_EQL(SUSPENDED, NULL, ResolVBus_LiveEncoder_QueuePacketFrame(Encoder, FourBytes));

    __WRAP(ResolVBus_LiveEncoder_HandleTimer(Encoder, 10000));
    __WRAP(ResolVBus_LiveEncoder_HandleTimer(Encoder, 10000));

    __ASSERT_EQL(RESOLVBUS_LIVEENCODERPHASE_SUSPENDED, Encoder->Phase);
    __ASSERT_EQL(false, Encoder->SuspendRequested);

    __ASSERT_RESULT_EQL(SUSPENDED, NULL, ResolVBus_LiveEncoder_QueuePacketFrame(Encoder, FourBytes));

    __WRAP(ResolVBus_LiveEncoder_Resume(Encoder));
    __WRAP(ResolVBus_LiveEncoder_QueuePacketFrame(Encoder, FourBytes));
    __WRAP(ResolVBus_LiveEncoder_SuspendWithTimeout(Encoder, 100000));

    __ASSERT_EQL(RESOLVBUS_LIVEENCODERPHASE_IDLE, Encoder->Phase);
    __ASSERT_EQL(true, Encoder->SuspendWithTimeoutRequested);

    __ASSERT_RESULT_EQL(SUSPENDED, NULL, ResolVBus_LiveEncoder_QueuePacketFrame(Encoder, FourBytes));

    __WRAP(ResolVBus_LiveEncoder_HandleTimer(Encoder, 10000));
    __WRAP(ResolVBus_LiveEncoder_HandleTimer(Encoder, 10000));

    __ASSERT_EQL(RESOLVBUS_LIVEENCODERPHASE_SUSPENDEDWITHTIMEOUT, Encoder->Phase);
    __ASSERT_EQL(false, Encoder->SuspendWithTimeoutRequested);

    __ASSERT_RESULT_EQL(SUSPENDED, NULL, ResolVBus_LiveEncoder_QueuePacketFrame(Encoder, FourBytes));

    __WRAP(ResolVBus_LiveEncoder_HandleTimer(Encoder, 0));

    __ASSERT_EQL(RESOLVBUS_LIVEENCODERPHASE_SUSPENDEDWITHTIMEOUT, Encoder->Phase);
    __ASSERT_EQL(false, Encoder->SuspendWithTimeoutRequested);

    __ASSERT_RESULT_EQL(SUSPENDED, NULL, ResolVBus_LiveEncoder_QueuePacketFrame(Encoder, FourBytes));

    return Result;
}


static RESOLVBUS_RESULT __TestQueuePacketFrames(void)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    __PREAMBLE();

    const uint8_t Bytes [8] = { 0x11, 0x44, 0x77, 0xAA, 0xDD, 0x22, 0x55, 0x88 };

    __WRAP(ResolVBus_LiveEncoder_QueuePacketFrames(Encoder, Bytes, 8));

    __WRAP(ResolVBus_LiveEncoder_HandleTimer(Encoder, 0));

    __ASSERT_LOG_EQL("Event = TRANSMIT, Data = 0x11 0x44 0x77 0x2A 0x08 0x01 0x5D 0x22 0x55 0x08 0x09 0x1A\n");

    __WRAP(ResolVBus_LiveEncoder_HandleTimer(Encoder, 20000));

    __ASSERT_LOG_EQL("Event = IDLE\n");

    __WRAP(ResolVBus_LiveEncoder_QueuePacketFrames(Encoder, Bytes, 6));

    __WRAP(ResolVBus_LiveEncoder_HandleTimer(Encoder, 0));

    __ASSERT_LOG_EQL("Event = TRANSMIT, Data = 0x11 0x44 0x77 0x2A 0x08 0x01 0x5D 0x22 0x7F 0x7F 0x0D 0x75\n");

    __WRAP(ResolVBus_LiveEncoder_HandleTimer(Encoder, 20000));

    __ASSERT_LOG_EQL("Event = IDLE\n");

    return Result;
}


static RESOLVBUS_RESULT __TestQueueTelegramXxx(void)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    __PREAMBLE();

    const uint8_t Bytes1 [14] = {
        0x60, 0x18, 0xAB, 0x04, 0x00, 0x00, 0x00,
    };

    const uint8_t Bytes2 [14] = {
        0x11, 0x44, 0x77, 0xAA, 0xDD, 0x22, 0x55,
        0x88, 0xBB, 0xEE, 0x33, 0x66, 0x99, 0xCC,
    };

    __WRAP(ResolVBus_LiveEncoder_QueueTelegramHeader(Encoder, 0x7771, 0x2011, 0x00, 0x05, 1));

    __WRAP(ResolVBus_LiveEncoder_HandleTimer(Encoder, 0));

    __ASSERT_LOG_EQL("Event = TRANSMIT, Data = 0xAA 0x71 0x77 0x11 0x20 0x30 0x25 0x11\n");

    __WRAP(ResolVBus_LiveEncoder_HandleTimer(Encoder, 20000));

    __ASSERT_LOG_EQL("Event = IDLE\n");

    __WRAP(ResolVBus_LiveEncoder_QueueTelegramFrame(Encoder, Bytes1));

    __WRAP(ResolVBus_LiveEncoder_HandleTimer(Encoder, 0));

    __ASSERT_LOG_EQL("Event = TRANSMIT, Data = 0x60 0x18 0x2B 0x04 0x00 0x00 0x00 0x04 0x54\n");

    __WRAP(ResolVBus_LiveEncoder_HandleTimer(Encoder, 20000));

    __ASSERT_LOG_EQL("Event = IDLE\n");

    __WRAP(ResolVBus_LiveEncoder_QueueTelegramFrames(Encoder, Bytes2, 14));

    __WRAP(ResolVBus_LiveEncoder_HandleTimer(Encoder, 0));

    __ASSERT_LOG_EQL("Event = TRANSMIT, Data = 0x11 0x44 0x77 0x2A 0x5D 0x22 0x55 0x18 0x1D 0x08 0x3B 0x6E 0x33 0x66 0x19 0x4C 0x67 0x69\n");

    __WRAP(ResolVBus_LiveEncoder_HandleTimer(Encoder, 40000));

    __ASSERT_LOG_EQL("Event = IDLE\n");

    __WRAP(ResolVBus_LiveEncoder_QueueTelegramFrames(Encoder, Bytes2, 10));

    __WRAP(ResolVBus_LiveEncoder_HandleTimer(Encoder, 0));

    __ASSERT_LOG_EQL("Event = TRANSMIT, Data = 0x11 0x44 0x77 0x2A 0x5D 0x22 0x55 0x18 0x1D 0x08 0x3B 0x6E 0x7F 0x7F 0x7F 0x7F 0x7F 0x53\n");

    __WRAP(ResolVBus_LiveEncoder_HandleTimer(Encoder, 40000));

    __ASSERT_LOG_EQL("Event = IDLE\n");

    return Result;
}


static RESOLVBUS_RESULT __TestSuspendAndResume(void)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    __PREAMBLE();

    const uint8_t FourBytes [4] = { 0 };

    // Scenario 1: calling `ResolVBus_LiveEncoder_Resume` while the encoder is idle does nothing

    __ASSERT_EQL(Encoder->SuspendRequested, false);
    __ASSERT_EQL(Encoder->SuspendWithTimeoutRequested, false);
    __ASSERT_EQL(Encoder->Phase, RESOLVBUS_LIVEENCODERPHASE_IDLE);

    __WRAP(ResolVBus_LiveEncoder_Resume(Encoder));

    __ASSERT_EQL(Encoder->SuspendRequested, false);
    __ASSERT_EQL(Encoder->SuspendWithTimeoutRequested, false);
    __ASSERT_EQL(Encoder->Phase, RESOLVBUS_LIVEENCODERPHASE_IDLE);

    // Scenario 2: multiple calls to `ResolVBus_LiveEncoder_SuspendWithTimeout` while the encoder
    // is idle choose the longest provided timeout

    __WRAP(ResolVBus_LiveEncoder_QueuePacketFrame(Encoder, FourBytes));

    __ASSERT_EQL(Encoder->SuspendRequested, false);
    __ASSERT_EQL(Encoder->SuspendWithTimeoutRequested, false);
    __ASSERT_EQL(Encoder->Phase, RESOLVBUS_LIVEENCODERPHASE_IDLE);

    __WRAP(ResolVBus_LiveEncoder_SuspendWithTimeout(Encoder, 100));

    __ASSERT_EQL(Encoder->SuspendWithTimeoutRequested, true);
    __ASSERT_EQL(Encoder->SuspendTimeoutUs, 100);
    __ASSERT_EQL(Encoder->Phase, RESOLVBUS_LIVEENCODERPHASE_IDLE);

    __WRAP(ResolVBus_LiveEncoder_SuspendWithTimeout(Encoder, 200));

    __ASSERT_EQL(Encoder->SuspendWithTimeoutRequested, true);
    __ASSERT_EQL(Encoder->SuspendTimeoutUs, 200);
    __ASSERT_EQL(Encoder->Phase, RESOLVBUS_LIVEENCODERPHASE_IDLE);

    __WRAP(ResolVBus_LiveEncoder_SuspendWithTimeout(Encoder, 100));

    __ASSERT_EQL(Encoder->SuspendWithTimeoutRequested, true);
    __ASSERT_EQL(Encoder->SuspendTimeoutUs, 200);
    __ASSERT_EQL(Encoder->Phase, RESOLVBUS_LIVEENCODERPHASE_IDLE);

    __WRAP(ResolVBus_LiveEncoder_Resume(Encoder));

    __ASSERT_EQL(Encoder->SuspendRequested, false);
    __ASSERT_EQL(Encoder->SuspendWithTimeoutRequested, false);
    __ASSERT_EQL(Encoder->Phase, RESOLVBUS_LIVEENCODERPHASE_IDLE);

    __WRAP(ResolVBus_LiveEncoder_HandleTimer(Encoder, 10000));
    __WRAP(ResolVBus_LiveEncoder_HandleTimer(Encoder, 10000));

    // Scenario 3: calling `ResolVBus_LiveEncoder_SuspendWithTimeout` while the encoder is already
    // suspended with a timeout, applies the new timeout only if it is longer than the remaining
    // active timeout.

    __ASSERT_EQL(Encoder->SuspendRequested, false);
    __ASSERT_EQL(Encoder->SuspendWithTimeoutRequested, false);
    __ASSERT_EQL(Encoder->Phase, RESOLVBUS_LIVEENCODERPHASE_IDLE);

    __WRAP(ResolVBus_LiveEncoder_SuspendWithTimeout(Encoder, 100));

    __ASSERT_EQL(Encoder->SuspendWithTimeoutRequested, false);
    __ASSERT_EQL(Encoder->Phase, RESOLVBUS_LIVEENCODERPHASE_SUSPENDEDWITHTIMEOUT);
    __ASSERT_EQL(Encoder->PhaseTimeoutUs, 100);

    __WRAP(ResolVBus_LiveEncoder_HandleTimer(Encoder, 50));

    __ASSERT_EQL(Encoder->SuspendWithTimeoutRequested, false);
    __ASSERT_EQL(Encoder->Phase, RESOLVBUS_LIVEENCODERPHASE_SUSPENDEDWITHTIMEOUT);
    __ASSERT_EQL(Encoder->PhaseTimeoutUs, 50);

    __WRAP(ResolVBus_LiveEncoder_SuspendWithTimeout(Encoder, 100));

    __ASSERT_EQL(Encoder->SuspendWithTimeoutRequested, false);
    __ASSERT_EQL(Encoder->Phase, RESOLVBUS_LIVEENCODERPHASE_SUSPENDEDWITHTIMEOUT);
    __ASSERT_EQL(Encoder->PhaseTimeoutUs, 100);

    __WRAP(ResolVBus_LiveEncoder_SuspendWithTimeout(Encoder, 50));

    __ASSERT_EQL(Encoder->SuspendWithTimeoutRequested, false);
    __ASSERT_EQL(Encoder->Phase, RESOLVBUS_LIVEENCODERPHASE_SUSPENDEDWITHTIMEOUT);
    __ASSERT_EQL(Encoder->PhaseTimeoutUs, 100);

    // Scenario 4: calling `ResolVBus_LiveEncoder_SuspendWithTimeout` while the encoder is already
    // suspended without a timeout, returns an error

    __WRAP(ResolVBus_LiveEncoder_Resume(Encoder));
    __WRAP(ResolVBus_LiveEncoder_Suspend(Encoder));

    __ASSERT_EQL(Encoder->SuspendRequested, false);
    __ASSERT_EQL(Encoder->SuspendWithTimeoutRequested, false);
    __ASSERT_EQL(Encoder->Phase, RESOLVBUS_LIVEENCODERPHASE_SUSPENDED);

    __ASSERT_RESULT_EQL(SUSPENDED, NULL, ResolVBus_LiveEncoder_SuspendWithTimeout(Encoder, 100));

    __WRAP(ResolVBus_LiveEncoder_HandleTimer(Encoder, 0));

    __ASSERT_EQL(Encoder->SuspendRequested, false);
    __ASSERT_EQL(Encoder->SuspendWithTimeoutRequested, false);
    __ASSERT_EQL(Encoder->Phase, RESOLVBUS_LIVEENCODERPHASE_SUSPENDED);

    __ASSERT_RESULT_EQL(SUSPENDED, NULL, ResolVBus_LiveEncoder_SuspendWithTimeout(Encoder, 100));

    // Scenario 5: calling `ResolVBus_LiveEncoder_Suspend` while the encoder is already suspended
    // with a timeout removes that timeout

    __WRAP(ResolVBus_LiveEncoder_Resume(Encoder));
    __WRAP(ResolVBus_LiveEncoder_QueuePacketFrame(Encoder, FourBytes));
    __WRAP(ResolVBus_LiveEncoder_SuspendWithTimeout(Encoder, 100));

    __ASSERT_EQL(Encoder->SuspendRequested, false);
    __ASSERT_EQL(Encoder->SuspendWithTimeoutRequested, true);
    __ASSERT_EQL(Encoder->SuspendTimeoutUs, 100);
    __ASSERT_EQL(Encoder->Phase, RESOLVBUS_LIVEENCODERPHASE_IDLE);

    __WRAP(ResolVBus_LiveEncoder_Suspend(Encoder));

    __ASSERT_EQL(Encoder->SuspendRequested, true);
    __ASSERT_EQL(Encoder->SuspendWithTimeoutRequested, false);
    __ASSERT_EQL(Encoder->Phase, RESOLVBUS_LIVEENCODERPHASE_IDLE);

    __WRAP(ResolVBus_LiveEncoder_HandleTimer(Encoder, 10000));
    __WRAP(ResolVBus_LiveEncoder_HandleTimer(Encoder, 10000));
    __WRAP(ResolVBus_LiveEncoder_Resume(Encoder));

    __ASSERT_EQL(Encoder->Phase, RESOLVBUS_LIVEENCODERPHASE_IDLE);

    __WRAP(ResolVBus_LiveEncoder_SuspendWithTimeout(Encoder, 100));

    __ASSERT_EQL(Encoder->SuspendRequested, false);
    __ASSERT_EQL(Encoder->SuspendWithTimeoutRequested, false);
    __ASSERT_EQL(Encoder->Phase, RESOLVBUS_LIVEENCODERPHASE_SUSPENDEDWITHTIMEOUT);
    __ASSERT_EQL(Encoder->PhaseTimeoutUs, 100);

    __WRAP(ResolVBus_LiveEncoder_Suspend(Encoder));

    __ASSERT_EQL(Encoder->SuspendRequested, false);
    __ASSERT_EQL(Encoder->SuspendWithTimeoutRequested, false);
    __ASSERT_EQL(Encoder->Phase, RESOLVBUS_LIVEENCODERPHASE_SUSPENDED);

    return Result;
}


static RESOLVBUS_RESULT __SuspendCallback(RESOLVBUS_LIVEENCODER *Encoder, void *UntypedContext)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    if (UntypedContext) {
        __CALLBACKCONTEXT *Context = UntypedContext;

        if (Context->Encoder == Encoder) {
            __APPEND_LOG("__SuspendCallback(Encoder, Context)\n");
        } else {
            __APPEND_LOG("__SuspendCallback(Encoder, UnknownContext)\n");
        }
    } else {
        __APPEND_LOG("__SuspendCallback(Encoder, NULL)\n");
    }

    return Result;
}


static RESOLVBUS_RESULT __TestSuspendWithTimeoutAndCallback(void)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    __PREAMBLE();

    const uint8_t FourBytes [4] = { 0 };

    __CALLBACKCONTEXT Context = {
        .Encoder = Encoder,
    };

    // Scenario 1: calling the function while the encoder is idle suspends the encoder

    __ASSERT_EQL(Encoder->SuspendRequested, false);
    __ASSERT_EQL(Encoder->SuspendWithTimeoutRequested, false);
    __ASSERT_EQL(Encoder->Phase, RESOLVBUS_LIVEENCODERPHASE_IDLE);

    __WRAP(ResolVBus_LiveEncoder_SuspendWithTimeoutAndCallback(Encoder, 100, __SuspendCallback, &Context));

    __ASSERT_EQL(Encoder->SuspendRequested, false);
    __ASSERT_EQL(Encoder->SuspendWithTimeoutRequested, false);
    __ASSERT_EQL((size_t) Encoder->SuspendCallback, (size_t) __SuspendCallback);
    __ASSERT_EQL((size_t) Encoder->SuspendCallbackContext, (size_t) &Context);
    __ASSERT_EQL(Encoder->Phase, RESOLVBUS_LIVEENCODERPHASE_SUSPENDEDWITHTIMEOUT);
    __ASSERT_EQL(Encoder->PhaseTimeoutUs, 100);
    __ASSERT_LOG_EQL("");

    // Scenario 2: letting the timeout elapse calls the callback

    __WRAP(ResolVBus_LiveEncoder_HandleTimer(Encoder, 100));
    __ASSERT_LOG_EQL("__SuspendCallback(Encoder, Context)\nEvent = IDLE\n");

    // Scenario 3: like scenario 1, but with data in transmit buffer

    __WRAP(ResolVBus_LiveEncoder_QueuePacketFrame(Encoder, FourBytes));
    __WRAP(ResolVBus_LiveEncoder_SuspendWithTimeoutAndCallback(Encoder, 1000, __SuspendCallback, __HandlerLog));

    __ASSERT_EQL(Encoder->SuspendRequested, false);
    __ASSERT_EQL(Encoder->SuspendWithTimeoutRequested, true);
    __ASSERT_EQL(Encoder->SuspendTimeoutUs, 1000);
    __ASSERT_EQL((size_t) Encoder->SuspendCallback, (size_t) __SuspendCallback);
    __ASSERT_EQL((size_t) Encoder->SuspendCallbackContext, (size_t) __HandlerLog);
    __ASSERT_EQL(Encoder->Phase, RESOLVBUS_LIVEENCODERPHASE_IDLE);
    __ASSERT_LOG_EQL("");

    __WRAP(ResolVBus_LiveEncoder_HandleTimer(Encoder, 0));
    __WRAP(ResolVBus_LiveEncoder_HandleTimer(Encoder, 10000));

    __ASSERT_EQL(Encoder->Phase, RESOLVBUS_LIVEENCODERPHASE_SUSPENDEDWITHTIMEOUT);
    __ASSERT_EQL(Encoder->PhaseTimeoutUs, 250);
    __ASSERT_LOG_EQL("Event = TRANSMIT, Data = 0x00 0x00 0x00 0x00 0x00 0x7F\n");

    // Scenario 4: resuming timeout does not call callback

    __WRAP(ResolVBus_LiveEncoder_Resume(Encoder));

    __ASSERT_EQL(Encoder->Phase, RESOLVBUS_LIVEENCODERPHASE_IDLE);
    __ASSERT_LOG_EQL("");

    // Scenario 5: like scenario 1, but with NULL as context

    __WRAP(ResolVBus_LiveEncoder_SuspendWithTimeoutAndCallback(Encoder, 1000, __SuspendCallback, NULL));

    __ASSERT_EQL(Encoder->SuspendRequested, false);
    __ASSERT_EQL(Encoder->SuspendWithTimeoutRequested, false);
    __ASSERT_EQL((size_t) Encoder->SuspendCallback, (size_t) __SuspendCallback);
    __ASSERT_EQL((size_t) Encoder->SuspendCallbackContext, (size_t) NULL);
    __ASSERT_EQL(Encoder->Phase, RESOLVBUS_LIVEENCODERPHASE_SUSPENDEDWITHTIMEOUT);
    __ASSERT_EQL(Encoder->PhaseTimeoutUs, 1000);
    __ASSERT_LOG_EQL("");

    // Scenario 6: suspending again with a callback returns an error

    __ASSERT_RESULT_EQL(SUSPENDED, NULL, ResolVBus_LiveEncoder_SuspendWithTimeoutAndCallback(Encoder, 1000, __SuspendCallback, NULL));

    // Scenario 7: suspending again without a callback removes the callback

    __WRAP(ResolVBus_LiveEncoder_SuspendWithTimeout(Encoder, 1000));

    __ASSERT_EQL(Encoder->SuspendRequested, false);
    __ASSERT_EQL(Encoder->SuspendWithTimeoutRequested, false);
    __ASSERT_EQL((size_t) Encoder->SuspendCallback, (size_t) NULL);
    __ASSERT_EQL((size_t) Encoder->SuspendCallbackContext, (size_t) NULL);
    __ASSERT_EQL(Encoder->Phase, RESOLVBUS_LIVEENCODERPHASE_SUSPENDEDWITHTIMEOUT);
    __ASSERT_EQL(Encoder->PhaseTimeoutUs, 1000);
    __ASSERT_LOG_EQL("");

    return Result;
}


static RESOLVBUS_RESULT __IntegrationTest(void)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    __PREAMBLE();

    __WRAP(ResolVBus_LiveEncoder_HandleTimer(Encoder, 0));

    __ASSERT_LOG_EQL("Event = IDLE\n");
    __ASSERT_EQL(Encoder->BufferWriteIndex, 0);
    __ASSERT_EQL(Encoder->Phase, RESOLVBUS_LIVEENCODERPHASE_IDLE);

    uint32_t Timeout = 0;
    __WRAP(ResolVBus_LiveEncoder_GetTimeout(Encoder, &Timeout));

    __ASSERT_EQL(Timeout, 0);

    __WRAP(ResolVBus_LiveEncoder_QueuePacketHeader(Encoder, 0x0010, 0x7E11, 0x10, 0x0100, 10));

    __ASSERT_EQL(Encoder->BufferWriteIndex, 10);
    __ASSERT_EQL(Encoder->Phase, RESOLVBUS_LIVEENCODERPHASE_IDLE);

    const uint8_t FrameBytes [4] = { 0x11, 0x44, 0x77, 0xAA };

    __WRAP(ResolVBus_LiveEncoder_QueuePacketFrame(Encoder, FrameBytes));

    __ASSERT_EQL(Encoder->BufferReadIndex, 0);
    __ASSERT_EQL(Encoder->BufferWriteIndex, 16);
    __ASSERT_EQL(Encoder->Phase, RESOLVBUS_LIVEENCODERPHASE_IDLE);

    __WRAP(ResolVBus_LiveEncoder_QueuePacketFrame(Encoder, FrameBytes));

    __ASSERT_EQL(Encoder->BufferReadIndex, 0);
    __ASSERT_EQL(Encoder->BufferWriteIndex, 22);
    __ASSERT_EQL(Encoder->Phase, RESOLVBUS_LIVEENCODERPHASE_IDLE);

    __HandlerResult = RESOLVBUS_WOULDBLOCK;

    __WRAP(ResolVBus_LiveEncoder_HandleTimer(Encoder, 0));

    __ASSERT_LOG_EQL("Event = TRANSMIT, Data = 0xAA 0x10 0x00 0x11 0x7E 0x10 0x00 0x01 0x0A 0x45 0x11 0x44 0x77 0x2A 0x08 0x01 0x11 0x44 0x77 0x2A\n");
    __ASSERT_EQL(Encoder->BufferReadIndex, 0);
    __ASSERT_EQL(Encoder->BufferWriteIndex, 22);
    __ASSERT_EQL(Encoder->Phase, RESOLVBUS_LIVEENCODERPHASE_IDLE);

    __HandlerResult = RESOLVBUS_OK;

    __WRAP(ResolVBus_LiveEncoder_HandleTimer(Encoder, 0));

    __ASSERT_LOG_EQL("Event = TRANSMIT, Data = 0xAA 0x10 0x00 0x11 0x7E 0x10 0x00 0x01 0x0A 0x45 0x11 0x44 0x77 0x2A 0x08 0x01 0x11 0x44 0x77 0x2A\n");
    __ASSERT_EQL(Encoder->BufferReadIndex, 20);
    __ASSERT_EQL(Encoder->BufferWriteIndex, 22);
    __ASSERT_EQL(Encoder->Phase, RESOLVBUS_LIVEENCODERPHASE_TRANSMITTING);
    __ASSERT_EQL(Encoder->PhaseTimeoutUs, 20834);  // 20 bytes at @ 9600 bps 8N1

    __WRAP(ResolVBus_LiveEncoder_GetTimeout(Encoder, &Timeout));

    __ASSERT_EQL(Timeout, 20834);

    __WRAP(ResolVBus_LiveEncoder_HandleTimer(Encoder, 20000));

    __ASSERT_LOG_EQL("");
    __ASSERT_EQL(Encoder->BufferReadIndex, 20);
    __ASSERT_EQL(Encoder->BufferWriteIndex, 22);
    __ASSERT_EQL(Encoder->Phase, RESOLVBUS_LIVEENCODERPHASE_TRANSMITTING);
    __ASSERT_EQL(Encoder->PhaseTimeoutUs, 834);  // 20 bytes at @ 9600 bps 8N1

    __WRAP(ResolVBus_LiveEncoder_GetTimeout(Encoder, &Timeout));

    __ASSERT_EQL(Timeout, 834);

    __WRAP(ResolVBus_LiveEncoder_HandleTimer(Encoder, 1000));

    __ASSERT_LOG_EQL("");
    __ASSERT_EQL(Encoder->BufferReadIndex, 20);
    __ASSERT_EQL(Encoder->BufferWriteIndex, 22);
    __ASSERT_EQL(Encoder->Phase, RESOLVBUS_LIVEENCODERPHASE_GAININGENERGY);
    __ASSERT_EQL(Encoder->PhaseTimeoutUs, 10000 - 166);  // 10000 us regaining energy minus 166 us from previous phase

    __WRAP(ResolVBus_LiveEncoder_GetTimeout(Encoder, &Timeout));

    __ASSERT_EQL(Timeout, 9834);

    __WRAP(ResolVBus_LiveEncoder_HandleTimer(Encoder, 9834));

    __ASSERT_LOG_EQL("Event = TRANSMIT, Data = 0x08 0x01\n");
    __ASSERT_EQL(Encoder->BufferReadIndex, 22);
    __ASSERT_EQL(Encoder->BufferWriteIndex, 22);
    __ASSERT_EQL(Encoder->Phase, RESOLVBUS_LIVEENCODERPHASE_TRANSMITTING);
    __ASSERT_EQL(Encoder->PhaseTimeoutUs, 2084);  // 2 bytes at @ 9600 bps 8N1

    __WRAP(ResolVBus_LiveEncoder_HandleTimer(Encoder, 2000));

    __ASSERT_LOG_EQL("");
    __ASSERT_EQL(Encoder->BufferReadIndex, 22);
    __ASSERT_EQL(Encoder->BufferWriteIndex, 22);
    __ASSERT_EQL(Encoder->Phase, RESOLVBUS_LIVEENCODERPHASE_TRANSMITTING);
    __ASSERT_EQL(Encoder->PhaseTimeoutUs, 84);  // 2 bytes at @ 9600 bps 8N1

    __WRAP(ResolVBus_LiveEncoder_HandleTimer(Encoder, 100));

    __ASSERT_LOG_EQL("");
    __ASSERT_EQL(Encoder->BufferReadIndex, 0);
    __ASSERT_EQL(Encoder->BufferWriteIndex, 0);
    __ASSERT_EQL(Encoder->Phase, RESOLVBUS_LIVEENCODERPHASE_GAININGENERGY);
    __ASSERT_EQL(Encoder->PhaseTimeoutUs, 1000 - 16);  // 1000 us regaining energy minus 16 us from previous phase

    __WRAP(ResolVBus_LiveEncoder_HandleTimer(Encoder, 1000));

    __ASSERT_LOG_EQL("Event = IDLE\n");
    __ASSERT_EQL(Encoder->BufferReadIndex, 0);
    __ASSERT_EQL(Encoder->BufferWriteIndex, 0);
    __ASSERT_EQL(Encoder->Phase, RESOLVBUS_LIVEENCODERPHASE_IDLE);

    __WRAP(ResolVBus_LiveEncoder_QueuePacketHeader(Encoder, 0x0010, 0x7E11, 0x10, 0x0100, 10));

    __ASSERT_EQL(Encoder->BufferReadIndex, 0);
    __ASSERT_EQL(Encoder->BufferWriteIndex, 10);
    __ASSERT_EQL(Encoder->SuspendRequested, false);
    __ASSERT_EQL(Encoder->Phase, RESOLVBUS_LIVEENCODERPHASE_IDLE);

    __WRAP(ResolVBus_LiveEncoder_Suspend(Encoder));

    __ASSERT_EQL(Encoder->BufferReadIndex, 0);
    __ASSERT_EQL(Encoder->BufferWriteIndex, 10);
    __ASSERT_EQL(Encoder->SuspendRequested, true);
    __ASSERT_EQL(Encoder->Phase, RESOLVBUS_LIVEENCODERPHASE_IDLE);

    __WRAP(ResolVBus_LiveEncoder_HandleTimer(Encoder, 0));

    __ASSERT_LOG_EQL("Event = TRANSMIT, Data = 0xAA 0x10 0x00 0x11 0x7E 0x10 0x00 0x01 0x0A 0x45\n");
    __ASSERT_EQL(Encoder->BufferReadIndex, 10);
    __ASSERT_EQL(Encoder->BufferWriteIndex, 10);
    __ASSERT_EQL(Encoder->SuspendRequested, true);
    __ASSERT_EQL(Encoder->Phase, RESOLVBUS_LIVEENCODERPHASE_TRANSMITTING);
    __ASSERT_EQL(Encoder->PhaseTimeoutUs, 10417);

    __WRAP(ResolVBus_LiveEncoder_HandleTimer(Encoder, 10417));

    __ASSERT_LOG_EQL("");
    __ASSERT_EQL(Encoder->BufferReadIndex, 0);
    __ASSERT_EQL(Encoder->BufferWriteIndex, 0);
    __ASSERT_EQL(Encoder->SuspendRequested, true);
    __ASSERT_EQL(Encoder->Phase, RESOLVBUS_LIVEENCODERPHASE_GAININGENERGY);
    __ASSERT_EQL(Encoder->PhaseTimeoutUs, 5000);

    __WRAP(ResolVBus_LiveEncoder_HandleTimer(Encoder, 5000));

    __ASSERT_LOG_EQL("");
    __ASSERT_EQL(Encoder->BufferReadIndex, 0);
    __ASSERT_EQL(Encoder->BufferWriteIndex, 0);
    __ASSERT_EQL(Encoder->SuspendRequested, false);
    __ASSERT_EQL(Encoder->SuspendWithTimeoutRequested, false);
    __ASSERT_EQL(Encoder->Phase, RESOLVBUS_LIVEENCODERPHASE_SUSPENDED);
    __ASSERT_EQL(Encoder->PhaseTimeoutUs, 0);

    __WRAP(ResolVBus_LiveEncoder_GetTimeout(Encoder, &Timeout));

    __ASSERT_EQL(Timeout, UINT32_MAX);

    __WRAP(ResolVBus_LiveEncoder_HandleTimer(Encoder, 5000));

    __ASSERT_LOG_EQL("");
    __ASSERT_EQL(Encoder->BufferReadIndex, 0);
    __ASSERT_EQL(Encoder->BufferWriteIndex, 0);
    __ASSERT_EQL(Encoder->SuspendRequested, false);
    __ASSERT_EQL(Encoder->SuspendWithTimeoutRequested, false);
    __ASSERT_EQL(Encoder->Phase, RESOLVBUS_LIVEENCODERPHASE_SUSPENDED);
    __ASSERT_EQL(Encoder->PhaseTimeoutUs, 0);

    __WRAP(ResolVBus_LiveEncoder_GetTimeout(Encoder, &Timeout));

    __ASSERT_EQL(Timeout, UINT32_MAX);

    __WRAP(ResolVBus_LiveEncoder_Resume(Encoder));

    __ASSERT_EQL(Encoder->BufferReadIndex, 0);
    __ASSERT_EQL(Encoder->BufferWriteIndex, 0);
    __ASSERT_EQL(Encoder->SuspendRequested, false);
    __ASSERT_EQL(Encoder->SuspendWithTimeoutRequested, false);
    __ASSERT_EQL(Encoder->Phase, RESOLVBUS_LIVEENCODERPHASE_IDLE);

    __WRAP(ResolVBus_LiveEncoder_SuspendWithTimeout(Encoder, 200));

    __ASSERT_LOG_EQL("");
    __ASSERT_EQL(Encoder->BufferReadIndex, 0);
    __ASSERT_EQL(Encoder->BufferWriteIndex, 0);
    __ASSERT_EQL(Encoder->SuspendWithTimeoutRequested, false);
    __ASSERT_EQL(Encoder->SuspendTimeoutUs, 0);
    __ASSERT_EQL(Encoder->Phase, RESOLVBUS_LIVEENCODERPHASE_SUSPENDEDWITHTIMEOUT);
    __ASSERT_EQL(Encoder->PhaseTimeoutUs, 200);

    __WRAP(ResolVBus_LiveEncoder_GetTimeout(Encoder, &Timeout));

    __ASSERT_EQL(Timeout, 200);

    __WRAP(ResolVBus_LiveEncoder_HandleTimer(Encoder, 200));

    __ASSERT_LOG_EQL("Event = IDLE\n");
    __ASSERT_EQL(Encoder->Phase, RESOLVBUS_LIVEENCODERPHASE_IDLE);

    // === Test without energy management ===
    Encoder->MaxEnergy = 0;

    __WRAP(ResolVBus_LiveEncoder_QueuePacketHeader(Encoder, 0x0010, 0x7E11, 0x10, 0x0100, 10));

    __WRAP(ResolVBus_LiveEncoder_HandleTimer(Encoder, 1000));

    __ASSERT_LOG_EQL("Event = TRANSMIT, Data = 0xAA 0x10 0x00 0x11 0x7E 0x10 0x00 0x01 0x0A 0x45\n");
    __ASSERT_EQL(Encoder->Phase, RESOLVBUS_LIVEENCODERPHASE_TRANSMITTING);
    __ASSERT_EQL(Encoder->PhaseTimeoutUs, 10417);

    __WRAP(ResolVBus_LiveEncoder_HandleTimer(Encoder, 10417));

    __ASSERT_LOG_EQL("Event = IDLE\n");
    __ASSERT_EQL(Encoder->Phase, RESOLVBUS_LIVEENCODERPHASE_IDLE);

    return Result;
}



//---------------------------------------------------------------------------
// PUBLIC METHODS
//---------------------------------------------------------------------------

RESOLVBUS_RESULT RunTestSuite_LiveEncoder(void)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    __WRAP(__TestInitialize());
    __WRAP(__TestHandleTimer());
    __WRAP(__TestQueueXxxWhileSuspended());
    __WRAP(__TestQueuePacketFrames());
    __WRAP(__TestQueueTelegramXxx());
    __WRAP(__TestSuspendAndResume());
    __WRAP(__TestSuspendWithTimeoutAndCallback());
    __WRAP(__IntegrationTest());

    return Result;
}

