#ifndef __RESOLVBUS_H__
#define __RESOLVBUS_H__

//
//  ResolVBus.h
//  resol-vbus
//
//
//  Copyright (C) 2022, Daniel Wippermann. All rights reserved.
//


//---------------------------------------------------------------------------
// PUBLIC INCLUDES
//---------------------------------------------------------------------------

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>



//---------------------------------------------------------------------------
// PUBLIC DEFINES
//---------------------------------------------------------------------------

#define RESOLVBUS_CONTAINEROF(__Pointer__, __Type__, __Member__) \
    ((__Type__ *) (((uint8_t *) (__Pointer__)) - ((uint8_t *) (&((__Type__ *) 0)->__Member__))))


#define RESOLVBUS_COUNTOF(__Array__) \
    (sizeof (__Array__) / sizeof ((__Array__) [0]))


#define RESOLVBUS_PACKETFRAMECOUNTOF(__Type__) \
    ((sizeof (__Type__) + 3) >> 2)



//---------------------------------------------------------------------------
// PUBLIC TYPEDEFS
//---------------------------------------------------------------------------

/**
 * Used as return type for most functions.
 *
 * It is guaranteed, that `RESOLVBUS_OK` always has `0` assigned to it. That means, that
 * `if (Result == RESOLVBUS_OK) ...` and `if (Result == 0) ...` are equivalent.
 *
 * It is also guaranteed, that `RESOLVBUS_WOULDBLOCK` always has `1` assigned to it.
 *
 * All other values are not guaranteed to keep their numerical value and / or their order.
 */
typedef enum RESOLVBUS_RESULT {
    RESOLVBUS_OK = 0,
    RESOLVBUS_WOULDBLOCK = 1,

    RESOLVBUS_ERROR_UNKNOWN = 2,
    RESOLVBUS_ERROR_NYI = 3,
    RESOLVBUS_ERROR_NULLPOINTER = 4,
    RESOLVBUS_ERROR_UNSUPPORTEDPROTOCOL = 5,
    RESOLVBUS_ERROR_INDEXOUTOFBOUNDS = 6,
    RESOLVBUS_ERROR_INVALIDARGUMENT = 7,
    RESOLVBUS_ERROR_INVALIDCHECKSUM = 8,
    RESOLVBUS_ERROR_INVALIDSTATE = 9,
} RESOLVBUS_RESULT;


typedef enum RESOLVBUS_LIVEENCODEREVENTTYPE {
    /**
     * Emitted when buffers are empty and timeouts have run out.
     *
     * Receiving this event can e.g. be used to send another piece of information.
     */
    RESOLVBUS_LIVEENCODEREVENTTYPE_IDLE,

    /**
     * Emitted when raw data are ready to be transmitted.
     *
     * Raw data is passed in the `TransmitBytes` and `TransmitLength` fields of the event struct.
     */
    RESOLVBUS_LIVEENCODEREVENTTYPE_TRANSMIT,
} RESOLVBUS_LIVEENCODEREVENTTYPE;


typedef enum RESOLVBUS_LIVEENCODERPHASE {
    /**
     * The encoder has no work to do at the moment.
     */
    RESOLVBUS_LIVEENCODERPHASE_IDLE = 0,

    /**
     * The encoder is currently transmitting data.
     */
    RESOLVBUS_LIVEENCODERPHASE_TRANSMITTING = 1,

    /**
     * The encoder is waiting for the VBus to regain energy.
     */
    RESOLVBUS_LIVEENCODERPHASE_GAININGENERGY = 2,

    /**
     * The encoder has been suspended without a timeout.
     */
    RESOLVBUS_LIVEENCODERPHASE_SUSPENDED = 3,

    /**
     * The encoder has been suspended with a timeout.
     */
    RESOLVBUS_LIVEENCODERPHASE_SUSPENDEDWITHTIMEOUT = 4,
} RESOLVBUS_LIVEENCODERPHASE;


