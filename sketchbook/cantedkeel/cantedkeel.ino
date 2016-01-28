
#include <Wire.h>
#include <pololu_champ.h>
#include <SoftwareSerial.h>
#include "pins.h"


//#include <WSWire.h>

#include <barnacle_client.h>
#include <memoryget.h>

#define ENCODER_PIN    2 // Pin 2

#define SCALE 2  // accel full-scale, should be 2, 4, or 8

/* LSM303 Address definitions */
#define LSM303_MAG  0x1E  // assuming SA0 grounded
#define LSM303_ACC  0x18  // assuming SA0 grounded

#define X 0
#define Y 1
#define Z 2

//winch variables
#define KEEL_HIGH_SPEED 800
#define KEEL_MED_SPEED 500
#define KEEL_LOW_SPEED 200

#define KEEL_HIGH_THRESH 2000
#define KEEL_MED_THRESH 500
#define KEEL_TARGET_THRESH 50

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

//angle cutoffs for canted keel
#define MIN_ANGLE -0.2
#define MAX_ANGLE 0.2

/* Global variables */
int accel[3];  // we'll store the raw acceleration values here
int mag[3];  // raw magnetometer values stored here
float realAccel[3];  // calculated acceleration values here

volatile uint16_t enc_ticks = 0;

int32_t keel_current_position;
int32_t keel_target;
uint8_t keel_current_direction;
pchamp_controller keel_control;

const uint32_t MIN_RC_WAIT = 10; // (msec) Minimum time before updating
const uint32_t PCHAMP_REQ_WAIT = 5; // (msec) Time to wait for response


void setup()
{
  Serial.begin(9600);  // Serial is used for debugging
  Wire.begin();  // Start up I2C, required for LSM303 communication
	SERIAL_PORT_POLOLU.begin(SERIAL_BAUD_POLOLU);
 initLSM303(SCALE);  // Initialize the LSM303, using a SCALE full-scale range
  //init motor
   keel_control.id = 12; //figure out what this number means (pin?)
    keel_control.line = &SERIAL_PORT_POLOLU;

	 //attachInterrupt( digitalPinToInterrupt(ENCODER_PIN), count_tick, FALLING );
}

void loop()
{
  getLSM303_accel(accel);  // get the acceleration values and store them in the accel array
  
  for (int i=0; i<3; i++)
    realAccel[i] = accel[i] / pow(2, 15) * SCALE;  // calculate real acceleration values, in units of g

  if(realAccel[X]>MAX_ANGLE || realAccel[X] < MIN_ANGLE)
  cantKeel(realAccel[X]);
  
  Serial.print("\nX: ");
  Serial.print(realAccel[0]);
  Serial.print("\tY: ");
  Serial.print(realAccel[1]);
  Serial.print("\tZ: ");
  Serial.print(realAccel[2]);
  
  delay(100);  // delay for serial readability
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
  motor_unlock();
  int i;
  if(accel > MAX_ANGLE){
    
	motor_set_vel(-1000); // this and the 1000 may need ot be flipped, depends on direction when motor connected
	Serial.println("go!");
	delay(abs(accel) *1000); //wait some amount of time to let the keel move. higher angle = longer so the keel moves more  
	
  //delay but printing encoder ticks within delay
	//for(i = 0; i< abs(accel)*10; i++){
    //Serial.println(enc_ticks);
  //  delay(100);
//	}
	//motor_abs()
  }
  else if(accel < MIN_ANGLE){ // if to decide whether or not to act
	//int target = accel; //make this a proper scaling function thing
	motor_set_vel(1000); // once encoding figured out, use modified motor_keel_abs for better results
	Serial.println("go!");
	delay(abs(accel) * 1000);
 //
 //for(i=0; i< abs(accel)*10; i++){
   // Serial.println(enc_ticks);
    //delay(100);
 //}
  }
  
  /*
  if(accel > MAX_ANGLE){
	motor_set_vel(-1000); // this and the 1000 may need ot be flipped, depends on direction when motor connected
	delay(abs(accel) *1000); //wait some amount of time to let the keel move. higher angle = longer so the keel moves more  
  }
  else if(accel < MIN_ANGLE){ // if to decide whether or not to act
	//int target = accel; //make this a proper scaling function thing
	motor_set_vel(1000); // once encoding figured out, use modified motor_keel_abs for better results
	delay(abs(accel) * 1000);
  }
  
  */
}

