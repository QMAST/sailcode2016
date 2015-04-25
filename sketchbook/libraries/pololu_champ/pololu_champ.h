#ifndef POLOLU_CHAMP_H
#define POLOLU_CHAMP_H

#include <Arduino.h>

#define PCHAMP_DEFAULT 0xAA
#define PCHAMP_DEFAULT_TIMEOUT_MS 1000
//changed from 1000

#define PCHAMP_DC_FORWARD 0
#define PCHAMP_DC_REVERSE 1

// Mode bytes
#define PCHAMP_DC_SAFE_START 0x03
#define PCHAMP_DC_UNSAFE 0x60

#define PCHAMP_DC_REQUEST_VAR 0x21
#define PCHAMP_DC_MOTOR_FORWARD 0x05
#define PCHAMP_DC_MOTOR_REVERSE 0x06

#define PCHAMP_SERVO_SET_TARGET 0x04

// Limits
#define PCHAMP_DC_MAX 3200
#define PCHAMP_DC_MIN 0

// Variable code bytes
#define PCHAMP_DC_VAR_ERROR 0x00
#define PCHAMP_DC_VAR_VOLTAGE 0x17
#define PCHAMP_DC_VAR_TEMPERATURE 24
#define PCHAMP_DC_VAR_BAUD 27
#define PCHAMP_DC_VAR_TIME_LOW 28
#define PCHAMP_DC_VAR_TIME_HIGH 29

#define PCHAMP_SERVO_VAR_ERROR 0x21
#define PCHAMP_SERVO_VAR_POSITION 0x10

typedef struct {
    uint8_t id;
    Stream* line;
} pchamp_controller;

typedef struct {
    uint8_t channel_id;
    pchamp_controller* controller;
} pchamp_servo;

/** Write the byte array to the serial port of the given controller
 *
 * Takes the pointer to the byte array and the number of bytes in the array
 */
void pchamp_write_command( pchamp_controller*, uint8_t*, uint8_t );

/** Request and return a byte from the controller
 *
 * Returns 0xFFFF if there is a timeout waiting for a response
 *
 * Optionally takes a channel number, only relevant to pololu servo controller
 */
uint16_t pchamp_request_value( pchamp_controller*, uint8_t );

/** Send the safe start signal to the controller
 *
 * Optionally, can re-engage the safe-start lock:
 * 0 - lock as unsafe
 * 1 - release lock, set safe
 */
void pchamp_request_safe_start( pchamp_controller* dev, uint8_t = 1 );

/** Check the status byte of the controller and return it
 */
uint16_t pchamp_check_status( pchamp_controller* );

/** Set the target speed for the motor
 *
 *  Target speed may be between 0 and 3200
 *
 *  Direction is either:
 *  0 - PCHAMP_DC_FORWARD
 *  1 - PCHAMP_DC_REVERSE
 */
void pchamp_set_target_speed( pchamp_controller*, uint16_t, uint8_t );

/** Set maestro servo controller motor to a target position
 *
 * Takes a target position between 0 and 6000
 *
 * Pololu protocol: 0xAA, device number, 0x04, channel number, target low bits,
 * target high bits
 */
void pchamp_servo_set_position( pchamp_servo*, uint16_t );

/** Get a variable from a pololu maestro controller which requires a specific
 * channel as an argument. Channel is provided by the pchamp_servo struct
 *
 */
uint16_t pchamp_servo_request_value( pchamp_servo*, uint8_t, uint8_t = 0 );


/** Get and set functions
 */

/*
 *  Returns the temperature variable from the controller.
 */
uint16_t pchamp_get_temperature( pchamp_controller* );

#endif

