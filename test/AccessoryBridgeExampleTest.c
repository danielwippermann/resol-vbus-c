//
//  AccessoryBridgeExampleTest.c
//  resol-vbus-c
//
//
//  Copyright (C) 2024, Daniel Wippermann. All rights reserved.
//

#define __IGNORE_MAIN_FOR_TESTING


#include "../examples/AccessoryBridge.c"


#include "Testing.h"



//---------------------------------------------------------------------------
// PRIVATE DEFINES
//---------------------------------------------------------------------------

#define __APPEND_LOG(...) \
    if (Result == RESOLVBUS_OK) { \
        AppendToLog(__HandlerLog, sizeof (__HandlerLog), __VA_ARGS__); \
    } \


#define __RESET_LOG() \
    if (Result == RESOLVBUS_OK) { \
        __HandlerLog [0] = 0; \
    } \


#define __ASSERT_LOG_EQL(__Expected__) \
    __WRAP_SHORT(AssertStringEql(__Expected__, "Expected", __HandlerLog, "__HandlerLog")); \
    __RESET_LOG(); \


#define __ASSERT_TIMEOUT_EQL(__Expected__) \
    __WRAP_SHORT(__AssertTimeoutEql(Bridge, __Expected__)); \


#define __ASSERT_VBUSP_PHASE_EQL(__PhaseSuffix__) \
    __ASSERT_EQL(Bridge->VBusPPhase, __PHASE_##__PhaseSuffix__); \



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

static char __HandlerLog [4096];

static const uint8_t __ControllerPacket1 [] = {
    0xaa, 0x10, 0x00, 0x00, 0x10, 0x10, 0x00, 0x01, 0x00, 0x4e,
};

static const uint8_t __AccessoryPacket1 [] = {
	0xaa, 0x10, 0x00, 0x00, 0x40, 0x10, 0x00, 0x01, 0x00, 0x1e,
};

static const uint8_t __ControllerBusOfferDatagram [] = {
    0xaa, 0x00, 0x00, 0x00, 0x10, 0x20, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4a,
};

static const uint8_t __ControllerChangesetIdDatagram [] = {
	0xaa, 0x20, 0x00, 0x00, 0x10, 0x20, 0x00, 0x01, 0x00, 0x00, 0x78, 0x56, 0x34, 0x12, 0x00, 0x1a,
};

static const uint8_t __PvsPacket1TooShort [] = {
	0xaa, 0x20, 0x00, 0x16, 0x11, 0x10, 0x10, 0x01, 0x00, 0x17,
};

static const uint8_t __PvsPacket1WrongVersion [] = {
	0xaa, 0x20, 0x00, 0x16, 0x11, 0x10, 0x10, 0x01, 0x01, 0x16, 0x01, 0x00, 0x00, 0x00, 0x00, 0x7e,
};

static const uint8_t __PvsPacket1 [] = {
	0xaa, 0x20, 0x00, 0x16, 0x11, 0x10, 0x10, 0x01, 0x01, 0x16, 0x02, 0x00, 0x00, 0x00, 0x00, 0x7d,
};

static const uint8_t __PvsPacket2 [] = {
	0xaa, 0x20, 0x00, 0x16, 0x11, 0x10, 0x20, 0x01, 0x00, 0x07, 0x02, 0x01, 0x00, 0x00, 0x00, 0x7c,
};

static const uint8_t __PvsPacket3TooShort [] = {
	0xaa, 0x20, 0x00, 0x16, 0x11, 0x10, 0x30, 0x01, 0x00, 0x77,
};

static const uint8_t __PvsPacket3 [] = {
	0xaa, 0x20, 0x00, 0x16, 0x11, 0x10, 0x30, 0x01, 0x02, 0x75, 0x02, 0x01, 0x00, 0x00, 0x00, 0x7c,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x7f,
};

static const uint8_t __UnrelatedDatagram1 [] = {
    0xaa, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5f,
};

static const uint8_t __UnrelatedPacket1 [] = {
	0xaa, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x6f,
};

static const uint8_t __UnrelatedPacket2 [] = {
	0xaa, 0x20, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x4f,
};

static const uint8_t __UnrelatedPacket3 [] = {
	0xaa, 0x20, 0x00, 0x16, 0x11, 0x10, 0x00, 0x00, 0x00, 0x28,
};



//---------------------------------------------------------------------------
// NON-PUBLIC METHODS
//---------------------------------------------------------------------------

static RESOLVBUS_RESULT __AssertTimeoutEql(__ACCESSORYBRIDGE *Bridge, uint32_t Expected)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    uint32_t TimeoutUs;
    __WRAP(__GetTimeout(Bridge, &TimeoutUs));

    __ASSERT_EQL(TimeoutUs, Expected);

    return Result;
}


static RESOLVBUS_RESULT __TestableVBusPLiveTransceiverHandler(RESOLVBUS_LIVETRANSCEIVER *Transceiver, const RESOLVBUS_LIVETRANSCEIVEREVENT *Event)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    if (Result == RESOLVBUS_OK) {
        if (Event->EventType == RESOLVBUS_LIVETRANSCEIVEREVENTTYPE_DECODER) {
            const RESOLVBUS_LIVEDECODEREVENT *DecoderEvent = Event->DecoderEvent;

            if (DecoderEvent->EventType == RESOLVBUS_LIVEDECODEREVENTTYPE_PACKETHEADER) {
                __APPEND_LOG("PACKETHEADER@Dec(VBusP) %04X_%04X_%02X_%04X_%02X\n",
                    DecoderEvent->DestinationAddress,
                    DecoderEvent->SourceAddress,
                    DecoderEvent->ProtocolVersion,
                    DecoderEvent->Command,
                    DecoderEvent->FrameCount);
            } else if (DecoderEvent->EventType == RESOLVBUS_LIVEDECODEREVENTTYPE_PACKETFRAME) {
                __APPEND_LOG("PACKETFRAME@Dec(VBusP) %04X_%04X_%02X_%04X_%02X\n",
                    DecoderEvent->DestinationAddress,
                    DecoderEvent->SourceAddress,
                    DecoderEvent->ProtocolVersion,
                    DecoderEvent->Command,
                    DecoderEvent->FrameIndex);
            } else if (DecoderEvent->EventType == RESOLVBUS_LIVEDECODEREVENTTYPE_PACKETEND) {
                __APPEND_LOG("PACKETEND@Dec(VBusP) %04X_%04X_%02X_%04X_%02X\n",
                    DecoderEvent->DestinationAddress,
                    DecoderEvent->SourceAddress,
                    DecoderEvent->ProtocolVersion,
                    DecoderEvent->Command,
                    DecoderEvent->FrameCount);
            } else if (DecoderEvent->EventType == RESOLVBUS_LIVEDECODEREVENTTYPE_DATAGRAM) {
                __APPEND_LOG("DATAGRAM@Dec(VBusP) %04X_%04X_%02X_%04X_%04X_%08X\n",
                    DecoderEvent->DestinationAddress,
                    DecoderEvent->SourceAddress,
                    DecoderEvent->ProtocolVersion,
                    DecoderEvent->Command,
                    DecoderEvent->Param16,
                    DecoderEvent->Param32);
            } else {
                __APPEND_LOG("?@Dec(VBusP) %d\n", DecoderEvent->EventType);
            }
        } else if (Event->EventType == RESOLVBUS_LIVETRANSCEIVEREVENTTYPE_ENCODER) {
            const RESOLVBUS_LIVEENCODEREVENT *EncoderEvent = Event->EncoderEvent;

            if (EncoderEvent->EventType == RESOLVBUS_LIVEENCODEREVENTTYPE_TRANSMIT) {
                __APPEND_LOG("TRANSMIT@Enc(VBusP)\n");
                for (size_t i = 0; i < EncoderEvent->TransmitLength; i++) {
                    if ((i & 15) == 0) {
                        if (i > 0) {
                            __APPEND_LOG("\n");
                        }
                        __APPEND_LOG("    ");
                    } else {
                        __APPEND_LOG(" ");
                    }
                    __APPEND_LOG("0x%02X,", ((const uint8_t *) EncoderEvent->TransmitBytes) [i]);
                }
                __APPEND_LOG("\n");
            } else {
                __APPEND_LOG("?@Enc(VBusP) %d\n", EncoderEvent->EventType);
            }
        } else {
            __APPEND_LOG("?@VBusP %d\n", Event->EventType);
        }
    }

    __WRAP(__VBusPLiveTransceiverHandler(Transceiver, Event));

    return Result;
}


