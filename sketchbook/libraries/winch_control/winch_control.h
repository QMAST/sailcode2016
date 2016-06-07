#include <BarnacleDriver.h>
#include <pololu_champ.h>
#include <SoftwareSerial.h>

#define SERIAL_PORT_BARN    Serial3

extern BarnacleDriver *barnacle_client;
extern SoftwareSerial XBEE_SERIAL_PORT;

// Sets the winch_current_target_offset variable and clears the current encoder count.
void winch_set_target_offset_ticks(int16_t offset);

// Sets the winch motors' speed and direction, then exits.
uint8_t winch_update_motor_speed();