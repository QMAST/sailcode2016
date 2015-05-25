#define AUTOSAIL_TIMEOUT 500
#define AUTOSAIL_ALGORITHM_TICKS 100
#define AUTOSAIL_MIN_DELAY 10
#define NAV_MIN_DELAY 2000
#define devCount 360
#define CHDEG 15 //Closest degree offset from wind direction that boat can sail
#define DIRCONST 15 //Offset of green area from dirDeg (plus or minus)

uint32_t autosail_start_time;
uint32_t time_since_lastNav;
uint32_t last_degree;
//int degree = 0;


//const int winDeg = 0;
  
 typedef struct DegScore{
	int degree;
	int score;
}DegScore;

DegScore navScore[devCount];

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
	
	//motor_winch_update();
	

	if(tick_counter % AUTOSAIL_ALGORITHM_TICKS == 0){
		
		//run autosail algorithm
		//ie.
		/*cli.port->print((int)wind_tag.wind_speed);
		Serial.print(F("WIND SPEED ^^\n"));
		cli.port->print((int)wind_tag.wind_angle);
		Serial.print(F("WIND ANGLE ^^\n"));
		cli.port->print((int)head_tag.mag_angle_deg);
		Serial.print(F("HEADING ^^\n"));*/
		if(abs(wind_tag.wind_angle) != 0){
			int head_grease = head_tag.mag_angle_deg / 10;
			delay(10);
			int degree = calcNavScore(wind_tag.wind_angle,head_grease);
			delay(10);
			if(last_degree == degree){
				//Serial.print(F("Same Score\n"));
				if((millis() - time_since_lastNav) > 5000){
					Serial.print(F("Too long, resetting\n"));
					while(degree == last_degree){
						update_airmar_tags();
						head_grease = head_tag.mag_angle_deg / 10;
						degree = calcNavScore(wind_tag.wind_angle,head_grease); 
					}
					time_since_lastNav = millis();
				}
			}
			else{
				time_since_lastNav = millis();
				last_degree = degree;
			}
			int target;
			cli.port->print((int)degree);
			
			if (degree > 5 && degree <= 180){
				Serial.print(F("Turning LEFT\n"));
				target = degree * (1000/180);
				motor_set_rudder(target);
			}
			else if(degree > 180 && degree < 355){
				Serial.print(F("Turning RIGHT\n"));
				target = ((degree - 170) * (-1000/180));
				motor_set_rudder(target);
			}
			else{
				Serial.print(F("Sailing straight\n"));
				motor_set_rudder(0);
			}
		}
		else{
			Serial.print(F("WTF 0 wind\n"));
		}
		/*
		int rud_set = wind_tag.wind_angle;
		if(rud_set >0 && rud_set < 1800){
			 motor_set_rudder(-700);
		}
		else{
			motor_set_rudder(700);
		}

		//Serial.print(F("hahahah\n"));
		//autosail_print_tags();
		//motor_winch_abs(10*tick_counter); */
	}
	
	tick_counter++;
}

