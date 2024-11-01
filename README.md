# BACnet Client Example C++

A basic BACnet IP client example written in C++ using the [CAS BACnet Stack](https://store.chipkin.com/services/stacks/bacnet-stack). This client example is meant to be used with the [BACnet IP server example](https://github.com/chipkin/BACnetServerExampleCPP). Supports WhoIs, Read property, and Write property services.
This example shows how to use some of the BACnet client functionality of the CAS BACnet Stack and can be used to test communications with the BACnetServerExampleCPP.

## Supported CAS BACnet Stack Version

This example project uses version 5.1.2 of the CAS BACnet Stack

## Releases

Build versions of this example can be downloaded from the releases page:

[https://github.com/chipkin/BACnetClientExampleCPP/releases](https://github.com/chipkin/BACnetClientExampleCPP/releases)

## Installation

Download the latest release executable on the releases page.

## Usage

The target device for testing with this Client example application is the [BACnetServerExampleCPP](https://github.com/chipkin/BACnetServerExampleCPP)

```txt
Usage: BACnetClient {IPAddress}
Example: BACnetClient 192.168.1.126

Help:
- W - Send WhoIs message
- R - Send ReadProperty messages
- U - Send WriteProperty messages
- S - Send SubscribeCOV Request
- P - Send SubscribeCOVProperty Request
- M - Send ConfirmedTextMessage Request
- T - Send TimeSynchronization Request
- R - Send ReinitializedDevice Request
- D - Send DeviceCommunicationControl Request to Disable
- E - Send DeviceCommunicationControl Request to Enable
- H - Display Help information
- Q - Quit
```
Client expects a device with the following objects:

- Device: 389999 (Device Rainbow)
  - analog_input: 0            (AnalogInput Bronze)
  - analog_output: 1           (AnalogOutput Chartreuse)
  - analog_value: 2            (AnalogValue Diamond)
  - binary_input: 3            (BinaryInput Emerald)
  - binary_output: 4           (BinaryOutput Fuchsia)
  - binary_value: 5            (BinaryValue Gold)
  - multi_state_input: 13      (MultiStateInput Hot Pink)
  - multi_state_output: 14     (MultiStateOutput Indigo)
  - multi_state_value: 19      (MultiStateValue Kiwi)
  - trend_log: 20              (TrendLog Lilac)
  - bitstring_value: 39        (BitstringValue Magenta)
  - characterstring_value: 40  (CharacterstringValue Nickel)
  - data_value: 42             (DateValue Onyx)
  - integer_value: 45          (IntegerValue Purple)
  - large_analog_value: 46     (LargeAnalogValue Quartz)
  - octetstring_value: 47      (OctetstringValue Red)
  - positive_integer_value: 48 (PositiveIntegerValue Silver)
  - time_value: 50             (TimeValue Turquoise)
  - NetworkPort: 56            (NetworkPort Umber)

## Build

A [Visual studio 2022](https://visualstudio.microsoft.com/downloads/) project is included with this project. 

The CAS BACnet Stack submodule is required for compilation. Please contact Chipkin Automation Systems to purchase the CAS BACnet Stack.

More information about the CAS BACnet Stack can be found here: [CAS BACnet Stack](https://store.chipkin.com/services/stacks/bacnet-stack)

## Example Output
```txt
CAS BACnet Stack Client Example v1.0.0.0
https://github.com/chipkin/BACnetClientExampleCPP

FYI: Loading CAS BACnet Stack functions... OK
FYI: CAS BACnet Stack version: 5.1.2.0
FYI: Connecting UDP Resource to port=[47808]... OK, Connected to port
FYI: Registering the callback Functions with the CAS BACnet Stack
Setting up client device. device.instance=[389002]
Created Device.
Generated the connection string for the downstream device.
FYI: Entering main loop...
```