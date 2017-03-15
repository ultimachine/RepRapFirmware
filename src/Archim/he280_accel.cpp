#include <He280_accel.h>

#include <Wire.h>

#define ACCELEROMETER_I2C_ADDR 0x19

#define Z_PROBE_SENSITIVITY 20

#define PSTR(x) (x)

#define He280 Wire1

void accelerometer_send(uint8_t val)
{
  He280.beginTransmission(ACCELEROMETER_I2C_ADDR);
  He280.write(val);

  uint8_t ret = He280.endTransmission(false);
  if(ret) {
    //Myserial.println(F("send i2c error."));
    SerialUSB.print(PSTR("accelerometer send i2c error: "));
    SerialUSB.println(ret);
  }
}

void accelerometer_write(uint8_t reg, uint8_t val)
{
  He280.beginTransmission(ACCELEROMETER_I2C_ADDR);
  He280.write(reg);
  He280.write(val);
  //He280.endTransmission();
  //return;
  if(He280.endTransmission())
    //Myserial.println(F("write i2c error."));
    SerialUSB.println(PSTR("accelerometer write i2c error."));
}

void accelerometer_recv(uint8_t reg)
{
  uint8_t receiveByte;

  accelerometer_send(reg); //Send an 8bit register to be read

  He280.requestFrom(ACCELEROMETER_I2C_ADDR,1); //Request one 8bit response

  //delayMicroseconds(200);
  if(He280.available())
  {
    receiveByte = He280.read();

    SerialUSB.print(PSTR("read reg "));
    SerialUSB.print(reg);
    SerialUSB.print(PSTR(" value: "));
    SerialUSB.println(receiveByte);
  }
  else
  {
    SerialUSB.println(PSTR("accelerometer i2c recv error."));
    Serial.println(PSTR("i2c recv error."));
  }
}

void accelerometer_init()
{
  //SerialUSB.println(PSTR("iis2dh accelerometer initializing..."));
  He280.begin(); // join i2c bus

  accelerometer_recv(0x0F); //WHO AM I = 0x6A

  accelerometer_recv(0x31); //INT1_SRC (31h)

  //CTRL_REG1 (20h)
  accelerometer_recv(0x20);
  accelerometer_write(0x20,0b10011100); // ODR 5.376kHz in LPMode [7-4]. Low power enable [3]. Z enable [2].
  accelerometer_recv(0x20);

  //CTRL_REG4 (23h)
  accelerometer_recv(0x23);
  accelerometer_write(0x23,0b00110000); // Full-scale selection 16G [5-4]. High resolution mode [3].
  accelerometer_recv(0x23);


  //CTRL_REG5 (24h)
  accelerometer_recv(0x24);
  accelerometer_write(0x24,0b01001010); // FIFO enable [6]. Latch INT1 [3]. Latch INT2 until cleared by read [1].
  accelerometer_recv(0x24);

  //INT1_CFG (30h)
  accelerometer_recv(0x30);
  accelerometer_write(0x30,0b100000); // ZHI events enabled [5]. ZLO events enabled [4].
  accelerometer_recv(0x30);

  //INT1_SRC (31h)
  accelerometer_recv(0x31);

  //INT1_THS (32h)  this is the i2c probe
  accelerometer_recv(0x32);
  accelerometer_write(0x32,Z_PROBE_SENSITIVITY); // 7bits
  accelerometer_recv(0x32);

  //INT1_DURATION (33h)
  accelerometer_recv(0x33);
  accelerometer_write(0x33,0);
  accelerometer_recv(0x33);


}

void accelerometer_status()
{
    accelerometer_recv(0x31); //INT1_SRC (31h)
    accelerometer_recv(0x2D);
}
