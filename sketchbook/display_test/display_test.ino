#include <SoftwareSerial.h>

SoftwareSerial* display;

void setup() {
    // put your setup code here, to run once:
    display = new SoftwareSerial( 7, 8 );
    display->begin(9600);
    delay(500);
    // put your main code here, to run repeatedly: 
    display->write(254);
    display->write(128);
    display->write("QMAST");

    display->write(254);
    display->write(192);
    display->write("Haha, science!");
}

void loop() {
    static uint32_t count = 0;

    display->write(254);
    display->write(135);
    display->print(count++);
    delay(100);
}
