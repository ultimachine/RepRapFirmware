#ifndef PINS_ARCHIM_H__
#define PINS_ARCHIM_H__

#include <stdint.h>
#include <stddef.h>
#include "Core.h"

#define FIRMWARE_NAME "RepRapFirmware for Archim"

const size_t NumFirmwareUpdateModules = 3;
#define IAP_UPDATE_FILE "iap.bin"
#define IAP_FIRMWARE_FILE "RepRapFirmware.bin"
#define WIFI_FIRMWARE_FILE	"DuetWiFiServer.bin"
#define WIFI_WEB_FILE		"DuetWebControl.bin"

// Default board type
#define DEFAULT_BOARD_TYPE BoardType::Archim

#define SUPPORT_INKJET		0					// set nonzero to support inkjet control
#define SUPPORT_ROLAND		0					// set nonzero to support Roland mill

// The physical capabilities of the machine

const size_t DRIVES = 5;						// The number of drives in the machine, including X, Y, and Z plus extruder drives
#define DRIVES_(a,b,c,d,e,...) { a,b,c,d,e } //HELPER MACRO TO REDUCE SIZE OF ARRAYS
const size_t MaxDriversPerAxis = 4;				// The maximum number of stepper drivers assigned to one axis

const int8_t HEATERS = 4;						// The number of heaters in the machine; 0 is the heated bed even if there isn't one
#define HEATERS_(a,b,c,d,e,f,g,h) { a,b,c,d } //HELPER MACRO TO REDUCE SIZE OF ARRAYS

#define AXES_(a,b,c,d,e,...) { a,b,c,d,e } //HELPER MACRO TO REDUCE SIZE OF ARRAYS
const size_t MAX_AXES = DRIVES;						// The maximum number of movement axes in the machine, usually just X, Y and Z, <= DRIVES
const size_t MIN_AXES = 3;						// The minimum and default number of axes
//const size_t DELTA_AXES = 3;					// The number of axis involved in delta movement
//const size_t CART_AXES = 3;						// The number of Cartesian axes
const size_t MaxExtruders = DRIVES - MIN_AXES;	// The maximum number of extruders

const size_t NUM_SERIAL_CHANNELS = 2;			// The number of serial IO channels (USB and two auxiliary UARTs)
#define SERIAL_MAIN_DEVICE SerialUSB
#define SERIAL_AUX_DEVICE Serial
//#define SERIAL_AUX2_DEVICE Serial1

// The numbers of entries in each array must correspond with the values of DRIVES, AXES, or HEATERS. Set values to -1 to flag unavailability.

// DRIVES
#define HAVE_TMC2130_DRIVERS
#define ARCHIM20
#if defined( ARCHIM10 )
	const Pin ENABLE_PINS[DRIVES] = {
			41,
			48,
			104, //ArchimAddons-96 //PC10 EN Z -AddOns
			108, //ArchimAddons-97 //PB24 EN3 -Addons
			28 };
	const Pin STEP_PINS[DRIVES] = { 40, 49, 36, 78, 26 };
	const Pin DIRECTION_PINS[DRIVES] = {
			59,
			47,
			113, //ArchimAddons-107 PB10 DIR Z
			22,
			27,
	};
#elif defined(ARCHIM20)
	const Pin ENABLE_PINS[DRIVES] = {
			41,    //PC9  X-EN
			49,    //PC14 Y-EN
			44,    //PC19 Z-EN
			106,   //PB22 E1-EN   -CoreNG_Archim*
			18,    //PA11 E2-EN
	};
	const Pin STEP_PINS[DRIVES] = {
			38,    //PC6  X-STEP
			51,    //PC12 Y-STEP
			46,    //PC17 Z-STEP
			113,   //PB10 E1-STEP -CoreNG_Archim*
			22,    //PB26 E2_STEP
	};
	const Pin DIRECTION_PINS[DRIVES] = {
			37,    //PC5  X-DIR
			96,    //PC11 Y-DIR   -CoreNG_Archim*
			47,    //PC16 Z-DIR
			104,   //PC10 E1-DIR  -CoreNG_Archim*
			108,   //PB24 E2_DIR  -CoreNG_Archim*
	};


