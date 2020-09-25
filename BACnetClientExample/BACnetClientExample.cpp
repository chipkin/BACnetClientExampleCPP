/*
 * BACnet Client Example C++
 * ----------------------------------------------------------------------------
 * BACnetClientExample.cpp
 * 
 * A basic BACnet IP client example written with C++ using the CAS BACnet Stack.
 *
 * More information https://github.com/chipkin/BACnetClientExampleCPP
 * 
 * This file contains the 'main' function. Program execution begins and ends there.
 * 
 * Created by: Steven Smethurst
*/

#include <iostream>

#include "CASBACnetStackAdapter.h"
#include "CASBACnetStackExampleConstants.h"
#include "CIBuildSettings.h"

// Helpers 
#include "SimpleUDP.h"
#include "ChipkinEndianness.h"
#include "ChipkinConvert.h"
#include "ChipkinUtilities.h"

#include <iostream>
#ifndef __GNUC__ // Windows
#include <conio.h> // _kbhit
#else // Linux 
#include <sys/ioctl.h>
#include <termios.h>
bool _kbhit() {
	termios term;
	tcgetattr(0, &term);
	termios term2 = term;
	term2.c_lflag &= ~ICANON;
	tcsetattr(0, TCSANOW, &term2);
	int byteswaiting;
	ioctl(0, FIONREAD, &byteswaiting);
	tcsetattr(0, TCSANOW, &term);
	return byteswaiting > 0;
}
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
void Sleep(int milliseconds) {
	usleep(milliseconds * 1000);
}

#endif // __GNUC__


// Globals
// =======================================
CSimpleUDP g_udp; // UDP resource

uint8_t downstreamConnectionString[6];
uint8_t invokeId;


// Constants
// =======================================
const std::string APPLICATION_VERSION = "0.0.3";  // See CHANGELOG.md for a full list of changes.
const uint32_t MAX_XML_RENDER_BUFFER_LENGTH = 1024 * 20;

// Settings 
// =======================================
const uint16_t SETTING_BACNET_IP_PORT = 47808; 
const uint32_t SETTING_CLIENT_DEVICE_INSTANCE = 389002; 
const uint16_t SETTING_DOWNSTREAM_DEVICE_PORT = SETTING_BACNET_IP_PORT;
const uint32_t SETTING_DOWNSTREAM_DEVICE_INSTANCE = 389999; 
const std::string SETTING_DEFAULT_DOWNSTREAM_DEVICE_IP_ADDRESS = "192.168.1.126";

// Downstream IP Initialization
// =======================================
std::string downstream_Device_ip_address;


// Callback Functions to Register to the DLL
// Message Functions
uint16_t CallbackReceiveMessage(uint8_t* message, const uint16_t maxMessageLength, uint8_t* receivedConnectionString, const uint8_t maxConnectionStringLength, uint8_t* receivedConnectionStringLength, uint8_t* networkType);
uint16_t CallbackSendMessage(const uint8_t* message, const uint16_t messageLength, const uint8_t* connectionString, const uint8_t connectionStringLength, const uint8_t networkType, bool broadcast);

// System Functions
time_t CallbackGetSystemTime();


// Hooks for unconfirmed requests
void HookIAm(const uint32_t deviceIdentifier, const uint32_t maxApduLengthAccepted, const uint8_t segmentationSupported, const uint16_t vendorIdentifier, const uint8_t* connectionString, const uint8_t connectionStringLength, const uint8_t networkType, const uint16_t network, const uint8_t* sourceAddress, const uint8_t sourceAddressLength);
void HookIHave(const uint32_t deviceIdentifier, const uint16_t objectType, const uint32_t objectInstance, const char* objectName, const uint32_t objectNameLength, const uint8_t objectNameEncoding, const uint8_t* connectionString, const uint8_t connectionStringLength, const uint8_t networkType, const uint16_t network, const uint8_t* sourceAddress, const uint8_t sourceAddressLength);

// Hooks for simple PDUs
void HookError(const uint8_t originalInvokeId, const uint32_t errorChoice, const uint32_t errorClass, const uint32_t errorCode, const uint8_t* connectionString, const uint8_t connectionStringLength, const uint8_t networkType, const uint16_t network, const uint8_t* sourceAddress, const uint8_t sourceAddressLength, const bool useObjectProperty, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier);
void HookReject(const uint8_t originalInvokeId, const uint32_t rejectReason, const uint8_t* connectionString, const uint8_t connectionStringLength, const uint8_t networkType, const uint16_t network, const uint8_t* sourceAddress, const uint8_t sourceAddressLength);
void HookAbort(const uint8_t originalInvokeId, const bool sentByServer, const uint32_t abortReason, const uint8_t* connectionString, const uint8_t connectionStringLength, const uint8_t networkType, const uint16_t network, const uint8_t* sourceAddress, const uint8_t sourceAddressLength);
void HookSimpleAck(const uint8_t originalInvokeId, const uint32_t serverAckChoice, const uint8_t* connectionString, const uint8_t connectionStringLength, const uint8_t networkType, const uint16_t network, const uint8_t* sourceAddress, const uint8_t sourceAddressLength);
void HookTimeout(const uint8_t originalInvokeId, const uint8_t* connectionString, const uint8_t connectionStringLength, const uint8_t networkType, const uint16_t network, const uint8_t* sourceAddress, const uint8_t sourceAddressLength);

