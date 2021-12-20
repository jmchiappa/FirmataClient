#ifndef FirmataClient_h
#define FirmataClient_h

#include "Arduino.h"
#include <Servo.h>
#include <Wire.h>
#include <Firmata.h>
// In order to use software serial, you will need to compile this sketch with
// Arduino IDE v1.6.6 or higher. Hardware serial should work back to Arduino 1.0.
#include "utility/SerialFirmata.h"

#define I2C_WRITE                   B00000000
#define I2C_READ                    B00001000
#define I2C_READ_CONTINUOUSLY       B00010000
#define I2C_STOP_READING            B00011000
#define I2C_READ_WRITE_MODE_MASK    B00011000
#define I2C_10BIT_ADDRESS_MODE_MASK B00100000
#define I2C_END_TX_MASK             B01000000
#define I2C_STOP_TX                 1
#define I2C_RESTART_TX              0
#define I2C_MAX_QUERIES             8
#define I2C_REGISTER_NOT_SPECIFIED  -1

#define SYSEX_CMD_PROXIMITY	0x01
#define SYSEX_CMD_STEPPER  	0x02
#define SYSEX_CMD_STRIPLED 	0x03
#define SYSEX_CMD_COLOR    	0x04
#define SYSEX_CMD_OLED			0x05
#define SYSEX_ALT_CONFIG    0x06
#define PERIPHERAL_OPT			0x40 // dedicated response for CAPABILITY_QUERY message (pin mode table)

// the minimum interval for sampling analog input
#define MINIMUM_SAMPLING_INTERVAL   1

/* i2c data */
struct i2c_device_info {
  byte addr;
  int reg;
  byte bytes;
  byte stopTX;
};


#ifdef __cplusplus
 extern "C" {
#endif
void firmata_begin(HardwareSerial &serial,uint32_t Baudrate);
void firmata_execute(void);
void wireWrite(byte data);
byte wireRead(void);
void attachServo(byte pin, int minPulse, int maxPulse);
void detachServo(byte pin);
void enableI2CPins(void);
void disableI2CPins();
void readAndReportData(byte address, int theRegister, byte numBytes, byte stopTX);
void outputPort(byte portNumber, byte portValue, byte forceSend);
void checkDigitalInputs(void);
void setPinModeCallback(byte pin, int mode);
void setPinValueCallback(byte pin, int value);
void analogWriteCallback(byte pin, int value);
void digitalWriteCallback(byte port, int value);
void reportAnalogCallback(byte analogPin, int value);
void reportDigitalCallback(byte port, int value);
void sysexCallback(byte command, byte argc, byte *argv);
void systemResetCallback();
uint8_t decodeByteStream(size_t bytec, uint8_t * bytev);
#ifdef __cplusplus
}
#endif

#endif // FirmataClient_h