#define X_DIAG_PIN         59 //PA4 X_DIAG
#define Y_DIAG_PIN         48 //PC15 Y_DIAG
#define Z_DIAG_PIN         36 //PC4 Z_DIAG
#define E0_DIAG_PIN        78 //PB23 E1_DIAG
#define E1_DIAG_PIN        25 //PD0 E2_DIAG
const Pin DIAG_PINS[DRIVES] = { X_DIAG_PIN, Y_DIAG_PIN, Z_DIAG_PIN, E0_DIAG_PIN, E1_DIAG_PIN };

#define X_CS_PIN           39 //PC7 X_nCS
#define Y_CS_PIN           50 //PC13 Y_nCS
#define Z_CS_PIN           45 //PC18 Z_nCS
#define E0_CS_PIN         100 //PC20 E1_nCS -CoreNG_Archim*
#define E1_CS_PIN          19 //PA10 E2_nCS
const Pin CS_PINS[DRIVES] = { X_CS_PIN, Y_CS_PIN, Z_CS_PIN, E0_CS_PIN, E1_CS_PIN };

#define TMC2130_USES_SW_SPI
#define TMC_SWSPI_MISO_PIN 26 //PD1 MISO
#define TMC_SWSPI_SCK_PIN  27 //PD2 SCK
#define TMC_SWSPI_MOSI_PIN 28 //PD3 MOSI

//Verbose TMC2130
#define MYSERIAL SerialUSB

const uint8_t TMC_IRUN_CURRENT[] = { 14, 14, 16, 11, 11 }; // Internal Running Current Scale 0..31 for X Y Z E0 E1
const uint8_t TMC_IHOLD_CURRENT[] = { 7,  7,  7,  7,  7 };     // Internal Holding Current Scale 0..31 for X Y Z E0 E1

#endif //ARCHIM 2.0


#if defined( ARCHIM10 )
#define MOTOR_CURRENT_PWM_X_PIN   12 //PD8  REF1 TIOB8
#define MOTOR_CURRENT_PWM_Y_PIN   58 //PA6  REF2 TIOB2
#elif defined( ARCHIM20 )
#define MOTOR_CURRENT_PWM_Y_PIN   12 //PD8  REF1 TIOB8
#define MOTOR_CURRENT_PWM_X_PIN   58 //PA6  REF2 TIOB2
#endif
#define MOTOR_CURRENT_PWM_Z_PIN   10 //PC29 REFZ TIOB7
#define MOTOR_CURRENT_PWM_E0_PIN   3 //PC28 REF3 TIOA7
#define MOTOR_CURRENT_PWM_E1_PIN  11 //PD7  REF4 TIOA8
const Pin MOTOR_CURRENT_PINS[DRIVES] = {
  MOTOR_CURRENT_PWM_X_PIN,
  MOTOR_CURRENT_PWM_Y_PIN,
  MOTOR_CURRENT_PWM_Z_PIN,
  MOTOR_CURRENT_PWM_E0_PIN,
  MOTOR_CURRENT_PWM_E1_PIN
};

// Endstops
// RepRapFirmware only has a single endstop per axis.
// Gcode defines if it is a max ("high end") or min ("low end") endstop and sets if it is active HIGH or LOW.
const Pin END_STOP_PINS[DRIVES] = { //11, 28, 60, 31, 24, 46, 45, 44, X9 };
		//14, //PD4 MIN ES1
		//29, //PD6 MIN ES2
		//31, //PA7 MIN ES3
        32, //PD10 MAX ES1
        15, //PD5 MAX ES2
		30, //PD9 MAX ES3
		31, //RepRap wiki "Configuring RepRapFirmware for a Cartesian printer" refers to this as the E0 end stop to be used for a switch probe. Because reasons?
		29 // Y Min pin for filament endstop
};

