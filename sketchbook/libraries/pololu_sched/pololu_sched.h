#ifndef POLOLU_SCHED_H
#define POLOLU_SCHED_H

#include <Arduino.h>
#include <pololu_champ.h>
#include <inttypes.h>

#define PSCHED_EVENT_SET_OKAY       0
#define PSCHED_EVENT_SET_INVALID    1

#define DEBUG

typedef uint16_t (*pulse_function)(void);

typedef struct {
    uint8_t target_reached;
    int16_t target_speed;      /// Set motor to this when event happens
    
    uint16_t target_pulses;    /// React when this value is reached
    uint16_t travel;           /// How many pulses between source and target
} psched_event;

typedef struct {
    pchamp_controller *con;    /// Pololu controller target

    psched_event event;        /// A single event

    /// Function to call to get number of pulses
    pulse_function get_pulses;
} psched_motor;

/** Initialise psched_motor with values
 *
 * No processing done yet, just sets the values and explicitly makes event NULL
 */
void psched_init_motor( 
        psched_motor*,
        pchamp_controller*,
        pulse_function );

/** Set the values
 *
 * TODO: Only allow for valid settings
 *
 * Target travel is the number of pulses you want the encoder to allow before
 * the event fires. It makes the call to get_pulses() to determine the absolute
 * encoder value required.
 */
uint8_t psched_set_target(
        psched_motor*,
        uint16_t target_speed,
        uint16_t target_travel );

/** Print the result of calling the get_pulses() function
 *
 * Repeats forever until a 'q' character is received on the Stream*
 *
 * Contains built in timer function checking to prevent calling more than once
 * per millisecond
 */
void psched_dbg_print_pulses(
        psched_motor*,
        Stream* );

/** Return 1 if the target has been reached
 */
uint8_t psched_check_target( psched_motor* );

/** Execute motor command from event details
 * 
 * Returns  1 if the event target has already been reached
 *          0 if the event has been executed
 */
uint8_t psched_exec_event( psched_motor* );

#endif

