/** Skoonercode
 *
 * Supercedes ferrycode, I thought the name looked cooler than Schoonercode
 *
 * Uses a simple polling loop with timeouts enforced by the functions
 * themselves. Each function is passed the amount of time it is allowed to take
 * for execution, and either exits as soon as possible, or attempts to perform
 * its function for the entire time until it resalises it needs to exit.
 *
 * Main interface, command line functions. No function may lock the polling
 * loop into a single function. Certain polling functions may be
 * enabled/disabled on the fly.
 */
#ifndef _SKOONERCODE_INO
#define _SKOONERCODE_INO
#include "pins.h"

#include <avr/pgmspace.h>
#include <inttypes.h>

#include <Wire.h>
#include <DS3232RTC.h>
#include <Time.h>

#include <SoftwareSerial.h>

#include <bstrlib.h>
#include <constable.h>
#include <conshell.h>
#include <memoryget.h>
#include <anmea.h>

#include <barnacle_client.h>
#include <pololu_champ.h>
#include <radiocontrol.h>
#include <latlong.h>


/** Global Variable instances
 ******************************************************************************
 */

// Instances necessary for command line usage
cons_line cli;
cmdlist functions;

// AIRMAR NMEA String buffer
bstring airmar_nmea_buffer;
anmea_tag_wiwmv_t airmar_nmea_wimwv_tag;

// Motor object definitions
pchamp_servo p_rudder[2];
pchamp_controller pservo_0;

pchamp_controller pdc_mast_motors[2];

event_time_motor_t test_motor = {
    0,
    1,
    0,
    PCHAMP_DC_FORWARD,
    &(pdc_mast_motors[0])
};

// Global to track current mode of operation
uint16_t gaelforce = MODE_COMMAND_LINE;

/// Initialise pin numbers and related calibration values, most values should
//be overwritten by eeprom during setup()
rc_mast_controller radio_controller = {
    {
        MAST_RC_RSX_PIN,
        1852,
        1091,
        30
    },
    {
        MAST_RC_RSY_PIN,
        1851,
        1120,
        55
    },
    {
        MAST_RC_LSX_PIN,
        1843,
        1056,
        0
    },
    {
        MAST_RC_LSY_PIN,
        1892,
        1086,
        0
    },
    {
        MAST_RC_GEAR_PIN,
        1888,
        1091,
        0
    },
    {
        MAST_RC_AUX_PIN,
        1887,
        1077,
        0
    }
};

/// To store the amount of time taken by one control loop is ms
uint32_t time_loop_current;
uint32_t time_loop_last;
uint32_t time_loop_highest;

/******************************************************************************
 */


