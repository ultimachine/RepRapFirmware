#ifndef PINS_ARCHIM_H__
#define PINS_ARCHIM_H__

#define FIRMWARE_NAME "RepRapFirmware for Archim"

const size_t NumFirmwareUpdateModules = 1;
#define IAP_UPDATE_FILE "iap.bin"
#define IAP_FIRMWARE_FILE "RepRapFirmware.bin"

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

#define MOTOR_CURRENT_PWM_X_PIN   12 //PD8  REF1 TIOB8
#define MOTOR_CURRENT_PWM_Y_PIN   58 //PA6  REF2 TIOB2
#define MOTOR_CURRENT_PWM_Z_PIN   10 //PC29 REFZ TIOB7
#define MOTOR_CURRENT_PWM_E0_PIN   3 //PC28 REF3 TIOA7
#define MOTOR_CURRENT_PWM_E1_PIN  11 //PD7  REF4 TIOA8

// Endstops
// RepRapFirmware only has a single endstop per axis.
// Gcode defines if it is a max ("high end") or min ("low end") endstop and sets if it is active HIGH or LOW.
const Pin END_STOP_PINS[DRIVES] = { //11, 28, 60, 31, 24, 46, 45, 44, X9 };
		14, //PD4 MIN ES1
		29, //PD6 MIN ES2
		31, //PA7 MIN ES3
		NoPin,NoPin
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
		A10, // A10 PB19 THERM AN1 (T0)
		A9,  // A9 PB18 THERM AN2  (T1)
		A8,  // A8 PB17 THERM AN4  (T2)
		A11, // A11 PB20 THERM AN3 (BED)
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
#error WTF_INKJECT
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

// Timer allocation
#define NETWORK_TC			(TC1)
#define NETWORK_TC_CHAN		(1)
#define NETWORK_TC_IRQN		TC4_IRQn
#define NETWORK_TC_HANDLER	TC4_Handler

#define STEP_TC				(TC1)
#define STEP_TC_CHAN		(0)
#define STEP_TC_IRQN		TC3_IRQn
#define STEP_TC_HANDLER		TC3_Handler

#endif