typedef enum RESOLVBUS_LIVEDECODEREVENTTYPE {
    /**
     * Emitted when the header of a VBus packet has been decoded.
     *
     * The following fields of the decoder contain additional information:
     * - `DestinationAddress`
     * - `SourceAddress`
     * - `ProtocolVersion`
     * - `MajorVersion`
     * - `Command`
     * - `FrameCount`
     */
    RESOLVBUS_LIVEDECODEREVENTTYPE_PACKETHEADER,

    /**
     * Emitted when a frame of a VBus packet has been decoded.
     *
     * The following fields of the decoder contain additional information:
     * - `DestinationAddress`
     * - `SourceAddress`
     * - `ProtocolVersion`
     * - `MajorVersion`
     * - `Command`
     * - `FrameCount`
     * - `FrameIndex`
     * - `FrameData`
     * - `FrameDataLength`
     * - `FrameDataBuffer`
     */
    RESOLVBUS_LIVEDECODEREVENTTYPE_PACKETFRAME,

    /**
     * Emitted when the header and all of its associated frames of a VBus packet have been decoded.
     *
     * The following fields of the decoder contain additional information:
     * - `DestinationAddress`
     * - `SourceAddress`
     * - `ProtocolVersion`
     * - `MajorVersion`
     * - `Command`
     * - `FrameCount`
     * - `FrameIndex`
     * - `FrameData`
     * - `FrameDataLength`
     * - `FrameDataBuffer`
     */
    RESOLVBUS_LIVEDECODEREVENTTYPE_PACKETEND,


    /**
     * Emitted when a VBus datagram has been decoded.
     *
     * The following fields of the decoder contain additional information:
     * - `DestinationAddress`
     * - `SourceAddress`
     * - `ProtocolVersion`
     * - `MajorVersion`
     * - `Command`
     * - `Param16`
     * - `Param32`
     */
    RESOLVBUS_LIVEDECODEREVENTTYPE_DATAGRAM,


    /**
     * Emitted when the header of a VBus telegram has been decoded.
     *
     * The following fields of the decoder contain additional information:
     * - `DestinationAddress`
     * - `SourceAddress`
     * - `ProtocolVersion`
     * - `MajorVersion`
     * - `Command`
     * - `FrameCount`
     */
    RESOLVBUS_LIVEDECODEREVENTTYPE_TELEGRAMHEADER,

    /**
     * Emitted when a frame of a VBus telegram has been decoded.
     *
     * The following fields of the decoder contain additional information:
     * - `DestinationAddress`
     * - `SourceAddress`
     * - `ProtocolVersion`
     * - `MajorVersion`
     * - `Command`
     * - `FrameCount`
     * - `FrameIndex`
     * - `FrameData`
     * - `FrameDataLength`
     * - `FrameDataBuffer`
     */
    RESOLVBUS_LIVEDECODEREVENTTYPE_TELEGRAMFRAME,

    /**
     * Emitted when the header and all of its associated frames of a VBus telegram have been decoded.
     *
     * The following fields of the decoder contain additional information:
     * - `DestinationAddress`
     * - `SourceAddress`
     * - `ProtocolVersion`
     * - `MajorVersion`
     * - `Command`
     * - `FrameCount`
     * - `FrameIndex`
     * - `FrameData`
     * - `FrameDataLength`
     * - `FrameDataBuffer`
     */
    RESOLVBUS_LIVEDECODEREVENTTYPE_TELEGRAMEND,
} RESOLVBUS_LIVEDECODEREVENTTYPE;


typedef struct RESOLVBUS_LIVEENCODEREVENT RESOLVBUS_LIVEENCODEREVENT;
typedef struct RESOLVBUS_LIVEENCODER RESOLVBUS_LIVEENCODER;
typedef struct RESOLVBUS_LIVEDECODEREVENT RESOLVBUS_LIVEDECODEREVENT;
typedef struct RESOLVBUS_LIVEDECODER RESOLVBUS_LIVEDECODER;



//---------------------------------------------------------------------------
// PUBLIC CALLBACKS
//---------------------------------------------------------------------------

typedef RESOLVBUS_RESULT (*RESOLVBUS_LIVEENCODERHANDLER)(RESOLVBUS_LIVEENCODER *Encoder, const RESOLVBUS_LIVEENCODEREVENT *Event);
typedef RESOLVBUS_RESULT (*RESOLVBUS_LIVEDECODERHANDLER)(RESOLVBUS_LIVEDECODER *Decoder, const RESOLVBUS_LIVEDECODEREVENT *Event);



