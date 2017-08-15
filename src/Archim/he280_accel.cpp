#include <he280_accel.h>

#include <Wire.h>

#define ACCELEROMETER_I2C_ADDR 0x19

#define Z_PROBE_SENSITIVITY 20

#define PSTR(x) (x)

#define He280 Wire1

uint8_t AccelerometerSend(uint8_t val)
{
  He280.beginTransmission(ACCELEROMETER_I2C_ADDR);
  He280.write(val);

  uint8_t ret = He280.endTransmission(false);
  if(ret) {
    SerialUSB.print(PSTR("accelerometer send i2c error: "));
    SerialUSB.println(ret);
    return -1;
  }
  else{
    return 1;
  }
}

uint8_t AccelerometerWrite(uint8_t reg, uint8_t val)
{
  He280.beginTransmission(ACCELEROMETER_I2C_ADDR);
  He280.write(reg);
  He280.write(val);
  //He280.endTransmission();
  //return;
  if(He280.endTransmission()){
    //Myserial.println(F("write i2c error."));
    SerialUSB.println(PSTR("accelerometer write i2c error."));
    return -1;
  }
  else{
    return 1;
  }
}

uint8_t AccelerometerRecv(uint8_t reg)
{
  uint8_t receiveByte;

  AccelerometerSend(reg); //Send an 8bit register to be read

  He280.requestFrom(ACCELEROMETER_I2C_ADDR,1); //Request one 8bit response

  //delayMicroseconds(200);
  if(He280.available())
  {
    receiveByte = He280.read();

//    SerialUSB.print(PSTR("read reg "));
//    SerialUSB.print(reg);
//    SerialUSB.print(PSTR(" value: "));
//    SerialUSB.println(receiveByte);
    return(receiveByte);
  }
  else
  {
    SerialUSB.println(PSTR("accelerometer i2c recv error."));
    Serial.println(PSTR("i2c recv error."));
    return -1;
  }
}

uint8_t AccelerometerInit()
{

  uint8_t retVal = 1;

  //SerialUSB.println(PSTR("iis2dh accelerometer initializing..."));
  He280.begin(); // join i2c bus

  if(AccelerometerRecv(0x0F) < 0) retVal = -1; //WHO AM I = 0x6A

  if(AccelerometerRecv(0x31) < 0) retVal = -1; //INT1_SRC (31h)

  //CTRL_REG1 (20h)
  if(AccelerometerRecv(0x20) < 0) retVal = -1;
  AccelerometerWrite(0x20,0b10011100); // ODR 5.376kHz in LPMode [7-4]. Low power enable [3]. Z enable [2].
  if(AccelerometerRecv(0x20) < 0) retVal = -1;

  //CTRL_REG3 (22h)
  if(AccelerometerRecv(0x22) < 0) retVal = -1;
  AccelerometerWrite(0x22,0b01000000); // CLICK interrupt on INT1 pin [7]. AOI (And Or Interrupt) on INT1 en [6]. AOI on INT2 en [5].
  if(AccelerometerRecv(0x22) < 0) retVal = -1;

  //CTRL_REG4 (23h)
  if(AccelerometerRecv(0x23) < 0) retVal = -1;
  AccelerometerWrite(0x23,0b00110000); // Full-scale selection 16G [5-4]. High resolution mode [3].
  if(AccelerometerRecv(0x23) < 0) retVal = -1;


  //CTRL_REG5 (24h)
  if(AccelerometerRecv(0x24) < 0) retVal = -1;
  AccelerometerWrite(0x24,0b01001010); // FIFO enable [6]. Latch INT1 [3]. Latch INT2 until cleared by read [1].
  if(AccelerometerRecv(0x24) < 0) retVal = -1;

#ifdef HE280_INVERT_INTERRUPT
  if(AccelerometerRecv(0x25) < 0) retVal = -1;
  AccelerometerWrite(0x25,0b10); //Interrupt outputs Active-LOW signal from chip.
  if(AccelerometerRecv(0x25) < 0) retVal = -1;
#endif

  //INT1_CFG (30h)
  if(AccelerometerRecv(0x30) < 0) retVal = -1;
  AccelerometerWrite(0x30,0b100000); // ZHI events enabled [5]. ZLO events enabled [4].
  if(AccelerometerRecv(0x30) < 0) retVal = -1;

  //INT1_SRC (31h)
  if(AccelerometerRecv(0x31) < 0) retVal = -1;

  //INT1_THS (32h)  this is the i2c probe
  if(AccelerometerRecv(0x32) < 0) retVal = -1;
  AccelerometerWrite(0x32,Z_PROBE_SENSITIVITY); // 7bits
  if(AccelerometerRecv(0x32) < 0) retVal = -1;

  //INT1_DURATION (33h)
  if(AccelerometerRecv(0x33) < 0) retVal = -1;
  AccelerometerWrite(0x33,0);
  if(AccelerometerRecv(0x33) < 0) retVal = -1;

  return(retVal);

}

uint8_t AccelerometerStatus()
{
  uint8_t retVal = 1;
  if(AccelerometerRecv(0x31) < 0) retVal = -1; //INT1_SRC (31h)
  if(AccelerometerRecv(0x2D) < 0) retVal = -1;
  return(retVal);
}
