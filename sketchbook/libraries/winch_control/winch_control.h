#include <barnacle_client.h>
#include <pololu_champ.h>

#define WINCH_FULL_SPEED_FORWARD 3200
#define WINCH_FULL_SPEED_BACKWARD -3200

// Sets the winch_current_target_offset variable and clears the current encoder count.
void winch_set_target_offset(int16_t offset);

// Sets the winch motors' speed and direction, then exits.
uint8_t winch_update_motor_speed(void);

// Blocks until the winch has moved the specified number of ticks.
void winch_move_ticks(int16_t offset);

// void winch_move_cm(int16_t offset); // Not written yet
