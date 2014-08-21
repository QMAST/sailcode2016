#include "barnacle_client.h"

Stream* barnacle_port = NULL;

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
    barnacle_port->write(com);
}

uint16_t
barn_receive_response()
{
    uint32_t timeout;
    uint16_t received_value;

    // Allow for timeout in case device doesn't respond
    timeout = millis() + BARNACLE_RESPONSE_TIMEOUT_MS;
    while( barnacle_port->available() < 2 ) {
        if( millis() > timeout ) {
            return 0xFFFF;
        }
    }

    received_value = barnacle_port->read();
    received_value |= (barnacle_port->read() << 8);

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