// Hooks for different ComplexAck PDUs
void HookPropertyBitString(const uint32_t originalInvokeId, const uint8_t service, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, const bool usePropertyArrayIndex, const uint32_t propertyArrayIndex, const bool* value, const uint32_t length, const uint8_t* connectionString, const uint8_t connectionStringLength, const uint8_t networkType, const uint16_t network, const uint8_t* sourceAddress, const uint8_t sourceAddressLength);
void HookPropertyBool(const uint32_t originalInvokeId, const uint8_t service, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, const bool usePropertyArrayIndex, const uint32_t propertyArrayIndex, const bool value, const uint8_t* connectionString, const uint8_t connectionStringLength, const uint8_t networkType, const uint16_t network, const uint8_t* sourceAddress, const uint8_t sourceAddressLength);
void HookPropertyCharString(const uint32_t originalInvokeId, const uint8_t service, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, const bool usePropertyArrayIndex, const uint32_t propertyArrayIndex, const char* value, const uint32_t length, const uint8_t encoding, const uint8_t* connectionString, const uint8_t connectionStringLength, const uint8_t networkType, const uint16_t network, const uint8_t* sourceAddress, const uint8_t sourceAddressLength);
void HookPropertyDate(const uint32_t originalInvokeId, const uint8_t service, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, const bool usePropertyArrayIndex, const uint32_t propertyArrayIndex, const uint8_t year, const uint8_t month, const uint8_t day, const uint8_t weekday, const uint8_t* connectionString, const uint8_t connectionStringLength, const uint8_t networkType, const uint16_t network, const uint8_t* sourceAddress, const uint8_t sourceAddressLength);
void HookPropertyDouble(const uint32_t originalInvokeId, const uint8_t service, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, const bool usePropertyArrayIndex, const uint32_t propertyArrayIndex, const double value, const uint8_t* connectionString, const uint8_t connectionStringLength, const uint8_t networkType, const uint16_t network, const uint8_t* sourceAddress, const uint8_t sourceAddressLength);
void HookPropertyEnum(const uint32_t originalInvokeId, const uint8_t service, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, const bool usePropertyArrayIndex, const uint32_t propertyArrayIndex, const uint32_t value, const uint8_t* connectionString, const uint8_t connectionStringLength, const uint8_t networkType, const uint16_t network, const uint8_t* sourceAddress, const uint8_t sourceAddressLength);
void HookPropertyNull(const uint32_t originalInvokeId, const uint8_t service, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, const bool usePropertyArrayIndex, const uint32_t propertyArrayIndex, const uint8_t* connectionString, const uint8_t connectionStringLength, const uint8_t networkType, const uint16_t network, const uint8_t* sourceAddress, const uint8_t sourceAddressLength);
void HookPropertyObjectIdentifier(const uint32_t originalInvokeId, const uint8_t service, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, const bool usePropertyArrayIndex, const uint32_t propertyArrayIndex, const uint16_t objectTypeValue, const uint32_t objectInstanceValue, const uint8_t* connectionString, const uint8_t connectionStringLength, const uint8_t networkType, const uint16_t network, const uint8_t* sourceAddress, const uint8_t sourceAddressLength);
void HookPropertyOctString(const uint32_t originalInvokeId, const uint8_t service, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, const bool usePropertyArrayIndex, const uint32_t propertyArrayIndex, const uint8_t* value, const uint32_t length, const uint8_t* connectionString, const uint8_t connectionStringLength, const uint8_t networkType, const uint16_t network, const uint8_t* sourceAddress, const uint8_t sourceAddressLength);
void HookPropertyInt(const uint32_t originalInvokeId, const uint8_t service, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, const bool usePropertyArrayIndex, const uint32_t propertyArrayIndex, const int32_t value, const uint8_t* connectionString, const uint8_t connectionStringLength, const uint8_t networkType, const uint16_t network, const uint8_t* sourceAddress, const uint8_t sourceAddressLength);
void HookPropertyReal(const uint32_t originalInvokeId, const uint8_t service, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, const bool usePropertyArrayIndex, const uint32_t propertyArrayIndex, const float value, const uint8_t* connectionString, const uint8_t connectionStringLength, const uint8_t networkType, const uint16_t network, const uint8_t* sourceAddress, const uint8_t sourceAddressLength);
void HookPropertyTime(const uint32_t originalInvokeId, const uint8_t service, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, const bool usePropertyArrayIndex, const uint32_t propertyArrayIndex, const uint8_t hour, const uint8_t minute, const uint8_t second, const uint8_t hundrethSecond, const uint8_t* connectionString, const uint8_t connectionStringLength, const uint8_t networkType, const uint16_t network, const uint8_t* sourceAddress, const uint8_t sourceAddressLength);
void HookPropertyUInt(const uint32_t originalInvokeId, const uint8_t service, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, const bool usePropertyArrayIndex, const uint32_t propertyArrayIndex, const uint32_t value, const uint8_t* connectionString, const uint8_t connectionStringLength, const uint8_t networkType, const uint16_t network, const uint8_t* sourceAddress, const uint8_t sourceAddressLength);

// Helper functions 
bool DoUserInput();
void WaitForResponse(unsigned int timeout=3); 
void HelperPrintCommonHookParameters(const uint8_t* connectionString, const uint8_t connectionStringLength, const uint8_t networkType, const uint16_t network, const uint8_t* sourceAddress, const uint8_t sourceAddressLength);
void HelperPrintCommonHookPropertyParameters(const uint32_t originalInvokeId, const uint8_t service, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, const bool usePropertyArrayIndex, const uint32_t propertyArrayIndex);

// Example Services
void ExampleWhoIs();
void ExampleReadProperty(); 
void ExampleWriteProperty();

int main(int argc, char ** argv )
{
	// Print the application version information 
	std::cout << "CAS BACnet Stack Server Example v" << APPLICATION_VERSION << "." << CIBUILDNUMBER << std::endl;
	std::cout << "https://github.com/chipkin/BACnetServerExampleCPP" << std::endl << std::endl;


	// Load the arguments
	downstream_Device_ip_address = SETTING_DEFAULT_DOWNSTREAM_DEVICE_IP_ADDRESS; 
	if (argc >= 2) {
		downstream_Device_ip_address = argv[1];
		std::cout << "FYI: Using " << downstream_Device_ip_address << " for the downstream device IP address" << std::endl;
	}


	// 1. Load the CAS BACnet stack functions
	// ---------------------------------------------------------------------------
	std::cout << "FYI: Loading CAS BACnet Stack functions... ";
	if (!LoadBACnetFunctions()) {
		std::cerr << "Failed to load the functions from the DLL" << std::endl;
		return 0;
	}
	std::cout << "OK" << std::endl;
	std::cout << "FYI: CAS BACnet Stack version: " << fpGetAPIMajorVersion() << "." << fpGetAPIMinorVersion() << "." << fpGetAPIPatchVersion() << "." << fpGetAPIBuildVersion() << std::endl;

	// 2. Connect the UDP resource to the BACnet Port
	// ---------------------------------------------------------------------------
	std::cout << "FYI: Connecting UDP Resource to port=[" << SETTING_BACNET_IP_PORT << "]... ";
	if (!g_udp.Connect(SETTING_BACNET_IP_PORT)) {
		std::cerr << "Failed to connect to UDP Resource" << std::endl;
		std::cerr << "Press any key to exit the application..." << std::endl;
		(void)getchar();
		return -1;
	}
	std::cout << "OK, Connected to port" << std::endl;


	// 3. Setup the callbacks
	// ---------------------------------------------------------------------------
	std::cout << "FYI: Registering the callback Functions with the CAS BACnet Stack" << std::endl;

	// Message Callback Functions
	fpRegisterCallbackReceiveMessage(CallbackReceiveMessage);
	fpRegisterCallbackSendMessage(CallbackSendMessage);

	// System Time Callback Functions
	fpRegisterCallbackGetSystemTime(CallbackGetSystemTime);

	// Non-confirmed responses 
	fpRegisterHookIAm(HookIAm);
	fpRegisterHookIHave(HookIHave);

	// Hooks for simple PDUs
	fpRegisterHookError(HookError);
	fpRegisterHookReject(HookReject);
	fpRegisterHookAbort(HookAbort);
	fpRegisterHookSimpleAck(HookSimpleAck);
	fpRegisterHookTimeout(HookTimeout);

	// Hooks for different ComplexAck PDUs
	fpRegisterHookPropertyBitString(HookPropertyBitString);
	fpRegisterHookPropertyBool(HookPropertyBool);
	fpRegisterHookPropertyCharString(HookPropertyCharString);
	fpRegisterHookPropertyDate(HookPropertyDate);
	fpRegisterHookPropertyDouble(HookPropertyDouble);
	fpRegisterHookPropertyEnum(HookPropertyEnum);
	fpRegisterHookPropertyNull(HookPropertyNull);
	fpRegisterHookPropertyObjectIdentifier(HookPropertyObjectIdentifier);
	fpRegisterHookPropertyOctString(HookPropertyOctString);
	fpRegisterHookPropertyInt(HookPropertyInt);
	fpRegisterHookPropertyReal(HookPropertyReal);
	fpRegisterHookPropertyTime(HookPropertyTime);
	fpRegisterHookPropertyUInt(HookPropertyUInt);

	// 4. Setup the BACnet device
	// ---------------------------------------------------------------------------
	std::cout << "Setting up client device. device.instance=[" << SETTING_CLIENT_DEVICE_INSTANCE << "]" << std::endl;

	// Create the Device
	if (!fpAddDevice(SETTING_CLIENT_DEVICE_INSTANCE)) {
		std::cerr << "Failed to add Device." << std::endl;
		return false;
	}
	std::cout << "Created Device." << std::endl;

	// Enable services supported by this device 
	fpSetServiceEnabled(SETTING_CLIENT_DEVICE_INSTANCE, CASBACnetStackExampleConstants::SERVICE_I_AM, true);
	fpSetServiceEnabled(SETTING_CLIENT_DEVICE_INSTANCE, CASBACnetStackExampleConstants::SERVICE_I_HAVE, true);
	fpSetServiceEnabled(SETTING_CLIENT_DEVICE_INSTANCE, CASBACnetStackExampleConstants::SERVICE_WHO_IS, true);
	fpSetServiceEnabled(SETTING_CLIENT_DEVICE_INSTANCE, CASBACnetStackExampleConstants::SERVICE_WHO_HAS, true);	
	fpSetServiceEnabled(SETTING_CLIENT_DEVICE_INSTANCE, CASBACnetStackExampleConstants::SERVICE_READ_PROPERTY_MULTIPLE, true);
	fpSetServiceEnabled(SETTING_CLIENT_DEVICE_INSTANCE, CASBACnetStackExampleConstants::SERVICE_WRITE_PROPERTY, true);
	fpSetServiceEnabled(SETTING_CLIENT_DEVICE_INSTANCE, CASBACnetStackExampleConstants::SERVICE_WRITE_PROPERTY_MULTIPLE, true);

 
	// 5. Convert the IP Address and Port to a connection string
	// ---------------------------------------------------------------------------
	std::cout << "Generated the connection string for the downstream device. " << std::endl ;
	if (!ChipkinCommon::ChipkinConvert::IPAddressToBytes(downstream_Device_ip_address.c_str(), downstreamConnectionString, 6)) {
		std::cerr << "Failed to convert the ip address to a connection string" << std::endl ;
		return -4;
	}
	downstreamConnectionString[4] = SETTING_DOWNSTREAM_DEVICE_PORT / 256;
	downstreamConnectionString[5] = SETTING_DOWNSTREAM_DEVICE_PORT % 256;


	// 6. Start the main loop
	// ---------------------------------------------------------------------------
	std::cout << "FYI: Entering main loop..." << std::endl;
	for (;;) {
		// Call the DLLs loop function which checks for messages and processes them.
		fpLoop();

		// Handle any user input.
		if (!DoUserInput()) {
			// User press 'q' to quit the example application
			break;
		}
		
		// Call Sleep to give some time back to the system
		Sleep(0); 
	}

	// All done. 
	return 0;
}

