/**
*	This class assumes that a linear potentiometer is used to encode the position
*	of the canted keel.
*/

#ifndef CANTED_KEEL_h
#define CANTED_KEEL_h

#include <pololu_champ.h>

class Canted_Keel
{
public:
	/**
	*	@brief empty constructor
	*/
	Canted_Keel();
	
	/**
	*	@brief Constructor for Canted_Keel object.
	*	@param pololu_num ID set on Pololu motor controller.
	*	@param pololu_serial Serial port that pololu is connected to.
	*	@param potentiometer_pin Pin that potentiometer is connected to.
	*	@param max_potentiometer_value Voltage of potentiometer when keel is as far to the starboard side as possible (volts).
	*	@param min_potentiometer_value Voltage of potentiometer when keel is as far to the port side as possible (volts).
	*	@param center_potentiometer_value Voltage of potentiometer when keel is centered under hull (volts).
	*	@param max_angle Maximum absolute angle keel can displace on either side with respect to center position(degrees).
	*/
	Canted_Keel(
				int pololu_num, 
				Stream* pololu_serial, 
				int potentiometer_pin, 
				double max_potentiometer_voltage, 
				double min_potentiometer_voltage,
				double center_potentiometer_voltage,
				double max_angle
				);
				
	/**
	*	@brief Destructor for Canted_Keel object
	*/
	~Canted_Keel();
	
	/**
	*	@brief Must be called in void setup().
	*/
	void init();
	
	/**
	*	@brief Returns the keel's current angle.  Starboard is positive angle, port is negative angle (degrees)
	*/
	double getAngle();
	
	/**
	*	@brief Sets the keel's angle
	*	@param new_angle Angle at which to position keel (degrees)
	*	@param motor_speed Absolute speed of keel (0 to 1000)
	*/
	bool setAngle(double new_angle, int motor_speed);
	
	void setSpeed(int speed, int direction);
	
private:
	// Attributes
	pchamp_controller m_keel_control;
	int m_potentiometer_pin;
	double m_max_potentiometer_value; //10-bit A-to-D voltage (range 0 - 1024, representing 0 - % volts)
	double m_min_potentiometer_value; //10-bit A-to-D voltage
	double m_center_potentiometer_value; //10-bit A-to-D voltage
	double m_max_potentiometer_resistance; //Ohms
	double m_min_potentiometer_resistance; //Ohms
	double m_center_potentiometer_resistance; //Ohms
	double m_max_angle; //in degrees
	double m_angle_per_ohm; //degree per ohm
	const uint32_t PCHAMP_REQ_WAIT = 5; // (msec) Time to wait for response
	
	//Functions
	/**
	*	@brief Lock keel's motor's position.
	*/
	void motor_lock();
	
	/**
	*	@brief Enable keel motor.
	*/
	void motor_unlock();
	
	void motor_set_speed(int target_speed, int direction);
	
	/**
	*	@brief Calculates the R2 resistance of a voltage divider given all other parameters
	*	@param vin Input voltage to voltage divider (volts)
	*	@param vout Output voltage of voltage diviter (volts)
	*	@param r1 R1 resistance in voltage diviter (ohms)
	*/
	double findVoltageDividerR2 (double vin, double vout, double r1);
};

#endif