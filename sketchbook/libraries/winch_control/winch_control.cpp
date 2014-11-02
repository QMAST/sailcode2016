#include "winch_control.h"

#define WINCH_FULL_SPEED_FORWARD 3200
#define WINCH_FULL_SPEED_BACKWARD -3200

// Not accessible outside this library.
int16_t winch_target_offset = 0;

void winch_set_target_offset_ticks(int16_t offset) {
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

void winch_set_speed(pchamp_controller *motor, int16_t speed) {
	if( speed == 0 ) {
		// Stop and lock motors
		pchamp_set_target_speed(
			motor, 0, PCHAMP_DC_MOTOR_FORWARD );
		pchamp_request_safe_start( motor, false );
	} else {
		// Unlock and start motors
		pchamp_request_safe_start( motor );
		pchamp_set_target_speed(
			motor, abs(speed), 
			speed > 0 ? PCHAMP_DC_FORWARD : PCHAMP_DC_REVERSE);
	}
}

uint8_t winch_update_motor_speed(pchamp_controller *motor) {
	uint16_t distance_moved = barn_get_w1_ticks();
	
	if( distance_moved >= abs(winch_target_offset) ) {
		winch_set_speed(motor, 0);
		return 0;
	} else if( winch_target_offset > 0 ) {
		winch_set_speed(motor, WINCH_FULL_SPEED_FORWARD);
	} else if( winch_target_offset < 0 ) {
		winch_set_speed(motor, WINCH_FULL_SPEED_BACKWARD);
	}
	
	return 1;
}