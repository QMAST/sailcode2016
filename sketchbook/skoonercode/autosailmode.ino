#define AUTOSAIL_TIMEOUT 1000
#define AUTOSAIL_ALGORITHM_TICKS 100
#define AUTOSAIL_MIN_DELAY 10

uint32_t autosail_start_time;

void autosail_main(){
	if(millis() - autosail_start_time < AUTOSAIL_MIN_DELAY)
		return;
	
	autosail_start_time = millis();
	static uint32_t tick_counter = 0;
	
	update_airmar_tags();
	
	if(autosail_check_timeout()){
		tick_counter++;
		return;
	}
	
	motor_winch_update();
	
	if(autosail_check_timeout()){
		tick_counter++;
		return;
	}
	
	if(tick_counter % AUTOSAIL_ALGORITHM_TICKS == 0){
		//run autosail algorithm
		//ie.
		
		motor_winch_abs(10*tick_counter);
	}
	
	tick_counter++;
}

void update_airmar_tags(){
	anmea_poll_string(
			&SERIAL_PORT_AIRMAR,
			&airmar_buffer,
			AIRMAR_TAGS[airmar_turn_counter]
		);
		
	if( airmar_buffer.state == ANMEA_BUF_MATCH ) {
		if(airmar_turn_counter==0){
			anmea_update_hchdg(&head_tag, airmar_buffer.data);
		}
		
		else if(airmar_turn_counter==1){
			anmea_update_wiwmv(&wind_tag, airmar_buffer.data);
		}
		
		else{
			anmea_update_gpgll(&gps_tag, airmar_buffer.data);
		}
		
		airmar_turn_counter++;
		airmar_turn_counter = airmar_turn_counter%NUMBER_OF_TAGS;
		
		autosail_print_tags();
		
		anmea_poll_erase( &airmar_buffer );
	}
}

uint8_t autosail_check_timeout(){
	if(millis() - autosail_start_time > AUTOSAIL_TIMEOUT){
		Serial.print(F("Autosail Timeout\n"));
		return 1;
	}
		
	else
		return 0;
}


void autosail_print_tags(){
	anmea_print_wiwmv(&wind_tag, &Serial);
	anmea_print_hchdg(&head_tag, &Serial);
	anmea_print_gpgll(&gps_tag, &Serial);
	Serial.print("\n");
}

