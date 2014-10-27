#include <barnacle_client.h>
#include <SoftwareSerial.h>

SoftwareSerial* barnacle;

void setup() {
    // put your setup code here, to run once:
    Serial.begin(19200);
    barnacle = new SoftwareSerial( 10, 11 );
    barnacle->begin(19200);

    barnacle_port = barnacle;

}

void loop() {
    // put your main code here, to run repeatedly:
    /*if( Serial.available() ) barnacle->write( (uint8_t) Serial.read() );*/
    /*if( barnacle->available() ) Serial.print( barnacle->read(), HEX );*/
    static char buf[80];

    snprintf_P( buf, sizeof(buf),
        PSTR("A0: %u A1: %u"),
        barn_get_battery_voltage(),
        barn_get_battery_current() );
    Serial.println(buf);
    delay(500);
}
