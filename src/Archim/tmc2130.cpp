
#include "Pins_Archim.h"
#include "TMC2130Stepper.h"

#ifndef MYSERIAL
	#define MYSERIAL(x)
#endif

#ifdef TMC2130_USES_SW_SPI_nvm_using_library_now
  #define tmc_transfer(x) shiftData( TMC_SWSPI_MOSI_PIN, TMC_SWSPI_SCK_PIN, TMC_SWSPI_MISO_PIN, MSBFIRST, x)

  //Combined shiftOut and shiftIn from Arduino wiring_shift.c
  uint32_t shiftData( uint32_t ulDataOutPin,  uint32_t ulClockPin, uint32_t ulDataInPin, uint32_t ulBitOrder, uint32_t ulVal )
  {
    uint8_t value = 0 ;
    uint8_t i ;

    for ( i=0 ; i < 8 ; ++i )
      {
      // Write bit
      if ( ulBitOrder == LSBFIRST )
          {
        digitalWrite( ulDataOutPin, !!(ulVal & (1 << i)) ) ;
          }
      else
          {
        digitalWrite( ulDataOutPin, !!(ulVal & (1 << (7 - i))) ) ;
          }

      // Start clock pulse
      digitalWrite( ulClockPin, HIGH ) ;

      // Read bit
      if ( ulBitOrder == LSBFIRST )
          {
        value |= (digitalRead( ulDataInPin ) ? 1 : 0) << i ;
          }
      else
          {
        value |= (digitalRead( ulDataInPin ) ? 1 : 0) << (7 - i) ;
          }

      // Stop clock pulse
      digitalWrite( ulClockPin, LOW ) ;
    }

    return value;
  }
#endif
#if 0
void tmc2130_transfer_begin() {
  #ifdef TMC2130_USES_HW_SPI
    SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE3));
  #endif
}

void tmc2130_transfer_end() {
  #ifdef TMC2130_USES_HW_SPI
    SPI.endTransaction();
  #endif
}

byte tmc2130_transfer(uint8_t val) {
  #ifdef TMC2130_USES_HW_SPI
    return SPI.transfer(val);
  #endif

  #ifdef TMC2130_USES_SW_SPI
    return tmc_transfer(val);
  #endif
}

void tmc2130_write(uint8_t chipselect, uint8_t address,uint8_t wval1,uint8_t wval2,uint8_t wval3,uint8_t wval4)
{
  uint32_t val32;
  uint8_t val0;
  uint8_t val1;
  uint8_t val2;
  uint8_t val3;
  uint8_t val4;

  //datagram1 - write
  tmc2130_transfer_begin();
  digitalWrite(chipselect,LOW);
  tmc2130_transfer(address+0x80);
  tmc2130_transfer(wval1);
  tmc2130_transfer(wval2);
  tmc2130_transfer(wval3);
  tmc2130_transfer(wval4);
  digitalWrite(chipselect, HIGH);
  tmc2130_transfer_end();

  //datagram2 - response
  tmc2130_transfer_begin();
  digitalWrite(chipselect,LOW);
  val0 = tmc2130_transfer(0);
  val1 = tmc2130_transfer(0);
  val2 = tmc2130_transfer(0);
  val3 = tmc2130_transfer(0);
  val4 = tmc2130_transfer(0);
  digitalWrite(chipselect, HIGH);
  tmc2130_transfer_end();

  MYSERIAL.print("WriteRead 0x");
  MYSERIAL.print(address,HEX);
  MYSERIAL.print(" Status:");
  MYSERIAL.print(val0 & 0b00000111,BIN);
  MYSERIAL.print("  ");
  MYSERIAL.print(val1,BIN);
  MYSERIAL.print("  ");
  MYSERIAL.print(val2,BIN);
  MYSERIAL.print("  ");
  MYSERIAL.print(val3,BIN);
  MYSERIAL.print("  ");
  MYSERIAL.print(val4,BIN);

  val32 = (uint32_t)val1<<24 | (uint32_t)val2<<16 | (uint32_t)val3<<8 | (uint32_t)val4;
  MYSERIAL.print(" 0x");
  MYSERIAL.println(val32,HEX);
}

