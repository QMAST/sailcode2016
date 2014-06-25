#include "pololu_sched.h"

void
event_time_motor( event_time_motor_t* event )
{
    if(     event->completed == true
        ||  event->target > millis() ) {
        return;
    }

    pchamp_set_target_speed(
            event->motor,
            event->speed,
            event->dir
        );

    event->completed = true;
}

