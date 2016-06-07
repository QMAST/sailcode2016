#include "cantedKeel.h"
#include <pololu_champ.h>
#include <math.h>

Canted_Keel::Canted_Keel(
				int pololu_num, 
				Stream* pololu_serial, 
				int potentiometer_pin, 
				double max_potentiometer_voltage, 
				double min_potentiometer_voltage,
				double center_potentiometer_voltage,
				double max_angle)
{
	m_keel_control.id = pololu_num;
	m_keel_control.line = pololu_serial;
	m_potentiometer_pin = potentiometer_pin;
	m_center_potentiometer_value = ( center_potentiometer_voltage * 1024 ) / 5;
	m_max_potentiometer_value = ( max_potentiometer_voltage * 1024 ) / 5;
	m_min_potentiometer_value = ( min_potentiometer_voltage * 1024 ) / 5;
	m_center_potentiometer_resistance = findVoltageDividerR2( 5, center_potentiometer_voltage, 10000 );
	double max_potentiometer_resistance = findVoltageDividerR2( 5, max_potentiometer_voltage, 10000 );
	double min_potentiometer_resistance = findVoltageDividerR2( 5, min_potentiometer_voltage, 10000 );
	// re-ajust resistance so that it's linear with respect to the center resistance
	double potentiometer_resistance_offset_from_center = fmin(max_potentiometer_resistance - m_center_potentiometer_resistance, m_center_potentiometer_resistance - min_potentiometer_resistance);
	m_max_potentiometer_resistance = m_center_potentiometer_resistance + potentiometer_resistance_offset_from_center;
	m_min_potentiometer_resistance = m_center_potentiometer_resistance - potentiometer_resistance_offset_from_center;
	m_max_angle = max_angle;
	m_angle_per_ohm = max_angle / potentiometer_resistance_offset_from_center;
} // end constructor

void Canted_Keel::init()
{
	pinMode(m_potentiometer_pin, INPUT);	
}

double Canted_Keel::getAngle()
{
	double position_raw = analogRead(m_potentiometer_pin);
	double position_voltage = ( position_raw * 5.0 ) / 1024.0;
	double position_resistance = findVoltageDividerR2( 5.0, position_voltage, 10000.0 );
	double position_angle = (position_resistance - m_center_potentiometer_resistance) * m_angle_per_ohm;
	return position_angle;
} // end getKeelPosition()

bool Canted_Keel::setAngle(double new_position, int speed)
{
	bool success;
	double current_position = getAngle();
	
	//Check if legal speed
	//TODO: create exception to throw "illegal speed"
	if (speed > 1000 || speed < 0)
	{
		success = false;
	}
	
	//Check if new_position in legal
	//TODO: create exception to throw "illegal position"
	if (new_position > m_max_angle ||
		new_position < -m_max_angle){
			success = false;
	}
	else if (current_position <= new_position + 3 &&
			 current_position >= new_position - 3){
		motor_lock();
		success = true;
	}
	else if (current_position > new_position + 3 ) // keel shift towards starboard
	{
		motor_unlock();
		motor_set_speed( speed, 1);
		success = true;
	}
	else if (current_position < new_position - 3 )
	{
		motor_unlock();
		motor_set_speed( speed, 0);
		success = true;
	}

	return success;
} // end setKeelPosition()

void Canted_Keel::motor_lock()
{
	pchamp_set_target_speed( &(m_keel_control), 0, PCHAMP_DC_MOTOR_FORWARD );
    pchamp_request_safe_start( &(m_keel_control), false );
} // end motor_lock()

void Canted_Keel::motor_unlock()
{
	pchamp_request_safe_start( &(m_keel_control) );
} // end motor_unlock()

void Canted_Keel::motor_set_speed(int target_speed, int direction)
{	
	char buf[40];       // buffer for printing debug messages
    uint16_t rvar = 0;  // hold result of remote device status (pololu controller)
	uint16_t rvar_serial = 0;  // hold result of remote device status (pololu controller)

    target_speed = constrain( target_speed, 0, 1000);
    target_speed = map( abs(target_speed), 0, 1000, 0, 3200 );
	target_speed = target_speed;
	
    pchamp_set_target_speed( &(m_keel_control), target_speed, direction );
    delay(1);

	//Check for error:
    rvar = pchamp_request_value( &(m_keel_control), PCHAMP_DC_VAR_ERROR );	//General Error Request
    if( rvar != 0 ) {
		rvar_serial = pchamp_request_value( &(m_keel_control), PCHAMP_DC_VAR_ERROR_SERIAL );	//Serial Error Request
		
		if(rvar_serial != 0){
			snprintf_P( buf, sizeof(buf), PSTR("W0ERR_SERIAL: 0x%02x\n"), rvar_serial );
			pchamp_request_safe_start( &(m_keel_control) );	//If there is a serial error, ignore it and immediately restart
		}
		else
			snprintf_P( buf, sizeof(buf), PSTR("W0ERR: 0x%02x\n"), rvar );
        Serial.print(buf);
    }
}

double Canted_Keel::findVoltageDividerR2(double vin, double vout, double r1)
{
	return ( vout * r1 ) / ( vin - vout );
}