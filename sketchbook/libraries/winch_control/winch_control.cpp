#include "winch_control.h"

#define WINCH_FULL_SPEED_FORWARD 1000
#define WINCH_FULL_SPEED_BACKWARD -1000

// Not accessible outside this library.
int16_t winch_target_offset = 0;

void winch_set_target_offset_ticks(int16_t offset) {
	// Barnacle latency is (relatively) huge, so don't
	// poll it if we don't need to.
	if( offset == 0 ) return;
	
	int16_t current_position = barnacle_client->barn_getandclr_w1_ticks();
	XBEE_SERIAL_PORT.listen();
	// Add the new offset to what's left of the old one so that multiple calls
	// with a short period of time won't affect the final position
	int16_t distance_from_old_target = winch_target_offset - current_position;
	winch_target_offset = distance_from_old_target + offset;
}

void winch_set_speed( int16_t speed) {
//	set_winch(speed, pdc_winch_motor, p_rudder);
}

uint8_t winch_update_motor_speed(void) {
	uint16_t distance_moved = barnacle_client->barn_get_w1_ticks();
	XBEE_SERIAL_PORT.listen();
	
	if( distance_moved >= abs(winch_target_offset) ) {
		winch_set_speed(0);
		return 0;
	} else if( winch_target_offset > 0 ) {
		winch_set_speed(WINCH_FULL_SPEED_FORWARD);
	} else if( winch_target_offset < 0 ) {
		winch_set_speed(WINCH_FULL_SPEED_BACKWARD);
	}
	
	return 1;
}