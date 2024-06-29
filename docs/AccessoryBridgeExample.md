# AccessoryBridge example

An example application that queries information from unsupported accessories and publishes their information on the DL2 Plus's virtual VBus. It features:

- a LiveTransceiver for decoding and encoding data on the physical VBus #1
- a LiveEncoder for encoding summary information on the virtual VBus (#255)
- an example state machine for getting measurement from the "DeltaTherm E sensor XL" (https://www.resol.de/en/produktdetail/251) module on a controller that does not directly support it

All OS-specific parts like opening connections to the DL2 Plus and performing non-blocking I/O are not implemented, but just marked with a respective comment.


## The `main` function

The `main` function:

- initializes necessary components in the `__Initialize` function
- opens connections to the DL2 Plus (not implemented)
- enters a loop
    - reads from the DL2 Plus connection with a small timeout / non-blocking
    - calculates the time passed since the last loop cycle
    - calls `__HandleLoopCycle` handing over that information for state machine handling


## Initialization

The `__Initialize` function:

- initializes the LiveTransceiver for the physical VBus (`VBusP`)
- sets the VBusP state machine to `WAITINGFORFREEBUS`
- initializes the LiveEncoder for the virtual VBus (`VBusV`)
- suspends the VBusV LiveEncoder (will be resumed manually later)


## Handling state machine

The `__HandleLoopCycle` function:

- handles time-based state transitions based on the time passed since the last loop cycle
- decodes the data received from the DL2 Plus connection


## Reacting to VBusP LiveTransceiver events

The `__VBusPLiveTransceiverHandler` function is called whenever the LiveTransceiver connected to the physical VBus emits an event:

- `RESOLVBUS_LIVETRANSCEIVEREVENTTYPE_ENCODER`  with `RESOLVBUS_LIVEENCODEREVENTTYPE_TRANSMIT` must be implemented to transmit the provided bytes over the physical VBus using the connection to the DL2 Plus


## Reacting to VBusV LiveEncoder events

The `__VBusVLiveEncoderHandler` function is called whenever the LiveEncoder connected to the virtual VBus emits an event:

- `RESOLVBUS_LIVEENCODEREVENTTYPE_TRANSMIT` must be implemented to transmit the provided bytes over the virtual VBus using the connection to the DL2 Plus


## The VBusP state machine

To read out the measurement values of a "DeltaTherm E sensor XL" module on a controller that does not directly support it, several steps need to be performed:

- wait for the controller to send its bus offer datagram (command 0x0500)
- wait for a short duration to rule out that any other bus participant (like a RPT) would like to use that bus offer
- send a "get changeset ID" datagram to the controller to signal, that bus control should be handed over
- once the controller has replied to that datagram, control over the VBus has been handed over for at least 10 seconds (or until releases manually)
- optionally send configuration packets to the sensor module if necessary and wait for their responses
- send a measurement request packet to the sensor module
- once the measurement response packet is received, perform some calculation/conversion to the measurement value and send the result in a packet over the virtual VBus
- send a release bus datagram (command 0x0600) to the controller to hand back bus control
- repeat waiting for another bus offer datagram to repeat that loop

This example state machine is realized in a few components:

- a `__PHASE` enum for the aforementioned states
- the `__PerformVBusPPhaseAction` function that starts the necessary action based on the current state
- the `__VBusPLiveTransceiverActionHandler` function which is called when a given state receives a reply or times out
- a few helper function that actually cosntruct and send the necessary packets for several of the states

The `__PHASE` enum looks like this:

```
typedef enum __PHASE {
    __PHASE_UNKNOWN,
    __PHASE_WAITFORFREEBUS,
    __PHASE_WAITFORANYDATA,
    __PHASE_GETCHANGESETID,
    __PHASE_SENDPACKET1,
    __PHASE_SENDPACKET2,
    __PHASE_SENDPACKET3,
    __PHASE_RELEASEBUS,
} __PHASE;
```

