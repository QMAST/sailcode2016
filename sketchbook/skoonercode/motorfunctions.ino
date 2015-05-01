const uint32_t MIN_RC_WAIT = 10; // (msec) Minimum time before updating
const uint32_t PCHAMP_REQ_WAIT = 5; // (msec) Time to wait for response

//between -1000 to 1000 max
void motor_set_rudder( int target ){
			
	char buf[40];       // buffer for printing debug messages
    uint16_t rvar = 0;  // hold result of remote device status (pololu controller)
			
    target = constrain( target, -1000, 1000 );
    target = map( target,
            -1000, 1000,
            POLOLU_SERVO_RUD_MIN,
            POLOLU_SERVO_RUD_MAX );
    /*snprintf_P( buf, sizeof(buf), PSTR("Call rudder: %d\n"), rc_output );*/
    /*Serial.print( buf );*/
	//Rudder 0
    pchamp_servo_set_position( &(rudder_servo[0]), target );
    rvar = pchamp_servo_request_value( &(rudder_servo[0]), PCHAMP_SERVO_VAR_ERROR );
    delay(PCHAMP_REQ_WAIT);
    if( rvar != 0 ) {
        snprintf_P( buf, sizeof(buf), PSTR("SERVERR0: 0x%02X\n"), rvar );
        Serial.print(buf);
    }
	
	//Rudder 1
    pchamp_servo_set_position( &(rudder_servo[1]), target );
    rvar = pchamp_servo_request_value( &(rudder_servo[1]), PCHAMP_SERVO_VAR_ERROR );
    delay(PCHAMP_REQ_WAIT);
    if( rvar != 0 ) {
        snprintf_P( buf, sizeof(buf), PSTR("SERVERR1: 0x%02X\n"), rvar );
        Serial.print(buf);
    }
}

//between -1000 to 1000

void motor_set_winch( int target ){
	
	uint8_t motor_direction;
	char buf[40];       // buffer for printing debug messages
    uint16_t rvar = 0;  // hold result of remote device status (pololu controller)

    target = constrain( target, -1000, 1000);
    motor_direction = target > 0 ? PCHAMP_DC_FORWARD : PCHAMP_DC_REVERSE;
    target = map( abs(target), 0, 1000, 0, 3200 );
	
	//winch 0
    //pchamp_request_safe_start( &(winch_control[0]) );
    pchamp_set_target_speed( &(winch_control[0]), target, motor_direction );
    delay(PCHAMP_REQ_WAIT);

    rvar = pchamp_request_value( &(winch_control[0]), PCHAMP_DC_VAR_ERROR );
    if( rvar != 0 ) {
		rvar = pchamp_request_value( &(winch_control[0]), PCHAMP_DC_VAR_ERROR_SERIAL );
		
		if(rvar != 0)
			snprintf_P( buf, sizeof(buf), PSTR("W0ERR_SERIAL: 0x%02x\n"), rvar );
		else
			snprintf_P( buf, sizeof(buf), PSTR("W0ERR: 0x%02x\n"), rvar );
        Serial.print(buf);
    }

    //winch 1
	/*
    pchamp_request_safe_start( &(mast[1]) );
    pchamp_set_target_speed( &(mast[1]), target, motor_direction );
    delay(PCHAMP_REQ_WAIT);

    rvar = pchamp_request_value( &(mast[1]), PCHAMP_DC_VAR_ERROR );
    if( rvar != 0 ) {
        snprintf_P( buf, sizeof(buf), PSTR("W1ERR: 0x%02x\n"), rvar );
        Serial.print(buf);
    }*/
	
}

//Locks all motors
// - Servos are set to 0
// - PDC is locked and turned off
void motor_lock(){
	pchamp_set_target_speed(
        &(winch_control[0]), 0, PCHAMP_DC_MOTOR_FORWARD );
    pchamp_request_safe_start( &(winch_control[0]), false );
    //pchamp_set_target_speed(
    //    &(winch_control[1]), 0, PCHAMP_DC_MOTOR_FORWARD );
    //pchamp_request_safe_start( &(winch_control[1]), false );

    // Servos (disengage)
    pchamp_servo_set_position( &(rudder_servo[0]), 0 );
    pchamp_servo_set_position( &(rudder_servo[1]), 0 );
}

//Unlocks PDC winch.
void motor_unlock(){
	pchamp_request_safe_start( &(winch_control[0]) );
    //pchamp_request_safe_start( &(winch_control[1]) );
    
}