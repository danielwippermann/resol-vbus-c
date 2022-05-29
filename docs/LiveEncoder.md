# LiveEncoder

The "LiveEncoder" module implements the `RESOLVBUS_LIVEENCODER` type and its associated functions, which can be used to convert VBus primitives (like e.g. packets) into their over-the-wire representation.

In addition to that it also supports energy management functions and timeout handling.


## Initialization and integration

Before the encoder can be used, it must be initialized by calling `ResolVBus_LiveEncoder_Initialize`:

```
// ...
RESOLVBUS_LIVEENCODER Encoder = RESOLVBUS_LIVEENCODER_INITIALIZER;
uint8_t Buffer [512] = { 0 };
if (Result == RESOLVBUS_OK) {
    Result = ResolVBus_LiveEncoder_Initialize(&Encoder, Buffer, sizeof (Buffer), LiveEncoderEventHandler));
}
// ...
```

The function takes four arguments: the encoder instance itself, a transmission buffer as well as its length and an event handler function.

The transmission buffer is used by the `ResolVBus_LiveEncoder_Queue...` functions to store the over-the-wire representation of the VBus primitives. It must be large enough to hold the largest VBus primitive that is expected to be transmitted.

The event handler is a function that is called whenever the encoder wants to inform its user about an event:

- `RESOLVBUS_LIVEENCODEREVENTTYPE_IDLE`: the encoder is idle and would accept queueing VBus primitives for transmission
- `RESOLVBUS_LIVEENCODEREVENTTYPE_TRANSMIT`: the encoder wants to transmit a chunk of a VBus primitive

The event handler is called from the event pump implemented in `ResolVBus_LiveEncoder_HandleTimer`. This function must be called "frequently" (see its documentation for details). The function takes the time that has passed since the last call as its second argument. That time is used to drive forward the internal state machine.


## The state machine and its phases

The encoder contains a state machine that can be in one of the following phases:

- `RESOLVBUS_LIVEENCODERPHASE_IDLE`: nothing to do yet, can be used to queue VBus primitives for transmission, emits `RESOLVBUS_LIVEENCODEREVENTTYPE_IDLE` event
- `RESOLVBUS_LIVEENCODERPHASE_TRANSMITTING`: currently transmitting VBus data, emits `RESOLVBUS_LIVEENCODEREVENTTYPE_TRANSMIT` event
- `RESOLVBUS_LIVEENCODERPHASE_GAININGENERGY`: used to regain energy after transmitting VBus data
- `RESOLVBUS_LIVEENCODERPHASE_SUSPENDED`: suspended until manually resumed
- `RESOLVBUS_LIVEENCODERPHASE_SUSPENDEDWITHTIMEOUT`: suspended until manually resumed or timeout elapses


## Queuing VBus primitives for transmission

The encoder provides several `ResolVBus_LiveEncoder_Queue...` functions to queue VBus primitives into its internal transmit buffer. On the next call to `ResolVBus_LiveEncoder_HandleTimer` the actual transmission is started.

Queuing VBus primitives is only allowed while the encoder is idle. If the encoder is used in a VBus master context, it is recommended to queue VBus primitives for the `RESOLVBUS_LIVEENCODEREVENTTYPE_IDLE` event handling code. If the encoder is used in a VBus minion context, it is recommended to manually resume the encoder from its `RESOLVBUS_LIVEENCODERPHASE_SUSPENDED` phase before actually queueing the response.


## Energy management

The VBus transports both information and electrical energy. Since transmitting information disrupts transporting energy, care must be taken to not transmit too much data at once. For that purpose the encoder implements a basic energy management functionality.

That functionality can be configured using three configuration fields:

- `MaxEnergy`: maximum amount of energy that can be consume before a transmission must be segmented into multiple parts
- `EnergyLossPerByte`: assumed loss of energy per transmitted byte
- `EnergyGainPerUs`: assumed gain of energy per microsecond in the `RESOLVBUS_LIVEENCODERPHASE_GAININGENERGY` phase

When the encoder is in its idle phase the energy level is assumed to be at its maximum capacity. When data is queued into the transmit buffer, the encoder switches into its `RESOLVBUS_LIVEENCODERPHASE_TRANSMITTING` phase. It then transmits as many bytes as possible, summing up the energy loss accordingly, but not exceeding the `MaxEnergy` value. After the bytes have been transmitted the encoder switches into the `RESOLVBUS_LIVEENCODERPHASE_GAININGENERGY` phase, waiting for the consumed energy to refill. Once the energy level is back at full capacity the encoder either continues transmitting the remaining bytes from the buffer or switches into one of the suspended or idle phases.

Setting `MaxEnergy` to zero disables the energy management functionality.


## Suspension and timeout handling

The VBus uses a single-master topology. Only the master is allowed to transmit at any time. Minions attached to the VBus must not transmit unless they have been requested to do so by the master. In those situations the master has to grant the minion a predefined time period to transmit its reply.

The encoder supports both the VBus master and VBus minion contexts.

In a VBus master context the user of the encoder can queue a VBus request primitive and then use `ResolVBus_LiveEncoder_SuspendWithTimeout` to request that the encoder enters a suspended state for a predefined period of time after the request was transmitted, giving the minion enough time to respond to the request.

In a VBus minion context the user of the encoder can immediately use `ResolVBus_LiveEncoder_Suspend` to switch the encoder into a suspended state for an indefinite period of time. Once the request has been received, the encoder can be manually resumed using `ResolVBus_LiveEncoder_Resume`, the reply can be queued for transmission and then `ResolVBus_LiveEncoder_Suspend` can be used to request entering the suspended state again after the transmission.