bool DoUserInput()
{
	// Check to see if the user hit any key
	if (!_kbhit()) {
		// No keys have been hit
		return true;
	}

	// Extract the letter that the user hit and convert it to lower case
	char action = tolower(getchar());

	// Handle the action 
	switch (action) {
		// Quit
		case 'q': {
			return false;
		}
		case 'w': {
			ExampleWhoIs();
			break;
		}
		case 'r': {
			ExampleReadProperty();
			break;
		}
		case 'u': {
			ExampleWriteProperty();
			break; 
		}
		default: {
			// Print the Help
			std::cout << std::endl << std::endl;
			// Print the application version information 
			std::cout << "CAS BACnet Stack Client Example v" << APPLICATION_VERSION << "." << CIBUILDNUMBER << std::endl;
			std::cout << "https://github.com/chipkin/BACnetClientExampleCPP" << std::endl;
			std::cout << "Usage: BACnetClient {IPAddress}" << std::endl; 
			std::cout << "Example: BACnetClient 192.168.1.126" << std::endl << std::endl;

			std::cout << "Help: " << std::endl;
			std::cout << "- Q - Quit" << std::endl;
			std::cout << "- W - Send WhoIs message" << std::endl;
			std::cout << "- R - Send Read property messages" << std::endl;
			std::cout << "- U - Send Write property messages" << std::endl;
			std::cout << std::endl;
			break;
		}
	}
	return true; 
}

// ================================================================================================
// User input functions 
// ================================================================================================

void WaitForResponse(unsigned int timeout /*=3*/) {
	time_t expireTime = time(0) + timeout;
	while (time(0) < expireTime) {
		fpLoop();
	}
}


void ExampleWhoIs() {
	// Send the message
	std::cout << "Sending WhoIs with no range. timeout=[3]..." << std::endl ;
	fpSendWhoIs(downstreamConnectionString, 6, 0, true, 0, NULL, 0);

	// Enter a 3 second loop giving time for the hooks to be called if a message responds.
	WaitForResponse();

	std::cout << "Sending WhoIs with range, low=[389900], high=[389999] 3 second timeout..." << std::endl;
	fpSendWhoIsWithLimits(389900, 389999, downstreamConnectionString, 6, 0, true, 0, NULL, 0);

	WaitForResponse();

	std::cout << "Sending WhoIs to specific network. network=[15], timeout=[3]" << std::endl;
	fpSendWhoIs(downstreamConnectionString, 6, 0, true, 15, NULL, 0);

	WaitForResponse();

	std::cout << "Sending WhoIs to broadcast network. network=[65535], timeout=[3]" << std::endl;
	fpSendWhoIs(downstreamConnectionString, 6, 0, true, 65535, NULL, 0);

	WaitForResponse();
}