// Indices for motor current digipots (if any): first 4 are for digipot 1 (on duet), second 4 for digipot 2 (on expansion board)
//const uint8_t POT_WIPES[8] = { 1, 3, 2, 0, 1, 3, 2, 0 };
//const float SENSE_RESISTOR = 0.1;										// Stepper motor current sense resistor
//const float MAX_STEPPER_DIGIPOT_VOLTAGE = (3.3 * 2.5 / (2.7 + 2.5));	// Stepper motor current reference voltage
//const float STEPPER_DAC_VOLTAGE_RANGE = 2.02;							// Stepper motor current reference voltage for E1 if using a DAC
//const float STEPPER_DAC_VOLTAGE_OFFSET = -0.025;						// Stepper motor current offset voltage for E1 if using a DAC

// HEATERS

const bool HEAT_ON = true;												// false for inverted heater (e.g. Duet v0.6), true for not (e.g. Duet v0.4)

const Pin TEMP_SENSE_PINS[HEATERS] = { // Analogue pin numbers
		11, // A11 PB20 THERM AN3 (BED)
		10, // A10 PB19 THERM AN1 (T0)
		9,  // A9 PB18 THERM AN2  (T1)
		8,  // A8 PB17 THERM AN4  (T2)
};

const Pin HEAT_ON_PINS[HEATERS] = {
		9, // D9 PC21 BED_PWM
		6, // D6 PC24 FET_PWM3
		7, // D7 PC23 FET_PWM4
		8, // D8 PC22 FET_PWM5
};

// Default thermistor parameters
// Bed thermistor: http://uk.farnell.com/epcos/b57863s103f040/sensor-miniature-ntc-10k/dp/1299930?Ntt=129-9930
// Hot end thermistor: http://www.digikey.co.uk/product-search/en?x=20&y=11&KeyWords=480-3137-ND
const float BED_R25 = 100000.0;
const float BED_BETA = 3988.0;
const float BED_SHC = 0.0;
const float EXT_R25 = 100000.0;
const float EXT_BETA = 4388.0;
const float EXT_SHC = 0.0;

// Thermistor series resistor value in Ohms
// On later Duet 0.6 and all Duet 0.8.5 boards it is 4700 ohms. However, if we change the default then machines that have 1K series resistors
// and don't have R1000 in the M305 commands in config.g will overheat. So for safety we leave the default as 1000.
const float THERMISTOR_SERIES_RS = 4700.0;

// Number of SPI temperature sensors to support

#if SUPPORT_ROLAND

// chrishamm's pin assignments
const size_t MaxSpiTempSensors = 2;

// Digital pins the 31855s have their select lines tied to
const Pin SpiTempSensorCsPins[MaxSpiTempSensors] = { 77, 87 };

#else

const size_t MaxSpiTempSensors = 0; //4

// Digital pins the 31855s have their select lines tied to
//const Pin SpiTempSensorCsPins[MaxSpiTempSensors] = { 77, 87, 16, 17 };
const Pin SpiTempSensorCsPins[0] = { };

#endif

// Arduino Due pin number that controls the ATX power on/off
const Pin ATX_POWER_PIN = NoPin; //12;											// Arduino Due pin number that controls the ATX power on/off

// Analogue pin numbers
const Pin Z_PROBE_PIN = NoPin; //10;												// Analogue pin number

// Digital pin number to turn the IR LED on (high) or off (low)
const Pin Z_PROBE_MOD_PIN = NoPin; //52;											// Digital pin number to turn the IR LED on (high) or off (low) on Duet v0.6 and v1.0 (PB21)
const Pin Z_PROBE_MOD_PIN07 = NoPin; //X12;										// Digital pin number to turn the IR LED on (high) or off (low) on Duet v0.7 and v0.8.5 (PC10)

// Pin number that the DAC that controls the second extruder motor current on the Duet 0.8.5 is connected to
const int Dac0DigitalPin = NoPin; //66;											// Arduino Due pin number corresponding to DAC0 output pin

