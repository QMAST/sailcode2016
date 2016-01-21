
#include <Wire.h>
#include <math.h>
#include <pololu_champ.h>
#include <barnacle_client.h>
#include "pins.h"
#include <SoftwareSerial.h>

#include <avr/pgmspace.h>
#include <inttypes.h>

//#include <WSWire.h>
#include <DS3232RTC.h>
#include <Time.h>
#include <barnacle_client.h>

#include <bstrlib.h>
#include <constable.h>
#include <conshell.h>

#include <memoryget.h>
#include <anmea.h>

#include <pololu_champ.h>

#include <radiocontrol.h>
#include <latlong.h>

#include <winch_control.h>


#define SCALE 2  // accel full-scale, should be 2, 4, or 8

/* LSM303 Address definitions */
#define LSM303_MAG  0x1E  // assuming SA0 grounded
#define LSM303_ACC  0x18  // assuming SA0 grounded

#define X 0
#define Y 1
#define Z 2

//winch variables
#define WINCH_HIGH_SPEED 800
#define WINCH_MED_SPEED 500
#define WINCH_LOW_SPEED 200

#define WINCH_HIGH_THRESH 2000
#define WINCH_MED_THRESH 500
#define WINCH_TARGET_THRESH 50

/* LSM303 Register definitions */
#define CTRL_REG1_A 0x20
#define CTRL_REG2_A 0x21
#define CTRL_REG3_A 0x22
#define CTRL_REG4_A 0x23
#define CTRL_REG5_A 0x24
#define HP_FILTER_RESET_A 0x25
#define REFERENCE_A 0x26
#define STATUS_REG_A 0x27
#define OUT_X_L_A 0x28
#define OUT_X_H_A 0x29
#define OUT_Y_L_A 0x2A
#define OUT_Y_H_A 0x2B
#define OUT_Z_L_A 0x2C
#define OUT_Z_H_A 0x2D
#define INT1_CFG_A 0x30
#define INT1_SOURCE_A 0x31
#define INT1_THS_A 0x32
#define INT1_DURATION_A 0x33
#define CRA_REG_M 0x00
#define CRB_REG_M 0x01
#define MR_REG_M 0x02
#define OUT_X_H_M 0x03
#define OUT_X_L_M 0x04
#define OUT_Y_H_M 0x05
#define OUT_Y_L_M 0x06
#define OUT_Z_H_M 0x07
#define OUT_Z_L_M 0x08
#define SR_REG_M 0x09
#define IRA_REG_M 0x0A
#define IRB_REG_M 0x0B
#define IRC_REG_M 0x0C

//angle cutoffs for canted keel
#define MIN_ANGLE -0.5
#define MAX_ANGLE 0.5

/* Global variables */
int accel[3];  // we'll store the raw acceleration values here
int mag[3];  // raw magnetometer values stored here
float realAccel[3];  // calculated acceleration values here

pchamp_controller keel_control;
int lastAccel;
void setup()
{
  Serial.begin(9600);  // Serial is used for debugging
  Wire.begin();  // Start up I2C, required for LSM303 communication
  initLSM303(SCALE);  // Initialize the LSM303, using a SCALE full-scale range
  //init motor
   keel_control.id = 12; //figure out what this number means (pin?)
    keel_control.line = &SERIAL_PORT_POLOLU;
}

void loop()
{
  getLSM303_accel(accel);  // get the acceleration values and store them in the accel array
  
  for (int i=0; i<3; i++)
    realAccel[i] = accel[i] / pow(2, 15) * SCALE;  // calculate real acceleration values, in units of g

  if(realAccel[0]>MAX_ANGLE || realAccel[0] < MIN_ANGLE)
    cantKeel(realAccel[0]);


	
  
  
  /* print both the level, and tilt-compensated headings below to compare */
  //Serial.print(getHeading(mag), 3);  // this only works if the sensor is level
  Serial.print("\t\t");  // print some tabs
  //Serial.println(getTiltHeading(mag, realAccel), 3);  // see how awesome tilt compensation is?!
  Serial.print("\nX: ");
  Serial.print(realAccel[0]);
  Serial.print("\tY: ");
  Serial.print(realAccel[1]);
  Serial.print("\tZ: ");
  Serial.print(realAccel[2]);
  delay(300);  // delay for serial readability
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

void printValues(int * magArray, int * accelArray)
{
  /* print out mag and accel arrays all pretty-like */
  Serial.print("X: ");
  Serial.print(accelArray[X], DEC);
  Serial.print("\tY:");
  Serial.print(accelArray[Y], DEC);
  Serial.print("\tZ:");
  Serial.print(accelArray[Z], DEC);
  Serial.print("\t\t");
}


void getLSM303_accel(int * rawValues)
{
  rawValues[Z] = ((int)LSM303_read(OUT_X_L_A) << 8) | (LSM303_read(OUT_X_H_A));
  rawValues[X] = ((int)LSM303_read(OUT_Y_L_A) << 8) | (LSM303_read(OUT_Y_H_A));
  rawValues[Y] = ((int)LSM303_read(OUT_Z_L_A) << 8) | (LSM303_read(OUT_Z_H_A));  
  // had to swap those to right the data with the proper axis
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
  while(!Wire.available())
    ;
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

void cantKeel(float accel){
  //do something to motor. yay!
  if(accel > MAX_ANGLE){
	motor_set_winch(-1000); // this and the 1000 may need ot be flipped, depends on direction when motor connected
	delay(abs(accel) *1000); //wait some amount of time to let the keel move. higher angle = longer so the keel moves more  
  }
  else if(accel < MIN_ANGLE){ // if to decide whether or not to act
	//int target = accel; //make this a proper scaling function thing
	motor_set_winch(1000); // once encoding figured out, use modified motor_winch_abs for better results
	delay(abs(accel) * 1000);
  }
}



//stuff taken from motorfunctions and modified

int32_t winch_current_position;
int32_t winch_target;

const uint32_t MIN_RC_WAIT = 10; // (msec) Minimum time before updating
const uint32_t PCHAMP_REQ_WAIT = 5; // (msec) Time to wait for response



uint8_t winch_current_direction;
void motor_set_winch( int target ){
	
	char buf[40];       // buffer for printing debug messages
    uint16_t rvar = 0;  // hold result of remote device status (pololu controller)
	uint16_t rvar_serial = 0;  // hold result of remote device status (pololu controller)

    target = constrain( target, -1000, 1000);
    winch_current_direction = target > 0 ? PCHAMP_DC_FORWARD : PCHAMP_DC_REVERSE;
    target = map( abs(target), 0, 1000, 0, 3200 );
	target = target/5;
	//winch 0
    //pchamp_request_safe_start( &(winch_control[0]) );
    pchamp_set_target_speed( &(keel_control), target, winch_current_direction );
    delay(PCHAMP_REQ_WAIT);

	//Check for error:
    rvar = pchamp_request_value( &(keel_control), PCHAMP_DC_VAR_ERROR );	//General Error Request
    if( rvar != 0 ) {
		rvar_serial = pchamp_request_value( &(keel_control), PCHAMP_DC_VAR_ERROR_SERIAL );	//Serial Error Request
		
		if(rvar_serial != 0){
			snprintf_P( buf, sizeof(buf), PSTR("W0ERR_SERIAL: 0x%02x\n"), rvar_serial );
			pchamp_request_safe_start( &(keel_control) );	//If there is a serial error, ignore it and immediately restart
		}
		else
			snprintf_P( buf, sizeof(buf), PSTR("W0ERR: 0x%02x\n"), rvar );
        Serial.print(buf);
    }
	
}

//Locks all motors
// - Servos are set to 0
// - PDC is locked and turned off
void motor_lock(){
	pchamp_set_target_speed(
        &(keel_control), 0, PCHAMP_DC_MOTOR_FORWARD );
    pchamp_request_safe_start( &(keel_control), false );
    
}

//Unlocks motor
void motor_unlock(){
	pchamp_request_safe_start( &(keel_control) );
}

void motor_winch_abs(int32_t target_abs){
	//Clear current ticks;
	uint16_t ticks = barn_getandclr_w1_ticks();
	if(winch_current_direction == PCHAMP_DC_REVERSE)
		winch_current_position -= ticks;
	else
		winch_current_position += ticks;
	
	//Set new target:
	winch_target = target_abs;
	Serial.print("Target set to: ");
	Serial.println(target_abs);
	
	motor_winch_update();
}

void motor_winch_rel(int32_t target_rel){
	//Clear current ticks;
	uint16_t ticks = barn_getandclr_w1_ticks();
	if(winch_current_direction == PCHAMP_DC_REVERSE)
		winch_current_position -= ticks;
	else
		winch_current_position += ticks;
	
	//Set new target:
	winch_target = winch_current_position + target_rel;
	
	motor_winch_update();
}

//Motor update function used for absolute positioning, call as frequently as possible
//Returns 1 if target has been reached, 0 otherwise
//Motor update function used for absolute positioning, call as frequently as possible
//Returns 1 if target has been reached, 0 otherwise
int motor_winch_update(){
  char buf[500];
  static int16_t winch_velocity = 0;
  static int16_t last_winch_velocity;
  uint8_t winch_target_reached = 0;
    
  //Update current position tracker
  uint16_t ticks = barn_getandclr_w1_ticks();
  if(winch_current_direction == PCHAMP_DC_REVERSE)
    winch_current_position -= (int32_t)ticks;
  else
    winch_current_position += (int32_t)ticks;
  
  int32_t offset = (int32_t) winch_target - winch_current_position;
  
  //Get velocity magnitude:
  if(abs(offset)>    WINCH_HIGH_THRESH)
    winch_velocity = WINCH_HIGH_SPEED;
  else if(abs(offset)> WINCH_MED_THRESH)
    winch_velocity = WINCH_MED_SPEED;
  else if(abs(offset)> WINCH_TARGET_THRESH)
    winch_velocity = WINCH_LOW_SPEED;
  else{
    winch_velocity = 0;
    winch_target_reached = 1;
  }
    
  
  //Get velocity direction
  if(offset < 0)
    winch_velocity = abs(winch_velocity) * -1;
  
  //Exit if no change in vel
  //Perhaps do not use to avoid interference errors (ie. something else set motor speed)
  //if(winch_velocity == last_winch_velocity)
    //return winch_target_reached;
  
  motor_set_winch(winch_velocity);
  
  /*Serial.print("Speed: ");
  Serial.print(winch_velocity);
  Serial.print("\tPos: ");
  Serial.print(winch_current_position);
  Serial.print("\tOff: ");
  Serial.println(offset);*/
  
  return winch_target_reached;
}

void motor_cal_winch(){
	winch_current_position = 0;
}

