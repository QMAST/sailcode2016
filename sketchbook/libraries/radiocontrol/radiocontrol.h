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


typedef struct {
    const uint8_t pin;

    uint16_t high;
    uint16_t low;

} rc_channel_t;


// All of the channels required to model our RC controller
typedef struct {
    rc_channel_t rsx;
    rc_channel_t rsy;

    rc_channel_t lsx;
    rc_channel_t lsy;

    rc_channel_t gear;
} rc_mast_controller;

int16_t rc_get_raw_analog( rc_channel_t );
int16_t rc_get_mapped_analog( rc_channel_t, int16_t, int16_t);

/** Print out all the values in the mast_controller_struct
 *
 * Uses F() macro to keep strings in program memory
 */
void rc_print_controller_mapped( Stream*, rc_mast_controller* );
void rc_print_controller_raw( Stream*, rc_mast_controller* );
void rc_write_calibration_eeprom( uint16_t, rc_mast_controller* );
void rc_read_calibration_eeprom( uint16_t, rc_mast_controller* );

/** If the given value is greater than the constant max or less than the
 * constant minimum, the constant is updated to reflect the new higher or lower
 * value
 */
//void rc_check_minmax( uint16_t, rc_channel_t* );

#endif