// COOLING FANS
const size_t NUM_FANS = 2;
const Pin COOLING_FAN_PINS[NUM_FANS] = {
		4,   //D4 PC26 FET_PWM1
		5 }; //D5 PC25 FET_PWM2

const Pin COOLING_FAN_RPM_PIN = NoPin; //23;										// Pin PA15

// SD cards
const size_t NumSdCards = 1;
const Pin SdCardDetectPins[NumSdCards] = {85}; //{13, NoPin};
const Pin SdWriteProtectPins[NumSdCards] = {NoPin};
const Pin SdSpiCSPins[1] = {NoPin}; //{67};										// Note: this clashes with inkjet support

#if SUPPORT_INKJET
// Inkjet control pins
const Pin INKJET_SERIAL_OUT = 21;										// Serial bitpattern into the shift register
const Pin INKJET_SHIFT_CLOCK = 20;										// Shift the register
const Pin INKJET_STORAGE_CLOCK = 67;									// Put the pattern in the output register
const Pin INKJET_OUTPUT_ENABLE = 66;									// Make the output visible
const Pin INKJET_CLEAR = 65;											// Clear the register to 0

#endif

#if SUPPORT_ROLAND
#error WTF
// Roland mill
const Pin ROLAND_CTS_PIN = 16;											// Expansion pin 11, PA12_TXD1
const Pin ROLAND_RTS_PIN = 17;											// Expansion pin 12, PA13_RXD1

#endif

// M42 and M208 commands now use logical pin numbers, not firmware pin numbers.
// This is the mapping from logical pins 60+ to firmware pin numbers
const Pin SpecialPinMap[] =
{
	NoPin
//	19, 18,	17, 16, 23, 	// PA10/RXD0 PA11/TXD0 PA12/RXD1 PA13/TXD1 PA14/RTS1
//	20, 21, 67, 52, 		// PB12/TWD1 PB13/TWCK1 PB16/DAC1 PB21/AD14
//	36						// PC4
};

// This next definition defines the highest one.
const int HighestLogicalPin = 60 + ARRAY_SIZE(SpecialPinMap) - 1;		// highest logical pin number on this electronics

// SAM3X Flash locations (may be expanded in the future)
const uint32_t IAP_FLASH_START = 0x000F0000;
const uint32_t IAP_FLASH_END = 0x000FFBFF;		// don't touch the last 1KB, it's used for NvData

// Duet pin numbers to control the WiFi interface
//(ESP-PIN)  //(SAM-PIN)
//RST //PB27 D13
//EN  //PA3 D60
//GPIO0 //PB25 D2
//GPIO4 //PB13 D21
//CS0 //PIO_PA29A_SPI0_NPCS1 D87
const Pin EspResetPin = 20; //PB12 //13;	//PB27 D13					// Low on this in holds the WiFi module in reset (ESP_RESET)
const Pin EspEnablePin = 66; //PB15 //60; //PA3 D60					// High to enable the WiFi module, low to power it down (ESP_CH_PD)
const Pin EspTransferRequestPin = 89; //PB14 //2; //PB25 D2			// Input from the WiFi module indicating that it wants to transfer data (ESP GPIO0)
const Pin SamTfrReadyPin =  21; //PB13 D21				// Output from the SAM to the WiFi module indicating we can accept a data transfer (ESP GPIO4 via 7474)
const Pin SamCsPin = 77; //PIO_PA29A_SPI0_NPCS1 D87		// SPI NPCS pin, input from WiFi module
const Pin EspUartTXD = 1; //UTXD PA9 D1 (5-pin host connector on Archim)
#define USES_WIFI
/*
// Timer allocation
#define NETWORK_TC			(TC1)
#define NETWORK_TC_CHAN		(1)
#define NETWORK_TC_IRQN		TC4_IRQn
#define NETWORK_TC_HANDLER	TC4_Handler
*/
#define STEP_TC				(TC1)
#define STEP_TC_CHAN		(0)
#define STEP_TC_IRQN		TC3_IRQn
#define STEP_TC_HANDLER		TC3_Handler

#endif