//---------------------------------------------------------------------------
// PUBLIC DATA TYPES
//---------------------------------------------------------------------------

struct RESOLVBUS_LIVEENCODEREVENT {
    /**
     * Type of the event.
     */
    RESOLVBUS_LIVEENCODEREVENTTYPE EventType;

    /**
     * Buffer containing data to transmit.
     */
    const void *TransmitBytes;

    /**
     * Length of the `TransmitBytes` buffer.
     */
    size_t TransmitLength;
};


struct RESOLVBUS_LIVEENCODER {
    /**
     * Transmit buffer to queue VBus primitives into.
     *
     * This buffer is provided as an argument to `ResolVBus_LiveEncoder_Initialize`.
     */
    uint8_t *Buffer;

    /**
     * Length of the `Buffer`.
     *
     * This length is provided as an argument to `ResolVBus_LiveEncoder_Initialize`.
     */
    size_t BufferLength;

    /**
     * Index into the `Buffer` where the next bytes are written to.
     */
    size_t BufferWriteIndex;

    /**
     * Index into the `Buffer` where the next bytes are read from.
     */
    size_t BufferReadIndex;

    /**
     * Baudrate used to calculate the timeout of the `TRANSMITTING` phase.
     */
    int Baudrate;

    /**
     * Maximum energy level.
     *
     * Used to calculate the maximum transmit chunk size (`MaxEnergy / EnergyLossPerByte`).
     *
     * Can be set to 0 to disable energy management.
     */
    int MaxEnergy;

    /**
     * Energy loss per byte transmitted.
     *
     * Used to calculate the maximum transmit chunk size (`MaxEnergy / EnergyLossPerByte`).
     *
     * Can be set to 0 to disable energy loss management.
     */
    int EnergyLossPerByte;

    /**
     * Energy gain per microseconds.
     *
     * Used to calculate the timeout for the `GAININGENERGY` phase (`CurrentEnergy / EnergyGainPerUs`).
     *
     * Can be set to 0 to disable energy gain management.
     */
    int EnergyGainPerUs;

    /**
     * Current energy level after transmitting a chunk of data.
     *
     * Used to calculate the timeout for the `GAININGENERGY` phase (`CurrentEnergy / EnergyGainPerUs`).
     *
     */
    int CurrentEnergy;

    /**
     * Request to suspend without a timeout.
     *
     * If the encoder is not idle when `ResolVBus_LiveEncoder_Suspend` is called, this flag is set to true
     * to request transitioning into a suspend state after the transmit buffer has been emptied.
     */
    bool SuspendRequested;

    /**
     * Request to suspend with a timeout.
     *
     * If the encoder is not idle when `ResolVBus_LiveEncoder_SuspendWithTimeout` is called, this flag is set to true
     * to request transitioning into a suspend state after the transmit buffer has been emptied.
     */
    bool SuspendWithTimeoutRequested;

    /**
     * Requested suspend timeout.
     */
    uint32_t SuspendTimeoutUs;

    /**
     * Current phase of the internal state machine.
     */
    RESOLVBUS_LIVEENCODERPHASE Phase;

    /**
     * Remaining timeout of the current phase of the internal state machine.
     */
    uint32_t PhaseTimeoutUs;

    /**
     * Protocol version used in the last header queued to the transmit buffer.
     */
    uint8_t ProtocolVersion;

    /**
     * Handler to be called when an event is emitted.
     */
    RESOLVBUS_LIVEENCODERHANDLER Handler;
};


struct RESOLVBUS_LIVEDECODEREVENT {
    /**
     * Type of the event.
     */
    RESOLVBUS_LIVEDECODEREVENTTYPE EventType;

    /**
     * Destination address decoded from VBus headers.
     *
     * See `RESOLVBUS_LIVEDECODEREVENTTYPE` for details when this is available.
     */
    uint16_t DestinationAddress;

