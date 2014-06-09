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
    ANMEA_STRING_INVALID,
    ANMEA_STRING_VALID
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

/** Modes the airmar may be in at any point in time
 *
 * SEARCHING:   There is a tag in the target buffer, and the device is looking
 *              for the beginning of a new nmea sentence
 * BUF_SEARCH:  Found the beginning of a sentence, waiting for the rest of the
 *              tag name
 * BUF_SENTENCE:The correct tag has been found and the rest of the sentence is
 *              being buffered.
 * IDLE:        Not doing anything, cache will be flushed on mode change
 * TAG_READY:   The tag was found and now the whole sentence is ready for
 *              processing
 */
typedef enum {
    ANMEA_AIRMAR_MODE_SEARCHING,
    ANMEA_AIRMAR_MODE_BUF_SEARCH,
    ANMEA_AIRMAR_MODE_BUF_SENTENCE,
    ANMEA_AIRMAR_MODE_IDLE,
    ANMEA_AIRMAR_MODE_TAG_READY
} anmea_airmar_mode_t;

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

// Function prototypes

/** Update the WIWMV tag with values from a _valid_ string
 *
 * The string MUST have already been validated.
 */
void anmea_update_wiwmv( anmea_tag_wiwmv_t*, bstring );
void anmea_print_wiwmv( anmea_tag_wiwmv_t*, Stream* );

/** Build an nmea string one character at a time
 */
anmea_poll_status_t anmea_poll_char( bstring, Stream* );


uint8_t anmea_is_string_valid( bstring );

/** Set the target airmar's target tag
 *
 * Returns  0 if mode is changed
 *          1 if the mode can't be changed (device not idle)
 */
uint8_t anmea_airmar_set_target( anmea_airmar_t*, const char[6] );



#endif