//stuff taken from motorfunctions and modified

//uint8_t keel_current_direction;
void motor_set_vel( int target ){
	Serial.println("entering set_vel");
	char buf[40];       // buffer for printing debug messages
    uint16_t rvar = 0;  // hold result of remote device status (pololu controller)
	uint16_t rvar_serial = 0;  // hold result of remote device status (pololu controller)

    target = constrain( target, -1000, 1000);
    keel_current_direction = target > 0 ? PCHAMP_DC_FORWARD : PCHAMP_DC_REVERSE;
    target = map( abs(target), 0, 1000, 0, 3200 );
	target = target/5;
	//keel 0
    //pchamp_request_safe_start( &(keel_control[0]) );
    pchamp_set_target_speed( &(keel_control), target, keel_current_direction );
    delay(PCHAMP_REQ_WAIT);
Serial.println("speed set");
	//Check for error:
    rvar = pchamp_request_value( &(keel_control), PCHAMP_DC_VAR_ERROR );	//General Error Request
    Serial.println("got rvar");
    if( rvar != 0 ) {
      Serial.println("error!");
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
  Serial.println("Unlocked");
}
/*
void motor_abs(int32_t target_abs){
	//Clear current ticks;
	
	if(keel_current_direction == PCHAMP_DC_REVERSE)
		keel_current_position -= enc_ticks;
	else
		keel_current_position += enc_ticks;
	
	//Set new target:
	keel_target = target_abs;
	Serial.print("Target set to: ");
	Serial.println(target_abs);
	
	motor_keel_update();
}

void motor_rel(int32_t target_rel){
	//Clear current ticks;
	uint16_t ticks = barn_getandclr_w1_ticks();
	if(keel_current_direction == PCHAMP_DC_REVERSE)
		keel_current_position -= ticks;
	else
		keel_current_position += ticks;
	
	//Set new target:
	keel_target = keel_current_position + target_rel;
	
	motor_keel_update();
}

//Motor update function used for absolute positioning, call as frequently as possible
//Returns 1 if target has been reached, 0 otherwise
//Motor update function used for absolute positioning, call as frequently as possible
//Returns 1 if target has been reached, 0 otherwise
int motor_keel_update(){
  char buf[500];
  static int16_t keel_velocity = 0;
  static int16_t last_keel_velocity;
  uint8_t keel_target_reached = 0;
    
  //Update current position tracker
  uint16_t ticks = barn_getandclr_w1_ticks();
  if(keel_current_direction == PCHAMP_DC_REVERSE)
    keel_current_position -= (int32_t)ticks;
  else
    keel_current_position += (int32_t)ticks;
  
  int32_t offset = (int32_t) keel_target - keel_current_position;
  
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
    
  
  //Get velocity direction
  if(offset < 0)
    keel_velocity = abs(keel_velocity) * -1;
  
  //Exit if no change in vel
  //Perhaps do not use to avoid interference errors (ie. something else set motor speed)
  //if(keel_velocity == last_keel_velocity)
    //return keel_target_reached;
  
  motor_set_vel(keel_velocity);
  
  return keel_target_reached;
}

void motor_cal_keel(){
	keel_current_position = 0;
}

*/

void
count_tick()
{
    cli();
	if(keel_current_direction  = PCHAMP_DC_FORWARD)
		enc_ticks++;
	
	else 
		enc_ticks--;
    sei();
}