    /**
     * Source address decoded from VBus headers.
     *
     * See `RESOLVBUS_LIVEDECODEREVENTTYPE` for details when this is available.
     */
    uint16_t SourceAddress;

    /**
     * Protocol version decoded from VBus headers.
     *
     * See `RESOLVBUS_LIVEDECODEREVENTTYPE` for details when this is available.
     */
    uint8_t ProtocolVersion;

    /**
     * Major version (upper nibble) of the protocol version decoded from VBus headers.
     *
     * See `RESOLVBUS_LIVEDECODEREVENTTYPE` for details when this is available.
     */
    uint8_t MajorVersion;

    /**
     * Command decoded from VBus headers.
     *
     * See `RESOLVBUS_LIVEDECODEREVENTTYPE` for details when this is available.
     */
    uint16_t Command;

    /**
     * Frame count decoded from VBus headers.
     *
     * See `RESOLVBUS_LIVEDECODEREVENTTYPE` for details when this is available.
     */
    uint8_t FrameCount;

    /**
     * Index of the currently decoded frame (can be between `0` and `FrameCount - 1`).
     *
     * See `RESOLVBUS_LIVEDECODEREVENTTYPE` for details when this is available.
     */
    uint8_t FrameIndex;

    /**
     * Data of the currently decoded frame.
     *
     * See `RESOLVBUS_LIVEDECODEREVENTTYPE` for details when this is available.
     */
    uint8_t FrameData [7];

    /**
     * Length of the data in `FrameData` for the currently decoded frame.
     *
     * See `RESOLVBUS_LIVEDECODEREVENTTYPE` for details when this is available.
     */
    size_t FrameDataLength;

    /**
     * Optional buffer to reconstruct frame data into.
     *
     * This buffer is provided as an argument to `ResolVBus_LiveDecoder_Initialize`.
     * See `RESOLVBUS_LIVEDECODEREVENTTYPE` for details when this is available.
     */
    void *FrameDataBuffer;

    /**
     * Size of the optional `FrameDataBuffer`.
     *
     * This size is provided as an argument to `ResolVBus_LiveDecoder_Initialize`.
     * See `RESOLVBUS_LIVEDECODEREVENTTYPE` for details when this is available.
     */
    size_t FrameDataBufferLength;

    /**
     * 16-bit parameter of the decoded VBus datagram.
     *
     * See `RESOLVBUS_LIVEDECODEREVENTTYPE` for details when this is available.
     */
    uint16_t Param16;

    /**
     * 32-bit parameter of the decoded VBus datagram.
     *
     * See `RESOLVBUS_LIVEDECODEREVENTTYPE` for details when this is available.
     */
    uint32_t Param32;
};


struct RESOLVBUS_LIVEDECODER {
    /**
     * Temporary buffer to reconstruct VBus primitives into.
     */
    uint8_t Buffer [32];

    /**
     * Index where the next decoded byte should be stored to in the `Buffer`.
     */
    size_t BufferIndex;

    /**
     * Handler to be called when an event is emitted.
     */
    RESOLVBUS_LIVEDECODERHANDLER Handler;

    /**
     * Event to reconstruct decoded VBus information into.
     */
    RESOLVBUS_LIVEDECODEREVENT Event;
};



//---------------------------------------------------------------------------
// PUBLIC CONSTANTS
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// PUBLIC VARIABLES
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// PUBLIC METHODS
//---------------------------------------------------------------------------

#if RESOLVBUS_DEBUG > 0
/**
 * Clear the backtrace buffer and fill it with the provided information.
 *
 * @param Message Message to put above the backtrace
 * @param Expression Expression used in the backtrace line
 * @param File Filename used in the backtrace line
 * @param Line Line number used in the backtrace line
 * @param Func Function name used in the backtrace line
 */
void ResolVBus_ResetBacktrace(const char *Message, const char *Expression, const char *File, int Line, const char *Func);

/**
 * Adds another line to the backtrace.
 *
 * @param Expression Expression used in the backtrace line
 * @param File Filename used in the backtrace line
 * @param Line Line number used in the backtrace line
 * @param Func Function name used in the backtrace line
 */
