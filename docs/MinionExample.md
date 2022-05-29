# Minion Example

A minimal VBus minion is implemented in the `examples/Minion.c` example. It features:

- a LiveDecoder instance for decoding incoming VBus bytes
- a LiveEncoder instance for transmitting VBus primitives


## The `main` function

The `main` functoin:

- initializes both the LiveDecoder and LiveEnceder instances using the `__Initialize` function
- opens a serial port / network socket / ...
- enters a loop
    - reads from the serial port / network socket / ... with a small timeout / non-blocking
    - calculates the time passed since the last loop cycle
    - calls `__HandleLoopCycle` handing over that information for state machine handling


## Initialization

The `__Initialize` function:

- intializes both the LiveDecoder and LiveEncoder instances
- suspends the LiveEncoder (it will be resumed once it is supposed to transmit information)


## Handling state machine

The `__HandleLoopCycle` function:

- decodes received VBus bytes using the LiveDecoder, calling `__DecoderHandler` if an event occurred
- advances the LiveEncoder's state machine, calling `__EncoderHandler` if an event occurred


## Reacting to LiveDecoder events

The `__DecoderHandler` function is called whenever the LiveDecoder emits an event. This function can be used to react to requests from the VBus master. For that it must:

- first resume the suspended LiveEncoder to allow any `ResolVBus_LiveEncoder_Queue...` functions to be called
- call the appropriate `ResolVBus_LiveEncoder_Queue...` function
- suspend the LiveEncoder again to wait for the next request


## Reacting to LiveEncoder events

The `__EncoderHandler` function is called whenever the LiveEncoder emits an event:

- `RESOLVBUS_LIVEENCODEREVENTTYPE_IDLE` should never be emitted because the LiveEncoder is kept in a suspended state most of the time
- `RESOLVBUS_LIVEENCODEREVENTTYPE_TRANSMIT` must be used to transmit the provided bytes over the serial port / network socket / ...