static RESOLVBUS_RESULT __TestableVBusVLiveEncoderHandler(RESOLVBUS_LIVEENCODER *Encoder, const RESOLVBUS_LIVEENCODEREVENT *Event)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    if (Event->EventType == RESOLVBUS_LIVEENCODEREVENTTYPE_TRANSMIT) {
        __APPEND_LOG("TRANSMIT@Enc(VBusV)\n");
        for (size_t i = 0; i < Event->TransmitLength; i++) {
            if ((i & 15) == 0) {
                if (i > 0) {
                    __APPEND_LOG("\n");
                }
                __APPEND_LOG("    ");
            } else {
                __APPEND_LOG(" ");
            }
            __APPEND_LOG("0x%02X,", ((const uint8_t *) Event->TransmitBytes) [i]);
        }
        __APPEND_LOG("\n");
    } else {
        __APPEND_LOG("?@Enc(VBusV) %d\n", Event->EventType);
    }

    __WRAP(__VBusVLiveEncoderHandler(Encoder, Event));

    return Result;
}


static RESOLVBUS_RESULT __SetBackToPhaseWaitForFreeBus(__ACCESSORYBRIDGE *Bridge)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    switch (Bridge->VBusPPhase) {
    case __PHASE_WAITFORFREEBUS:
        // nop
        break;
    case __PHASE_GETCHANGESETID:
        __WRAP(__HandleLoopCycle(Bridge, 400000, NULL, 0));
        break;
    case __PHASE_SENDPACKET1:
    case __PHASE_SENDPACKET2:
    case __PHASE_SENDPACKET3:
        __WRAP(__HandleLoopCycle(Bridge, 400000, NULL, 0));
        __ASSERT_LOG_EQL(
            "TRANSMIT@Enc(VBusP)\n"
            "    0xAA, 0x00, 0x10, 0x20, 0x00, 0x20, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x29,\n"
        );
        // NOTE(daniel): fall-through
    case __PHASE_RELEASEBUS:
        __WRAP(__HandleLoopCycle(Bridge, 400000, NULL, 0));
        __WRAP(__HandleLoopCycle(Bridge, 0, __ControllerPacket1, sizeof (__ControllerPacket1)));
        __ASSERT_LOG_EQL(
            "PACKETHEADER@Dec(VBusP) 0010_1000_10_0100_00\n"
            "PACKETEND@Dec(VBusP) 0010_1000_10_0100_00\n"
        );
        break;
    default:
        __DLOG("Phase = %d", Bridge->VBusPPhase);
        __FAIL(NYI);
        break;
    }

    __ASSERT_VBUSP_PHASE_EQL(WAITFORFREEBUS);
    __ASSERT_TIMEOUT_EQL(20000000);
    __ASSERT_LOG_EQL("");
    __ASSERT_EQL(Bridge->VBusPPeerAddress, 0);

    return Result;
}


