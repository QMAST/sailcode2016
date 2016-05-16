#include <SoftwareSerial.h>
#define MSG 0x01

SoftwareSerial mySerial(10, 11);

void setup() {
  Serial.begin(57600);
  mySerial.begin(19200);
}

void loop() {
  /*mySerial.write(5);
  if (mySerial.available()) {
    Serial.write(mySerial.read());
  }*/
  if (Serial.available()) {
    mySerial.write(Serial.read());
    Serial.read();
  }
  if (mySerial.available()) {
    Serial.write(mySerial.read());
  }  
}
