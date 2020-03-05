# BACnet Client Example CPP

A basic BACnet IP client example written with C++ using the [CAS BACnet Stack](https://www.bacnetstack.com/). This client example is ment to be used with the [BACnet IP server example](https://github.com/chipkin/BACnetServerExampleCPP) and expects a device with the following objects.

- Device: 389999 (Device Rainbow)
  - analog_input: 0  (AnalogInput Bronze)
  - analog_output: 1  (AnalogOutput Chartreuse)
  - analog_value: 2  (AnalogValue Diamond)
  - binary_input: 3  (BinaryInput Emerald)
  - binary_output: 4  (BinaryOutput Fuchsia)
  - binary_value: 5  (BinaryValue Gold)
  - multi_state_input: 13  (MultiStateInput Hot Pink)
  - multi_state_output: 14  (MultiStateOutput Indigo)
  - multi_state_value: 19  (MultiStateValue Kiwi)

## Building

A [Visual studio 2019](https://visualstudio.microsoft.com/downloads/) project is included with this project.

This project also auto built using [Gitlab CI](https://docs.gitlab.com/ee/ci/) on every commit.

## Releases

Build versions of this example can be downloaded from the releases page:

[https://github.com/chipkin/BACnetClientExampleCPP/releases](https://github.com/chipkin/BACnetClientExampleCPP/releases)
