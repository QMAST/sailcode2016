
#define FORWARDS 0
#define BACKWARDS 1

void motor_go_forwards(void) {

}

void motor_go_backwards(void) {

}

void motor_stop(void) {

}

uint8_t update_motor_encoder(uint16_t target, uint8_t direction) {
	
	uint16_t current_value = barn_get_w1_ticks();
	
	if(current_value >= target){
		motor_stop();
		return 0;
	} else if( direction == FORWARDS ) motor_go_forwards();
	  else if( direction == BACKWARDS) motor_go_backwards();

	return 1;
}

void set_motor_target(int16_t target) {
	barn_clr_w1_ticks();
	if(target == 0) return;
	uint8_t direction = (target > 0) ? FORWARDS : BACKWARDS; 
	while(update_motor_encoder(abs(target), direction));
}