void ResolVBus_AddBacktrace(const char *Expression, const char *File, int Line, const char *Func);

/**
 * Get the currently stored backtrace.
 *
 * @returns Pointer to the currently stored backtrace
 */
const char *ResolVBus_GetBacktrace(void);

/**
 * Print the currently stored backtrace.
 */
void ResolVBus_PrintBacktrace(void);

/**
 * Print debug related information on `stdout`.
 *
 * Will print something like `FormattedMessage  [Func (File:Line)]\n` to `stdout`.
 *
 * @param File Filename used in the debug output
 * @param Line Line number used in the debug output
 * @param Func Function name used in the debug output
 * @param Format `printf`-style format string followed by optional arguments
 */
void ResolVBus_DebugLog(const char *File, int Line, const char *Func, const char *Format, ...);
#endif


/**
 * Read an unsigned, little-endian 16-bit integer from the buffer.
 *
 * @param Buffer Buffer of at least 2 bytes to read integer from
 * @returns Read integer
 */
uint16_t ResolVBus_ReadUInt16LE(const uint8_t *Buffer);

/**
 * Write an unsigned, little-endian 16-bit integer to the buffer.
 *
 * @param Buffer Buffer of at least 2 bytes to write integer into
 * @param Value Integer to write
 */
void ResolVBus_WriteUInt16LE(uint8_t *Buffer, uint16_t Value);

/**
 * Read an unsigned, little-endian 32-bit integer from the buffer.
 *
 * @param Buffer Buffer of at least 4 bytes to read integer from
 * @returns Read integer
 */
uint32_t ResolVBus_ReadUInt32LE(const uint8_t *Buffer);

/**
 * Write an unsigned, little-endian 32-bit integer to the buffer.
 *
 * @param Buffer Buffer of at least 4 bytes to write integer into
 * @param Value Integer to write
 */
void ResolVBus_WriteUInt32LE(uint8_t *Buffer, uint32_t Value);


/**
 * Calculate the checksum according to VBus protocol version X.0.
 *
 * @param Buffer Buffer to calculate checksum over
 * @param Length Size of buffer to calculate checksum over
 * @param pCrc Pointer to store calculated checksum into
 * @returns RESOLVBUS_OK if no error occurred
 */
RESOLVBUS_RESULT ResolVBus_CalcCrcV0(const uint8_t *Buffer, size_t Length, uint8_t *pCrc);

/**
 * Calculate the checksum according to the provided protocol version.
 *
 * @param ProtocolVersion Protocol version to select checksum algorithm
 * @param Buffer Buffer to calculate checksum over
 * @param Length Size of buffer to calculate checksum over
 * @param pCrc Pointer to store calculated checksum into
 * @returns RESOLVBUS_OK if no error occurred
 */
RESOLVBUS_RESULT ResolVBus_CalcCrc(uint8_t ProtocolVersion, const uint8_t *Buffer, size_t Length, uint8_t *pCrc);

/**
 * Calculate the checksum over (Length - 1) bytes and set the result into the last byte of the buffer.
 *
 * @param ProtocolVersion Protocol version to select checksum algorithm
 * @param Buffer Buffer to calculate and store checksum
 * @param Length Size of buffer including checksum byte at the end
 * @returns RESOLVBUS_OK if no error occurred
 */
RESOLVBUS_RESULT ResolVBus_CalcAndSetCrc(uint8_t ProtocolVersion, uint8_t *Buffer, size_t Length);

/**
 * Calculate the checksum over (Length - 1) bytes and compare the result to the last byte of the buffer.
 *
 * @param ProtocolVersion Protocol version to select checksum algorithm
 * @param Buffer Buffer to calculate and compare checksum
 * @param Length Size of buffer including checksum byte at the end
 * @param pCrcIsValid Pointer to store checksum comparison result into
 * @returns RESOLVBUS_OK if no error occurred
 */
RESOLVBUS_RESULT ResolVBus_CalcAndCompareCrc(uint8_t ProtocolVersion, const uint8_t *Buffer, size_t Length, bool *pCrcIsValid);