static RESOLVBUS_RESULT __ProgressToPhase(__ACCESSORYBRIDGE *Bridge, __PHASE ToPhase)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    if (Bridge->VBusPPhase == __PHASE_RELEASEBUS) {
        uint32_t TimeoutUs = 0;
        __WRAP(__GetTimeout(Bridge, &TimeoutUs));

        if ((Result == RESOLVBUS_OK) && (TimeoutUs > 20000)) {
            TimeoutUs = 20000;
        }

        __WRAP(__HandleLoopCycle(Bridge, TimeoutUs, NULL, 0));

        if (Bridge->VBusPPhase == __PHASE_RELEASEBUS) {
            __WRAP(__HandleLoopCycle(Bridge, 0, __ControllerPacket1, sizeof (__ControllerPacket1)));
        }

        __RESET_LOG();
    }

    __ASSERT_VBUSP_PHASE_EQL(WAITFORFREEBUS);
    __ASSERT_TIMEOUT_EQL(20000000);
    __ASSERT_EQL(Bridge->VBusPPeerAddress, 0);

    if (Result == RESOLVBUS_OK) {
        Bridge->VBusPPvsSeqNr = 0;
        Bridge->VBusPPvsInitialized = false;
    }

    if (ToPhase >= __PHASE_WAITFORANYDATA) {
        __WRAP(__HandleLoopCycle(Bridge, 0, __ControllerBusOfferDatagram, sizeof (__ControllerBusOfferDatagram)));

        __ASSERT_VBUSP_PHASE_EQL(WAITFORANYDATA);
        __ASSERT_TIMEOUT_EQL(100000);
        __ASSERT_LOG_EQL(
            "DATAGRAM@Dec(VBusP) 0000_1000_20_0500_0000_00000000\n"
        );
        __ASSERT_EQL(Bridge->VBusPPeerAddress, 0x1000);
    }

    if (ToPhase >= __PHASE_GETCHANGESETID) {
        __WRAP(__HandleLoopCycle(Bridge, 100000, NULL, 0));

        __ASSERT_VBUSP_PHASE_EQL(GETCHANGESETID);
        __ASSERT_TIMEOUT_EQL(16667);
        __ASSERT_LOG_EQL(
            "TRANSMIT@Enc(VBusP)\n"
            "    0xAA, 0x00, 0x10, 0x20, 0x00, 0x20, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2C,\n"
        );
        __ASSERT_EQL(Bridge->VBusPPeerAddress, 0x1000);

        __WRAP(__HandleLoopCycle(Bridge, 16667, NULL, 0));

        __ASSERT_VBUSP_PHASE_EQL(GETCHANGESETID);
        __ASSERT_TIMEOUT_EQL(83333);  // 100000 - 16667
        __ASSERT_LOG_EQL("");
    }

    if (ToPhase >= __PHASE_SENDPACKET1) {
        __WRAP(__HandleLoopCycle(Bridge, 0, __ControllerChangesetIdDatagram, sizeof(__ControllerChangesetIdDatagram)));

        __ASSERT_VBUSP_PHASE_EQL(SENDPACKET1);
        __ASSERT_TIMEOUT_EQL(10417);
        __ASSERT_LOG_EQL(
            "DATAGRAM@Dec(VBusP) 0020_1000_20_0100_0000_12345678\n"
            "TRANSMIT@Enc(VBusP)\n"
            "    0xAA, 0x16, 0x11, 0x20, 0x00, 0x10, 0x10, 0x03, 0x00, 0x15,\n"
        );
        __ASSERT_EQL(Bridge->VBusPPeerAddress, 0x1000);

        __WRAP(__HandleLoopCycle(Bridge, 10417, NULL, 0));

        __ASSERT_VBUSP_PHASE_EQL(SENDPACKET1);
        __ASSERT_TIMEOUT_EQL(389583);
        __ASSERT_LOG_EQL("");
        __ASSERT_EQL(Bridge->VBusPPeerAddress, 0x1000);
    }

    if (ToPhase >= __PHASE_SENDPACKET2) {
        __WRAP(__HandleLoopCycle(Bridge, 0, __PvsPacket1, sizeof(__PvsPacket1)));

        __ASSERT_VBUSP_PHASE_EQL(SENDPACKET2);
        __ASSERT_TIMEOUT_EQL(29167);
        __ASSERT_LOG_EQL(
            "PACKETHEADER@Dec(VBusP) 0020_1116_10_0110_01\n"
            "PACKETFRAME@Dec(VBusP) 0020_1116_10_0110_00\n"
            "PACKETEND@Dec(VBusP) 0020_1116_10_0110_01\n"
            "TRANSMIT@Enc(VBusP)\n"
            "    0xAA, 0x16, 0x11, 0x20, 0x00, 0x10, 0x20, 0x02, 0x03, 0x03, 0x02, 0x01, 0x00, 0x00, 0x00, 0x7C,\n"
            "    0x64, 0x00, 0x1E, 0x00, 0x00, 0x7D, 0x64, 0x00, 0x1E, 0x00, 0x00, 0x7D,\n"
        );
        __ASSERT_EQL(Bridge->VBusPPeerAddress, 0x1000);

        __WRAP(__HandleLoopCycle(Bridge, 54167, NULL, 0));

        __ASSERT_VBUSP_PHASE_EQL(SENDPACKET2);
        __ASSERT_TIMEOUT_EQL(345833);
        __ASSERT_LOG_EQL("");
        __ASSERT_EQL(Bridge->VBusPPeerAddress, 0x1000);
    }

    if (ToPhase >= __PHASE_SENDPACKET3) {
        __WRAP(__HandleLoopCycle(Bridge, 0, __PvsPacket2, sizeof(__PvsPacket2)));

        __ASSERT_VBUSP_PHASE_EQL(SENDPACKET3);
        __ASSERT_TIMEOUT_EQL(54167);
        __ASSERT_LOG_EQL(
            "PACKETHEADER@Dec(VBusP) 0020_1116_10_0120_00\n"
            "PACKETEND@Dec(VBusP) 0020_1116_10_0120_00\n"
            "TRANSMIT@Enc(VBusP)\n"
            "    0xAA, 0x16, 0x11, 0x20, 0x00, 0x10, 0x30, 0x02, 0x07, 0x6F, 0x02, 0x01, 0x00, 0x00, 0x00, 0x7C,\n"
            "    0x02, 0x02, 0x00, 0x00, 0x00, 0x7B, 0x02, 0x02, 0x00, 0x00, 0x00, 0x7B, 0x02, 0x02, 0x00, 0x00,\n"
            "    0x00, 0x7B, 0x02, 0x02, 0x00, 0x00, 0x00, 0x7B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7F, 0x00, 0x00,\n"
            "    0x00, 0x00, 0x00, 0x7F,\n"
        );
        __ASSERT_EQL(Bridge->VBusPPeerAddress, 0x1000);

        __WRAP(__HandleLoopCycle(Bridge, 54167, NULL, 0));

        __ASSERT_VBUSP_PHASE_EQL(SENDPACKET3);
        __ASSERT_TIMEOUT_EQL(345833);
        __ASSERT_LOG_EQL("");
        __ASSERT_EQL(Bridge->VBusPPeerAddress, 0x1000);
    }

    if (ToPhase >= __PHASE_RELEASEBUS) {
        __WRAP(__HandleLoopCycle(Bridge, 0, __PvsPacket3, sizeof(__PvsPacket3)));

        __ASSERT_VBUSP_PHASE_EQL(RELEASEBUS);
        __ASSERT_TIMEOUT_EQL(16667);
        __ASSERT_LOG_EQL(
            "PACKETHEADER@Dec(VBusP) 0020_1116_10_0130_02\n"
            "PACKETFRAME@Dec(VBusP) 0020_1116_10_0130_00\n"
            "PACKETFRAME@Dec(VBusP) 0020_1116_10_0130_01\n"
            "PACKETEND@Dec(VBusP) 0020_1116_10_0130_02\n"
            "TRANSMIT@Enc(VBusP)\n"
            "    0xAA, 0x00, 0x10, 0x20, 0x00, 0x20, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x29,\n"
            "TRANSMIT@Enc(VBusV)\n"
            "    0xAA, 0x10, 0x00, 0x2E, 0x11, 0x10, 0x01, 0x01, 0x02, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7F,\n"
            "    0x00, 0x00, 0x00, 0x00, 0x00, 0x7F,\n"
        );
        __ASSERT_EQL(Bridge->VBusPPeerAddress, 0x1000);

        __WRAP(__HandleLoopCycle(Bridge, 16667, NULL, 0));

        __ASSERT_VBUSP_PHASE_EQL(RELEASEBUS);
        __ASSERT_TIMEOUT_EQL(6250);
        __ASSERT_LOG_EQL("");
        __ASSERT_EQL(Bridge->VBusPPeerAddress, 0x1000);

        __WRAP(__HandleLoopCycle(Bridge, 6250, NULL, 0));

        __ASSERT_VBUSP_PHASE_EQL(RELEASEBUS);
        __ASSERT_TIMEOUT_EQL(1477083);
        __ASSERT_LOG_EQL("");
        __ASSERT_EQL(Bridge->VBusPPeerAddress, 0x1000);
    }

    return Result;
}


