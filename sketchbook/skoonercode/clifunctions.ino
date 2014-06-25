/******************************************************************************
 */
/** Locally defined command line accessible functions
 ******************************************************************************
 */
/** Utility function
 * Checks to see if the bstring matches a given constant string without making
 * a copy of the bstrings contents.
 *
 * There is no guarantee that the char* in the bstring is valid or that the
 * given constant string will not contain anything weird
 *
 * Returns:
 * 1 If strings match
 * 0 Otherwise
 */
uint8_t
arg_matches( bstring arg, const char* ref )
{
    uint8_t match;

    match = strncasecmp(
            (char*) arg->data,
            ref,
            min( arg->slen, strlen(ref) )
        );

    // Return one if the string matches
    return match == 0 ? 1 : 0;
}

int cabout( blist list )
{
    Serial.println("GAELFORCE Skoonercode edition");
    Serial.println("Compiled " __DATE__ " " __TIME__ ".");
#ifdef DEBUG
    Serial.println(F("DEBUGGING BUILD"));
#endif

#ifndef _MEGA
    Serial.println(F("UNO BUILD: FUNCTIONS LIMITED"));
#endif

    char* buf;

    for( uint8_t i = 0; i < functions.index; i++ ) {
        Serial.print( (uint16_t) functions.cmds[i].func, HEX );
        Serial.print(F("-> "));

        buf = bstr2cstr( functions.cmds[i].name, '\0' );
        Serial.println( buf );
        free(buf);
    }
}

int cdiagnostic_report( blist list )
{
    while( Serial.read() != 'q' ) {
        diagnostics( &cli );
        delay(500);
    }

    return 0;
}

/** Take a number of seconds to engage the 0 motor
 */
int ctest( blist list )
{
    char buf[40];
    uint32_t target_time;
    uint16_t target_speed = 3000;
    uint8_t  target_dir = PCHAMP_DC_FORWARD;

    if( !list || list->qty <= 1 ) return CONSHELL_FUNCTION_BAD_ARGS;

    target_time = strtoul( (char*) list->entry[1]->data, NULL, 10 );

    if( list->qty >= 3 ) {
        target_speed = strtoul( (char*) list->entry[2]->data, NULL, 10);
    }

    if( list->qty >= 4 ) {
        target_dir = strtoul( (char*) list->entry[3]->data, NULL, 10 );
    }

    test_motor.speed     = target_speed;
    test_motor.target    = target_time + millis();
    test_motor.completed = false;
    test_motor.dir       = target_dir;

    snprintf_P( buf, sizeof(buf),
            PSTR("Time: %lu\n"
                 "Speed: %u\n"
                 "Direction: %u\n"
                ),
            target_time,
            target_speed,
            target_dir );
    cli.port->print(buf);
}

/*** Monitor relay a serial connection back to serial 0
 *
 *   Assuming Serial is the connection to the user terminal, it takes all
 *   characters received on the specified serial connection and prints them.
 *
 *   Not tested for communication from user to device
 *
 *   Pressing q terminates monitor
 */
#ifdef _MEGA
int cmon( blist list )
{
    char* buf; // For parsing input number
    uint8_t target_port_number;
    Stream* target_port;

    char input = '\0'; // For mirroring data between ports

    if( !list || list->qty <= 1 ) return CONSHELL_FUNCTION_BAD_ARGS;

    buf = bstr2cstr( list->entry[1], '\0' );
    target_port_number = strtoul( buf, NULL, 10 );
    free(buf);

#ifdef DEBUG
    Serial.print("Using port ");
    Serial.println( target_port_number );
#endif

    if( target_port_number == 1 ) target_port = &Serial1;
    else if( target_port_number == 2 ) target_port = &Serial2;
    else if( target_port_number == 3 ) target_port = &Serial3;

    while( input != 'q' ) {
        if( target_port->available() >= 1 ) {
            Serial.print( (char) target_port->read() );
        }

        if( Serial.available() >= 1 ) {
            target_port->write( (input = Serial.read()) );
        }
    }
}
#endif

/*** Get the maximum and minimum values for each rc channel interactively
 *
 *  Just prints the values to the screen. This is a convenience function for
 *  figuring out the values.
 */
