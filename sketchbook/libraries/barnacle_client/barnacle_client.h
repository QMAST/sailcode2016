#ifndef BARNACLE_CLIENT
#define BARNACLE_CLIENT

#include <Arduino.h>
#include <Wire.h>

#define BARNACLE_ADDR 0x2C
#define BARNACLE_RESPONSE_TIMEOUT_MS 1000UL

// Command constants
#define BARNACLE_CMD_BATT_VOLT 0x01
#define BARNACLE_CMD_BATT_CURR 0x02

#define BARNACLE_CMD_CHRG_VOLT 0x03
#define BARNACLE_CMD_CHRG_CURR 0x04

#define BARNACLE_CMD_GET_TICKS 0x05

// Publicically available
uint16_t barn_get_battery_voltage();
uint16_t barn_get_battery_current();

uint16_t barn_get_charger_voltage();
uint16_t barn_get_charger_current();

uint16_t barn_get_ticks();

// Used internally
void barn_write_command( uint8_t );
uint16_t barn_receive_response();
#endif

