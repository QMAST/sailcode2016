#ifndef POLOLU_SCHED_H
#define POLOLU_SCHED_H

#include <Arduino.h>
#include <pololu_champ.h>
#include <inttypes.h>

/** Time event for scheduling a motor state
 *
 * Holds the time a motor should change its state, and the state it should
 * change itself to.
 */
typedef struct {
    uint32_t target;
    uint8_t completed;

    // New motor state
    uint16_t speed;
    uint8_t  dir;

    // Motor controller
    pchamp_controller* motor;
} event_time_motor_t;

/** Queue implementation for timed events
 *
 * Allows multiple time events to be stored in a queue
 */
typedef struct {

} event_time_motor_queue_t;

/** Check if its time for a motor to do something
 *
 * Looks at the time the motor is scheduled to go off, then looks at the
 * current time. If the current time is greater or equal to the scheduled time,
 * it executes a motor function and sets the event state to completed.
 */
void event_time_motor( event_time_motor_t* );

#endif
