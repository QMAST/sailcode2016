#include "pololu_sched.h"

void
psched_init_motor(  psched_motor* mot,
                    pchamp_controller* con,
                    pulse_function pfunc )
{
    mot->con = con;
    mot->event = NULL;
    mot->get_pulses = pfunc;
}

uint8_t
psched_set_target(
        psched_event* tevent,
        int16_t target_speed,
        uint16_t target_travel )
{
    // Its possible the event hasn't been initialised
    if( tevent == NULL ) {
        return 1;
    }

    tevent->target_reached = 0;
    tevent->target_speed = target_speed;
    tevent->travel = target_travel;
    tevent->target_pulses = target_travel + mot->get_pulses();
    tevent->next = NULL;

#ifdef DEBUG
    char buf[40];
    snprintf_P( buf, sizeof(buf),
            PSTR("Set: M(%u) (%u/%u)"),
            mot->con->id,
            mot->get_pulses(),
            mot->event->target_pulses );
    Serial.print(buf);
#endif

    return 0;
}

void
psched_dbg_print_pulses(
        psched_motor* mot,
        Stream* port )
{
    char buf[40];
    snprintf_P(buf, sizeof(buf),
            PSTR("CUR: %u TAR: %u"),
            mot->get_pulses(),
            mot->event->target_pulses );
    Serial.println(buf);
}

uint8_t
psched_check_target( psched_motor* mot )
{
    // Built in time delay to prevent calling too quickly
    static uint32_t next_check = 0;
    uint16_t pulses;

    if( next_check > micros() ) {
        // Called too quickly, assume target not reached
        return 0;
    }
    next_check = micros() + 1000;

    pulses = mot->get_pulses();
    if( pulses >= mot->event->target_pulses ) {
#ifdef DEBUG
        Serial.println(F("TARGET REACHED"));
#endif
        return 1;
    }

    return 0;
}

uint8_t
psched_exec_event( psched_motor* mot )
{
    uint16_t new_speed;
    uint8_t new_direction;

    if( mot->event->target_reached ) {
        return 1;
    }

    new_speed = abs(mot->event->target_speed);
    new_direction = mot->event->target_speed > 0 ? 1 : 0;

    pchamp_set_target_speed(
            mot->con,
            new_speed,
            new_direction
        );

    mot->event->target_reached = 1;

    return 0;
}

uint8_t psched_new_target(
        psched_motor* mot,
        int16_t target_speed,
        uint16_t target_travel )
{
    psched_event* new_event, idx_event;
    uint8_t i;

    // Allocate the new event
    new_event = (psched_event*) malloc(sizeof(psched_event));
    if( new_event == 0 ) {
        return 1;
    }

    // Set up the values
    psched_set_target( new_event, target_speed, target_travel );

    // Add to the event queue
    if( mot->event == NULL ) {
        mot->event = new_event;
    } else {
        for(    idx_event = mot->event, i = 0;
                idx_event->next != NULL;
                idx_event = idx_event->next, i++ )
        {
#ifdef DEBUG
            char buf[40];
            snprintf_P(buf, sizeof(buf),
                    PSTR("List %u: 0x%X, nxt: 0x%X"),
                    i,
                    idx_event,
                    idx_event->next );
#endif
        }
        
        idx_event->next = new_event;
    }

    return 0;
}

