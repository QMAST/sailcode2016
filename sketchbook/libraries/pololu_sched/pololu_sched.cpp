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
    Serial.println("FOR THIS WORLD TO STOP HATING");
}

void
event_encoder_motor(
        event_encoder_motor_t* event,
        uint16_t pulses )
{
    //if( millis() % 1000 == 0 ) {
        //Serial.print("CHECK:");
        //Serial.println(pulses);
    //}
    if(     event->completed == true
        ||  event->target > pulses
        ||  pulses >= 65535 ) {
        return;
    }

    pchamp_set_target_speed(
            event->motor,
            event->speed,
            event->dir
        );

    event->completed = true;
    Serial.println("TURN DOWN FOR WHAT");
}
