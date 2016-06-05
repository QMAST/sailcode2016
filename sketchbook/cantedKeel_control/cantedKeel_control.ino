#include <cantedKeel.h>
#include <LSM303.h>
#include <math.h>

#define SERIAL_PORT_POLOLU Serial
#define SERIAL_BAUD_POLOLU  9600
#define POLOLU_NUMBER 12
#define POTENTIOMETER_PIN A0
#define MAX_POTENTIOMETER_VOLTAGE 1.62
#define MIN_POTENTIOMETER_VOLTAGE 0.81
#define CENTER_POTENTIOMETER_VOLTAGE 1.24
#define MAX_KEEL_ANGLE 35

//Keel speed must be a number between 0 and 1000
#define KEEL_SPEED_HIGH 1000
#define KEEL_SPEED_MED 800
#define KEEL_SPEED_LOW 500

Canted_Keel canted_keel;
LSM303 accelerometer;
										  
void setup()
{
	SERIAL_PORT_POLOLU.begin(SERIAL_BAUD_POLOLU);
	canted_keel = new Canted_Keel(POLOLU_NUMBER, &SERIAL_PORT_POLOLU,
							      POTENTIOMETER_PIN, MAX_POTENTIOMETER_VOLTAGE,
								  MIN_POTENTIOMETER_VOLTAGE, CENTER_POTENTIOMETER_VOLTAGE,
								  MAX_KEEL_ANGLE);
	accelerometer.init(device_auto, sa0_auto);
	attachInterrupt( digitalPinToInterrupt(ENCODER_PIN), count_tick, FALLING );
}

void loop()
{
	cantedKeel.setPosition(0, KEEL_SPEED_MED);
	accelerometer.readAcc();
	
}