void ExampleReadProperty() {
	// Send the message
	std::cout << "Sending Read Property. DeviceID=["<< SETTING_DOWNSTREAM_DEVICE_INSTANCE <<"], property=["<< CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_ALL <<"], timeout=[3]..." << std::endl;

	// Get the object names. All objects have object names. 
	fpBuildReadProperty(CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_INPUT, 0, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_OBJECT_NAME, false, 0);
	fpBuildReadProperty(CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_OUTPUT, 1, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_OBJECT_NAME, false, 0);
	fpBuildReadProperty(CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_VALUE, 2, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_OBJECT_NAME, false, 0);
	fpBuildReadProperty(CASBACnetStackExampleConstants::OBJECT_TYPE_BINARY_INPUT, 3, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_OBJECT_NAME, false, 0);
	fpBuildReadProperty(CASBACnetStackExampleConstants::OBJECT_TYPE_BINARY_OUTPUT, 4, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_OBJECT_NAME, false, 0);
	fpBuildReadProperty(CASBACnetStackExampleConstants::OBJECT_TYPE_BINARY_VALUE, 5, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_OBJECT_NAME, false, 0);
	fpBuildReadProperty(CASBACnetStackExampleConstants::OBJECT_TYPE_DEVICE, 8, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_OBJECT_NAME, false, 0);
	fpBuildReadProperty(CASBACnetStackExampleConstants::OBJECT_TYPE_MULTI_STATE_INPUT, 13, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_OBJECT_NAME, false, 0);
	fpBuildReadProperty(CASBACnetStackExampleConstants::OBJECT_TYPE_MULTI_STATE_OUTPUT, 14, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_OBJECT_NAME, false, 0);
	fpBuildReadProperty(CASBACnetStackExampleConstants::OBJECT_TYPE_MULTI_STATE_VALUE, 19, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_OBJECT_NAME, false, 0);
	fpBuildReadProperty(CASBACnetStackExampleConstants::OBJECT_TYPE_TREND_LOG, 20, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_OBJECT_NAME, false, 0);
	fpBuildReadProperty(CASBACnetStackExampleConstants::OBJECT_TYPE_BITSTRING_VALUE, 39, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_OBJECT_NAME, false, 0);
	fpBuildReadProperty(CASBACnetStackExampleConstants::OBJECT_TYPE_CHARACTERSTRING_VALUE, 40, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_OBJECT_NAME, false, 0);
	fpBuildReadProperty(CASBACnetStackExampleConstants::OBJECT_TYPE_DATE_VALUE, 42, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_OBJECT_NAME, false, 0);
	fpBuildReadProperty(CASBACnetStackExampleConstants::OBJECT_TYPE_INTEGER_VALUE, 45, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_OBJECT_NAME, false, 0);
	fpBuildReadProperty(CASBACnetStackExampleConstants::OBJECT_TYPE_LARGE_ANALOG_VALUE, 46, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_OBJECT_NAME, false, 0);
	fpBuildReadProperty(CASBACnetStackExampleConstants::OBJECT_TYPE_OCTETSTRING_VALUE, 47, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_OBJECT_NAME, false, 0);
	fpBuildReadProperty(CASBACnetStackExampleConstants::OBJECT_TYPE_POSITIVE_INTEGER_VALUE, 48, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_OBJECT_NAME, false, 0);
	fpBuildReadProperty(CASBACnetStackExampleConstants::OBJECT_TYPE_TIME_VALUE, 50, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_OBJECT_NAME, false, 0);
	fpBuildReadProperty(CASBACnetStackExampleConstants::OBJECT_TYPE_NETWORK_PORT, 56, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_OBJECT_NAME, false, 0);

	// Get the present value from objects that have a present value property. 
	fpBuildReadProperty(CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_INPUT, 0, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE, false, 0);
	fpBuildReadProperty(CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_OUTPUT, 1, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE, false, 0);
	fpBuildReadProperty(CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_VALUE, 2, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE, false, 0);
	fpBuildReadProperty(CASBACnetStackExampleConstants::OBJECT_TYPE_BINARY_INPUT, 3, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE, false, 0);
	fpBuildReadProperty(CASBACnetStackExampleConstants::OBJECT_TYPE_BINARY_OUTPUT, 4, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE, false, 0);
	fpBuildReadProperty(CASBACnetStackExampleConstants::OBJECT_TYPE_BINARY_VALUE, 5, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE, false, 0);
	fpBuildReadProperty(CASBACnetStackExampleConstants::OBJECT_TYPE_MULTI_STATE_INPUT, 13, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE, false, 0);
	fpBuildReadProperty(CASBACnetStackExampleConstants::OBJECT_TYPE_MULTI_STATE_OUTPUT, 14, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE, false, 0);
	fpBuildReadProperty(CASBACnetStackExampleConstants::OBJECT_TYPE_MULTI_STATE_VALUE, 19, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE, false, 0);
	fpBuildReadProperty(CASBACnetStackExampleConstants::OBJECT_TYPE_BITSTRING_VALUE, 39, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE, false, 0);
	fpBuildReadProperty(CASBACnetStackExampleConstants::OBJECT_TYPE_CHARACTERSTRING_VALUE, 40, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE, false, 0);
	fpBuildReadProperty(CASBACnetStackExampleConstants::OBJECT_TYPE_DATE_VALUE, 42, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE, false, 0);
	fpBuildReadProperty(CASBACnetStackExampleConstants::OBJECT_TYPE_INTEGER_VALUE, 45, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE, false, 0);
	fpBuildReadProperty(CASBACnetStackExampleConstants::OBJECT_TYPE_LARGE_ANALOG_VALUE, 46, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE, false, 0);
	fpBuildReadProperty(CASBACnetStackExampleConstants::OBJECT_TYPE_OCTETSTRING_VALUE, 47, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE, false, 0);
	fpBuildReadProperty(CASBACnetStackExampleConstants::OBJECT_TYPE_POSITIVE_INTEGER_VALUE, 48, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE, false, 0);
	fpBuildReadProperty(CASBACnetStackExampleConstants::OBJECT_TYPE_TIME_VALUE, 50, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE, false, 0);
	
	// Send the Read property message
	fpSendReadProperty(&invokeId, downstreamConnectionString, 6, 0, 0, NULL, 0);
	WaitForResponse();
}

void ExampleWriteProperty() 
{
	// Get and read the value of AnalogValue
	std::cout << "Sending Read Property. AnalogValue, INSTANCE=[2], property=[" << CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE << "], timeout=[3]..." << std::endl;
	fpBuildReadProperty(CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_VALUE, 2, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE, false, 0);
	fpSendReadProperty(&invokeId, downstreamConnectionString, 6, 0, 0, NULL, 0);

	WaitForResponse();

	// Get and write to AnalogValue on server
	std::cout << "Sending WriteProperty to the Present Value of Analog Value 2..." << std::endl;
	fpBuildWriteProperty(4, "1.0", 3, CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_VALUE, 2, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE, false, 0, false, 16);
	fpSendWriteProperty(&invokeId, downstreamConnectionString, 6, 0, 0, NULL, 0);

	WaitForResponse();

	// Verify that AanalogValue has changed
	std::cout << "Sending Read Property. AnalogValue, INSTANCE=[2], property=[" << CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE << "], timeout=[3]..." << std::endl;
	fpBuildReadProperty(CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_VALUE, 2, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE, false, 0);
	fpSendReadProperty(&invokeId, downstreamConnectionString, 6, 0, 0, NULL, 0);

	WaitForResponse();
}


// ================================================================================================
// Callbacks 
// ================================================================================================


// Callback used by the BACnet Stack to check if there is a message to process
uint16_t CallbackReceiveMessage(uint8_t* message, const uint16_t maxMessageLength, uint8_t* receivedConnectionString, const uint8_t maxConnectionStringLength, uint8_t* receivedConnectionStringLength, uint8_t* networkType)
{
	// Check parameters
	if (message == NULL || maxMessageLength == 0) {
		std::cerr << "Invalid input buffer" << std::endl;
		return 0;
	}
	if (receivedConnectionString == NULL || maxConnectionStringLength == 0) {
		std::cerr << "Invalid connection string buffer" << std::endl;
		return 0;
	}
	if (maxConnectionStringLength < 6) {
		std::cerr << "Not enough space for a UDP connection string" << std::endl;
		return 0;
	}

	char ipAddress[32];
	uint16_t port = 0;

	// Attempt to read bytes
	int bytesRead = g_udp.GetMessage(message, maxMessageLength, ipAddress, &port);
	if (bytesRead > 0) {
		ChipkinCommon::CEndianness::ToBigEndian(&port, sizeof(uint16_t));
		std::cout << "FYI: Received message from [" << ipAddress << ":" << port << "], length [" << bytesRead << "]" << std::endl;

		// Convert the IP Address to the connection string
		if (!ChipkinCommon::ChipkinConvert::IPAddressToBytes(ipAddress, receivedConnectionString, maxConnectionStringLength)) {
			std::cerr << "Failed to convert the ip address into a connectionString" << std::endl;
			return 0;
		}
		receivedConnectionString[4] = port / 256;
		receivedConnectionString[5] = port % 256;

		*receivedConnectionStringLength = 6;
		*networkType = CASBACnetStackExampleConstants::NETWORK_TYPE_IP;

		// Process the message as XML
		static char xmlRenderBuffer[MAX_XML_RENDER_BUFFER_LENGTH];
		if (fpDecodeAsXML((char*)message, bytesRead, xmlRenderBuffer, MAX_XML_RENDER_BUFFER_LENGTH) > 0) {
			std::cout << xmlRenderBuffer << std::endl << std::endl;
			memset(xmlRenderBuffer, 0, MAX_XML_RENDER_BUFFER_LENGTH);

		}
	}

	return bytesRead;
}

