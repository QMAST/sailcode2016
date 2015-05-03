/** Skoonercode
 *
 * Supercedes ferrycode, I thought the name looked cooler than Schoonercode
 *
 * Uses a simple polling loop with timeouts enforced by the functions
 * themselves. Each function is passed the amount of time it is allowed to take
 * for execution, and either exits as soon as possible, or attempts to perform
 * its function for the entire time until it realises it needs to exit.
 *
 * Main interface, command line functions. No function may lock the polling
 * loop into a single function. Certain polling functions may be
 * enabled/disabled on the fly.
 */
#ifndef _SKOONERCODE_INO
#define _SKOONERCODE_INO
#include "pins.h"
#include <SoftwareSerial.h>

#include <avr/pgmspace.h>
#include <inttypes.h>

#include <WSWire.h>
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

/** Global Variable instances
 ******************************************************************************
 */

// Instances necessary for command line usage
cons_line cli;
cmdlist functions;

// AIRMAR NMEA String buffer
//char airmar_buffer_char[80];
anmea_buffer_t airmar_buffer;
//anmea_tag_wiwmv_t airmar_nmea_wimwv_tag;

// Motor object definitions
pchamp_controller rudder_control; // Rudder
pchamp_servo rudder_servo[2];

pchamp_controller winch_control[2]; // Drum winches


// Global to track current mode of operation
uint16_t gaelforce = MODE_COMMAND_LINE;

// Software serial instances
SoftwareSerial* Serial4;

/// Initialise pin numbers and related calibration values, most values should
//be overwritten by eeprom during setup()
rc_mast_controller radio_controller = {
    { MAST_RC_RSX_PIN, 1900, 1100 },
    { MAST_RC_RSY_PIN, 1900, 1100 },
    { MAST_RC_LSX_PIN, 1900, 1100},
    { MAST_RC_LSY_PIN, 1900, 1100 },
    { MAST_RC_GEAR_PIN, 1900, 1100}
};

//Turn counter for the airmar tags.
int airmar_turn_counter;
const char *AIRMAR_TAGS[3] = {"$HCHDG","$WIMWV","$GPGLL"};
uint8_t NUMBER_OF_TAGS = 3;

//AIRMAR tags for holding data.
anmea_tag_hchdg_t head_tag;
anmea_tag_wiwmv_t wind_tag;
anmea_tag_gpgll_t gps_tag;

/******************************************************************************
 */


void setup() {
    // Set initial config for all serial ports
    SERIAL_PORT_CONSOLE.begin(SERIAL_BAUD_CONSOLE);
    delay(100);
    SERIAL_PORT_CONSOLE.println(F("Gaelforce starting up!"));
    SERIAL_PORT_CONSOLE.println(F("----------------------"));

    SERIAL_PORT_CONSOLE.print(F("Init all serial ports..."));
    // Set up the rest of the ports
    SERIAL_PORT_POLOLU.begin(SERIAL_BAUD_POLOLU);
    SERIAL_PORT_AIRMAR.begin(SERIAL_BAUD_AIRMAR);
    SERIAL_PORT_AIS.begin(SERIAL_BAUD_AIS);
    SERIAL_PORT_CONSOLE.println(F("OKAY!"));

    // NOTE: AUUUUUGGGHHH THIS IS DISGUSTING
    Serial4 = new SoftwareSerial( SERIAL_SW4_RXPIN, SERIAL_SW4_TXPIN );
    SERIAL_PORT_BARN->begin( SERIAL_BAUD_BARNACLE_SW );
    barnacle_port = Serial4;
    // Its a global pointer defined in another library that needs to be set
    // even though it isn't obvious. Fix the library as soon as members are
    // able to!
    
    SERIAL_PORT_CONSOLE.print(F("Registering cli funcs..."));
    // Register all command line functions
    cons_cmdlist_init( &functions );
    cons_reg_cmd( &functions, "help", (void*) cabout );
    cons_reg_cmd( &functions, "dia", (void*) cdiagnostic_report );
    cons_reg_cmd( &functions, "mon", (void*) cmon );
    cons_reg_cmd( &functions, "lltest", (void*) latlongtest);
    cons_reg_cmd( &functions, "calrc", (void*) calrc );
    cons_reg_cmd( &functions, "mode", (void*) csetmode );
    cons_reg_cmd( &functions, "ee", (void*) ceeprom );
    cons_reg_cmd( &functions, "rc", (void*) crcd );
    cons_reg_cmd( &functions, "mot", (void*) cmot );
    cons_reg_cmd( &functions, "now", (void*) cnow );
    cons_reg_cmd( &functions, "res", (void*) cres );
	cons_reg_cmd( &functions, "airmar", (void*) cairmar );

    // Last step in the cli initialisation, command line ready
    cons_init_line( &cli, &SERIAL_PORT_CONSOLE );
    SERIAL_PORT_CONSOLE.println(F("OKAY!"));

    SERIAL_PORT_CONSOLE.print(F("Setting motor information..."));
    // Initialise servo motor information
    rudder_control.id = 11;
    rudder_control.line = &SERIAL_PORT_POLOLU;

    rudder_servo[0].channel_id = 0;
    rudder_servo[0].controller = &rudder_control;

    rudder_servo[1].channel_id = 2;
    rudder_servo[1].controller = &rudder_control;

    // Initialise dc motor information
    winch_control[0].id = 12;
    winch_control[0].line = &SERIAL_PORT_POLOLU;

    winch_control[1].id = 13;
    winch_control[1].line = &SERIAL_PORT_POLOLU;

    SERIAL_PORT_CONSOLE.println(F("OKAY!"));

    pinMode( BARNACLE_RESET_PIN, OUTPUT );

    SERIAL_PORT_CONSOLE.print(F("Barnacle reboot (1)..."));
    reset_barnacle();
    SERIAL_PORT_CONSOLE.println(F("OKAY!"));

    SERIAL_PORT_CONSOLE.print(F("Init wire network..."));
    Wire.begin();
    SERIAL_PORT_CONSOLE.println(F("OKAY!"));

    SERIAL_PORT_CONSOLE.print(F("Setting system time..."));
    // Time set
    // the function to get the time from the RTC
    delay(100);
    setSyncProvider(RTC.get);
    if(timeStatus() != timeSet) {
        cli.port->println(F("Unable to sync with the RTC"));
    } else {
        cli.port->println(F("RTC has set the system time"));
    }
    display_time( cli.port );
    SERIAL_PORT_CONSOLE.println(F("OKAY!"));

    // Initialize the airmar buffer state
	SERIAL_PORT_AIRMAR.println("$PAMTC,EN,ALL,0");	//Disable all
	SERIAL_PORT_AIRMAR.println("$PAMTC,EN,GLL,1,10");	//tag, enable, target tag, [0 (dis) / 1 (en)], period (1s/10)
	SERIAL_PORT_AIRMAR.println("$PAMTC,EN,HDG,1,10");
	SERIAL_PORT_AIRMAR.println("$PAMTC,EN,MWVR,1,10");	//Use relative ie. apparent wind
	//SERIAL_PORT_AIRMAR.println("$PAMTC,EN,S");		//Save to eeprom
    airmar_buffer.state = ANMEA_BUF_SEARCHING;
    airmar_buffer.data  = bfromcstralloc( AIRMAR_NMEA_STRING_BUFFER_SIZE, "" );

    SERIAL_PORT_CONSOLE.print(F("Resetting barnacle tick counters..."));
    // Reset encoder counters to 0
    delay(100);
    barn_clr_w1_ticks();
    barn_clr_w2_ticks();
    SERIAL_PORT_CONSOLE.println(F("OKAY!"));

    SERIAL_PORT_CONSOLE.print(F("Barnacle reboot (2)..."));
    reset_barnacle();
    SERIAL_PORT_CONSOLE.println(F("OKAY!"));
	
	rc_read_calibration_eeprom( 0x08, &radio_controller );
	Serial.print(F("RC Settings Loaded - "));
	rc_print_calibration( &Serial , &radio_controller);

    // Yeah!
    cli.port->print(F("Initialisation complete, awaiting commands"));

    // Print the prefix to the command line, the return code from the previous
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

    if( gaelforce & MODE_DIAGNOSTICS_OUTPUT ) {
        static uint32_t diagnostics_last = millis();
        if( (millis() - diagnostics_last) > 1000 ) {
            diagnostics_last = millis();
            diagnostics( &cli );
            print_cli_prefix( &cli, res );
            cli.port->print( (char*) cli.line->data );
        }
    }

    if( gaelforce & MODE_RC_CONTROL ) {
        rmode_update_motors(
                &radio_controller,
                winch_control,
                rudder_servo
            );
    }

    if( gaelforce & MODE_AUTOSAIL ) {
		autosail_main();
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

    if( gaelforce & MODE_AUTOSAIL ) {
        line->print(F("AUTO|"));
    }

    if( gaelforce & MODE_DIAGNOSTICS_OUTPUT ) {
        line->print(F("DIA|"));
    }

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
    char buf[120] = { '\0' };
    uint32_t uptime[2];
    uint16_t req_value; //Requested value


    // Print the current time
    con->println();
    display_time( cli->port );

    // Check connection to pololus
	//Motor 0 disattached
	
    req_value =
        pchamp_request_value( &(winch_control[0]), PCHAMP_DC_VAR_TIME_LOW );
    uptime[0] = req_value & 0xFFFF;
    req_value =
        pchamp_request_value( &(winch_control[0]), PCHAMP_DC_VAR_TIME_HIGH );
    uptime[0] += req_value * 65536ULL;
	
    /*req_value =
        pchamp_request_value( &(winch_control[1]), PCHAMP_DC_VAR_TIME_LOW );
    uptime[1] = req_value & 0xFFFF;
    req_value =
        pchamp_request_value( &(winch_control[1]), PCHAMP_DC_VAR_TIME_HIGH );
    uptime[1] += req_value * 65536ULL;*/

    // Report for controller 0
    snprintf_P( buf,
            sizeof(buf),
            PSTR(
                "Pololu controllers\n"
                "M0: Uptime -> %lu msec\n"
                "    Voltag -> %u mV\n"
                "    TEMP   -> %u\n"
                "    ERRORS -> 0x%02x\n"
            ),
            uptime[0],
            pchamp_request_value( &(winch_control[0]), PCHAMP_DC_VAR_VOLTAGE ),
            pchamp_get_temperature( &(winch_control[0]) ),
            pchamp_request_value( &(winch_control[0]), PCHAMP_DC_VAR_ERROR )
        );
    con->print( buf );
    delay(100);

    // Report for controller 1
    /*snprintf_P( buf,
            sizeof(buf),
            PSTR(
                "M1: Uptime -> %lu msec\n"
                "    Voltag -> %u mV\n"
                "    TEMP   -> %u\n"
                "    ERRORS -> 0x%02x\n"
            ),
            uptime[1],
            pchamp_request_value( &(winch_control[1]), PCHAMP_DC_VAR_VOLTAGE ),
            pchamp_get_temperature( &(winch_control[0]) ),
            pchamp_request_value( &(winch_control[1]), PCHAMP_DC_VAR_ERROR )
        );
    con->print( buf );*/

    // Report for rudder controller

    // Check values from attopilot units
    con->println(F("Power box"));
    snprintf_P( buf,
            sizeof(buf),
            PSTR(
                "BATTERY: Voltage -> %u\n"
                "         Current -> %u\n"
            ),
            barn_get_battery_voltage(),
            barn_get_battery_current()
        );
    con->print( buf );

    snprintf_P( buf,
            sizeof(buf),
            PSTR(
                "CHARGER: Voltage -> %u\n"
                "         Current -> %u\n"
            ),
            barn_get_charger_voltage(),
            barn_get_charger_current()
        );
    con->print( buf );

    snprintf_P( buf, sizeof(buf),
            PSTR("ENC: W1: %u T1: %u\n"
                 "     W2: %u T2: %u\n"),
            barn_get_w1_ticks(),
            0,
            barn_get_w2_ticks(),
            0
        );
    con->print( buf );

    snprintf_P( buf, sizeof(buf),
            PSTR("BARNACLE_LATENCY: %lu\n"),
            barn_check_latency()
            );
    con->print(buf);
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

void reset_barnacle() {
    digitalWrite( BARNACLE_RESET_PIN, LOW );
    delay(250);
    digitalWrite( BARNACLE_RESET_PIN, HIGH );
    delay(250);
}
#endif // Include guard
// vim:ft=c:

