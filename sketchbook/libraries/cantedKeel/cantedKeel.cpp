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
	m_center_pototentiometer_resistance = findVoltageDividerR2( 5, center_potentiometer_voltage, 10000 );
	double max_potentiometer_resistance = findVoltageDividerR2( 5, max_potentiometer_voltage, 10000 );
	double min_potentiometer_resistance = findVoltageDividerR2( 5, min_potentiometer_voltage, 10000 );
	// re-ajust resistance so that it's linear with respect to the center resistance
	double potentiometer_resistance_offset_from_center = fmin(max_potentiometer_resistance - m_center_potentiometer_resistance, m_center_potentiometer_resistance - min_potentiometer_resistance);
	m_max_potentiometer_resistance = m_center_potentiometer_resistance + potentiometer_resistance_offset_from_center;
	m_min_potentiometer_resistance = m_center_potentiometer_resistance - potentiometer_resistance_offset_from_center;
	m_max_angle = max_angle;
	m_angle_per_ohm = max_angle / potentiometer_resistance_offset_from_center;
	
	//must be in void setup() loop
	pinMode(m_potentiometer_pin, INPUT);
} // end constructor

double Canted_Keel::getPosition()
{
	int position_raw = analogRead(m_potentiometer_pin);
	double position_voltage = ( position_raw * 5 ) / 1024;
	double position_resistance = findVoltageDividerR2( 5, position_voltage, 10000 );
	double position_angle = (position_resistance - m_center_pototentiometer_resistance) * m_angle_per_ohm;
	return position_angle; //TODO: math here to get position
} // end getKeelPosition()

bool Canted_Keel::setPosition(double new_position, int speed)
{
	bool success;
	double current_position = getKeelPosition();
	
	//Check if legal speed
	//TODO: create exception to throw "illegal speed"
	if (speed > 1000 || speed < 0)
	{
		success = false;
	}
	
	//Check if new_position in legal
	//TODO: create exception to throw "illegal position"
	if (new_position > m_max_starboard_angle ||
		new_position < -m_max_port_angle){
			success = false;
	}
	else if (current_position <= new_position + 1 &&
			 current_position >= new_position - 1){
		motor_lock();
		success = true;
	}
	else if (current_position > new_position + 1 ) // keel shift towards starboard
	{
		motor_unlock();
		pchamp_set_target_speed(&m_keel_control, speed, PCHAMP_DC_REVERSE);
		success = true;
	}
	else if (current_position < new_position - 1 )
	{
		motor_unlock();
		pchamp_set_target_speed(&m_keel_control, speed, PCHAMP_DC_REVERSE);
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

double Canted_Keel::findVoltageDividerR2(int vin, int vout, int r1)
{
	return ( vout * r1 ) / ( vin - vout );
}