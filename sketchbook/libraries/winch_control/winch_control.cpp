#include "winch_control.h"

// Not accessible outside this library.
int16_t winch_target_offset = 0;

void winch_set_target_offset(int16_t offset) {
	// Barnacle latency is (relatively) huge, so don't
	// poll it if we don't need to.
	if( offset == 0 ) return;
	
	int16_t current_position = barn_get_w1_ticks();
	barn_clr_w1_ticks();
	// Add the new offset to what's left of the old one so that multiple calls
	// with a short period of time won't affect the final position
	int16_t distance_from_old_target = winch_target_offset - current_position;
	winch_target_offset = distance_from_old_target + offset;
}

void winch_set_speed(int16_t speed) {
	if( speed == 0 ) {
		// Stop and lock motors
		pchamp_set_target_speed(
			&(pdc_mast_motors[0]), 0, PCHAMP_DC_MOTOR_FORWARD );
		pchamp_request_safe_start( &(pdc_mast_motors[0]), false );
	} else {
		// Unlock and start motors
		pchamp_request_safe_start( &(pdc_mast_motors[0]) );
		pchamp_set_target_speed(
			&(pdc_mast_motors[0]), abs(speed), 
			speed > 0 ? PCHAMP_DC_FORWARD : PCHAMP_DC_REVERSE);
	}
}

uint8_t winch_update_motor_speed(void) {
	uint16_t distance_moved = barn_get_w1_ticks();
	
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

void winch_move_ticks(int16_t offset) {
	winch_set_target_offset(offset);
	while(update_winch_motor_speed());
}