#ifndef ANMEA_H_
#define ANMEA_H_

#include <Arduino.h>

#include <bstrlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

// TODO check against NMEA 0183 standard
#define ANMEA_POLL_MAX_STRING_LEN 80

typedef enum {
    ANMEA_POLL_NOCHANGE,
    ANMEA_POLL_NEWCHAR,
    ANMEA_POLL_STRING_FAIL,
    ANMEA_POLL_STRING_READY,
    ANMEA_POLL_ERROR,
} anmea_poll_status_t;

typedef enum {
    ANMEA_STRING_VALID,
    ANMEA_STRING_INVALID
};

typedef struct bstrList* blist;

/** Device specific state
 *
 * For the AIRMAR PB100
 */
typedef struct {
    Stream* port;

    uint8_t is_broadcasting;
    uint16_t baud;

    uint8_t mode;

    char target[6];
} anmea_airmar_t;

/** A buffer for holding an nmea string
 */
typedef struct {
    typedef enum {
        ANMEA_BUF_SEARCHING,
        ANMEA_BUF_BUFFERING,
        ANMEA_BUF_COMPLETE
    } state;

    bstring data;
} anmea_buffer_t;

/** WIWMV Tag type
 *
 *  Flags used to track boolean values assciated with some fields
 */
typedef enum {
    ANMEA_TAG_WIMV_SPEED_VALID = 0x1,
    ANMEA_TAG_WIMV_DATA_VALID = 0x2,
    ANMEA_TAG_WIMV_WIND_RELATIVE = 0x4,
} anmea_tag_wiwmv_flags_t;

typedef struct {
    uint16_t wind_angle;
    uint16_t wind_speed;
    uint8_t flags;

    // Time updated
    uint32_t at_time;
} anmea_tag_wiwmv_t;

// Utility functions

/** Build an nmea string one character at a time
 */
anmea_poll_status_t anmea_poll_char( bstring, Stream* );

/** Performs checksum on string
 *
 * Returns:
 *  ANMEA_STRING_VALID
 *  ANMEA_STRING_INVALID
 */
uint8_t anmea_is_string_invalid( bstring );

// Tag specific functions

/** Update the WIWMV tag with values from a _valid_ string
 *
 * The string MUST have already been validated.
 */
void anmea_update_wiwmv( anmea_tag_wiwmv_t*, bstring );
void anmea_print_wiwmv( anmea_tag_wiwmv_t*, Stream* );

#endif