// Callback used by the BACnet Stack to send a BACnet message
uint16_t CallbackSendMessage(const uint8_t* message, const uint16_t messageLength, const uint8_t* connectionString, const uint8_t connectionStringLength, const uint8_t networkType, bool broadcast)
{
	if (message == NULL || messageLength == 0) {
		std::cout << "Nothing to send" << std::endl;
		return 0;
	}
	if (connectionString == NULL || connectionStringLength == 0) {
		std::cout << "No connection string" << std::endl;
		return 0;
	}

	// Verify Network Type
	if (networkType != CASBACnetStackExampleConstants::NETWORK_TYPE_IP) {
		std::cout << "Message for different network" << std::endl;
		return 0;
	}

	// Prepare the IP Address
	char ipAddress[32];
	if (broadcast) {
		/*
		snprintf(ipAddress, 32, "%u.%u.%u.%u",
			(connectionString[0] & g_database.networkPort.IPSubnetMask[0]) == 0 ? 255 : connectionString[0],
			(connectionString[1] & g_database.networkPort.IPSubnetMask[1]) == 0 ? 255 : connectionString[1],
			(connectionString[2] & g_database.networkPort.IPSubnetMask[2]) == 0 ? 255 : connectionString[2],
			(connectionString[3] & g_database.networkPort.IPSubnetMask[3]) == 0 ? 255 : connectionString[3]);
		*/
		snprintf(ipAddress, 32, "%u.%u.%u.%u", 255,255,255,255); 
	}
	else {
		snprintf(ipAddress, 32, "%u.%u.%u.%u", connectionString[0], connectionString[1], connectionString[2], connectionString[3]);
	}

	// Get the port
	uint16_t port = 0;
	port += connectionString[4] * 256;
	port += connectionString[5];

	std::cout << "FYI: Sending message to [" << ipAddress << ":" << port << "] length [" << messageLength << "]" << std::endl;

	// Send the message
	if (!g_udp.SendMessage(ipAddress, port, (unsigned char*)message, messageLength)) {
		std::cout << "Failed to send message" << std::endl;
		return 0;
	}

	// Get the XML rendered version of the just sent message
	static char xmlRenderBuffer[MAX_XML_RENDER_BUFFER_LENGTH];
	if (fpDecodeAsXML((char*)message, messageLength, xmlRenderBuffer, MAX_XML_RENDER_BUFFER_LENGTH) > 0) {
		std::cout << xmlRenderBuffer << std::endl << std::endl;
		memset(xmlRenderBuffer, 0, MAX_XML_RENDER_BUFFER_LENGTH);
	}

	return messageLength;
}

// Callback used by the BACnet Stack to get the current time
time_t CallbackGetSystemTime()
{
	return time(0) ;
}

