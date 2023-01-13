# LiveTransceiver

The "LiveTransceiver" module implements the `RESOLVBUS_LIVETRANSCEIVER` type and its associated functions, which combines the functionalities of the "LiveEncoder" and "LiveDecoder" modules to communicate with a VBus controller, e.g. to access its parameters.


## Initialization and integration

Before the transceiver can be used, it must be initialized by calling `ResolVBus_LiveTransceiver_Initialize`:

```
// ...
RESOLVBUS_LIVETRANSCEIVER Transceiver = RESOLVBUS_LIVETRANSCEIVER_INITIALIZER;
uint8_t EncoderBuffer [512];
uint8_t DecoderBuffer [512];
if (Result == RESOLVBUS_OK) {
    Result = ResolVBus_LiveTransceiver_Initialize(Transceiver, EncoderBuffer, sizeof (EncoderBuffer), DecoderBuffer, sizeof (DecoderBuffer), LiveTransceiverHandler);
}
// ...
```

The function takes six arguments: the transceiver instance itself, a transmission buffer as well as its length, an optional buffer for holding decoded frame data as well as its length and an event handler function.

The transmission buffer is used by the `ResolVBus_LiveEncoder_Queue...` functions to store the over-the-wire representation of the VBus primitives. It must be large enough to hold the largest VBus primitive that is expected to be transmitted.

The optional decoder buffer is used to reassemble the contents of multiple VBus packet or telegram frames. If no buffer is provided, the reassembly must be done manually.

The event handler is a function that is called whenever the transceiver wants to inform its user about an event:

- `RESOLVBUS_LIVETRANSCEIVEREVENTTYPE_TIMEOUT`: the action started on the transceiver has timed out (optionally after several retries)
- `RESOLVBUS_LIVETRANSCEIVEREVENTTYPE_ENCODER`: the "LiveEncoder" part of the transceiver has emitted an event (e.g. to transmit data)
- `RESOLVBUS_LIVETRANSCEIVEREVENTTYPE_DECODER`:  the "LiveDecoder" part of the transceiver has emitted an event (e.g. from receiving VBus primitives)

The event handler can be called from multiple sources:

- from the event pump implemented in `ResolVBus_LiveTransceiver_HandleTimer`: this function must be called "frequently" (see its documentation for details). The function takes the time that has passed since the last call as its second argument. That time is used to drive forward the internal state machine.
- from the decoding state machine in `ResolVBus_LiveTransceiver_Decode`: this function must be called for every chunk of VBus bytes. If a VBus primitive can be decoded from those bytes, an event is emitted.


## Accessing the "LiveEncoder"

To queue VBus primitives to be transmitted the user can access the "LiveEncoder" part using the `ResolVBus_LiveTransceiver_GetEncoder` function.


## Accessing the "LiveDecoder"

To decode a chunk of bytes received, those bytes have to be passed to the `ResolVBus_LiveTransceiver_Decode` function.


## Performing an action

To communicate with a VBus controller an action can be started on the "LiveTransceiver". Those actions can:

- optionally transmit VBus primitives (requests)
- wait for reception of VBus primitives (requests or responses)
- have an optional timeout and retry managment

The following actions are currently implemented:

- `ResolVBus_LiveTransceiver_WaitForFreeBus`: wait for a VBus datagram with command 0x0500, indicating that the VBus controller offers control over the VBus timing to another device
- `ResolVBus_LiveTransceiver_ReleaseBus`: return VBus timing control back to the VBus controller
- `ResolVBus_LiveTransceiver_GetValueById`: get a value from the controller
- `ResolVBus_LiveTransceiver_SetValueById`: set a value in the controller
- `ResolVBus_LiveTransceiver_GetValueIdByIdHash`: lookup a value's ID by the corresponding ID hash

All those actions take multiple arguments:

- the transceiver instance
- zero or more action related arguments like VBus controller address, value ID, etc.
- an optional `RESOLVBUS_LIVETRANSCEIVEROPTIONS` pointer, if custom timeout and/or retry options should be used
- a function pointer to report completion or timeout back to the caller
