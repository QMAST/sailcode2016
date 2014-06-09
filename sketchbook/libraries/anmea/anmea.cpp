#include "anmea.h"

anmea_poll_status_t
anmea_poll_char( bstring bufd, Stream* serial )
{
    //bstring bufd = 

    if( bufd == NULL || serial == NULL ) {
        return ANMEA_POLL_ERROR;
    }

    if( serial->available() <= 0 ) {
        return ANMEA_POLL_NOCHANGE;
    }

    char nchar = serial->read();

    // This is the start of the string, only valid if a sentence is not being
    // built already
    if( bufd->slen == 0 && nchar != '$' ) {
        return ANMEA_POLL_NOCHANGE;
    } else if( nchar == '$' && bufd->slen != 0 ) {
        //bassigncstr( bufd, "" );
        return ANMEA_POLL_STRING_FAIL;
    }

    // The star character plus two hex digits means end of sentence
    uint8_t len = bufd->slen;
    if( len > 3 && bufd->data[len - 3] == '*' ) {
        return ANMEA_POLL_STRING_READY;
    }

    // The string is too long to be valid
    if( len > ANMEA_POLL_MAX_STRING_LEN ) {
        //bassigncstr( bufd, "" );
        return ANMEA_POLL_STRING_FAIL;
    }

    // Ignore newline characters
    if( nchar == '\r' || nchar == '\n' ) {
        return ANMEA_POLL_NOCHANGE;
    }

    bconchar( bufd, nchar );
    return ANMEA_POLL_NEWCHAR;
}

uint8_t
anmea_is_string_invalid( bstring buffer )
{
    uint8_t xor_sum = 0;
    uint8_t checksum;
    bstring checksum_str;

    // Last two characters should be the hex number representing the checksum
    checksum_str = bmidstr( buffer, buffer->slen - 2, 2 );
    // Convert string hex chars to integer
    checksum = strtol( (char*) checksum_str->data, NULL, HEX );

    // Actually calulate checksum, using chars between $ and *
    for( uint8_t i = 1; i < buffer->slen - 3; i++ ) {
        xor_sum ^= buffer->data[i];
    }

    bdestroy( checksum_str );

    if( xor_sum == checksum ) {
        return ANMEA_STRING_VALID;
    }

    return ANMEA_STRING_INVALID;
}
