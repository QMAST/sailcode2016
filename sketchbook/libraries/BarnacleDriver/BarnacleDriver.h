#ifndef BARNACLE_DRIVER
#define BARNACLE_DRIVER

#include <Arduino.h>
#include <SoftwareSerial.h>

#define BARNACLE_RESPONSE_TIMEOUT_MS 1000UL

// Command constants
#define BARNACLE_CMD_BATT_VOLT 0x01
#define BARNACLE_CMD_BATT_CURR 0x02

#define BARNACLE_CMD_CHRG_VOLT 0x03
#define BARNACLE_CMD_CHRG_CURR 0x04

#define BARNACLE_CMD_GET_W1_TICKS 0x05
#define BARNACLE_CMD_CLR_W1_TICKS 0x06

#define BARNACLE_CMD_GET_W2_TICKS 0x07
#define BARNACLE_CMD_CLR_W2_TICKS 0x08

#define BARNACLE_CMD_GETANDCLR_W1_TICKS 0x09
#define BARNACLE_CMD_GETANDCLR_W2_TICKS 0x0A

#define BARNACLE_CMD_SUMMARY 'z'

class BarnacleDriver{
public:
	//constructor and destructor
	BarnacleDriver(Stream&); //int rx, int tx, int baudRate);
	~BarnacleDriver();
	//public accessors and mutators
	uint16_t barn_get_battery_voltage();
	uint16_t barn_get_battery_current();
	uint16_t barn_get_charger_voltage();
	uint16_t barn_get_charger_current();
	uint16_t barn_get_w1_ticks();
	uint16_t barn_clr_w1_ticks();
	uint16_t barn_get_w2_ticks();
	uint16_t barn_clr_w2_ticks();
	uint16_t barn_getandclr_w1_ticks();
	uint16_t barn_getandclr_w2_ticks();
	String barn_get_data_summary();
	uint32_t barn_check_latency();
private:
	// Serial communication
	uint16_t barn_send_receive(uint8_t command);
	// Private attributes
	//SoftwareSerial m_barnacleSerial; //m stands for 'member'
	Stream* m_barnacleSerial;
};
#endif

