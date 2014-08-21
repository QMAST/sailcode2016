#ifndef POLOLU_SCHED_H
#define POLOLU_SCHED_H

#include <Arduino.h>
#include <pololu_champ.h>
#include <inttypes.h>

#define PSCHED_EVENT_SET_OKAY       0
#define PSCHED_EVENT_SET_INVALID    1

#define PSCHED_EVENT_TIMEOUT        1000 // uS between check_target calls

#define DEBUG

/*  If this library is extended for use with timers (uint32_t), then this will
 *  need to be changed to a 32-bit return and figure out a way to cast 16 bit
 *  to 32, which should be straightforward
 */
typedef uint16_t (*pulse_function)(void);

typedef struct psched_event {
    uint8_t target_reached;
    int16_t target_speed;      /// Set motor to this when event happens

    uint16_t target_pulses;    /// React when this value is reached
    uint16_t travel;           /// How many pulses between source and target

    struct psched_event* next; /// Queue pointer
} psched_event;

typedef struct {
    pchamp_controller *con;    /// Pololu controller target

    psched_event* event;       /// A single event

    int32_t position;         /// Track the estimated absolute position

    /// Function to call to get number of pulses
    pulse_function get_pulses;
    /// _Optional_ function for clearing the pulse register on device
    pulse_function clr_pulses;
} psched_motor;

/** Initialise psched_motor with values
 *
 * Note function pointer is used for getting the current encoder pulses
 *
 * No processing done yet, just sets the values and explicitly makes event
 * NULL, which is really important for queue processing
 */
void psched_init_motor(
        psched_motor*,
        pchamp_controller*,
        pulse_function,
        pulse_function = NULL );

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

/** Print the event queue
 */
void psched_dbg_dump_queue( psched_motor*, Stream* );

/** Return 1 if the target has been reached
 */
uint8_t psched_check_target( psched_motor* );

/** Execute motor command from event details
 * 
 * Returns  1 if the event target has already been reached
 *          0 if the event has been executed
 */
uint8_t psched_exec_event( psched_motor* );

/** Allocate a new event from the heap and add it to the event queue
 *
 * Target travel is the number of pulses you want the encoder to allow before
 * the event fires. It makes the call to get_pulses() to determine the absolute
 * encoder value required.
 *
 * Returns  1 if malloc fails
 *          0 if everything is OKAY
 */
uint8_t psched_new_target(
        psched_motor*,
        int16_t target_speed,
        uint16_t target_travel );

/** Proceed event queue to next event
 *
 *  Doesn't execute the next event, just moves the head of the list to the next
 *  element and 
 *
 * Returns  1 if there are no more events
 *          2 if the current event is not complete
 *          0 if successful
 */
uint8_t psched_advance_target( psched_motor* );

#endif