// static RESOLVBUS_RESULT __VBusPLiveTransceiverActionHandler(RESOLVBUS_LIVETRANSCEIVER *Transceiver, const RESOLVBUS_LIVETRANSCEIVEREVENT *Event)
static RESOLVBUS_RESULT __TestVBusPLiveTransceiverActionHandler(__ACCESSORYBRIDGE *Bridge)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    // ========= Phase WAITFORFREEBUS =========

    // --------- Timeout ---------
    __ASSERT_VBUSP_PHASE_EQL(WAITFORFREEBUS);
    __ASSERT_TIMEOUT_EQL(20000000);
    __ASSERT_EQL(Bridge->VBusPPeerAddress, 0);

    __WRAP(__HandleLoopCycle(Bridge, 20000000, NULL, 0));

    __ASSERT_VBUSP_PHASE_EQL(WAITFORFREEBUS);
    __ASSERT_TIMEOUT_EQL(20000000);
    __ASSERT_LOG_EQL("");
    __ASSERT_EQL(Bridge->VBusPPeerAddress, 0);

    // --------- Datagram ---------
    __WRAP(__ProgressToPhase(Bridge, __PHASE_WAITFORFREEBUS));

    __ASSERT_VBUSP_PHASE_EQL(WAITFORFREEBUS);
    __ASSERT_TIMEOUT_EQL(20000000);
    __ASSERT_LOG_EQL("");
    __ASSERT_EQL(Bridge->VBusPPeerAddress, 0);

    __WRAP(__HandleLoopCycle(Bridge, 0, __ControllerBusOfferDatagram, sizeof (__ControllerBusOfferDatagram)));

    __ASSERT_VBUSP_PHASE_EQL(WAITFORANYDATA);
    __ASSERT_TIMEOUT_EQL(100000);
    __ASSERT_LOG_EQL(
        "DATAGRAM@Dec(VBusP) 0000_1000_20_0500_0000_00000000\n"
    );
    __ASSERT_EQL(Bridge->VBusPPeerAddress, 0x1000);

    // ========= Phase WAITFORANYDATA =========

    // --------- Packet (as an example for any data) ---------
    __ASSERT_VBUSP_PHASE_EQL(WAITFORANYDATA);
    __ASSERT_TIMEOUT_EQL(100000);
    __ASSERT_EQL(Bridge->VBusPPeerAddress, 0x1000);

    __WRAP(__HandleLoopCycle(Bridge, 0, __AccessoryPacket1, sizeof (__AccessoryPacket1)));

    __ASSERT_VBUSP_PHASE_EQL(WAITFORFREEBUS);
    __ASSERT_TIMEOUT_EQL(20000000);
    __ASSERT_LOG_EQL(
        "PACKETHEADER@Dec(VBusP) 0010_4000_10_0100_00\n"
        "PACKETEND@Dec(VBusP) 0010_4000_10_0100_00\n"
    );
    __ASSERT_EQL(Bridge->VBusPPeerAddress, 0);

    // --------- Timeout ---------
    __WRAP(__ProgressToPhase(Bridge, __PHASE_WAITFORANYDATA));

    __ASSERT_VBUSP_PHASE_EQL(WAITFORANYDATA);
    __ASSERT_TIMEOUT_EQL(100000);
    __ASSERT_LOG_EQL("");
    __ASSERT_EQL(Bridge->VBusPPeerAddress, 0x1000);

    __WRAP(__HandleLoopCycle(Bridge, 100000, NULL, 0));

    __ASSERT_VBUSP_PHASE_EQL(GETCHANGESETID);
    __ASSERT_TIMEOUT_EQL(16667);
    __ASSERT_LOG_EQL(
        "TRANSMIT@Enc(VBusP)\n"
        "    0xAA, 0x00, 0x10, 0x20, 0x00, 0x20, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2C,\n"
    );
    __ASSERT_EQL(Bridge->VBusPPeerAddress, 0x1000);

    // ========= Phase GETCHANGESETID =========

    // --------- Timeout ---------
    __ASSERT_VBUSP_PHASE_EQL(GETCHANGESETID);
    __ASSERT_TIMEOUT_EQL(16667);

    __WRAP(__HandleLoopCycle(Bridge, 16667, NULL, 0));

    __ASSERT_VBUSP_PHASE_EQL(GETCHANGESETID);
    __ASSERT_TIMEOUT_EQL(83333);  // 100000 - 16667
    __ASSERT_LOG_EQL("");

    __WRAP(__HandleLoopCycle(Bridge, 83333, NULL, 0));

    __ASSERT_VBUSP_PHASE_EQL(WAITFORFREEBUS);
    __ASSERT_TIMEOUT_EQL(20000000);
    __ASSERT_LOG_EQL("");
    __ASSERT_EQL(Bridge->VBusPPeerAddress, 0);

    // --------- Datagram (previously initialized) ---------
    __WRAP(__ProgressToPhase(Bridge, __PHASE_GETCHANGESETID));

    if (Result == RESOLVBUS_OK) {
        Bridge->VBusPPvsInitialized = true;
    }

    __ASSERT_VBUSP_PHASE_EQL(GETCHANGESETID);
    __ASSERT_TIMEOUT_EQL(83333);  // 100000 - 16667
    __ASSERT_LOG_EQL("");

    __WRAP(__HandleLoopCycle(Bridge, 0, __ControllerChangesetIdDatagram, sizeof(__ControllerChangesetIdDatagram)));

    __ASSERT_VBUSP_PHASE_EQL(SENDPACKET3);
    __ASSERT_TIMEOUT_EQL(54167);
    __ASSERT_LOG_EQL(
        "DATAGRAM@Dec(VBusP) 0020_1000_20_0100_0000_12345678\n"
        "TRANSMIT@Enc(VBusP)\n"
        "    0xAA, 0x16, 0x11, 0x20, 0x00, 0x10, 0x30, 0x02, 0x07, 0x6F, 0x02, 0x00, 0x00, 0x00, 0x00, 0x7D,\n"
        "    0x02, 0x02, 0x00, 0x00, 0x00, 0x7B, 0x02, 0x02, 0x00, 0x00, 0x00, 0x7B, 0x02, 0x02, 0x00, 0x00,\n"
        "    0x00, 0x7B, 0x02, 0x02, 0x00, 0x00, 0x00, 0x7B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7F, 0x00, 0x00,\n"
        "    0x00, 0x00, 0x00, 0x7F,\n"
    );
    __ASSERT_EQL(Bridge->VBusPPeerAddress, 0x1000);

    // --------- Datagram ---------
    __WRAP(__SetBackToPhaseWaitForFreeBus(Bridge));

    __WRAP(__ProgressToPhase(Bridge, __PHASE_GETCHANGESETID));

    __ASSERT_VBUSP_PHASE_EQL(GETCHANGESETID);
    __ASSERT_TIMEOUT_EQL(83333);  // 100000 - 16667
    __ASSERT_LOG_EQL("");

    __WRAP(__HandleLoopCycle(Bridge, 0, __ControllerChangesetIdDatagram, sizeof(__ControllerChangesetIdDatagram)));

    __ASSERT_VBUSP_PHASE_EQL(SENDPACKET1);
    __ASSERT_TIMEOUT_EQL(10417);
    __ASSERT_LOG_EQL(
        "DATAGRAM@Dec(VBusP) 0020_1000_20_0100_0000_12345678\n"
        "TRANSMIT@Enc(VBusP)\n"
        "    0xAA, 0x16, 0x11, 0x20, 0x00, 0x10, 0x10, 0x03, 0x00, 0x15,\n"
    );
    __ASSERT_EQL(Bridge->VBusPPeerAddress, 0x1000);

    // ========= Phase SENDPACKET1 =========

    // --------- Timeout ---------
    __ASSERT_VBUSP_PHASE_EQL(SENDPACKET1);
    __ASSERT_TIMEOUT_EQL(10417);

    __WRAP(__HandleLoopCycle(Bridge, 10417, NULL, 0));

    __ASSERT_VBUSP_PHASE_EQL(SENDPACKET1);
    __ASSERT_TIMEOUT_EQL(389583);
    __ASSERT_LOG_EQL("");
    __ASSERT_EQL(Bridge->VBusPPeerAddress, 0x1000);

    __WRAP(__HandleLoopCycle(Bridge, 389583, NULL, 0));

    __ASSERT_VBUSP_PHASE_EQL(RELEASEBUS);
    __ASSERT_TIMEOUT_EQL(16667);
    __ASSERT_LOG_EQL(
        "TRANSMIT@Enc(VBusP)\n"
        "    0xAA, 0x00, 0x10, 0x20, 0x00, 0x20, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x29,\n"
    );
    __ASSERT_EQL(Bridge->VBusPPeerAddress, 0x1000);

    // --------- Packet ---------
    __WRAP(__ProgressToPhase(Bridge, __PHASE_SENDPACKET1));

    __ASSERT_VBUSP_PHASE_EQL(SENDPACKET1);
    __ASSERT_TIMEOUT_EQL(389583);
    __ASSERT_LOG_EQL("");
    __ASSERT_EQL(Bridge->VBusPPeerAddress, 0x1000);

    __WRAP(__HandleLoopCycle(Bridge, 0, __PvsPacket1, sizeof(__PvsPacket1)));

    __ASSERT_VBUSP_PHASE_EQL(SENDPACKET2);
    __ASSERT_TIMEOUT_EQL(29167);
    __ASSERT_LOG_EQL(
        "PACKETHEADER@Dec(VBusP) 0020_1116_10_0110_01\n"
        "PACKETFRAME@Dec(VBusP) 0020_1116_10_0110_00\n"
        "PACKETEND@Dec(VBusP) 0020_1116_10_0110_01\n"
        "TRANSMIT@Enc(VBusP)\n"
        "    0xAA, 0x16, 0x11, 0x20, 0x00, 0x10, 0x20, 0x02, 0x03, 0x03, 0x02, 0x01, 0x00, 0x00, 0x00, 0x7C,\n"
        "    0x64, 0x00, 0x1E, 0x00, 0x00, 0x7D, 0x64, 0x00, 0x1E, 0x00, 0x00, 0x7D,\n"
    );
    __ASSERT_EQL(Bridge->VBusPPeerAddress, 0x1000);

    // ========= Phase SENDPACKET2 =========

    // --------- Timeout ---------
    __ASSERT_VBUSP_PHASE_EQL(SENDPACKET2);
    __ASSERT_TIMEOUT_EQL(29167);

    __WRAP(__HandleLoopCycle(Bridge, 29167, NULL, 0));

    __ASSERT_VBUSP_PHASE_EQL(SENDPACKET2);
    __ASSERT_TIMEOUT_EQL(370833);
    __ASSERT_LOG_EQL("");
    __ASSERT_EQL(Bridge->VBusPPeerAddress, 0x1000);

    __WRAP(__HandleLoopCycle(Bridge, 370833, NULL, 0));

    __ASSERT_VBUSP_PHASE_EQL(RELEASEBUS);
    __ASSERT_TIMEOUT_EQL(16667);
    __ASSERT_LOG_EQL(
        "TRANSMIT@Enc(VBusP)\n"
        "    0xAA, 0x00, 0x10, 0x20, 0x00, 0x20, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x29,\n"
    );
    __ASSERT_EQL(Bridge->VBusPPeerAddress, 0x1000);

    // --------- Packet ---------
    __WRAP(__ProgressToPhase(Bridge, __PHASE_SENDPACKET2));

    __ASSERT_VBUSP_PHASE_EQL(SENDPACKET2);
    __ASSERT_TIMEOUT_EQL(345833);
    __ASSERT_LOG_EQL("");
    __ASSERT_EQL(Bridge->VBusPPeerAddress, 0x1000);

    __WRAP(__HandleLoopCycle(Bridge, 0, __PvsPacket2, sizeof(__PvsPacket2)));

    __ASSERT_VBUSP_PHASE_EQL(SENDPACKET3);
    __ASSERT_TIMEOUT_EQL(54167);
    __ASSERT_LOG_EQL(
        "PACKETHEADER@Dec(VBusP) 0020_1116_10_0120_00\n"
        "PACKETEND@Dec(VBusP) 0020_1116_10_0120_00\n"
        "TRANSMIT@Enc(VBusP)\n"
        "    0xAA, 0x16, 0x11, 0x20, 0x00, 0x10, 0x30, 0x02, 0x07, 0x6F, 0x02, 0x01, 0x00, 0x00, 0x00, 0x7C,\n"
        "    0x02, 0x02, 0x00, 0x00, 0x00, 0x7B, 0x02, 0x02, 0x00, 0x00, 0x00, 0x7B, 0x02, 0x02, 0x00, 0x00,\n"
        "    0x00, 0x7B, 0x02, 0x02, 0x00, 0x00, 0x00, 0x7B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7F, 0x00, 0x00,\n"
        "    0x00, 0x00, 0x00, 0x7F,\n"
    );
    __ASSERT_EQL(Bridge->VBusPPeerAddress, 0x1000);

    // ========= Phase SENDPACKET3 =========

    // --------- Timeout ---------
    __ASSERT_VBUSP_PHASE_EQL(SENDPACKET3);
    __ASSERT_TIMEOUT_EQL(54167);

    __WRAP(__HandleLoopCycle(Bridge, 54167, NULL, 0));

    __ASSERT_VBUSP_PHASE_EQL(SENDPACKET3);
    __ASSERT_TIMEOUT_EQL(345833);
    __ASSERT_LOG_EQL("");
    __ASSERT_EQL(Bridge->VBusPPeerAddress, 0x1000);

    __WRAP(__HandleLoopCycle(Bridge, 345833, NULL, 0));

    __ASSERT_VBUSP_PHASE_EQL(RELEASEBUS);
    __ASSERT_TIMEOUT_EQL(16667);
    __ASSERT_LOG_EQL(
        "TRANSMIT@Enc(VBusP)\n"
        "    0xAA, 0x00, 0x10, 0x20, 0x00, 0x20, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x29,\n"
    );
    __ASSERT_EQL(Bridge->VBusPPeerAddress, 0x1000);

    // --------- Packet ---------
    __WRAP(__ProgressToPhase(Bridge, __PHASE_SENDPACKET3));

    __ASSERT_VBUSP_PHASE_EQL(SENDPACKET3);
    __ASSERT_TIMEOUT_EQL(345833);
    __ASSERT_LOG_EQL("");
    __ASSERT_EQL(Bridge->VBusPPeerAddress, 0x1000);

    __WRAP(__HandleLoopCycle(Bridge, 0, __PvsPacket3, sizeof(__PvsPacket3)));

    __ASSERT_VBUSP_PHASE_EQL(RELEASEBUS);
    __ASSERT_TIMEOUT_EQL(16667);
    __ASSERT_LOG_EQL(
        "PACKETHEADER@Dec(VBusP) 0020_1116_10_0130_02\n"
        "PACKETFRAME@Dec(VBusP) 0020_1116_10_0130_00\n"
        "PACKETFRAME@Dec(VBusP) 0020_1116_10_0130_01\n"
        "PACKETEND@Dec(VBusP) 0020_1116_10_0130_02\n"
        "TRANSMIT@Enc(VBusP)\n"
        "    0xAA, 0x00, 0x10, 0x20, 0x00, 0x20, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x29,\n"
        "TRANSMIT@Enc(VBusV)\n"
        "    0xAA, 0x10, 0x00, 0x2E, 0x11, 0x10, 0x01, 0x01, 0x02, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7F,\n"
        "    0x00, 0x00, 0x00, 0x00, 0x00, 0x7F,\n"
    );
    __ASSERT_EQL(Bridge->VBusPPeerAddress, 0x1000);

    // ========= Phase RELEASEBUS =========

    // --------- Timeout ---------
    __ASSERT_VBUSP_PHASE_EQL(RELEASEBUS);
    __ASSERT_TIMEOUT_EQL(16667);

    __WRAP(__HandleLoopCycle(Bridge, 16667, NULL, 0));

    __ASSERT_VBUSP_PHASE_EQL(RELEASEBUS);
    __ASSERT_TIMEOUT_EQL(6250);
    __ASSERT_LOG_EQL("");
    __ASSERT_EQL(Bridge->VBusPPeerAddress, 0x1000);

    __WRAP(__HandleLoopCycle(Bridge, 6250, NULL, 0));

    __ASSERT_VBUSP_PHASE_EQL(RELEASEBUS);
    __ASSERT_TIMEOUT_EQL(1477083);
    __ASSERT_LOG_EQL("");
    __ASSERT_EQL(Bridge->VBusPPeerAddress, 0x1000);

    __WRAP(__HandleLoopCycle(Bridge, 1477083, NULL, 0));

    __ASSERT_VBUSP_PHASE_EQL(RELEASEBUS);
    __ASSERT_TIMEOUT_EQL(16667);
    __ASSERT_LOG_EQL(
        "TRANSMIT@Enc(VBusP)\n"
        "    0xAA, 0x00, 0x10, 0x20, 0x00, 0x20, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x29,\n"
    );
    __ASSERT_EQL(Bridge->VBusPPeerAddress, 0x1000);

    __WRAP(__HandleLoopCycle(Bridge, 16667, NULL, 0));

    __ASSERT_VBUSP_PHASE_EQL(RELEASEBUS);
    __ASSERT_TIMEOUT_EQL(1483333);
    __ASSERT_LOG_EQL("");
    __ASSERT_EQL(Bridge->VBusPPeerAddress, 0x1000);

    __WRAP(__HandleLoopCycle(Bridge, 1483333, NULL, 0));

    __ASSERT_VBUSP_PHASE_EQL(WAITFORFREEBUS);
    __ASSERT_TIMEOUT_EQL(20000000);
    __ASSERT_LOG_EQL("");
    __ASSERT_EQL(Bridge->VBusPPeerAddress, 0);

    // --------- Packet ---------
    __WRAP(__ProgressToPhase(Bridge, __PHASE_RELEASEBUS));

    __ASSERT_VBUSP_PHASE_EQL(RELEASEBUS);
    __ASSERT_TIMEOUT_EQL(1477083);
    __ASSERT_LOG_EQL("");
    __ASSERT_EQL(Bridge->VBusPPeerAddress, 0x1000);

    __WRAP(__HandleLoopCycle(Bridge, 0, __ControllerPacket1, sizeof(__ControllerPacket1)));

    __ASSERT_VBUSP_PHASE_EQL(WAITFORFREEBUS);
    __ASSERT_TIMEOUT_EQL(20000000);
    __ASSERT_LOG_EQL(
        "PACKETHEADER@Dec(VBusP) 0010_1000_10_0100_00\n"
        "PACKETEND@Dec(VBusP) 0010_1000_10_0100_00\n"
    );
    __ASSERT_EQL(Bridge->VBusPPeerAddress, 0);

    // ========= Branch `!Handled` for coverage =========

    __ASSERT_VBUSP_PHASE_EQL(WAITFORFREEBUS);
    __ASSERT_TIMEOUT_EQL(20000000);

    Bridge->VBusPPhase = __PHASE_UNKNOWN;

    __WRAP(__HandleLoopCycle(Bridge, 20000000, NULL, 0));

    __ASSERT_VBUSP_PHASE_EQL(WAITFORFREEBUS);
    __ASSERT_TIMEOUT_EQL(20000000);
    __ASSERT_LOG_EQL("");
    __ASSERT_EQL(Bridge->VBusPPeerAddress, 0);

    return Result;
}