#ifdef RC_CALIBRATION
int calrc( blist list )
{
    Serial.println(F(
                "Please setup controller by: \n"
                "   - Set all sticks to middle position\n"
                "   - Pull enable switch towards you\n"
                "   - Rudder knob to minimum\n"
                "Press enter to continue."
                ));
    while( Serial.available() == 0 )
        ;
    if( Serial.read() == 'q' ) return -1;

    // Left stick
    radio_controller.rsx.neutral =
        rc_get_raw_analog( radio_controller.rsx );
    radio_controller.rsy.neutral =
        rc_get_raw_analog( radio_controller.rsy );

    // Right stick
    radio_controller.lsx.neutral =
        rc_get_raw_analog( radio_controller.lsx );
    radio_controller.lsy.neutral =
        rc_get_raw_analog( radio_controller.lsy );

    // Switch and knob
    radio_controller.gear_switch.neutral =
        rc_get_raw_analog( radio_controller.gear_switch );
    radio_controller.aux.neutral =
        rc_get_raw_analog( radio_controller.aux );

    // Write the values to eeprom
    /*rc_write_calibration_eeprom( 0x08, &radio_controller );*/

    Serial.println(F("Calibration values set"));
}
#endif // RC_CALIBRATION


/** Given a +/- and a mode name, apply or remove that mode bit
 *
 *  Valid mode names are
 *  -   RC      <- RC Control mode
 *  -   AIR     <- Polling AIRMAR for data
 *
 *  There must be a space between the +/- and modename
 */
int csetmode( blist list )
{
    uint8_t mode_bit;
    bstring arg;

    if( list->qty <= 2 ) return -1;

    // Get the mode name first
    arg = list->entry[2];
    if( strncasecmp( (char*) arg->data, "RC", min(arg->slen, 2) ) == 0 ) {
#ifdef DEBUG
        Serial.print(F("SELECT RC"));
#endif
        mode_bit = MODE_RC_CONTROL;
    } else if( strncasecmp( (char*) arg->data, "AIR", min(arg->slen, 3) ) == 0 ) {
#ifdef DEBUG
        Serial.print(F("SELECT AIR"));
#endif
        mode_bit = MODE_AIRMAR_POLL;
    }

    if( strncmp( (char*) list->entry[1]->data, "+", 1 ) == 0 ) {
#ifdef DEBUG
        Serial.println(F(" ENGAGE"));
#endif
        gaelforce |= mode_bit;
    } else {
#ifdef DEBUG
        Serial.println(F(" DISENGAGE"));
#endif
        gaelforce &= ~mode_bit;
    }

    bdestroy(arg);
}

/** Read write predetermined structures to and from eeprom
 *
 *  Uses two arguments, first is
 *      - r : for read
 *      - w : for write
 *
 *  Second is the item to be read/written, these structures are hardcoded in
 *  the sense that their eeprom address is written here. Available modules
 *
 *      - rc : RC calibration settings
 *      - ...
 *
 *      Returns -1 in case of error 0 otherwise
 */
int ceeprom( blist list )
{
    uint8_t rw = 0; // 0 is read-only
    bstring arg;

    if( list->qty <= 2 ) return -1;

    // Check the first argument
    arg = list->entry[1];
    if( arg_matches(arg, "w") ) {
        rw = 1;
    } else if( arg_matches(arg, "r") ) {
        rw = 0;
    } else {
#ifdef DEBUG
        Serial.print(F("USE EITHER r OR w"));
#endif
        return -1;
    }

    // Get the module name from the second argument
    arg = list->entry[2];
    if( arg_matches(arg, "rc") ) {
#ifdef DEBUG
        Serial.println(F("RC SETTINGS"));
#endif
        if( rw == 0 ) {
            rc_read_calibration_eeprom( 0x08, &radio_controller );
        } else if( rw == 1 ) {
            rc_write_calibration_eeprom( 0x08, &radio_controller );
        }

        rc_DEBUG_print_controller( &Serial, &radio_controller );

        return 0;
    } else {
#ifdef DEBUG
        Serial.print(F("NOT A MODULE"));
#endif
    }

    return -1;
}