// Outputs fetched data to console in readable format
void HelperPrintCommonHookParameters(const uint8_t* connectionString, const uint8_t connectionStringLength, const uint8_t networkType, const uint16_t network, const uint8_t* sourceAddress, const uint8_t sourceAddressLength) {
	std::cout << "networkType=[" << (int)networkType << "], ";
	std::cout << "connectionString=[" << (int)connectionString[0] << "." << (int)connectionString[1] << "." << (int)connectionString[2] << "." << (int)connectionString[3] << ":" << ((connectionString[4] * 256) + connectionString[5]) << "], "; 
}
void HelperPrintCommonHookPropertyParameters(const uint32_t originalInvokeId, const uint8_t service, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, const bool usePropertyArrayIndex, const uint32_t propertyArrayIndex) {
	std::cout << "InvokeID=[" << (int)originalInvokeId << "], service=[" << (int)service << "], objectInstance = [" << objectInstance << "], ";
	
	// Decode some of the objectType for readablity. 
	std::cout << "objectType=[";
	switch (objectType) {
		case CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_INPUT: {
			std::cout << "analog_input(" << CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_INPUT << ")";
			break;
		}
		case CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_OUTPUT: {
			std::cout << "analog_output(" << CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_OUTPUT << ")";
			break;
		}
		case CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_VALUE: {
			std::cout << "analog_value(" << CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_VALUE << ")";
			break;
		}
		case CASBACnetStackExampleConstants::OBJECT_TYPE_BINARY_INPUT: {
			std::cout << "binary_input(" << CASBACnetStackExampleConstants::OBJECT_TYPE_BINARY_INPUT << ")";
			break;
		}
		case CASBACnetStackExampleConstants::OBJECT_TYPE_BINARY_OUTPUT: {
			std::cout << "binary_output(" << CASBACnetStackExampleConstants::OBJECT_TYPE_BINARY_OUTPUT << ")";
			break;
		}
		case CASBACnetStackExampleConstants::OBJECT_TYPE_BINARY_VALUE: {
			std::cout << "binary_value(" << CASBACnetStackExampleConstants::OBJECT_TYPE_BINARY_VALUE << ")";
			break;
		}
		case CASBACnetStackExampleConstants::OBJECT_TYPE_DEVICE: {
			std::cout << "device(" << CASBACnetStackExampleConstants::OBJECT_TYPE_DEVICE << ")";
			break;
		}
		case CASBACnetStackExampleConstants::OBJECT_TYPE_MULTI_STATE_INPUT: {
			std::cout << "multi_state_input(" << CASBACnetStackExampleConstants::OBJECT_TYPE_MULTI_STATE_INPUT << ")";
			break;
		}
		case CASBACnetStackExampleConstants::OBJECT_TYPE_MULTI_STATE_OUTPUT: {
			std::cout << "multi_state_output(" << CASBACnetStackExampleConstants::OBJECT_TYPE_MULTI_STATE_OUTPUT << ")";
			break;
		}
		case CASBACnetStackExampleConstants::OBJECT_TYPE_MULTI_STATE_VALUE: {
			std::cout << "multi_state_value(" << CASBACnetStackExampleConstants::OBJECT_TYPE_MULTI_STATE_VALUE << ")";
			break;
		}
		case CASBACnetStackExampleConstants::OBJECT_TYPE_TREND_LOG: {
			std::cout << "trend_log(" << CASBACnetStackExampleConstants::OBJECT_TYPE_TREND_LOG << ")";
			break;
		}
		case CASBACnetStackExampleConstants::OBJECT_TYPE_BITSTRING_VALUE: {
			std::cout << "bitstring_value(" << CASBACnetStackExampleConstants::OBJECT_TYPE_BITSTRING_VALUE << ")";
			break;
		}
		case CASBACnetStackExampleConstants::OBJECT_TYPE_CHARACTERSTRING_VALUE: {
			std::cout << "characterstring_value(" << CASBACnetStackExampleConstants::OBJECT_TYPE_CHARACTERSTRING_VALUE << ")";
			break;
		}
		case CASBACnetStackExampleConstants::OBJECT_TYPE_DATE_VALUE: {
			std::cout << "date_value(" << CASBACnetStackExampleConstants::OBJECT_TYPE_DATE_VALUE << ")";
			break;
		}
		case CASBACnetStackExampleConstants::OBJECT_TYPE_INTEGER_VALUE: {
			std::cout << "integer_value(" << CASBACnetStackExampleConstants::OBJECT_TYPE_INTEGER_VALUE << ")";
			break;
		}
		case CASBACnetStackExampleConstants::OBJECT_TYPE_LARGE_ANALOG_VALUE: {
			std::cout << "large_analog_value(" << CASBACnetStackExampleConstants::OBJECT_TYPE_LARGE_ANALOG_VALUE << ")";
			break;
		}
		case CASBACnetStackExampleConstants::OBJECT_TYPE_OCTETSTRING_VALUE: {
			std::cout << "octetstring_value(" << CASBACnetStackExampleConstants::OBJECT_TYPE_OCTETSTRING_VALUE << ")";
			break;
		}
		case CASBACnetStackExampleConstants::OBJECT_TYPE_POSITIVE_INTEGER_VALUE: {
			std::cout << "positive_integer_value(" << CASBACnetStackExampleConstants::OBJECT_TYPE_POSITIVE_INTEGER_VALUE << ")";
			break;
		}
		case CASBACnetStackExampleConstants::OBJECT_TYPE_TIME_VALUE: {
			std::cout << "time_value(" << CASBACnetStackExampleConstants::OBJECT_TYPE_TIME_VALUE << ")";
			break;
		}
		case CASBACnetStackExampleConstants::OBJECT_TYPE_NETWORK_PORT: {
			std::cout << "network_port(" << CASBACnetStackExampleConstants::OBJECT_TYPE_NETWORK_PORT << ")";
			break;
		}
		case CASBACnetStackExampleConstants::OBJECT_TYPE_ELEVATOR_GROUP: {
			std::cout << "elevator_group(" << CASBACnetStackExampleConstants::OBJECT_TYPE_ELEVATOR_GROUP << ")";
			break;
		}
		case CASBACnetStackExampleConstants::OBJECT_TYPE_ESCALATOR: {
			std::cout << "escalator(" << CASBACnetStackExampleConstants::OBJECT_TYPE_ESCALATOR << ")";
			break;
		}
		case CASBACnetStackExampleConstants::OBJECT_TYPE_LIFT: {
			std::cout << "lift(" << CASBACnetStackExampleConstants::OBJECT_TYPE_LIFT << ")";
			break;
		}
		default: {
			std::cout << (int) objectType; 
			break; 
		}
	}
	std::cout << "], ";
	
	// Decode some of the propertyIdentifier for readablity. 
	std::cout << "propertyIdentifier=[";
	switch(propertyIdentifier) {
		case CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_ALL: {
			std::cout << "all(" << CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_ALL << ")";
			break;
		}
		case CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_COV_INCURMENT: {
			std::cout << "cov_incurment(" << CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_COV_INCURMENT << ")";
			break;
		}
		case CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_DAY_LIGHT_SAVINGS_STATUS: {
			std::cout << "day_light_savings_status(" << CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_DAY_LIGHT_SAVINGS_STATUS << ")";
			break;
		}
		case CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_DESCRIPTION: {
			std::cout << "description(" << CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_DESCRIPTION << ")";
			break;
		}
		case CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_LOCAL_DATE: {
			std::cout << "local_date(" << CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_LOCAL_DATE << ")";
			break;
		}
		case CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_LOCAL_TIME: {
			std::cout << "local_time(" << CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_LOCAL_TIME << ")";
			break;
		}
		case CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_NUMBER_OF_STATES: {
			std::cout << "number_of_states(" << CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_NUMBER_OF_STATES << ")";
			break;
		}
		case CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_OBJECT_NAME: {
			std::cout << "object_name(" << CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_OBJECT_NAME << ")";
			break;
		}
		case CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE: {
			std::cout << "present_value(" << CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE << ")";
			break;
		}
		case CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRIORITY_ARRAY: {
			std::cout << "priority_array(" << CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRIORITY_ARRAY << ")";
			break;
		}
		case CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_RELIABILITY: {
			std::cout << "reliability(" << CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_RELIABILITY << ")";
			break;
		}
		case CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_STATE_TEXT: {
			std::cout << "state_text(" << CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_STATE_TEXT << ")";
			break;
		}
		case CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_STATUS_FLAGS: {
			std::cout << "status_flags(" << CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_STATUS_FLAGS << ")";
			break;
		}
		case CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_SYSTEM_STATUS: {
			std::cout << "system_status(" << CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_SYSTEM_STATUS << ")";
			break;
		}
		case CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_UTC_OFFSET: {
			std::cout << "utc_offset(" << CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_UTC_OFFSET << ")";
			break;
		}
		case CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_BIT_TEXT: {
			std::cout << "bit_text(" << CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_BIT_TEXT << ")";
			break;
		}
		case CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_MAX_PRES_VALUE: {
			std::cout << "max_pres_value(" << CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_MAX_PRES_VALUE << ")";
			break;
		}
		case CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_MIN_PRES_VALUE: {
			std::cout << "min_pres_value(" << CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_MIN_PRES_VALUE << ")";
			break;
		}
		default: 
			std::cout << propertyIdentifier ;
			break; 
	}
	std::cout << "], ";
	
	// If usePropertyArrayIndex is true then show the propertyArrayIndex
	if (usePropertyArrayIndex) {
		std::cout << "propertyArrayIndex=[" << propertyArrayIndex << "], ";
	}
}


// Hooks for unconfirmed request
void HookIAm(const uint32_t deviceIdentifier, const uint32_t maxApduLengthAccepted, const uint8_t segmentationSupported, const uint16_t vendorIdentifier, const uint8_t* connectionString, const uint8_t connectionStringLength, const uint8_t networkType, const uint16_t network, const uint8_t* sourceAddress, const uint8_t sourceAddressLength) {

	std::cout << "HookIAm. deviceIdentifier=[" << deviceIdentifier << "], maxApduLengthAccepted=[" << maxApduLengthAccepted << "], segmentationSupported=[" << (int)segmentationSupported << "], vendorIdentifier=[" << vendorIdentifier << "], ";
	HelperPrintCommonHookParameters(connectionString, connectionStringLength, networkType, network, sourceAddress, sourceAddressLength);
	std::cout << std::endl; 
}

void HookIHave(const uint32_t deviceIdentifier, const uint16_t objectType, const uint32_t objectInstance, const char* objectName, const uint32_t objectNameLength, const uint8_t objectNameEncoding, const uint8_t* connectionString, const uint8_t connectionStringLength, const uint8_t networkType, const uint16_t network, const uint8_t* sourceAddress, const uint8_t sourceAddressLength) {
	
	std::cout << "HookIHave. deviceIdentifier=[" << deviceIdentifier << "], objectType=[" << objectType << "], objectInstance=[" << objectInstance << "], objectName=[" << objectName << "], objectNameEncoding=[" << (int)objectNameEncoding << "], "; 
	HelperPrintCommonHookParameters(connectionString, connectionStringLength, networkType, network, sourceAddress, sourceAddressLength);
	std::cout << std::endl;
}

