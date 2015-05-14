#define AUTOSAIL_TIMEOUT 1000
#define AUTOSAIL_ALGORITHM_TICKS 100
#define AUTOSAIL_MIN_DELAY 10

uint32_t autosail_start_time;

const int devCount = 360;
const int CHDEG = 45; //Closest degree offset from wind direction that boat can sail
const int DIRCONST = 15; //Offset of green area from dirDeg (plus or minus)
  
 typedef struct DegScore{
	int degree;
	int score;
}DegScore;

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
//Calculates best navscore, and returns it's degree
int calcNavScore(int windDeg, int dirDeg){ 
 //wind deg is degree of wind (clockwise) with respect to boat's bow (deg = 0). 
 // dirDeg is degree of waypoint (lockwise) with respect to boat's bow (deg = 0).
	DegScore navScore[devCount];
	for (int i = 0; i < devCount; i++){
		navScore[i].degree = 360 * i / devCount;
		//int windScore = calcWindScore(windDeg, navScore[i].degree);
		//int dirScore = calcDirScore(dirDeg, navScore[i].degree);
		navScore[i].score = calcWindScore(windDeg, navScore[i].degree) + calcDirScore(dirDeg, navScore[i].degree);
		for(int x = 0; x<i;i++){
			
	
	}
}

int calcWindScore(int windDeg, int scoreDeg){
	/*calculate red area*/
	if (abs(windDeg - scoreDeg) < CHDEG) //check if facing upwind
		return 360; //return high number for undesirable direction (upwind)
	
	int pointA = (winDeg + 180) % 360; //Point directly down wind from boatA
	int pointB = (windDeg + CHDEG) % 360; //Maximum face-up angle (clockwise from wind)
	int pointC = (windDeg - CHDEG) % 360; //Maximum face-up angle (counterclockwise from wind)
	/*Calculate green area if wind is to port (left) of bow*/
	if (Math.abs(windDeg - scoreDeg)  <= CHDEG ||
	Math.abs(360 - windDeg + scoreDeg) <= CHDEG || 
	Math.abs(360 + windDeg - scoreDeg) <= CHDEG) //check if facing upwind
				navScore[scoreDeg] += 170;	
	
	else if(windDeg == 0)
		return 10;
	else if (windDeg > 180 && (scoreDeg <= pointA || scoreDeg >= pointB))
		return 10; //arbitrary
	/*Calculate yellow area if wind is to port (left) of bow*/
	else if (windDeg > 180 && (scoreDeg >= pointA && scoreDeg <= pointC))
		return 100;//arbitrary
	/*Calculate green area if wind is to starboard (right) of bow*/
	else if (windDeg <= 180 && (scoreDeg >= pointA || scoreDeg <= pointC))
		return 10;//arbitrary
	/*Calculate yellow area if wind is to starboard (right) of bow*/
	else if (windDeg <= 180 && (scoreDeg <= pointA && scoreDeg >= pointB))
		return 100;//arbitrary
}

int calcDirScore(int dirDeg, int scoreDeg){
	/*green area*/
	if (abs(dirDeg - scoreDeg) < DIRCONST)
		return 2;//arbitrary
	/*yellow area*/
	if (abs(dirDeg - scoreDeg) <= 90)
		return 5;//arbitrary
	/*red area*/
	return 10;
	
}