void update_airmar_tags(){
	anmea_poll_string(
			&SERIAL_PORT_AIRMAR,
			&airmar_buffer,
			AIRMAR_TAGS[airmar_turn_counter]
		);
        
	if(airmar_buffer.state == ANMEA_BUF_COMPLETE){
		//Serial.print(F("COMPLETE\n"));
	}
	if(airmar_buffer.state ==  ANMEA_BUF_SEARCHING){
	//	Serial.print(F("SEARCHING\n"));
	}
	if(airmar_buffer.state ==   ANMEA_BUF_BUFFERING){
	//	Serial.print(F("BUFFERING\n"));
	}
		//cli.port->print(airmar_buffer.state);
	if( airmar_buffer.state == ANMEA_BUF_MATCH ) {
		Serial.print(F("MATCH\n"));
		if(airmar_turn_counter==0){
			anmea_update_hchdg(&head_tag, airmar_buffer.data);
		}
		
		else if(airmar_turn_counter==1){
			anmea_update_wiwmv(&wind_tag, airmar_buffer.data);
		}
		
		else{
			anmea_update_gpgll(&gps_tag, airmar_buffer.data);
					Serial.print(F("hahahah gps\n"));
		}
		
		airmar_turn_counter++;
		if(airmar_turn_counter > 1){
			airmar_turn_counter = 0;
		}
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
void
auto_update_winch(
        rc_mast_controller* rc,
        pchamp_controller mast[2])
{
    int16_t rc_input;

    char buf[40];       // buffer for printing debug messages
    uint16_t rvar = 0;  // hold result of remote device status (pololu controller)

    // Track positions from last call, prevent repeat calls to the same
    // position.

    static uint16_t old_winch = 0;
    
	//Winch
	rc_input = rc_get_mapped_analog( rc->lsy, -1200, 1200 );
	if(abs(rc_input)<200)
		rc_input = 0;
	rc_input = map(rc_input, -1200, 1200, -1000, 1000);
	
	if(abs(old_winch - rc_input)>=25){
		motor_set_winch(
                rc_input
            );
		old_winch = rc_input;
	}

}

//Calculates best navscore, and returns it's degree
int calcNavScore(int windDeg, int dirDeg){ 
 //wind deg is degree of wind (clockwise) with respect to boat's bow (deg = 0). 
 // dirDeg is degree of waypoint (clockwise) with respect to boat's bow (deg = 0).
	int best_score = 1000;
	int best_degree;
	for (int i = 0; i < devCount; i++){
		navScore[i].degree = 360 * i / devCount;
		//int windScore = calcWindScore(windDeg, navScore[i].degree);
		//int dirScore = calcDirScore(dirDeg, navScore[i].degree);
		navScore[i].score = calcWindScore(windDeg, navScore[i].degree) + calcDirScore(dirDeg, navScore[i].degree);
		if (navScore[i].score < best_score){
			best_score = navScore[i].score;
			best_degree = navScore[i].degree;
		}
	}
	if (abs(windDeg - dirDeg) > CHDEG && abs(360 - windDeg + dirDeg) > CHDEG && abs(360 + windDeg - dirDeg) > CHDEG){
	//check if waypoint is down wind from close hauled limit
			best_degree = dirDeg;
			//System.out.println(Math.abs(head_tag.mag_angle_deg - windDeg));
	}
	return (best_degree);
}

int calcWindScore(int windDeg, int scoreDeg){
	//calculate red area
	if (abs(windDeg - scoreDeg) < CHDEG) //check if facing upwind
		return 360; //return high number for undesirable direction (upwind)
	
	int pointA = (windDeg + 180) % 360; //Point directly down wind from boatA
	int pointB = (windDeg + CHDEG) % 360; //Maximum face-up angle (clockwise from wind)
	int pointC = (windDeg - CHDEG) % 360; //Maximum face-up angle (counterclockwise from wind)

	if (abs(windDeg - scoreDeg)  <= CHDEG ||
	abs(360 - windDeg + scoreDeg) <= CHDEG || 
	abs(360 + windDeg - scoreDeg) <= CHDEG) //check if facing upwind
				return 170;	
	//Calculate green area if wind is to port (left) of bow
	else if(windDeg == 0)
		return 10;
	else if (windDeg > 180 && (scoreDeg <= pointA || scoreDeg >= pointB))
		return 10; //arbitrary
	//Calculate yellow area if wind is to port (left) of bow
	else if (windDeg > 180 && (scoreDeg >= pointA && scoreDeg <= pointC))
		return 100;//arbitrary
	//Calculate green area if wind is to starboard (right) of bow
	else if (windDeg <= 180 && (scoreDeg >= pointA || scoreDeg <= pointC))
		return 10;//arbitrary
	//Calculate yellow area if wind is to starboard (right) of bow
	else if (windDeg <= 180 && (scoreDeg <= pointA && scoreDeg >= pointB))
		return 100;//arbitrary
}

int calcDirScore(int dirDeg, int scoreDeg){
	//green area
	if (abs(dirDeg - scoreDeg) < DIRCONST)
		return 2;//arbitrary
	//yellow area
	if (abs(dirDeg - scoreDeg) <= 90)
		return 5;//arbitrary
	//red area
	return 10;
	
}
