#include "BarnacleDriver.h"

//constructor
BarnacleDriver::BarnacleDriver(Stream& barnacleSerial){
	m_barnacleSerial = &barnacleSerial;
}

//empty destructor
BarnacleDriver::~BarnacleDriver(){
}

uint16_t BarnacleDriver::barn_get_battery_voltage()
{
    return barn_send_receive( BARNACLE_CMD_BATT_VOLT );
}

uint16_t BarnacleDriver::barn_get_battery_current()
{
    return barn_send_receive( BARNACLE_CMD_BATT_CURR );
}

uint16_t BarnacleDriver::barn_get_charger_voltage()
{
    return barn_send_receive( BARNACLE_CMD_CHRG_VOLT );
}

uint16_t BarnacleDriver::barn_get_charger_current()
{
    return barn_send_receive( BARNACLE_CMD_CHRG_CURR );
}

uint16_t BarnacleDriver::barn_get_w1_ticks()
{
    return barn_send_receive( BARNACLE_CMD_GET_W1_TICKS );
}

uint16_t BarnacleDriver::barn_clr_w1_ticks()
{
    return barn_send_receive( BARNACLE_CMD_CLR_W1_TICKS );
}

uint16_t BarnacleDriver::barn_get_w2_ticks()
{
    return barn_send_receive( BARNACLE_CMD_GET_W2_TICKS );
}

uint16_t BarnacleDriver::barn_clr_w2_ticks()
{
    return barn_send_receive( BARNACLE_CMD_CLR_W2_TICKS );
}

uint16_t BarnacleDriver::barn_getandclr_w1_ticks()
{
    return barn_send_receive( BARNACLE_CMD_GETANDCLR_W1_TICKS );
}

uint16_t BarnacleDriver::barn_getandclr_w2_ticks()
{
    return barn_send_receive( BARNACLE_CMD_GETANDCLR_W2_TICKS );
}


String BarnacleDriver::barn_get_data_summary()
{
	uint32_t timeout = 1000 + millis();
    String received_value;
    m_barnacleSerial->write('z');//command);
	while(!m_barnacleSerial->available()){
		if( millis() > timeout ) {
            return "Error - Timeout";
        }
	}
    return m_barnacleSerial->readStringUntil('\n');
}

uint16_t BarnacleDriver::barn_send_receive( uint8_t command )
{	
	uint32_t timeout = 1000 + millis();
    uint16_t received_value = 0;
    m_barnacleSerial->write(command);//command);
	//Get first bit
	while(!m_barnacleSerial->available()){
		if( millis() > timeout ) {
            return 0xFF;
        }
	}
    received_value = m_barnacleSerial->read() << 8;
	//Get second bit
	while(!m_barnacleSerial->available()){
		if( millis() > timeout ) {
            return 0xFF;
        }
	}
    received_value |= m_barnacleSerial->read();

    return received_value;
}

uint32_t BarnacleDriver::barn_check_latency()
{
    uint32_t time_start, time_end;

    time_start = micros();
    barn_get_battery_voltage();
    time_end = micros();

    return time_end - time_start;
}