void setup() {
    Wire.begin();

    SERIAL_PORT_CONSOLE.begin(SERIAL_BAUD_CONSOLE);
#ifdef DEBUG
    SERIAL_PORT_CONSOLE.print(F("Serial0 has address: "));
    SERIAL_PORT_CONSOLE.println( (unsigned int) &SERIAL_PORT_CONSOLE, HEX );
#endif

#ifdef _MEGA
    SERIAL_PORT_POLOLU.begin(SERIAL_BAUD_POLOLU);
    SERIAL_PORT_AIRMAR.begin(SERIAL_BAUD_AIRMAR);
    SERIAL_PORT_AIS.begin(SERIAL_BAUD_AIS);
#endif

    cons_cmdlist_init( &functions );
    cons_reg_cmd( &functions, "about", (void*) cabout );
    cons_reg_cmd( &functions, "help", (void*) cabout );
    cons_reg_cmd( &functions, "test", (void*) ctest );
    cons_reg_cmd( &functions, "dia", (void*) cdiagnostic_report );

#ifdef _MEGA
    cons_reg_cmd( &functions, "mon", (void*) cmon );
    cons_reg_cmd( &functions, "latlongtest", (void*) latlongtest);
    cons_reg_cmd( &functions, "servo", (void*) ccheckservo );

#ifdef RC_CALIBRATION
    cons_reg_cmd( &functions, "calrc", (void*) calrc );
    cons_reg_cmd( &functions, "mode", (void*) csetmode );
    cons_reg_cmd( &functions, "ee", (void*) ceeprom );
    cons_reg_cmd( &functions, "rc", (void*) crcd );
    cons_reg_cmd( &functions, "pol", (void*) ctest_pololu );
#endif // if RC_CALIBRATION

    // Initialise string buffer for NMEA
    airmar_nmea_buffer = bfromcstralloc( AIRMAR_NMEA_STRING_BUFFER_SIZE, "" );
#endif // if _MEGA

    // Last step in the initialisation, command line ready
    cons_init_line( &cli, &SERIAL_PORT_CONSOLE );

    // Update the looop timer with the current value
    time_loop_last = 0;
    time_loop_highest = 0;
    time_loop_current = millis();

    /*rc_read_calibration_eeprom( 0x08, &radio_controller );*/
    /*rc_DEBUG_print_controller( &Serial, &radio_controller );*/

    pinMode( MAST_RC_RSX_PIN, INPUT );
    pinMode( MAST_RC_RSY_PIN, INPUT );
    pinMode( MAST_RC_LSY_PIN, INPUT );
    pinMode( MAST_RC_LSX_PIN, INPUT );
    pinMode( MAST_RC_GEAR_PIN, INPUT );
    pinMode( MAST_RC_AUX_PIN, INPUT );

    // Initialise servo motor information
    pservo_0.id = 11;
    pservo_0.line = &SERIAL_PORT_POLOLU;

    p_rudder[0].channel_id = 0;
    p_rudder[0].controller = &pservo_0;

    p_rudder[1].channel_id = 2;
    p_rudder[1].controller = &pservo_0;

    // Initialise dc motor information
    pdc_mast_motors[0].id = 12;
    pdc_mast_motors[0].line = &SERIAL_PORT_POLOLU;

    pdc_mast_motors[1].id = 13;
    pdc_mast_motors[1].line = &SERIAL_PORT_POLOLU;

    // Time set
    setSyncProvider(RTC.get);   // the function to get the time from the RTC
    if(timeStatus() != timeSet) {
        cli.port->println(F("Unable to sync with the RTC"));
    } else {
        cli.port->println(F("RTC has set the system time"));
    }
    display_time( cli.port );


    // Yeah!
    cli.port->print(F("Initialisation complete, awaiting commands"));

    // Print the prefix to the command line, the return code from the previus
    // function doesn't exist, so default to zero
    print_cli_prefix( &cli, 0 );
}

/** Polling loop definition
 ******************************************************************************
 */
void loop() {
    static int res;

    if( gaelforce & MODE_COMMAND_LINE ) {
        res = cons_poll_line( &cli, CONSHELL_CLI_TIMEOUT );
        if( res == CONSHELL_LINE_END ) {
            res = cons_search_exec( &cli, &functions );
            print_cli_prefix( &cli, res );
            cons_clear_line( &cli );
        }
    }

    if( gaelforce & MODE_RC_CONTROL ) {
        rmode_update_motors(
                &radio_controller,
                pdc_mast_motors,
                p_rudder
            );
    }

    event_time_motor( &test_motor );

#ifdef _MEGA
    if( gaelforce & MODE_AIRMAR_POLL ) {
        res = anmea_poll_char(
                    airmar_nmea_buffer,
                    &Serial3 );
        if( res == ANMEA_POLL_STRING_READY ) {
            uint8_t str_match =
                strncasecmp( (char*) airmar_nmea_buffer->data,
                        "$WIMWV",
                        min( airmar_nmea_buffer->slen, 6 )
                    );

            if( str_match == 0 ) {
                anmea_update_wiwmv(
                        &airmar_nmea_wimwv_tag,
                        airmar_nmea_buffer );

                anmea_is_string_invalid( airmar_nmea_buffer );
                anmea_print_wiwmv( &airmar_nmea_wimwv_tag, cli.port );
                airmar_nmea_wimwv_tag.flags &= ~ANMEA_TAG_WIMV_WIND_RELATIVE;
            }
            bassigncstr( airmar_nmea_buffer, "" );
        } else if( res == ANMEA_POLL_ERROR ) {
            cli.port->println(F("POLL ERROR"));
            bassigncstr( airmar_nmea_buffer, "" );
        } else if( res == ANMEA_POLL_STRING_FAIL ) {
            cli.port->print(F("FAILED TO GET STRING: "));
            cli.port->println( (char*) airmar_nmea_buffer->data );
            bassigncstr( airmar_nmea_buffer, "" );
        }
    }
#endif

    // Update the amount of time the loop took to process
    time_loop_last = time_loop_current;
    time_loop_current = micros();
    if( (time_loop_current - time_loop_last) > time_loop_highest ) {
        time_loop_highest = time_loop_current - time_loop_last;
    }
}
/******************************************************************************
 */