/**
 * Inject a septett byte into the MSBs.
 *
 * This function converts `Length + 1` bytes of `Source` data into `Length` bytes of destination data.
 * The MSBs of up to 7 `Destination` bytes are injected from a septett byte which is stored at
 * `Source [Length]`.
 *
 * @param Source Buffer of `Length + 1` bytes (data plus septett byte)
 * @param Destination Buffer of `Length` bytes
 * @param Length Size of buffers without the septett byte (can be between 1 and 7)
 * @returns RESOLVBUS_OK if no error occurred
 */
RESOLVBUS_RESULT ResolVBus_InjectSeptett(const uint8_t *Source, uint8_t *Destination, size_t Length);

/**
 * Extract the MSBs into a septett byte.
 *
 * This function converts `Length` bytes of `Source` data into `Length + 1` bytes of destination data.
 * The MSBs of up to 7 `Source` bytes get extracted into a septett byte which is stored at
 * `Destination [Length]`.
 *
 * @param Source Buffer of `Length` bytes
 * @param Destination Buffer of `Length + 1` bytes (data plus septett byte)
 * @param Length Size of buffers without the septett byte (can be between 1 and 7)
 * @returns RESOLVBUS_OK if no error occurred
 */
RESOLVBUS_RESULT ResolVBus_ExtractSeptett(const uint8_t *Source, uint8_t *Destination, size_t Length);


/**
 * Initialize a `RESOLVBUS_LIVEENCODER` instance.
 *
 * Lifetime: `Buffer` must outlive `Encoder`.
 *
 * @param Encoder Encoder instance to initialize
 * @param Buffer Transmit buffer to construct VBus primitives into
 * @param Length Length of the `Buffer`
 * @param Handler Handler that is called when an event is emitted
 * @returns RESOLVBUS_OK if no error occurred
 */
RESOLVBUS_RESULT ResolVBus_LiveEncoder_Initialize(RESOLVBUS_LIVEENCODER *Encoder, uint8_t *Buffer, size_t Length, RESOLVBUS_LIVEENCODERHANDLER Handler);

/**
 * Return the amount of time that the encoder needs to pass into another phase.
 *
 * The resulting timeout depends on the phase the encoder is currently in:
 * - `..._IDLE`: timeout is always 0, because it will emit `..._IDLE` events every time `ResolVBus_LiveEncoder_HandleTimer` is called
 * - `..._TRANSMITTING`: calculates remaining time for the data to be transmitted at the configured baudrate
 * - `..._GAININGENERGY`: calculates remaining time for the VBus to reach full energy level
 * - `..._SUSPENDED`: timeout is always `UINT32_MAX`
 * - `..._SUSPENDEDWITHTIMEOUT`: calculates remaining time for the encoder to automatically resume
 *
 * @param Encoder Encoder instance
 * @param pMicroseconds Pointer to store the timeout value into
 * @returns RESOLVBUS_OK if no error occurred
 */
RESOLVBUS_RESULT ResolVBus_LiveEncoder_GetTimeout(RESOLVBUS_LIVEENCODER *Encoder, uint32_t *pMicroseconds);

/**
 * Inform the encoder that a certain amount of time has passed.
 *
 * This function should be called "frequently" to update the internal state machine of the encoder:
 * - after the amount of time returned by `ResolVBus_LiveEncoder_GetTimeout` has passed
 * - after resuming the encoder using `ResolVBus_LiveEncoder_Resume`
 * - after sending VBus primitives using `ResolVBus_LiveEncoder_Send...`
 *
 * Whenever the internal state machine transitions into the IDLE phase, the encoder checks whether it can
 * automatically transition into another phase:
 * - if data has been queued in the transmit buffer it will emit a `TRANSMIT` event and transition into
 *   the `TRANSMITTING` phase
 * - if a suspend without a timeout has been requested while the encoder was not idle it will transition
 *   into the `SUSPENDED` phase now
 * - if a suspend with a timeout has been requested while the encoder was not idle it will transition
 *   into the `SUSPENDEDWITHTIMEOUT` phase now
 * - otherwise it emits an `IDLE` event and stays in the `IDLE` phase
 *
 * @param Encoder Encoder instance
 * @param Microseconds Amount of time passed since last call to this function
 * @returns RESOLVBUS_OK if no error occurred
 */
RESOLVBUS_RESULT ResolVBus_LiveEncoder_HandleTimer(RESOLVBUS_LIVEENCODER *Encoder, uint32_t Microseconds);

/**
 * Suspend the encoder indefinitely.
 *
 * If the encoder is idle, it will be suspended immediately.
 * If the encoder is not idle, it will complete the currently running operation first.
 * After that it will enter a suspended state indefinitely. The encoder must be manually resumed using
 * `ResolVBus_LiveEncoder_Resume` to continue operation.
 *
 * @param Encoder Encoder instance
 * @returns RESOLVBUS_OK if no error occurred
 */
RESOLVBUS_RESULT ResolVBus_LiveEncoder_Suspend(RESOLVBUS_LIVEENCODER *Encoder);

/**
 * Suspend the encoder for the provided time.
 *
 * If the encoder is not idle, it will complete the currently running operation first.
 * After that it will enter a suspended state for the provided time. After that time has passed, the
 * encoder will automatically resume operation again. The encoder can be manually
 * resumed before the timeout has passed using `ResolVBus_LiveEncoder_Resume`.
 *
 * @param Encoder Encoder instance
 * @param Microseconds Amount of time after which the encoder automatically resumes operation
 * @returns RESOLVBUS_OK if no error occurred
 */
RESOLVBUS_RESULT ResolVBus_LiveEncoder_SuspendWithTimeout(RESOLVBUS_LIVEENCODER *Encoder, uint32_t Microseconds);

/**
 * Resume operation of a suspended encoder.
 *
 * @param Encoder Encoder instance
 * @returns RESOLVBUS_OK if no error occurred
 */
RESOLVBUS_RESULT ResolVBus_LiveEncoder_Resume(RESOLVBUS_LIVEENCODER *Encoder);

/**
 * Queue a VBus packet header into the transmit buffer.
 *
 * See `ResolVBus_LiveEncoder_HandleTimer` for transmit buffer handling details.
 *
 * @param Encoder Encoder instance
 * @param DestinationAddress Destination address of the packet
 * @param SourceAddress Source address of the packet
 * @param MinorVersion Minor protocol version of the packet (major version will always be 0x10)
 * @param Command Command of the packet
 * @param FrameCount Frame count of the packet
 * @returns RESOLVBUS_OK if no error occurred
 */
RESOLVBUS_RESULT ResolVBus_LiveEncoder_QueuePacketHeader(RESOLVBUS_LIVEENCODER *Encoder, uint16_t DestinationAddress, uint16_t SourceAddress, uint8_t MinorVersion, uint16_t Command, uint8_t FrameCount);

/**
 * Queue a VBus packet frame into the transmit buffer.
 *
 * See `ResolVBus_LiveEncoder_HandleTimer` for transmit buffer handling details.
 *
 * @param Encoder Encoder instance
 * @param FourBytes Buffer containing the four bytes payload of the frame
 * @returns RESOLVBUS_OK if no error occurred
 */
RESOLVBUS_RESULT ResolVBus_LiveEncoder_QueuePacketFrame(RESOLVBUS_LIVEENCODER *Encoder, const uint8_t *FourBytes);

/**
 * Queue one or more VBus packet frames into the transmit buffer.
 *
 * If `Length` is not a multiple of four, the last frame is padded with 0xFF.
 *
 * See `ResolVBus_LiveEncoder_HandleTimer` for transmit buffer handling details.
 *
 * @param Encoder Encoder instance
 * @param Bytes Buffer containing the payload of the frames
 * @param Length Length of the `Bytes` buffer
 * @returns RESOLVBUS_OK if no error occurred
 */
RESOLVBUS_RESULT ResolVBus_LiveEncoder_QueuePacketFrames(RESOLVBUS_LIVEENCODER *Encoder, const void *Bytes, size_t Length);