// static RESOLVBUS_RESULT __VBusPSendPacket1Handler(RESOLVBUS_LIVETRANSCEIVER *Transceiver, const RESOLVBUS_LIVETRANSCEIVEREVENT *Event)
static RESOLVBUS_RESULT __TestVBusPSendPacket1Handler(__ACCESSORYBRIDGE *Bridge)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    __WRAP(__ProgressToPhase(Bridge, __PHASE_SENDPACKET1));

    __WRAP(__HandleLoopCycle(Bridge, 0, __UnrelatedDatagram1, sizeof (__UnrelatedDatagram1)));

    __ASSERT_LOG_EQL(
        "DATAGRAM@Dec(VBusP) 0000_0000_20_0000_0000_00000000\n"
    );

    __WRAP(__HandleLoopCycle(Bridge, 0, __UnrelatedPacket1, sizeof (__UnrelatedPacket1)));

    __ASSERT_LOG_EQL(
        "PACKETHEADER@Dec(VBusP) 0000_0000_10_0000_00\n"
        "PACKETEND@Dec(VBusP) 0000_0000_10_0000_00\n"
    );

    __WRAP(__HandleLoopCycle(Bridge, 0, __UnrelatedPacket2, sizeof (__UnrelatedPacket2)));

    __ASSERT_LOG_EQL(
        "PACKETHEADER@Dec(VBusP) 0020_0000_10_0000_00\n"
        "PACKETEND@Dec(VBusP) 0020_0000_10_0000_00\n"
    );

    __WRAP(__HandleLoopCycle(Bridge, 0, __UnrelatedPacket3, sizeof (__UnrelatedPacket3)));

    __ASSERT_LOG_EQL(
        "PACKETHEADER@Dec(VBusP) 0020_1116_10_0000_00\n"
        "PACKETEND@Dec(VBusP) 0020_1116_10_0000_00\n"
    );

    __WRAP(__HandleLoopCycle(Bridge, 0, __PvsPacket1TooShort, sizeof (__PvsPacket1TooShort)));

    __ASSERT_LOG_EQL(
        "PACKETHEADER@Dec(VBusP) 0020_1116_10_0110_00\n"
        "PACKETEND@Dec(VBusP) 0020_1116_10_0110_00\n"
    );

    __WRAP(__HandleLoopCycle(Bridge, 0, __PvsPacket1WrongVersion, sizeof (__PvsPacket1WrongVersion)));

    __ASSERT_LOG_EQL(
        "PACKETHEADER@Dec(VBusP) 0020_1116_10_0110_01\n"
        "PACKETFRAME@Dec(VBusP) 0020_1116_10_0110_00\n"
        "PACKETEND@Dec(VBusP) 0020_1116_10_0110_01\n"
    );

    __WRAP(__HandleLoopCycle(Bridge, 0, __PvsPacket1, sizeof (__PvsPacket1)));

    __ASSERT_LOG_EQL(
        "PACKETHEADER@Dec(VBusP) 0020_1116_10_0110_01\n"
        "PACKETFRAME@Dec(VBusP) 0020_1116_10_0110_00\n"
        "PACKETEND@Dec(VBusP) 0020_1116_10_0110_01\n"
        "TRANSMIT@Enc(VBusP)\n"
        "    0xAA, 0x16, 0x11, 0x20, 0x00, 0x10, 0x20, 0x02, 0x03, 0x03, 0x02, 0x01, 0x00, 0x00, 0x00, 0x7C,\n"
        "    0x64, 0x00, 0x1E, 0x00, 0x00, 0x7D, 0x64, 0x00, 0x1E, 0x00, 0x00, 0x7D,\n"
    );

    __WRAP(__SetBackToPhaseWaitForFreeBus(Bridge));

    return Result;
}