// Hooks for simple PDUs
void HookError(const uint8_t originalInvokeId, const uint32_t errorChoice, const uint32_t errorClass, const uint32_t errorCode, const uint8_t* connectionString, const uint8_t connectionStringLength, const uint8_t networkType, const uint16_t network, const uint8_t* sourceAddress, const uint8_t sourceAddressLength, const bool useObjectProperty, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier) {

	std::cout << "HookError. originalInvokeId=[" << (int) originalInvokeId << "], errorChoice=[" << (int)errorChoice << "], errorClass=[" << (int)errorClass << "], errorCode=[" << (int)errorCode << "], ";
	std::cout << "useObjectProperty=[" << (int)useObjectProperty << "], objectType=[" << (int)objectType << "], objectInstance=[" << (int)objectInstance << "], propertyIdentifier=[" << (int)propertyIdentifier << "], ";
	HelperPrintCommonHookParameters(connectionString, connectionStringLength, networkType, network, sourceAddress, sourceAddressLength);
	std::cout << std::endl;
}

void HookReject(const uint8_t originalInvokeId, const uint32_t rejectReason, const uint8_t* connectionString, const uint8_t connectionStringLength, const uint8_t networkType, const uint16_t network, const uint8_t* sourceAddress, const uint8_t sourceAddressLength) {

	std::cout << "HookError. originalInvokeId=[" << (int) originalInvokeId << "], rejectReason=[" << (int)rejectReason << "], ";
	HelperPrintCommonHookParameters(connectionString, connectionStringLength, networkType, network, sourceAddress, sourceAddressLength);
	std::cout << std::endl;
}

void HookAbort(const uint8_t originalInvokeId, const bool sentByServer, const uint32_t abortReason, const uint8_t* connectionString, const uint8_t connectionStringLength, const uint8_t networkType, const uint16_t network, const uint8_t* sourceAddress, const uint8_t sourceAddressLength) {
	
	std::cout << "HookAbort. originalInvokeId=[" << (int) originalInvokeId << "], sentByServer=[" << sentByServer << "], abortReason=[" << abortReason << "], ";
	HelperPrintCommonHookParameters(connectionString, connectionStringLength, networkType, network, sourceAddress, sourceAddressLength);
	std::cout << std::endl;
}

void HookSimpleAck(const uint8_t originalInvokeId, const uint32_t serverAckChoice, const uint8_t* connectionString, const uint8_t connectionStringLength, const uint8_t networkType, const uint16_t network, const uint8_t* sourceAddress, const uint8_t sourceAddressLength) {

	std::cout << "HookSimpleAck. originalInvokeId=[" << (int) originalInvokeId << "], serverAckChoice=[" << serverAckChoice << "], ";
	HelperPrintCommonHookParameters(connectionString, connectionStringLength, networkType, network, sourceAddress, sourceAddressLength);
	std::cout << std::endl;
}

void HookTimeout(const uint8_t originalInvokeId, const uint8_t* connectionString, const uint8_t connectionStringLength, const uint8_t networkType, const uint16_t network, const uint8_t* sourceAddress, const uint8_t sourceAddressLength) {

	std::cout << "HookTimeout. originalInvokeId=[" << (int) originalInvokeId << "], ";
	HelperPrintCommonHookParameters(connectionString, connectionStringLength, networkType, network, sourceAddress, sourceAddressLength);
	std::cout << std::endl;
}

void HookPropertyBitString(const uint32_t originalInvokeId, const uint8_t service, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, const bool usePropertyArrayIndex, const uint32_t propertyArrayIndex, const bool* value, const uint32_t length, const uint8_t* connectionString, const uint8_t connectionStringLength, const uint8_t networkType, const uint16_t network, const uint8_t* sourceAddress, const uint8_t sourceAddressLength) {
	std::cout << "HookPropertyBitString. ";
	// ToDo: Encode bitstring as 'T' and 'F' 
	HelperPrintCommonHookPropertyParameters(originalInvokeId, service, objectType, objectInstance, propertyIdentifier, usePropertyArrayIndex, propertyArrayIndex);
	HelperPrintCommonHookParameters(connectionString, connectionStringLength, networkType, network, sourceAddress, sourceAddressLength);
	std::cout << std::endl;
}

void HookPropertyBool(const uint32_t originalInvokeId, const uint8_t service, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, const bool usePropertyArrayIndex, const uint32_t propertyArrayIndex, const bool value, const uint8_t* connectionString, const uint8_t connectionStringLength, const uint8_t networkType, const uint16_t network, const uint8_t* sourceAddress, const uint8_t sourceAddressLength) {
	std::cout << "HookPropertyBool. value=[" << value << "], ";
	HelperPrintCommonHookPropertyParameters(originalInvokeId, service, objectType, objectInstance, propertyIdentifier, usePropertyArrayIndex, propertyArrayIndex);
	HelperPrintCommonHookParameters(connectionString, connectionStringLength, networkType, network, sourceAddress, sourceAddressLength);
	std::cout << std::endl;
}

void HookPropertyCharString(const uint32_t originalInvokeId, const uint8_t service, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, const bool usePropertyArrayIndex, const uint32_t propertyArrayIndex, const char* value, const uint32_t length, const uint8_t encoding, const uint8_t* connectionString, const uint8_t connectionStringLength, const uint8_t networkType, const uint16_t network, const uint8_t* sourceAddress, const uint8_t sourceAddressLength) {
	std::cout << "HookPropertyCharString. value=[" << value << "], length=[" << length <<"], encoding=[" << (int) encoding << "], ";
	HelperPrintCommonHookPropertyParameters(originalInvokeId, service, objectType, objectInstance, propertyIdentifier, usePropertyArrayIndex, propertyArrayIndex);
	HelperPrintCommonHookParameters(connectionString, connectionStringLength, networkType, network, sourceAddress, sourceAddressLength);
	std::cout << std::endl;
}

void HookPropertyDate(const uint32_t originalInvokeId, const uint8_t service, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, const bool usePropertyArrayIndex, const uint32_t propertyArrayIndex, const uint8_t year, const uint8_t month, const uint8_t day, const uint8_t weekday, const uint8_t* connectionString, const uint8_t connectionStringLength, const uint8_t networkType, const uint16_t network, const uint8_t* sourceAddress, const uint8_t sourceAddressLength) {
	std::cout << "HookPropertyDate. year=[" << (int) year << "], month=[" << (int) month << "], day=[" << (int) day << "], ";
	HelperPrintCommonHookPropertyParameters(originalInvokeId, service, objectType, objectInstance, propertyIdentifier, usePropertyArrayIndex, propertyArrayIndex);
	HelperPrintCommonHookParameters(connectionString, connectionStringLength, networkType, network, sourceAddress, sourceAddressLength);
	std::cout << std::endl;
}

