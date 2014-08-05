#include "barnacle_client.h"

uint16_t
barn_get_battery_voltage()
{
    barn_write_command( BARNACLE_CMD_BATT_VOLT );
    return barn_receive_response();
}

uint16_t
barn_get_battery_current()
{
    barn_write_command( BARNACLE_CMD_BATT_CURR );
    return barn_receive_response();
}

uint16_t
barn_get_charger_voltage()
{
    barn_write_command( BARNACLE_CMD_CHRG_VOLT );
    return barn_receive_response();
}

uint16_t
barn_get_charger_current()
{
    barn_write_command( BARNACLE_CMD_CHRG_CURR );
    return barn_receive_response();
}

uint16_t
barn_get_w1_ticks()
{
    barn_write_command( BARNACLE_CMD_GET_W1_TICKS );
    return barn_receive_response();
}

uint16_t
barn_clr_w1_ticks()
{
    barn_write_command( BARNACLE_CMD_CLR_W1_TICKS );
    return barn_receive_response();
}

uint16_t
barn_get_w2_ticks()
{
    barn_write_command( BARNACLE_CMD_GET_W2_TICKS );
    return barn_receive_response();
}

uint16_t
barn_clr_w2_ticks()
{
    barn_write_command( BARNACLE_CMD_CLR_W2_TICKS );
    return barn_receive_response();
}

void
barn_write_command( uint8_t com )
{
    Wire.beginTransmission( BARNACLE_ADDR );
    Wire.write( com );
    Wire.endTransmission();
}

uint16_t
barn_receive_response()
{
    uint32_t timeout;
    uint16_t received_value;
    Wire.requestFrom( BARNACLE_ADDR, 2 );

    // Allow for timeout in case device doesn't respond
    timeout = millis() + BARNACLE_RESPONSE_TIMEOUT_MS;
    while( Wire.available() < 2 ) {
        if( millis() > timeout ) {
            return 0xFFFF;
        }
    }

    received_value = Wire.read();
    received_value |= (Wire.read() << 8);

    return received_value;
}

uint32_t
barn_check_latency()
{
    uint32_t time_start, time_end;

    time_start = micros();
    barn_get_battery_voltage();
    time_end = micros();

    return time_end - time_start;
}

