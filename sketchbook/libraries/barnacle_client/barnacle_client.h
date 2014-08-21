#ifndef BARNACLE_CLIENT
#define BARNACLE_CLIENT

// Its always 2 byte responses

#include <Arduino.h>

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

// Publicly available
extern Stream* barnacle_port;

uint16_t barn_get_battery_voltage();
uint16_t barn_get_battery_current();

uint16_t barn_get_charger_voltage();
uint16_t barn_get_charger_current();

uint16_t barn_get_w1_ticks();
uint16_t barn_clr_w1_ticks();

uint16_t barn_get_w2_ticks();
uint16_t barn_clr_w2_ticks();

uint32_t barn_check_latency();

// Used internally
void barn_write_command( uint8_t );
uint16_t barn_receive_response();
#endif

