#include "FirmataClient.h"

HardwareSerial SerialPort(PA_10, PA_9); // USART 1

void setup() {
	firmata_begin(SerialPort,115200);
}

void loop() {
	firmata_execute();
}