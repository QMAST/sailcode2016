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

#define CMD_MODE_AUTO 0x01
#define CMD_MODE_MANUAL 0x02
#define CMD_LOCK_CENTER 0x03
#define CMD_POSE_CENTER 0x04
#define CMD_POSE_FULL_PORT 0x05
#define CMD_POSE_FULL_STARBOARD 0x06

#define MOVE_KEEL_DELAY 2000 // number of milliseconds to wait after boat surpasses threshold angle before activating keel

typedef struct vect3f{
	double x;
	double y;
	double z;
} vect3f;

typedef enum mode_t{
	MODE_AUTO, MODE_MANUAL
} mode_t;

typedef enum manual_position_t{
	CENTER, FULL_PORT, FULL_STARBOARD
} manual_position_t;

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
mode_t keel_mode = MODE_AUTO; //default
manual_position_t keel_manual_position = CENTER;

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
	if (Serial.available() > 0) 
	{
		uint8_t incoming_cmd = Serial.read();
		if(process_serial(incoming_cmd)){
			Serial.write(0xAA); // Tell master that command has been processed successfully
		}
		else{
			Serial.write(0xFF); // Tell user error processing command
		}
	}
	
	if (keel_mode == MODE_AUTO)
	{
		//TODO: set auto timeout protection
		auto_operation();
	}
	else if (keel_mode == MODE_MANUAL)
	{
		manual_operation();
	}
}

bool process_serial(uint8_t incoming_cmd)
{
	bool success = true;
	if (incoming_cmd == CMD_MODE_AUTO){
		keel_mode = MODE_AUTO;
	}
	else if (incoming_cmd == CMD_MODE_MANUAL){
		keel_mode = MODE_MANUAL;
	}
	else if (incoming_cmd == CMD_POSE_CENTER){
		keel_manual_position = CENTER;
	}
	else if (incoming_cmd == CMD_POSE_FULL_PORT){
		keel_manual_position = FULL_PORT;
	}
	else if (incoming_cmd == CMD_POSE_FULL_STARBOARD){
		keel_manual_position = FULL_STARBOARD;
	}
	else{ //invalid command
		success = false;
	}
	return success;
}

void auto_operation()
{
	if (time_till_next_refresh <= millis()){
		time_to_refresh = true;
	}
	if(time_to_refresh)
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
	canted_keel->setAngle(keel_angle, KEEL_SPEED_HIGH);
	//Serial.println(keel_angle);
}

void manual_operation()
{
	if (keel_manual_position == CENTER){
		canted_keel->setAngle(0, KEEL_SPEED_HIGH);		
	}
	else if (keel_manual_position == FULL_PORT){
		canted_keel->setAngle(MAX_KEEL_ANGLE * -1, KEEL_SPEED_HIGH);		
	}
	else if (keel_manual_position == FULL_STARBOARD){
		canted_keel->setAngle(MAX_KEEL_ANGLE, KEEL_SPEED_HIGH);
	}
}

double get_avg_y_accel(int n)
{
	double sum = 0;
	for (int i = 0; i < n; i++ ){
		sum += accelerometer->getY();
	}
	return sum / n;
}

