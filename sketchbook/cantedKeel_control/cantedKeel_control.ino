#include <cantedKeel.h>
//#include <LSM303.h>
#include <math.h>
#include <SoftwareSerial.h>
#include <Adafruit_ADXL345_U.h>

#define SERIAL_BAUD_POLOLU  9600
#define SERIAL_SW_POLOLU_RXPIN 4
#define SERIAL_SW_POLOLU_TXPIN 5
#define POLOLU_NUMBER 12
#define POTENTIOMETER_PIN A0
#define MAX_POTENTIOMETER_VOLTAGE 1.62
#define MIN_POTENTIOMETER_VOLTAGE 0.81
#define CENTER_POTENTIOMETER_VOLTAGE 1.21
#define MAX_KEEL_ANGLE 35

//Keel speed must be a number between 0 and 1000
#define KEEL_SPEED_HIGH 1000
#define KEEL_SPEED_MED 800
#define KEEL_SPEED_LOW 500

#define ACCELEROMETER_TIMEOUT 1000 // msec

/* LSM303 Address definitions */
#define LSM303_MAG  0x1E  // assuming SA0 grounded
#define LSM303_ACC  0x18  // assuming SA0 grounded

/* LSM303 Register definitions */
#define CTRL_REG1_A 0x20
#define CTRL_REG4_A 0x23
#define OUT_X_L_A 0x28
#define OUT_X_H_A 0x29
#define OUT_Y_L_A 0x2A
#define OUT_Y_H_A 0x2B
#define OUT_Z_L_A 0x2C
#define OUT_Z_H_A 0x2D
#define CRA_REG_M 0x00
#define MR_REG_M 0x02

#define MOVE_KEEL_DELAY 2000 // number of milliseconds to wait after boat surpasses threshold angle before activating keel

typedef struct vect3f{
	double x;
	double y;
	double z;
} vect3f;

SoftwareSerial SERIAL_PORT_POLOLU(SERIAL_SW_POLOLU_RXPIN, SERIAL_SW_POLOLU_TXPIN);

Canted_Keel *canted_keel = new Canted_Keel(POLOLU_NUMBER, &SERIAL_PORT_POLOLU,
							      POTENTIOMETER_PIN, MAX_POTENTIOMETER_VOLTAGE,
								  MIN_POTENTIOMETER_VOLTAGE, CENTER_POTENTIOMETER_VOLTAGE,
								  MAX_KEEL_ANGLE);
Adafruit_ADXL345_Unified *accelerometer = new Adafruit_ADXL345_Unified();

double keel_angle = 0;	
bool time_to_refresh = true;
int time_till_next_refresh = 0;
vect3f acceleration;

void setup()
{
	canted_keel->init();
	SERIAL_PORT_POLOLU.begin(SERIAL_BAUD_POLOLU);
	Serial.begin(57600);
	accelerometer->begin();
	accelerometer->setRange(ADXL345_RANGE_2_G);
}

void loop()
{
	canted_keel->setAngle(10, KEEL_SPEED_HIGH);
	/*if(time_to_refresh)
	{		
		time_to_refresh = false;
		
		acceleration.y = get_avg_y_accel(10);
		
		if(acceleration.y > 100)
		{
			keel_angle += 5;
		}
		else if (acceleration.y < -100)
		{
			keel_angle -= 5;
		}
		
		// Check limits
		if (keel_angle > 35)
		{
			keel_angle = 35;
		}
		else if (keel_angle < -35)
		{
			keel_angle = -35;
		}
		time_till_next_refresh = MOVE_KEEL_DELAY + millis();
	}
	if (time_till_next_refresh <= millis()){
		time_to_refresh = true;
	}
	canted_keel->setAngle(keel_angle, KEEL_SPEED_HIGH);
	Serial.println(keel_angle);*/
	delay(50);
}

double get_avg_y_accel(int n)
{
	double sum = 0;
	for (int i = 0; i < n; i++ ){
		sum += accelerometer->getY();
	}
	return sum / n;
}

