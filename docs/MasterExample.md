# Master example

A minimal VBus master is implemented in the `examples/Master.c` example. It features:

- a LiveDecoder instance for decoding incoming VBus bytes
- a LiveEncoder instance for transmitting VBus primitives
- a minimal state machine for repeatedly transmitting information


## The `main` function

The `main` function:

- initializes both the LiveDecoder and LiveEncoder instances using the `__Initialize` function
- opens a serial port / network socket / ...
- enters a loop
    - reads from the serial port / network socket / ... with a small timeout / non-blocking
    - calculates the time passed since the last loop cycle
    - decides whether it would be a appropriate time to start sending information (e.g. once every second)
    - calls `__HandleLoopCycle` handing over that information for state machine handling


## Initialization

The `__Initialize` function:

- intializes both the LiveDecoder and LiveEncoder instances
- suspends the LiveEncoder (it will be resumed once it is supposed to transmit information)


## Handling state machine

The `__HandleLoopCycle` function:

- starts the internal state machine if requested and possible (because the LiveEncoder is idle)
- decodes received VBus bytes using the LiveDecoder, calling `__DecoderHandler` if an event occurred
- advances the LiveEncoder's state machine, calling `__EncoderHandler` if an event occurred


## Reacting to LiveDecoder events

The `__DecoderHandler` function is called whenever the LiveDecoder emits an event. This function can be used to store received information from VBus minions.


## Reacting to LiveEncoder events

The `__EncoderHandler` function is called whenever the LiveEncoder emits an event:

- `RESOLVBUS_LIVEENCODEREVENTTYPE_IDLE` can be used to advance the internal state machine
- `RESOLVBUS_LIVEENCODEREVENTTYPE_TRANSMIT` must be used to transmit the provided bytes over the serial port / network socket / ...