// static RESOLVBUS_RESULT __VBusPSendPacket2Handler(RESOLVBUS_LIVETRANSCEIVER *Transceiver, const RESOLVBUS_LIVETRANSCEIVEREVENT *Event)
static RESOLVBUS_RESULT __TestVBusPSendPacket2Handler(__ACCESSORYBRIDGE *Bridge)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    __WRAP(__ProgressToPhase(Bridge, __PHASE_SENDPACKET2));

    __WRAP(__HandleLoopCycle(Bridge, 0, __UnrelatedDatagram1, sizeof (__UnrelatedDatagram1)));

    __ASSERT_LOG_EQL(
        "DATAGRAM@Dec(VBusP) 0000_0000_20_0000_0000_00000000\n"
    );

    __WRAP(__HandleLoopCycle(Bridge, 0, __UnrelatedPacket1, sizeof (__UnrelatedPacket1)));

    __ASSERT_LOG_EQL(
        "PACKETHEADER@Dec(VBusP) 0000_0000_10_0000_00\n"
        "PACKETEND@Dec(VBusP) 0000_0000_10_0000_00\n"
    );

    __WRAP(__HandleLoopCycle(Bridge, 0, __UnrelatedPacket2, sizeof (__UnrelatedPacket2)));

    __ASSERT_LOG_EQL(
        "PACKETHEADER@Dec(VBusP) 0020_0000_10_0000_00\n"
        "PACKETEND@Dec(VBusP) 0020_0000_10_0000_00\n"
    );

    __WRAP(__HandleLoopCycle(Bridge, 0, __UnrelatedPacket3, sizeof (__UnrelatedPacket3)));

    __ASSERT_LOG_EQL(
        "PACKETHEADER@Dec(VBusP) 0020_1116_10_0000_00\n"
        "PACKETEND@Dec(VBusP) 0020_1116_10_0000_00\n"
    );

    __WRAP(__HandleLoopCycle(Bridge, 0, __PvsPacket2, sizeof (__PvsPacket2)));

    __ASSERT_LOG_EQL(
        "PACKETHEADER@Dec(VBusP) 0020_1116_10_0120_00\n"
        "PACKETEND@Dec(VBusP) 0020_1116_10_0120_00\n"
        "TRANSMIT@Enc(VBusP)\n"
        "    0xAA, 0x16, 0x11, 0x20, 0x00, 0x10, 0x30, 0x02, 0x07, 0x6F, 0x02, 0x01, 0x00, 0x00, 0x00, 0x7C,\n"
        "    0x02, 0x02, 0x00, 0x00, 0x00, 0x7B, 0x02, 0x02, 0x00, 0x00, 0x00, 0x7B, 0x02, 0x02, 0x00, 0x00,\n"
        "    0x00, 0x7B, 0x02, 0x02, 0x00, 0x00, 0x00, 0x7B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7F, 0x00, 0x00,\n"
        "    0x00, 0x00, 0x00, 0x7F,\n"
    );

    __WRAP(__SetBackToPhaseWaitForFreeBus(Bridge));

    return Result;
}