void tmc2130_chopconf(uint8_t cs, bool extrapolate256 = 1, uint16_t microstep_resolution = 16)
{
  uint8_t mres=0b0100;
  if(microstep_resolution == 256) mres = 0b0000;
  if(microstep_resolution == 128) mres = 0b0001;
  if(microstep_resolution == 64)  mres = 0b0010;
  if(microstep_resolution == 32)  mres = 0b0011;
  if(microstep_resolution == 16)  mres = 0b0100;
  if(microstep_resolution == 8)   mres = 0b0101;
  if(microstep_resolution == 4)   mres = 0b0110;
  if(microstep_resolution == 2)   mres = 0b0111;
  if(microstep_resolution == 1)   mres = 0b1000;

  mres |= extrapolate256 << 4; //bit28 intpol

  tmc2130_write(cs,0x6C,mres,1,00,0xC5);
}


void tmc2130_init2()
{
#ifdef HAVE_TMC2130_DRIVERS
  #ifdef TMC2130_USES_HW_SPI
    //pinMode(PIN_SPI_SS0,OUTPUT);  // PA25
    pinMode(PIN_SPI_MOSI,OUTPUT); // PA10
    pinMode(PIN_SPI_SCK,OUTPUT);  // PB0
    SPI.end();
    SPI.begin();
  #endif

  #ifdef TMC2130_USES_SW_SPI
    pinMode(TMC_SWSPI_MISO_PIN, INPUT );
    pinMode(TMC_SWSPI_SCK_PIN, OUTPUT_LOW );
    pinMode(TMC_SWSPI_MOSI_PIN, OUTPUT_LOW );
  #endif

  for(uint8_t i=0;i<5;i++) {
    //digitalWrite(CS_PINS[i],HIGH);
    pinMode(CS_PINS[i],OUTPUT_HIGH);
  }

  for(uint8_t i=0;i<5;i++)
  {
	// Interpolated 32 Microstepping to 256 microstepping
    tmc2130_chopconf(CS_PINS[i],1,32);

	tmc2130_write(CS_PINS[i],0x10,0,15,TMC_IRUN_CURRENT[i],TMC_IHOLD_CURRENT[i]); //0x10 IHOLD_IRUN
	tmc2130_write(CS_PINS[i],0x0,0,0,0,0b100); //address=0x0 GCONF EXT VREF - STEALTH CHOP

	// pwm_autoscale=1 pwm_freq=0
    tmc2130_write(CS_PINS[i],0x70,0,0b100,0x01,0xC8); //address=0x70 PWM_CONF //reset default=0x00050480
  }
  
#endif //HAVE_TMC2130_DRIVERS
}

#endif //0

void tmc2130_init() {
	TMC2130Stepper stepperX(ENABLE_PINS[0], DIRECTION_PINS[0], STEP_PINS[0], CS_PINS[0]);
	TMC2130Stepper stepperY(ENABLE_PINS[1], DIRECTION_PINS[1], STEP_PINS[1], CS_PINS[1]);
	TMC2130Stepper stepperZ(ENABLE_PINS[2], DIRECTION_PINS[2], STEP_PINS[2], CS_PINS[2]);
	TMC2130Stepper stepperE0(ENABLE_PINS[3], DIRECTION_PINS[3], STEP_PINS[3], CS_PINS[3]);
	TMC2130Stepper stepperE1(ENABLE_PINS[4], DIRECTION_PINS[4], STEP_PINS[4], CS_PINS[4]);
}

#define HOLD_MULTIPLIER 0.5
#define R_SENSE 0.22
#define INTERPOLATE true

void tmc2130_init_stepper(TMC2130Stepper &st)
{
	st.begin();
    st.setCurrent(900, R_SENSE, HOLD_MULTIPLIER);
    st.microsteps(32);
    st.interpolate(INTERPOLATE);
}