/**
 * Queue a VBus datagram into the transmit buffer.
 *
 * See `ResolVBus_LiveEncoder_HandleTimer` for transmit buffer handling details.
 *
 * @param Encoder Encoder instance
 * @param DestinationAddress Destination address of the datagram
 * @param SourceAddress Source address of the datagram
 * @param MinorVersion Minor protocol version of the datagram (major version will always be 0x20)
 * @param Command Command of the datagram
 * @param Param16 16-bit parameter of the datagram
 * @param Param32 32-bit parameter of the datagram
 * @returns RESOLVBUS_OK if no error occurred
 */
RESOLVBUS_RESULT ResolVBus_LiveEncoder_QueueDatagram(RESOLVBUS_LIVEENCODER *Encoder, uint16_t DestinationAddress, uint16_t SourceAddress, uint8_t MinorVersion, uint16_t Command, uint16_t Param16, uint32_t Param32);

/**
 * Queue a VBus telegram header into the transmit buffer.
 *
 * See `ResolVBus_LiveEncoder_HandleTimer` for transmit buffer handling details.
 *
 * @param Encoder Encoder instance
 * @param DestinationAddress Destination address of the telegram
 * @param SourceAddress Source address of the telegram
 * @param MinorVersion Minor protocol version of the telegram (major version will always be 0x10)
 * @param Command Command of the telegram
 * @param FrameCount Frame count of the telegram
 * @returns RESOLVBUS_OK if no error occurred
 */
RESOLVBUS_RESULT ResolVBus_LiveEncoder_QueueTelegramHeader(RESOLVBUS_LIVEENCODER *Encoder, uint16_t DestinationAddress, uint16_t SourceAddress, uint8_t MinorVersion, uint8_t Command, uint8_t FrameCount);

/**
 * Queue a VBus telegram frame into the transmit buffer.
 *
 * See `ResolVBus_LiveEncoder_HandleTimer` for transmit buffer handling details.
 *
 * @param Encoder Encoder instance
 * @param SevenBytes Buffer containing the seven bytes payload of the frame
 * @returns RESOLVBUS_OK if no error occurred
 */
RESOLVBUS_RESULT ResolVBus_LiveEncoder_QueueTelegramFrame(RESOLVBUS_LIVEENCODER *Encoder, const uint8_t *SevenBytes);

/**
 * Queue one or more VBus telegram frames into the transmit buffer.
 *
 * If `Length` is not a multiple of seven, the last frame is padded with 0xFF.
 *
 * See `ResolVBus_LiveEncoder_HandleTimer` for transmit buffer handling details.
 *
 * @param Encoder Encoder instance
 * @param Bytes Buffer containing the payload of the frames
 * @param Length Length of the `Bytes` buffer
 * @returns RESOLVBUS_OK if no error occurred
 */
RESOLVBUS_RESULT ResolVBus_LiveEncoder_QueueTelegramFrames(RESOLVBUS_LIVEENCODER *Encoder, const void *Bytes, size_t Length);


/**
 * Initialize a `RESOLVBUS_LIVEDECODER` instance.
 *
 * Lifetime: `FrameDataBuffer` must outlive `Decoder`.
 *
 * @param Decoder Decoder instance to initialize
 * @param FrameDataBuffer Optional buffer to reconstruct frame data into
 * @param FrameDataBufferLength Length of the optional `FrameDataBuffer`
 * @param Handler Handler that is called when an event is emitted
 * @returns RESOLVBUS_OK if no error occurred
 */
RESOLVBUS_RESULT ResolVBus_LiveDecoder_Initialize(RESOLVBUS_LIVEDECODER *Decoder, void *FrameDataBuffer, size_t FrameDataBufferLength, RESOLVBUS_LIVEDECODERHANDLER Handler);

/**
 * Decode the provided bytes.
 *
 * This function decodes the provided bytes and emits events every time a valid VBus primitive is reconstructed.
 *
 * @param Decoder Decoder instance
 * @param Bytes Data to decode
 * @param Length Length of the `Bytes` buffer
 * @returns RESOLVBUS_OK if no error occurred
 */
RESOLVBUS_RESULT ResolVBus_LiveDecoder_Decode(RESOLVBUS_LIVEDECODER *Decoder, const uint8_t *Bytes, size_t Length);



#endif // __RESOLVBUS_H__