// static RESOLVBUS_RESULT __VBusPSendPacket3Handler(RESOLVBUS_LIVETRANSCEIVER *Transceiver, const RESOLVBUS_LIVETRANSCEIVEREVENT *Event)
static RESOLVBUS_RESULT __TestVBusPSendPacket3Handler(__ACCESSORYBRIDGE *Bridge)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    __WRAP(__ProgressToPhase(Bridge, __PHASE_SENDPACKET3));

    __WRAP(__HandleLoopCycle(Bridge, 0, __UnrelatedDatagram1, sizeof (__UnrelatedDatagram1)));

    __ASSERT_LOG_EQL(
        "DATAGRAM@Dec(VBusP) 0000_0000_20_0000_0000_00000000\n"
    );

    __WRAP(__HandleLoopCycle(Bridge, 0, __UnrelatedPacket1, sizeof (__UnrelatedPacket1)));

    __ASSERT_LOG_EQL(
        "PACKETHEADER@Dec(VBusP) 0000_0000_10_0000_00\n"
        "PACKETEND@Dec(VBusP) 0000_0000_10_0000_00\n"
    );

    __WRAP(__HandleLoopCycle(Bridge, 0, __UnrelatedPacket2, sizeof (__UnrelatedPacket2)));

    __ASSERT_LOG_EQL(
        "PACKETHEADER@Dec(VBusP) 0020_0000_10_0000_00\n"
        "PACKETEND@Dec(VBusP) 0020_0000_10_0000_00\n"
    );

    __WRAP(__HandleLoopCycle(Bridge, 0, __UnrelatedPacket3, sizeof (__UnrelatedPacket3)));

    __ASSERT_LOG_EQL(
        "PACKETHEADER@Dec(VBusP) 0020_1116_10_0000_00\n"
        "PACKETEND@Dec(VBusP) 0020_1116_10_0000_00\n"
    );

    __WRAP(__HandleLoopCycle(Bridge, 0, __PvsPacket3TooShort, sizeof (__PvsPacket3TooShort)));

    __ASSERT_LOG_EQL(
        "PACKETHEADER@Dec(VBusP) 0020_1116_10_0130_00\n"
        "PACKETEND@Dec(VBusP) 0020_1116_10_0130_00\n"
    );

    __WRAP(__HandleLoopCycle(Bridge, 0, __PvsPacket3, sizeof (__PvsPacket3)));

    __ASSERT_LOG_EQL(
        "PACKETHEADER@Dec(VBusP) 0020_1116_10_0130_02\n"
        "PACKETFRAME@Dec(VBusP) 0020_1116_10_0130_00\n"
        "PACKETFRAME@Dec(VBusP) 0020_1116_10_0130_01\n"
        "PACKETEND@Dec(VBusP) 0020_1116_10_0130_02\n"
        "TRANSMIT@Enc(VBusP)\n"
        "    0xAA, 0x00, 0x10, 0x20, 0x00, 0x20, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x29,\n"
        "TRANSMIT@Enc(VBusV)\n"
        "    0xAA, 0x10, 0x00, 0x2E, 0x11, 0x10, 0x01, 0x01, 0x02, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7F,\n"
        "    0x00, 0x00, 0x00, 0x00, 0x00, 0x7F,\n"
    );

    __WRAP(__SetBackToPhaseWaitForFreeBus(Bridge));

    return Result;
}


