# Change Log

## Version 1.0.x

### 1.0.0 (2024-Oct-29)

- Updated example to use with BACnetStack version 5.1.2.0
- Added TimeSynchronization example
- Added ReinitializeDevice example
- Added DeviceCommunicationControl example

## Version 0.0.x

### 0.0.9 (2023-Oct-12)

- Updated preprocessor macro to include ENABLE_STRING_LITERALS, ENABLE_DECODE_AS_JSON, ENABLE_DECODE_AS_XML, ENABLE_BACNET_API_DEBUG_LOGGING, ENABLE_ALL_OBJECT_TYPES, ENABLE_ALL_BIBBS
- Updated ReadProperty to read for device instance 389999, which is the instance for the device programmed in the BACnetServerExampleCPP project
- Tested using BACnetStack version 4.1.19.0

### 0.0.8 (2023-Sep-29)

- Updated preprocessor macro to include ENABLE_DATA_LINK_LAYER_IPV4
- Updated Windows SDK version [Issues/35](https://github.com/chipkin/BACnetServerExampleCPP/issues/35)
- Updated the project files for the CASBACnetStack to the latest version [Issues/37](https://github.com/chipkin/BACnetServerExampleCPP/issues/37)
- Tested using BACnetStack version 4.1.19.0

### 0.0.7 (2022-Aug-11)

- Updated to CAS BACnet Stack version 4.1.5

### 0.0.6 (2021-Jul-15)

- Added confirmed text message request example service

### 0.0.5 (2021-Jun-16)

- Added subscribe COV request example service

### 0.0.4 (2021-May-19)

- Updated to CAS BACnet Stack version 3.22.3

### 0.0.3 (2020-Sep-25)

- Updated to CAS BACnet Stack version 3.19.0

### 0.0.2 (2020-Mar-12)

- Updated to allow for user to specifiy the downstream device IP address via the command line.

### 0.0.1 (2020-Mar-03)

- Inital release.
