#include "radiocontrol.h"

rc_resolution_t
rc_get_raw_analog( rc_channel_t ch )
{
    int16_t value;
    value = pulseIn( ch.pin, HIGH, RC_STD_TIMEOUT );
    return value;
}

rc_resolution_t
rc_get_analog( rc_channel_t ch )
{
    int16_t value;

    value = pulseIn( ch.pin, HIGH, RC_STD_TIMEOUT );
    value -= ch.neutral;

    return value;
}

rc_resolution_t
rc_get_analog_mapped( rc_channel_t ch)
{
    int16_t value;

    value = pulseIn( ch.pin, HIGH, RC_STD_TIMEOUT );

    value = map( value, ch.constant_low, ch.constant_high, -1000, 1000 );
	

    return value;
}



/** Return whether or not switch is pushed _AWAY_ from user
 *
 *  Returns
 *      - 1 : If switch is pushed
 *      - 0 : Otherwise
 *
 *  Uses the neutral value rather than the constant highs and lows
 */
uint8_t rc_get_digital( rc_channel_t ch )
{
    int16_t value;

    value = pulseIn( ch.pin, HIGH, RC_STD_TIMEOUT );

    if( value > (ch.neutral + RC_BUTTON_BIAS) ) {
        return 1;
    }

    return 0;
}

void
rc_DEBUG_print_controller( Stream *port, rc_mast_controller *controller )
{
    // Bleagh, might switch to using string buffers for things like this
    port->println( F("Right stick") );
    port->print( F("X") );
    port->println( controller->rsx.neutral );
    port->print( F("Y") );
    port->println( controller->rsy.neutral );

    port->println( F("Left stick") );
    port->print( F("X") );
    port->println( controller->lsx.neutral );
    port->print( F("Y") );
    port->println( controller->lsy.neutral );

    port->print( F("Gear switch") );
    port->println( controller->gear_switch.neutral );
    port->print( F("Aux") );
    port->println( controller->aux.neutral );
}

void
rc_write_calibration_eeprom( uint16_t addr, rc_mast_controller *controller )
{
    eeprom_busy_wait();
    delay(100); // To prevent accidental destruction of eeprom
    cli();
    eeprom_write_block(
            (void*) controller,
            (void*) addr,
            sizeof(rc_mast_controller) );
    sei();
}

void
rc_read_calibration_eeprom( uint16_t addr, rc_mast_controller *controller )
{
    eeprom_busy_wait();
    cli();
    eeprom_read_block(
            (void*) controller,
            (void*) addr,
            sizeof(rc_mast_controller) );
    sei();
}
