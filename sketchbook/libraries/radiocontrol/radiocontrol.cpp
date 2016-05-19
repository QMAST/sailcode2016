#include "radiocontrol.h"

int16_t
rc_get_raw_analog( rc_channel_t ch )
{
    int16_t value;
    value = pulseIn( ch.pin, HIGH, RC_STD_TIMEOUT );

    return value;
}



int16_t
rc_get_mapped_analog( rc_channel_t ch, int16_t low_out, int16_t high_out)
{
    int16_t value;

    value = pulseIn( ch.pin, HIGH, RC_STD_TIMEOUT );

    value = map( value, ch.low, ch.high, low_out, high_out );
	
    return value;
}

void
rc_print_controller_mapped( Stream *port, rc_mast_controller *controller )
{
    // Bleagh, might switch to using string buffers for things like this
    port->print( F("\tLSX: ") );
    port->print( rc_get_mapped_analog(controller->lsx, -1000, 1000) );
    port->print( F("\tLSY: ") );
    port->print( rc_get_mapped_analog(controller->lsy, -1000, 1000) );

    port->print( F("\tRSX: ") );
    port->print( rc_get_mapped_analog(controller->rsx, -1000, 1000) );
    port->print( F("\tRSY: ") );
    port->print( rc_get_mapped_analog(controller->rsy, -1000, 1000) );

    port->print( F("\tGear: ") );
    port->println( rc_get_mapped_analog(controller->gear, 0, 2) );
}

void
rc_print_controller_raw( Stream *port, rc_mast_controller *controller )
{
    // Bleagh, might switch to using string buffers for things like this
    port->print( F("LSX: ") );
    port->print( rc_get_raw_analog(controller->lsx) );
    port->print( F("\tLSY: ") );
    port->print( rc_get_raw_analog(controller->lsy) );

    port->print( F("\tRSX: ") );
    port->print( rc_get_raw_analog(controller->rsx) );
    port->print( F("\tRSY: ") );
    port->print( rc_get_raw_analog(controller->rsy) );

    port->print( F("\tGear: ") );
    port->println( rc_get_raw_analog(controller->gear) );
}

void
rc_print_calibration( Stream *port, rc_mast_controller *controller )
{
    port->print( F("LSY: [") );
    port->print( controller->lsy.low );
    port->print( F(", ") );
    port->print( controller->lsy.high );
	port->print( F("]\t") );

    port->print( F("RSX: [") );
    port->print( controller->rsx.low );
    port->print( F(", ") );
    port->print( controller->rsx.high );
	port->println( F("]\t") );
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
