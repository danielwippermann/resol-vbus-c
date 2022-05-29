# LiveDecoder

The "LiveDecoder" module implements the `RESOLVBUS_LIVEDECODER` type and its associated functions, which can be used to parse a stream of over-the-wire VBus bytes into their respective primitives (like e.g. packets).


## Initialization and integration

Before the decoder can be used, it must be initialized by calling `ResolVBus_LiveDecoder_Initialize`:

```
// ...
RESOLVBUS_LIVEDECODER Decoder = RESOLVBUS_LIVEDECODER_INITIALIZER;
uint8_t FrameDataBuffer [512] = { 0 };
if (Result == RESOLVBUS_OK) {
    Result = ResolVBus_LiveDecoder_Initialize(Decoder, FrameDataBuffer, sizeof (FrameDataBuffer), LiveDecoderEventHandler);
}
// ...
```

The function takes four arguments: the decoder instance itself, an optional buffer for holding decoded frame data as well as its length and an event handler function.

The optional frame data buffer is used to reassemble the contents of multiple VBus packet or telegram frames. If no buffer is provided, the reassembly must be done manually.

The event handler is a function that is called whenever the decoder wants to inform its user about an event:

- `RESOLVBUS_LIVEDECODEREVENTTYPE_PACKETHEADER`: a VBus version 1.x packet header has been decoded
- `RESOLVBUS_LIVEDECODEREVENTTYPE_PACKETFRAME`: a VBus version 1.x packet frame has been decoded
- `RESOLVBUS_LIVEDECODEREVENTTYPE_PACKETEND`: a VBus version 1.x packet (consisting of the header and all associated frames) has been decoded
- `RESOLVBUS_LIVEDECODEREVENTTYPE_DATAGRAM`: a VBus version 2.x datagram has been decoded
- `RESOLVBUS_LIVEDECODEREVENTTYPE_TELEGRAMHEADER`: a VBus version 3.x telegram header has been decoded
- `RESOLVBUS_LIVEDECODEREVENTTYPE_TELEGRAMFRAME`: a VBus version 3.x telegram frame has been decoded
- `RESOLVBUS_LIVEDECODEREVENTTYPE_TELEGRAMEND`: a VBus version  3.x telegram (consisting of the header and all associated frames) has been decoded

The event handler is called from the `ResolVBus_LiveDecoder_Decode` function which should be called whenever VBus bytes have been received.
