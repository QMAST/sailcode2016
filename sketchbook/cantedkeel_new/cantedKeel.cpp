#include "cantedKeel.h"
#include <pololu_champ.h>

Canted_Keel::Canted_Keel(
				uint8_t pololu_num, 
				Stream* pololu_serial, 
				int potentiometer_pin, 
				double max_potentiometer_voltage, 
				double min_potentiometer_voltage,
				double center_potentiometer_voltage,
				double max_starboard_angle,
				double max_port_angle)
{
	m_keel_control.id = pololu_num;
	m_keel_control.line = pololu_serial;
	m_potentiometer_pin = potentiometer_pin;
	m_max_potentiometer_value = ( max_potentiometer_voltage * 1024 ) / 5;
	m_min_potentiometer_value = ( min_potentiometer_voltage * 1024 ) / 5;
	m_center_potentiometer_value = ( center_potentiometer_voltage * 1024 ) / 5;
	m_max_pototentiometer_resistance = findVoltageDividerR2( 5, max_potentiometer_voltage, 10000 );
	m_min_pototentiometer_resistance = findVoltageDividerR2( 5, min_potentiometer_voltage, 10000 );
	m_center_pototentiometer_resistance = findVoltageDividerR2( 5, center_potentiometer_voltage, 10000 );
	m_max_starboard_angle = max_starboard_angle;
	m_max_port_angle = max_port_angle;
	
	pinMode(m_potentiometer_pin, INPUT);
} // end constructor

double Canted_Keel::getPosition()
{
	int position_raw = analogRead(m_potentiometer_pin);
	double position_voltage = ( position_raw * 5 ) / 1024;
	double position_resisance = findVoltageDividerR2( 5, position_voltage, 10000 );
	double position_angle = 
	return ; //TODO: math here to get position
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
	else if (getKeelPosition() > new_position + 1 ) // keel shift towards starboard
	{
		motor_unlock();
		pchamp_set_target_speed(&m_keel_control, speed, PCHAMP_DC_REVERSE);
		success = true;
	}
	else if (getKeelPosition() < new_position - 1 )
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