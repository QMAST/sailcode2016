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

void
psched_dbg_dump_queue(
        psched_motor* mot,
        Stream* port )
{
    char buf[40];
    uint8_t i = 0;
    psched_event* idx;

    if( mot == NULL || mot->event == NULL ) {
        return;
    }

    for( idx = mot->event, i = 0; idx != NULL; idx = idx->next, i++ ) {
        snprintf_P(buf, sizeof(buf),
                PSTR("List %u: 0x%X, nxt: 0x%X"),
                i,
                idx,
                idx->next );
        port->println(buf);
    }
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
    psched_event* new_event;
    psched_event* idx_event;

    // Allocate the new event
    new_event = (psched_event*) malloc(sizeof(psched_event));
    if( new_event == 0 ) {
        return 1;
    }

    // Set up the values
    new_event->target_reached = 0;
    new_event->target_speed = target_speed;
    new_event->travel = target_travel;
    new_event->next = NULL;

#ifdef DEBUG
    char buf[40];
    snprintf_P( buf, sizeof(buf),
            PSTR("Set: M(%u) (%u/%u)"),
            mot->con->id,
            mot->get_pulses(),
            new_event->target_pulses );
    Serial.println(buf);
#endif

    // Add to the event queue
    if( mot->event == NULL ) {
        new_event->target_pulses = target_travel + mot->get_pulses();
        mot->event = new_event;
    } else {
        // For loop moves idx_event into the right position
        for(    idx_event = mot->event;
                idx_event->next != NULL;
                idx_event = idx_event->next);
        
        // Try and setup the correct target pulses
        new_event->target_pulses =
            idx_event->target_pulses + new_event->travel;
        idx_event->next = new_event;
    }

    return 0;
}

uint8_t
psched_advance_target( psched_motor* mot )
{
    psched_event* old_event;

    if( mot->event == NULL ) {
        return 1;
    }

    if( mot->event->target_reached = 0 ) {
        return 2;
    }

    old_event = mot->event;
    mot->event = old_event->next;

    free(old_event);

    return 0;
}