void HookPropertyDouble(const uint32_t originalInvokeId, const uint8_t service, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, const bool usePropertyArrayIndex, const uint32_t propertyArrayIndex, const double value, const uint8_t* connectionString, const uint8_t connectionStringLength, const uint8_t networkType, const uint16_t network, const uint8_t* sourceAddress, const uint8_t sourceAddressLength) {
	std::cout << "HookPropertyDouble. value=[" << value << "], "; 
	HelperPrintCommonHookPropertyParameters(originalInvokeId, service, objectType, objectInstance, propertyIdentifier, usePropertyArrayIndex, propertyArrayIndex);
	HelperPrintCommonHookParameters(connectionString, connectionStringLength, networkType, network, sourceAddress, sourceAddressLength);
	std::cout << std::endl;
}

void HookPropertyEnum(const uint32_t originalInvokeId, const uint8_t service, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, const bool usePropertyArrayIndex, const uint32_t propertyArrayIndex, const uint32_t value, const uint8_t* connectionString, const uint8_t connectionStringLength, const uint8_t networkType, const uint16_t network, const uint8_t* sourceAddress, const uint8_t sourceAddressLength) {
	std::cout << "HookPropertyEnum. value=[" << value << "], ";
	HelperPrintCommonHookPropertyParameters(originalInvokeId, service, objectType, objectInstance, propertyIdentifier, usePropertyArrayIndex, propertyArrayIndex);
	HelperPrintCommonHookParameters(connectionString, connectionStringLength, networkType, network, sourceAddress, sourceAddressLength);
	std::cout << std::endl;
}

void HookPropertyNull(const uint32_t originalInvokeId, const uint8_t service, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, const bool usePropertyArrayIndex, const uint32_t propertyArrayIndex, const uint8_t* connectionString, const uint8_t connectionStringLength, const uint8_t networkType, const uint16_t network, const uint8_t* sourceAddress, const uint8_t sourceAddressLength) {
	std::cout << "HookPropertyNull. value=[null], ";
	HelperPrintCommonHookPropertyParameters(originalInvokeId, service, objectType, objectInstance, propertyIdentifier, usePropertyArrayIndex, propertyArrayIndex);
	HelperPrintCommonHookParameters(connectionString, connectionStringLength, networkType, network, sourceAddress, sourceAddressLength);
	std::cout << std::endl;
}

void HookPropertyObjectIdentifier(const uint32_t originalInvokeId, const uint8_t service, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, const bool usePropertyArrayIndex, const uint32_t propertyArrayIndex, const uint16_t objectTypeValue, const uint32_t objectInstanceValue, const uint8_t* connectionString, const uint8_t connectionStringLength, const uint8_t networkType, const uint16_t network, const uint8_t* sourceAddress, const uint8_t sourceAddressLength) {
	std::cout << "HookPropertyObjectIdentifier. objectInstanceValue=[" << objectInstanceValue << "], objectTypeValue=[" << objectTypeValue << "], ";
	HelperPrintCommonHookPropertyParameters(originalInvokeId, service, objectType, objectInstance, propertyIdentifier, usePropertyArrayIndex, propertyArrayIndex);
	HelperPrintCommonHookParameters(connectionString, connectionStringLength, networkType, network, sourceAddress, sourceAddressLength);
	std::cout << std::endl;
}

void HookPropertyOctString(const uint32_t originalInvokeId, const uint8_t service, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, const bool usePropertyArrayIndex, const uint32_t propertyArrayIndex, const uint8_t* value, const uint32_t length, const uint8_t* connectionString, const uint8_t connectionStringLength, const uint8_t networkType, const uint16_t network, const uint8_t* sourceAddress, const uint8_t sourceAddressLength) {
	std::cout << "HookPropertyOctString. ";
	// ToDo: Encode Oct string as Hex 
	HelperPrintCommonHookPropertyParameters(originalInvokeId, service, objectType, objectInstance, propertyIdentifier, usePropertyArrayIndex, propertyArrayIndex);
	HelperPrintCommonHookParameters(connectionString, connectionStringLength, networkType, network, sourceAddress, sourceAddressLength);
	std::cout << std::endl;
}

void HookPropertyInt(const uint32_t originalInvokeId, const uint8_t service, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, const bool usePropertyArrayIndex, const uint32_t propertyArrayIndex, const int32_t value, const uint8_t* connectionString, const uint8_t connectionStringLength, const uint8_t networkType, const uint16_t network, const uint8_t* sourceAddress, const uint8_t sourceAddressLength) {
	std::cout << "HookPropertyInt. value=[" << value << "], ";
	HelperPrintCommonHookPropertyParameters(originalInvokeId, service, objectType, objectInstance, propertyIdentifier, usePropertyArrayIndex, propertyArrayIndex);
	HelperPrintCommonHookParameters(connectionString, connectionStringLength, networkType, network, sourceAddress, sourceAddressLength);
	std::cout << std::endl;
}

void HookPropertyReal(const uint32_t originalInvokeId, const uint8_t service, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, const bool usePropertyArrayIndex, const uint32_t propertyArrayIndex, const float value, const uint8_t* connectionString, const uint8_t connectionStringLength, const uint8_t networkType, const uint16_t network, const uint8_t* sourceAddress, const uint8_t sourceAddressLength) {
	std::cout << "HookPropertyReal. value=[" << value << "], ";
	HelperPrintCommonHookPropertyParameters(originalInvokeId, service, objectType, objectInstance, propertyIdentifier, usePropertyArrayIndex, propertyArrayIndex);
	HelperPrintCommonHookParameters(connectionString, connectionStringLength, networkType, network, sourceAddress, sourceAddressLength);
	std::cout << std::endl;
}

void HookPropertyTime(const uint32_t originalInvokeId, const uint8_t service, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, const bool usePropertyArrayIndex, const uint32_t propertyArrayIndex, const uint8_t hour, const uint8_t minute, const uint8_t second, const uint8_t hundrethSecond, const uint8_t* connectionString, const uint8_t connectionStringLength, const uint8_t networkType, const uint16_t network, const uint8_t* sourceAddress, const uint8_t sourceAddressLength) {
	std::cout << "HookPropertyTime. hour=[" << (int) hour << "], minute=[" << (int)minute << "], second=[" << (int)second << "], hundrethSecond=[" << (int)hundrethSecond << "], ";
	HelperPrintCommonHookPropertyParameters(originalInvokeId, service, objectType, objectInstance, propertyIdentifier, usePropertyArrayIndex, propertyArrayIndex);
	HelperPrintCommonHookParameters(connectionString, connectionStringLength, networkType, network, sourceAddress, sourceAddressLength);
	std::cout << std::endl;
}

void HookPropertyUInt(const uint32_t originalInvokeId, const uint8_t service, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, const bool usePropertyArrayIndex, const uint32_t propertyArrayIndex, const uint32_t value, const uint8_t* connectionString, const uint8_t connectionStringLength, const uint8_t networkType, const uint16_t network, const uint8_t* sourceAddress, const uint8_t sourceAddressLength) {
	std::cout << "HookPropertyUInt. value=[" << value << "], ";
	HelperPrintCommonHookPropertyParameters(originalInvokeId, service, objectType, objectInstance, propertyIdentifier, usePropertyArrayIndex, propertyArrayIndex);
	HelperPrintCommonHookParameters(connectionString, connectionStringLength, networkType, network, sourceAddress, sourceAddressLength);
	std::cout << std::endl;
}
