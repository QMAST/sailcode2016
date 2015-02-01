const uint32_t MIN_RC_WAIT = 10; // (msec) Minimum time before updating
const uint32_t PCHAMP_REQ_WAIT = 5; // (msec) Time to wait for response

//between -1000 to 1000 max
//between -600 to 600 effective
void
set_rudder(
        int target,
        pchamp_controller mast[2],
        pchamp_servo rudder[2] ){
			
	char buf[40];       // buffer for printing debug messages
    uint16_t rvar = 0;  // hold result of remote device status (pololu controller)
			
    target = constrain( target, -1000, 1000 );
    target = map( target,
            -1000, 1000,
            POLOLU_SERVO_RUD_MIN,
            POLOLU_SERVO_RUD_MAX );
    /*snprintf_P( buf, sizeof(buf), PSTR("Call rudder: %d\n"), rc_output );*/
    /*Serial.print( buf );*/
    pchamp_servo_set_position( &(rudder[0]), target );
    rvar = pchamp_servo_request_value( &(rudder[0]), PCHAMP_SERVO_VAR_ERROR );
    delay(PCHAMP_REQ_WAIT);
    if( rvar != 0 ) {
        snprintf_P( buf, sizeof(buf), PSTR("SERVERR: 0x%02X\n"), rvar );
        Serial.print(buf);
    }
}

//between -1000 to 1000

void
set_winch(
        int target,
        pchamp_controller mast[2],
        pchamp_servo rudder[2] ){
	
	uint8_t motor_direction;
	char buf[40];       // buffer for printing debug messages
    uint16_t rvar = 0;  // hold result of remote device status (pololu controller)

    target = constrain( target, -1000, 1000);
    motor_direction = target > 0 ? PCHAMP_DC_FORWARD : PCHAMP_DC_REVERSE;
    target = map( abs(target), 0, 1000, 0, 3200 );

    
    pchamp_request_safe_start( &(mast[1]) );
    pchamp_set_target_speed( &(mast[1]), target, motor_direction );
    delay(PCHAMP_REQ_WAIT);

    rvar = pchamp_request_value( &(mast[1]), PCHAMP_DC_VAR_ERROR );
    if( rvar != 0 ) {
        snprintf_P( buf, sizeof(buf), PSTR("W1ERR: 0x%02x\n"), rvar );
        Serial.print(buf);
    }
	
}