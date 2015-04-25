// vim:ft=c:
/** Radio control mode
 *
 * Grab values from the rc controller and set motor values based on them
 *
 * TODO: Implement timeout functionality and the ability to continue part way
 * through if interrupted
 *
 * TODO: Don't send a new command if values haven't changed
 *
 * TODO: Change to relational movement, stick doesn't correspond directly to
 * the position or speed of a motor. Simply changes the desired position state
 * as its being held in a particular direction
 */
void
rmode_update_motors(
        rc_mast_controller* rc,
        pchamp_controller mast[2],
        pchamp_servo rudder[2] )
{
    const uint32_t MIN_RC_WAIT = 10; // (msec) Minimum time before updating //10 before
    const uint32_t PCHAMP_REQ_WAIT = 5; // (msec) Time to wait for response

    int16_t rc_input;
    uint16_t rc_output;
    uint8_t motor_direction;

    char buf[40];       // buffer for printing debug messages
    uint16_t rvar = 0;  // hold result of remote device status (pololu controller)

    // Track positions from last call, prevent repeat calls to the same
    // position.
    static uint16_t pos_mast_left[4];
    static uint16_t pos_mast_right[4];

    static uint16_t pos_rudder_0[4];
    /*static uint16_t pos_rudder_1[4];*/

    static uint32_t time_last_updated;
    static uint32_t time_rudder_last_updated = 0;

    if( (millis() - time_last_updated) < MIN_RC_WAIT ) {
        return;
    }
    time_last_updated = millis();

    // Rudder controller by left stick y-axis, which stays in place when let
    // go, so just update with its absolute positon
    rc_input = rc_get_analog( rc->lsy );
    rc_input = constrain( rc_input, -500, 500 );
    rc_output = map( rc_input,
            -500, 500,
            POLOLU_SERVO_RUD_MIN,
            POLOLU_SERVO_RUD_MAX );
    /*snprintf_P( buf, sizeof(buf), PSTR("Call rudder: %d\n"), rc_output );*/
    /*Serial.print( buf );*/
    pchamp_servo_set_position( &(rudder[0]), rc_output );
    rvar = pchamp_servo_request_value( &(rudder[0]), PCHAMP_SERVO_VAR_ERROR );
    delay(PCHAMP_REQ_WAIT);
    if( rvar != 0 ) {
        snprintf_P( buf, sizeof(buf), PSTR("SERVERR: 0x%02X\n"), rvar );
        Serial.print(buf);
    }
	
	pchamp_servo_set_position( &(rudder[1]), rc_output );
    rvar = pchamp_servo_request_value( &(rudder[1]), PCHAMP_SERVO_VAR_ERROR );
    delay(PCHAMP_REQ_WAIT);
    if( rvar != 0 ) {
        snprintf_P( buf, sizeof(buf), PSTR("SERVERR: 0x%02X\n"), rvar );
        Serial.print(buf);
    }

    // Motor 0
    rc_input = rc_get_analog( rc->rsy );
    rc_input = constrain( rc_input, -500, 500 );
    motor_direction = rc_input > 0 ? PCHAMP_DC_FORWARD : PCHAMP_DC_REVERSE;
    rc_output = map( abs(rc_input), 0, 500, 0, 3200 );

    /*snprintf_P( buf, sizeof(buf), PSTR("Call m0: %d - %d\n"),*/
            /*rc_output, motor_direction );*/
    /*Serial.print( buf );*/
    pchamp_request_safe_start( &(mast[0]) );
    pchamp_set_target_speed( &(mast[0]), rc_output, motor_direction );
    delay(PCHAMP_REQ_WAIT);

    rvar = pchamp_request_value( &(mast[0]), PCHAMP_DC_VAR_ERROR );
    if( rvar != 0 ) {
        snprintf_P( buf, sizeof(buf), PSTR("W0ERR: 0x%02x\n"), rvar );
        Serial.print(buf);
    }

    // Motor 1

    /*rc_input = rc_get_analog( rc->rsx );*/
    /*rc_input = constrain( rc_input, -500, 500 );*/
    //motor_direction = rc_input > 0 ? PCHAMP_DC_REVERSE : PCHAMP_DC_FORWARD;
    /*rc_input = abs(rc_input);*/
    /*rc_output = map( rc_input, 0, 500, 0, 3200 );*/

    /*snprintf_P( buf, sizeof(buf), PSTR("Call m0: %d - %d\n"),*/
            /*rc_output, motor_direction );*/
    /*Serial.print( buf );*/
    /*pchamp_request_safe_start( &(mast[1]) );
    pchamp_set_target_speed( &(mast[1]), rc_output, motor_direction );
    delay(PCHAMP_REQ_WAIT);

    rvar = pchamp_request_value( &(mast[1]), PCHAMP_DC_VAR_ERROR );
    if( rvar != 0 ) {
        snprintf_P( buf, sizeof(buf), PSTR("W1ERR: 0x%02x\n"), rvar );
        Serial.print(buf);
    }*/

    snprintf_P( buf, sizeof(buf), PSTR("GEAR: %x (%u)"),
            rc_get_digital( rc->gear_switch ),
            rc_get_analog( rc->gear_switch ) );
    cli.port->println( buf );

}