// static RESOLVBUS_RESULT __PerformVBusPPhaseAction(__ACCESSORYBRIDGE *Bridge, __PHASE Phase)
static RESOLVBUS_RESULT __TestPerformVBusPPhaseAction(__ACCESSORYBRIDGE *Bridge)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    __WRAP(__ProgressToPhase(Bridge, __PHASE_RELEASEBUS));

    __WRAP(__SetBackToPhaseWaitForFreeBus(Bridge));

    // --------- Branch "Unknown phase" for coverage ---------
    if (Result == RESOLVBUS_OK) {
        Bridge->VBusPLiveTransceiver.ActionSet = false;
    }

    __WRAP(__PerformVBusPPhaseAction(Bridge, __PHASE_UNKNOWN));

    return Result;
}


// static RESOLVBUS_RESULT __VBusPLiveTransceiverHandler(RESOLVBUS_LIVETRANSCEIVER *Transceiver, const RESOLVBUS_LIVETRANSCEIVEREVENT *Event)
static RESOLVBUS_RESULT __TestVBusPLiveTransceiverHandler(__ACCESSORYBRIDGE *Bridge)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    __WRAP(__ProgressToPhase(Bridge, __PHASE_RELEASEBUS));

    __WRAP(__SetBackToPhaseWaitForFreeBus(Bridge));

    return Result;
}


// static RESOLVBUS_RESULT __VBusVLiveEncoderHandler(RESOLVBUS_LIVEENCODER *Encoder, const RESOLVBUS_LIVEENCODEREVENT *Event)
static RESOLVBUS_RESULT __TestVBusVLiveEncoderHandler(__ACCESSORYBRIDGE *Bridge)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    __WRAP(__ProgressToPhase(Bridge, __PHASE_RELEASEBUS));

    return Result;
}


static RESOLVBUS_RESULT __TestInitialize(__ACCESSORYBRIDGE *Bridge)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    __WRAP(__Initialize(Bridge));

    __ASSERT_POINTER_EQL((void *) Bridge->VBusPLiveTransceiver.Handler, (void *) __VBusPLiveTransceiverHandler);
    __ASSERT_POINTER_EQL((void *) Bridge->VBusVLiveEncoder.Handler, (void *) __VBusVLiveEncoderHandler);

    if (Result == RESOLVBUS_OK) {
        Bridge->VBusPLiveTransceiver.Handler = __TestableVBusPLiveTransceiverHandler;
        Bridge->VBusVLiveEncoder.Handler = __TestableVBusVLiveEncoderHandler;
    }

    return Result;
}


static RESOLVBUS_RESULT __TestGetTimeout(__ACCESSORYBRIDGE *Bridge)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    __ASSERT_TIMEOUT_EQL(20000000);

    __WRAP(__HandleLoopCycle(Bridge, 0, NULL, 0));

    __ASSERT_TIMEOUT_EQL(20000000);

    __WRAP(__HandleLoopCycle(Bridge, 19900000, NULL, 0));

    __ASSERT_TIMEOUT_EQL(100000);

    __WRAP(__HandleLoopCycle(Bridge, 100000, NULL, 0));

    __ASSERT_TIMEOUT_EQL(20000000);

    __ASSERT_LOG_EQL("");

    return Result;
}


static RESOLVBUS_RESULT __TestHandleLoopCycle(__ACCESSORYBRIDGE *Bridge)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    __WRAP(__HandleLoopCycle(Bridge, 0, NULL, 0));

    __WRAP(__HandleLoopCycle(Bridge, 0, __ControllerPacket1, 0));

    return Result;
}


static RESOLVBUS_RESULT __TestIntegration(__ACCESSORYBRIDGE *Bridge)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    // __FAIL(NYI);

    return Result;
}



//---------------------------------------------------------------------------
// PUBLIC METHODS
//---------------------------------------------------------------------------

RESOLVBUS_RESULT RunTest_AccessoryBridgeExample(void)
{
    RESOLVBUS_RESULT Result = RESOLVBUS_OK;

    __ACCESSORYBRIDGE Bridge = (__ACCESSORYBRIDGE) {
        .VBusPLiveTransceiver = RESOLVBUS_LIVETRANSCEIVER_INITIALIZER,
    };

    // NOTE(daniel): must be first "test" and must not be commented out!
    __WRAP(__TestInitialize(&Bridge));

    __WRAP(__TestGetTimeout(&Bridge));

    __WRAP(__TestHandleLoopCycle(&Bridge));

    __WRAP(__TestVBusPLiveTransceiverActionHandler(&Bridge));

    __WRAP(__TestVBusPSendPacket1Handler(&Bridge));

    __WRAP(__TestVBusPSendPacket2Handler(&Bridge));

    __WRAP(__TestVBusPSendPacket3Handler(&Bridge));

    __WRAP(__TestPerformVBusPPhaseAction(&Bridge));

    __WRAP(__TestVBusPLiveTransceiverHandler(&Bridge));

    __WRAP(__TestVBusVLiveEncoderHandler(&Bridge));

    __WRAP(__TestIntegration(&Bridge));

    __WRAP(__Finalize(&Bridge));

    __ASSERT_LOG_EQL("");

    return Result;
}