int crcd( blist list )
{
    int16_t rc_in;

    while(      Serial.available() <= 0
            &&  Serial.read() != 'q' )
    {
        rc_in = rc_get_analog( radio_controller.lsy );
        Serial.print(F( "L-Y: " ));
        Serial.print( rc_in );

        rc_in = rc_get_analog( radio_controller.rsy );
        Serial.print(F( " R-Y: " ));
        Serial.println( rc_in );
    }
}

int ctest_pololu( blist list )
{
    uint16_t value = 100;

    pchamp_request_safe_start( &(pdc_mast_motors[0]) );
    pchamp_request_safe_start( &(pdc_mast_motors[1]) );

    pchamp_set_target_speed(
            &(pdc_mast_motors[0]), 1000, PCHAMP_DC_MOTOR_FORWARD );
    pchamp_set_target_speed(
            &(pdc_mast_motors[1]), 1000, PCHAMP_DC_MOTOR_FORWARD );

    value = pchamp_request_value(
                &(pdc_mast_motors[0]),
                PCHAMP_DC_VAR_VOLTAGE );
    Serial.print(F("Voltage 0: "));
    Serial.println( value );

    value = pchamp_request_value(
                &(pdc_mast_motors[1]),
                PCHAMP_DC_VAR_VOLTAGE );
    Serial.print(F("Voltage 1: "));
    Serial.println( value );

}

int cmot( blist list )
{
    bstring arg;

    // Check if no actual arguments are given
    if( list->qty <= 1 ) {
        cli.port->print(F(
            "s - stop and lock all motors\n"
            "u - request unlock of both motors\n"
            "g (MOTOR) (SPEED) - set motor speed\n"
            "r (SERVO) (POS) - set rudder position\n"
            ));
        return -1;
    }

    // Check arguments
    arg = list->entry[1];
    if( arg_matches( arg, "s" ) ) {
        // Motors
        pchamp_set_target_speed(
                &(pdc_mast_motors[0]), 0, PCHAMP_DC_MOTOR_FORWARD );
        pchamp_request_safe_start( &(pdc_mast_motors[0]), false );
        pchamp_set_target_speed(
                &(pdc_mast_motors[1]), 0, PCHAMP_DC_MOTOR_FORWARD );
        pchamp_request_safe_start( &(pdc_mast_motors[1]), false );

        // Servos (disengage)
        pchamp_servo_set_position( &(p_rudder[0]), 0 );
        pchamp_servo_set_position( &(p_rudder[1]), 0 );
    } else if( arg_matches( arg, "g" ) ) {
        if( list->qty <= 2 ) {
            cli.port->println(F("Not enough args"));
        }

        int8_t mot_num =
            strtol( (char*) list->entry[2]->data, NULL, 10 );
        mot_num = constrain( mot_num, 0, 1 );

        int16_t mot_speed =
            strtol( (char*) list->entry[3]->data, NULL, 10 );
        mot_speed = constrain( mot_speed, -3200, 3200 );

        char buf[30];
        snprintf_P( buf, sizeof(buf),
                PSTR("Motor %d at %d"),
                mot_num,
                mot_speed
            );
        cli.port->println( buf );

        pchamp_set_target_speed(
                &(pdc_mast_motors[mot_num]),
                abs(mot_speed),
                mot_speed > 0 ? 0 : 1
            );
    } else if( arg_matches( arg, "u" ) ) {
        pchamp_request_safe_start( &(pdc_mast_motors[0]) );
        pchamp_request_safe_start( &(pdc_mast_motors[1]) );
    } else if( arg_matches( arg, "r" ) ) {
        if( list->qty <= 2 ) {
            cli.port->println(F("Not enough args"));
        }

        int8_t mot_num =
            strtol( (char*) list->entry[2]->data, NULL, 10 );
        mot_num = constrain( mot_num, 0, 1 );

        int16_t mot_pos =
            strtol( (char*) list->entry[3]->data, NULL, 10 );
        mot_pos = constrain( mot_pos, 0, 6400 );

        char buf[30];
        snprintf_P( buf, sizeof(buf),
                PSTR("Servo %d to %d"),
                mot_num,
                mot_pos
            );
        cli.port->println( buf );

        pchamp_servo_set_position( &(p_rudder[mot_num]), mot_pos );
    }
}
/******************************************************************************
 * END OF COMMAND LINE FUNCTIONS */
// vim:ft=c:
