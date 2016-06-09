#ifndef _PINS_H
#define _PINS_H

#include <inttypes.h>

#if defined(__AVR_ATmega2560__) || defined(__AVR_ATmega1280__)
#define _MEGA
#endif

#define DEBUG

// This function uses a lot of ram for something used rarely
#define RC_CALIBRATION

// Amount of memory to reserve for NMEA string build
//prev 80
#define AIRMAR_NMEA_STRING_BUFFER_SIZE 80

#define SERIAL_PORT_CONSOLE Serial
#define SERIAL_PORT_POLOLU  Serial1
#define SERIAL_PORT_AIRMAR  Serial2
#define SERIAL_PORT_BARN    Serial3
#define SERIAL_PORT_KEEL_SW    Serial4
//#define SERIAL_PORT_XBEE    Serial5

#define SERIAL_BAUD_XBEE 115200
#define SERIAL_BAUD_CONSOLE 57600
#define SERIAL_BAUD_AIRMAR  4800
#define SERIAL_BAUD_AIRMAR_BOOST 38400
//#define SERIAL_BAUD_AIS     38400
#define SERIAL_BAUD_POLOLU  9600
#define SERIAL_BAUD_BARNACLE 19200
#define SERIAL_BAUD_KEEL 57600

// Max and min usec values that can be sent to rudder servo motors
#define POLOLU_SERVO_0_RUD_MIN 100
#define POLOLU_SERVO_0_RUD_MAX 1900
#define POLOLU_SERVO_2_RUD_MIN 600
#define POLOLU_SERVO_2_RUD_MAX 2400

//Mid 0 = 1003 mid 2 = 1640
#define BARNACLE_RESET_PIN 7

/*Default Pin Values*/
// Plug each channel from the receiver in to each of these arduino pins
#define MAST_RC_RSX_PIN 22 // Channel 1
#define MAST_RC_RSY_PIN 25 // Channel 2
#define MAST_RC_LSY_PIN 24 // Channel 3
#define MAST_RC_LSX_PIN 23  // Channel 4

#define MAST_RC_GEAR_PIN 26 // Channel 5

#define XBEE_RX 51
#define XBEE_TX 52

#define KEEL_RX 10
#define KEEL_TX 11

#define KEEL_MODE_AUTO 0x01
#define KEEL_MODE_MANUAL 0x02
#define KEEL_POSE_CENTER 0x03
#define KEEL_POSE_FULL_PORT 0x04
#define KEEL_POSE_FULL_STARBOARD 0x05


/** Mode of operation definitions
 ******************************************************************************
 */
typedef enum {
    MODE_COMMAND_LINE = 0x1,
    MODE_MOTOR_TEST = 0x2,
    MODE_RC_CONTROL = 0x4,
    MODE_AUTOSAIL = 0x8,
    MODE_DIAGNOSTICS_OUTPUT = 0x10,
	MODE_WAYPOINT = 0x20
} gaelforce_mode_t;
/******************************************************************************
 */

/** EEPROM Memory mapping
 ******************************************************************************
 */
#define EE_RC_SETTINGS  0x08
#define EE_TEST_LOC     EE_RC_SETTINGS + sizeof( rc_mast_controller ) + 1
/******************************************************************************
 */

// GPIO pins on Odroid, for computer vision buoy detection
#define ODROID_COMPVI_LEFT 38
#define ODROID_COMPVI_RIGHT 39
 
 #endif
