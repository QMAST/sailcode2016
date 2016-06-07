#include <Wire.h>
#include <pololu_champ.h>
#include <SoftwareSerial.h>
#include "pins.h"
//#include <WSWire.h> //sometimes it complains and you need to uncomment this once
//#include <barnacle_client.h>
#include <memoryget.h>

#define ENCODER_PIN    2 // Pin 2
#define SCALE 2  // accelerometer full-scale in Gs, should be 2, 4, or 8

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

#define X 0
#define Y 1
#define Z 2

//Winch control variables. Change to tune once boat is built
#define KEEL_HIGH_SPEED 1000
#define KEEL_MED_SPEED 800
#define KEEL_LOW_SPEED 500

#define KEEL_HIGH_THRESH 200
#define KEEL_MED_THRESH 100
#define KEEL_TARGET_THRESH 20

//angle cutoffs to turn on canted keel
#define MIN_TILT -0.5
#define MAX_TILT 0.5

/* Global variables */
int accel[3];  // we'll store the raw acceleration values here
float realAccel[3];  // calculated acceleration values here

//keel control variables
volatile int enc_ticks = 0;
int32_t keel_current_position;
int32_t keel_target;
uint8_t keel_current_direction;
pchamp_controller keel_control;

const uint32_t PCHAMP_REQ_WAIT = 5; // (msec) Time to wait for response

void setup()
{
  Serial.begin(9600);  // Serial is used for debugging
  Wire.begin();  // Start up I2C, required for LSM303 communication
  SERIAL_PORT_POLOLU.begin(SERIAL_BAUD_POLOLU);
  initLSM303(SCALE);  // Initialize the LSM303, using a SCALE full-scale range
  /* init motor */
  keel_control.id = 12; 
  keel_control.line = &SERIAL_PORT_POLOLU;
  attachInterrupt( digitalPinToInterrupt(ENCODER_PIN), count_tick, FALLING );
  pinMode(2, INPUT);
  motor_unlock();
}

void loop()
{
  getLSM303_accel(accel);  // get the acceleration values and store them in the accel array

  realAccel[X] = accel[X] / pow(2, 15) * SCALE;  // calculate real acceleration values, in units of g

  if(realAccel[X]>MAX_TILT || realAccel[X] < MIN_TILT)
	  keel_target = realAccel[X] * 1000; // maps target from -1000 to 1000 based on measured angle
  else 
    keel_target = 0; //put keel back in middle

  motor_keel_update();
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

void getLSM303_accel(int * rawValues)
{
  rawValues[Z] = ((int)LSM303_read(OUT_X_L_A) << 8) | (LSM303_read(OUT_X_H_A));
  rawValues[X] = ((int)LSM303_read(OUT_Y_L_A) << 8) | (LSM303_read(OUT_Y_H_A));
  rawValues[Y] = ((int)LSM303_read(OUT_Z_L_A) << 8) | (LSM303_read(OUT_Z_H_A));  
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

 

/*  Functions modified from motorfunctions  */

//Sets the motor velocity to a number from -1000 to 1000
//updates the keel_current_direcction variable to reflect current motor direction 
void motor_set_vel( int target ){
	
	char buf[40];       // buffer for printing debug messages
    uint16_t rvar = 0;  // hold result of remote device status (pololu controller)
	uint16_t rvar_serial = 0;  // hold result of remote device status (pololu controller)

	 
    target = constrain( target, -1000, 1000);
    //sometimes target is 0. We're not sure why, it might be a communication issue, but the fxn just ignores those times
	if(target > 0){
      keel_current_direction = PCHAMP_DC_FORWARD;
    }
    else if(target < 0){
      keel_current_direction = PCHAMP_DC_REVERSE;
    }
   
    target = map( abs(target), 0, 1000, 0, 3200 );
	target = target/5;
	   
    pchamp_set_target_speed( &(keel_control), target, keel_current_direction );
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

//Motor update function used for absolute positioning, call as frequently as possible
//Returns 1 if target has been reached, 0 otherwise
int motor_keel_update(){
  char buf[500];
  static int16_t keel_velocity = 0;
  static int16_t last_keel_velocity;
  uint8_t keel_target_reached = 0;
    
  keel_current_position = enc_ticks; //may need to scale this somehow to match keel_target scaling 
  
  //Update current position tracker
  int32_t offset = (int32_t) keel_target - keel_current_position; // keel target is a number from -1000 to 1000 based on the boat tilt 

  //Get velocity magnitude:
  if(abs(offset)>    KEEL_HIGH_THRESH)
    keel_velocity = KEEL_HIGH_SPEED;
  else if(abs(offset)> KEEL_MED_THRESH)
    keel_velocity = KEEL_MED_SPEED;
  else if(abs(offset)> KEEL_TARGET_THRESH)
    keel_velocity = KEEL_LOW_SPEED;
  else{
    keel_velocity = 0;
    keel_target_reached = 1;
  }
    
//  
//  //Get velocity direction
  if(offset < 0)
    keel_velocity = abs(keel_velocity) * -1;
  
  motor_set_vel(keel_velocity);
  
  return 0;
}

void motor_cal_keel(){
	keel_current_position = 0;
}

void
count_tick()
{
    cli();
    
	if(keel_current_direction  == 0){
		enc_ticks++;
	}
	else 
		enc_ticks--;
   sei();
}