/** Command line prefix generation command
 ******************************************************************************
 * Call this every time a command completes
 */
void print_cli_prefix( cons_line* cli, int res ) {
    Stream* line = cli->port;

    line->println();
    line->print('[');
    line->print( res == CONSHELL_FUNCTION_NOT_FOUND ? '0' : '1' );
    line->print('|');

    if( gaelforce & MODE_RC_CONTROL ) {
        line->print(F("RC|"));
    }

    if( gaelforce & MODE_AIRMAR_POLL ) {
        line->print(F("AIR|"));
    }

    line->print(F("T"));
    line->print( time_loop_current - time_loop_last );
    line->print(F("|"));

    line->print( getAvailableMemory() );
    line->print(F("]> "));

}

/** System diagnostics
 ******************************************************************************
 */
void
diagnostics( cons_line* cli )
{
    Stream* con = cli->port; // Short for con(sole)
    char buf[80] = { '\0' };
    uint32_t uptime[2];
    uint16_t req_value; //Requested value


    // Print the current time
    con->println();
    display_time( cli->port );

    // Check connection to pololus
    req_value =
        pchamp_request_value( &(pdc_mast_motors[0]), PCHAMP_DC_VAR_TIME_LOW );
    uptime[0] = req_value & 0xFFFF;
    req_value =
        pchamp_request_value( &(pdc_mast_motors[0]), PCHAMP_DC_VAR_TIME_HIGH );
    uptime[0] += req_value * 65536ULL;

    req_value =
        pchamp_request_value( &(pdc_mast_motors[1]), PCHAMP_DC_VAR_TIME_LOW );
    uptime[1] = req_value & 0xFFFF;
    req_value =
        pchamp_request_value( &(pdc_mast_motors[1]), PCHAMP_DC_VAR_TIME_HIGH );
    uptime[1] += req_value * 65536ULL;

    // Report for controller 0
    snprintf_P( buf,
            sizeof(buf),
            PSTR(
                "Pololu controllers\n"
                "M0: Uptime -> %lu msec\n"
                "    Voltag -> %u mV\n"
            ),
            uptime[0],
            pchamp_request_value( &(pdc_mast_motors[0]), PCHAMP_DC_VAR_VOLTAGE )
        );
    con->print( buf );

    // Report for controller 1
    snprintf_P( buf,
            sizeof(buf),
            PSTR(
                "M1: Uptime -> %lu msec\n"
                "    Voltag -> %u mV\n"
            ),
            uptime[1],
            pchamp_request_value( &(pdc_mast_motors[1]), PCHAMP_DC_VAR_VOLTAGE )
        );
    con->print( buf );

    // Report for rudder controller

    // Check values from attopilot units
    con->println(F("Power box"));
    snprintf_P( buf,
            sizeof(buf),
            PSTR(
                "BATTERY: Current -> %u\n"
                "         Voltage -> %u\n"
            ),
            barn_get_battery_current(),
            barn_get_battery_voltage()
        );
    con->print( buf );

    snprintf_P( buf,
            sizeof(buf),
            PSTR(
                "CHARGER: Current -> %u\n"
                "         Voltage -> %u\n"
            ),
            barn_get_charger_current(),
            barn_get_charger_voltage()
        );
    con->print( buf );

    snprintf_P( buf, sizeof(buf),
            PSTR("ENC: W1: %u\n"
                 "     W2: %u\n"),
            barn_get_w1_ticks(),
            barn_get_w2_ticks()
        );
    con->print( buf );

}
/******************************************************************************
 */

void display_time( Stream* com )
{
    char buf[20];
    snprintf_P( buf, sizeof(buf),
            PSTR(
                "%d %d %d %02d:%02d:%02d\n"
            ),
            year(),
            month(),
            day(),
            hour(),
            minute(),
            second()
        );
    com->print( buf );
}

void
event_time_motor( event_time_motor_t* event )
{
    if(     event->completed == true
        ||  event->target > millis() ) {
        return;
    }

    pchamp_set_target_speed(
            event->motor,
            event->speed,
            event->dir
        );

    event->completed = true;
}
#endif // Include guard
// vim:ft=c:

