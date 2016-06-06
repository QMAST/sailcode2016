#include <cantedKeel.h>
//#include <LSM303.h>
#include <math.h>
#include <Wire.h>
#include <SoftwareSerial.h>

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

SoftwareSerial SERIAL_PORT_POLOLU(SERIAL_SW_POLOLU_RXPIN, SERIAL_SW_POLOLU_TXPIN);

Canted_Keel *canted_keel = new Canted_Keel(POLOLU_NUMBER, &SERIAL_PORT_POLOLU,
							      POTENTIOMETER_PIN, MAX_POTENTIOMETER_VOLTAGE,
								  MIN_POTENTIOMETER_VOLTAGE, CENTER_POTENTIOMETER_VOLTAGE,
								  MAX_KEEL_ANGLE);


double keel_angle;	
bool time_to_refresh = true;
int time_till_next_refresh = 0;						  
/*uint32_t timer_pos = millis() * 2;
uint32_t timer_neg = millis() * 2;
uint32_t timer_center = millis() * 2;
bool center_position = false;
bool neg_position = false;
bool pos_position = false;*/

typedef struct vect3f{
	double x;
	double y;
	double z;
} vect3f;

void setup()
{
	canted_keel->init();
	SERIAL_PORT_POLOLU.begin(SERIAL_BAUD_POLOLU);
	Serial.begin(57600);
	Wire.begin();
	initLSM303(8);
}

void loop()
{
	/*canted_keel->setAngle(0, 1200);
	Serial.println(canted_keel->getAngle());
	delay(100);*/
	vect3f acceleration = getLSM303_accel();
	
	// Re-ajust angle
	if(time_to_refresh)
	{
		time_to_refresh = false;
		if(acceleration.y > 1800)
		{
			keel_angle += 5;
		}
		else if (acceleration.y < -1800)
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
	if (time_till_next_refresh >= millis()){
		time_to_refresh = true;
	}
	canted_keel->setAngle(keel_angle, KEEL_SPEED_MED);
	Serial.println(acceleration.y);
	delay(50);
	
	////////////////////////////////////////////////////////////////////////////////////////	
	/*if (acceleration.y > 1800)
	{
		if (!pos_position)
		{
			timer_pos = MOVE_KEEL_DELAY + millis(); //wait before moving keel
			center_position = false;
			neg_position = false;
			pos_position = true;
		}
	}
	else if (acceleration.y < -1800)
	{
		if (!neg_position)
		{
			timer_neg = MOVE_KEEL_DELAY + millis(); //wait before moving keel
			center_position = false;
			neg_position = true;
			pos_position = false;
		}
	}
	else // -1800 <= acceleration.y <= 1800
	{
		if (!center_position)
		{
			timer_center = MOVE_KEEL_DELAY + millis(); //wait before moving keel
			center_position = true;
			neg_position = false;
			pos_position = false;
		}		
	}
	
	if (timer_pos <= millis())
	{
		Serial.println("Setting keel to positive angle...");
		canted_keel->setAngle(20, KEEL_SPEED_MED);
		timer_neg = millis() * 2;
		timer_center = millis() * 2;
	}
	if (timer_neg <= millis())
	{
		Serial.println("Setting keel to negative angle...");
		canted_keel->setAngle(-20, KEEL_SPEED_MED);
		timer_center = millis() * 2;
		timer_pos = millis() * 2;
	}
	if (timer_center <= millis())
	{
		Serial.println("Setting keel to center angle...");
		canted_keel->setAngle(0, KEEL_SPEED_MED);
		timer_neg = millis() * 2;
		timer_pos = millis() * 2;
	}*/
	
	/*Serial.print("x: ");
	Serial.print(acceleration.x);
	Serial.print("\ty: ");
	Serial.print(acceleration.y);
	Serial.print("\tz: ");
	Serial.println(acceleration.z);
	delay(50);*/
}

void initLSM303(int fs)
{
  LSM303_write(0x27, CTRL_REG1_A);  // 0x27 = normal power mode, all accel axes on
  if ((fs==8)||(fs==4))
    LSM303_write((0x00 | (fs-fs/2-1)<<4), CTRL_REG4_A);  // set full-scale
  else
    LSM303_write(0x00, CTRL_REG4_A);
  LSM303_write(0x14, CRA_REG_M);  // 0x14 = mag 30Hz output rate
  LSM303_write(0x00, MR_REG_M);  // 0x00 = continouous conversion mode
}

vect3f getLSM303_accel()
{
	vect3f rawValues;
	rawValues.z = -((int)LSM303_read(OUT_X_L_A) << 8) | (LSM303_read(OUT_X_H_A));
	rawValues.x = ((int)LSM303_read(OUT_Y_L_A) << 8) | (LSM303_read(OUT_Y_H_A));
	rawValues.y = ((int)LSM303_read(OUT_Z_L_A) << 8) | (LSM303_read(OUT_Z_H_A));
	return rawValues;
}

byte LSM303_read(byte address)
{
  byte temp;
  
  if (address >= 0x20)
    Wire.beginTransmission(LSM303_ACC);
  else
    Wire.beginTransmission(LSM303_MAG);
    
  Wire.write(address);
  
  if (address >= 0x20)
    Wire.requestFrom(LSM303_ACC, 1);
  else
    Wire.requestFrom(LSM303_MAG, 1);
  while(!Wire.available());
  temp = Wire.read();
  Wire.endTransmission();
  
  return temp;
}

void LSM303_write(byte data, byte address)
{
  if (address >= 0x20)
    Wire.beginTransmission(LSM303_ACC);
  else
    Wire.beginTransmission(LSM303_MAG);
    
  Wire.write(address);
  Wire.write(data);
  Wire.endTransmission();
}

