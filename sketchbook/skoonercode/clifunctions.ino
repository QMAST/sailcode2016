/******************************************************************************
 */
/** Locally defined command line accessiable functions
 ******************************************************************************
 */
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
    diagnostics( &cli );

    return 0;
}

/** Take an argument and print out the entire list of arguments
 */
int ctest( blist list )
{
    char buf[40];

    for( uint16_t i = 0, resp = 0; i < 3200; i++ ) {
        /*cli.port->println("setting speed");*/
        pchamp_set_target_speed( 
                &(pdc_mast_motors[0]),
                i,
                PCHAMP_DC_MOTOR_FORWARD
            );
        delay(10);

        /*cli.port->println("Getting response");*/
        resp = pchamp_request_value(
                &(pdc_mast_motors[0]),
                PCHAMP_DC_VAR_ERROR
            );
        /*delay(100);*/
        /*cli.port->println("Checking response");*/
        if( resp != 0 ) {
            snprintf_P(
                    buf,
                    sizeof(buf),
                    PSTR(
                        "\nError: %u at %u\n"
                    ),
                    resp,
                    i
                );
            cli.port->print( buf );
            /*cli.port->println( i );*/
            pchamp_request_safe_start( &(pdc_mast_motors[0]) );
        } else {
            if( i % 60 == 0 ) cli.port->println();
            cli.port->print(F("."));
        }
        delay(10);
    }
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
    rc_write_calibration_eeprom( 0x08, &radio_controller );

    Serial.println(F("Calibration values written to eeprom"));
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

/*** Check if the servo controller is working by moving rudder back and forth
 */
int ccheckservo( blist list )
{
    char input = '\0';
    //pservo_controller->restart();

    for( uint16_t i = 256, error = 0; i <= 2272; i++ ) {
        pchamp_servo_set_position( &(p_rudder[0]), i );

        error = pchamp_servo_request_value(
                &(p_rudder[0]),
                PCHAMP_SERVO_VAR_POSITION,
                1 );
        Serial.print("P: ");
        Serial.println(error);
        error = pchamp_servo_request_value(
                &(p_rudder[0]),
                PCHAMP_SERVO_VAR_ERROR );
    }

    return 0;
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
    if( strncasecmp( (char*) arg->data, "w", min(arg->slen, 1) ) == 0 ) {
#ifdef DEBUG
        Serial.print(F("WRITE TO EEPROM"));
#endif
        rw = 1;
    } else if( strncasecmp( (char*) arg->data, "r", min(arg->slen, 1) ) == 0 ) {
#ifdef DEBUG
        Serial.print(F("READ TO EEPROM"));
#endif
        rw = 0;
    } else {
#ifdef DEBUG
        Serial.print(F("USE EITHER r OR w"));
#endif
        return -1;
    }

    // Get the module name from the second argument
    arg = list->entry[2];
    if( strncasecmp( (char*) arg->data, "rc", min(arg->slen, 2) ) == 0 ) {
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

   pchamp_set_target_speed( &(pdc_mast_motors[0]), 1000, PCHAMP_DC_MOTOR_FORWARD );
   pchamp_set_target_speed( &(pdc_mast_motors[1]), 1000, PCHAMP_DC_MOTOR_FORWARD );

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
/******************************************************************************
 * END OF COMMAND LINE FUNCTIONS */
// vim:ft=c:
