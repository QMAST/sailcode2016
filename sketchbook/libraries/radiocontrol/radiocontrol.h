/** RC Arduino Library
 *
 * Data structures to model and store preferences for the analog joysticks and
 * switches on the RC controller/receiver.
 */
#ifndef _RADIO_CONTROL_H
#define _RADIO_CONTROL_H
#include <Arduino.h>
#include <avr/eeprom.h>

#define RC_STD_TIMEOUT 50000 //standard timeout, recommend at least 20000
#define RC_BUTTON_BIAS 100
#define RC_CONTROL_THRESHOLD 10

typedef int16_t rc_resolution_t;

typedef struct {
    const uint8_t pin;

    uint16_t constant_high;
    uint16_t constant_low;

    uint16_t neutral;
} rc_channel_t;


// All of the channels required to model our RC controller
typedef struct {
    rc_channel_t rsx;
    rc_channel_t rsy;

    rc_channel_t lsx;
    rc_channel_t lsy;

    rc_channel_t gear_switch;
    rc_channel_t aux;
} rc_mast_controller;

rc_resolution_t rc_get_raw_analog( rc_channel_t ch );
rc_resolution_t rc_get_analog( rc_channel_t );
rc_resolution_t rc_get_analog_mapped( rc_channel_t );
uint8_t rc_get_digital( rc_channel_t );

/** Print out all the values in the mast_controller_struct
 *
 * Uses F() macro to keep strings in program memory
 */
void rc_DEBUG_print_controller( Stream*, rc_mast_controller* );
void rc_write_calibration_eeprom( uint16_t, rc_mast_controller* );
void rc_read_calibration_eeprom( uint16_t, rc_mast_controller* );

/** If the given value is greater than the constant max or less than the
 * constant minimum, the constant is updated to reflect the new higher or lower
 * value
 */
//void rc_check_minmax( uint16_t, rc_channel_t* );

#endif

