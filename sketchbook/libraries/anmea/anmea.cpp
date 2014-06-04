#include "anmea.h"

void
anmea_update_wiwmv( anmea_tag_wiwmv_t* tag, bstring rawtag )
{
    //if( tag == NULL || data == NULL ) {
        //return;
    //}

    blist tokens = bsplit( rawtag, ',' );

    //Serial.println( (char*) tokens->entry[0]->data );

    // Store the first token to corresponding tag
    tag->wind_angle = (uint16_t)
       ( strtod( (const char*) tokens->entry[1]->data, NULL ) * 10 );

    // Check the Relative/Theoretical flag
    if(     tokens->entry[2]->slen > 0
        &&  strncmp( (char*) tokens->entry[2]->data, "T", 1 ) == 0
            ) {
        tag->flags &= ~ANEAM_TAG_WIMV_WIND_RELATIVE;
    } else if(
            tokens->entry[2]->slen > 0
        &&  strncmp( (char*) tokens->entry[2]->data, "R", 1 ) == 0
            ) {
        tag->flags |= ANEAM_TAG_WIMV_WIND_RELATIVE;
    }

    // Note first condition does nothing
    if( tokens->entry[5]->slen <= 0 )
        ;
    else if( strncmp( (char*) tokens->entry[5]->data, "A", 1 ) == 0 ) {
        tag->flags |= ANMEA_TAG_WIMV_DATA_VALID;
    } else if( strncmp( (char*) tokens->entry[5]->data, "V", 1 ) == 0  ) {
        tag->flags &= ~ANMEA_TAG_WIMV_DATA_VALID;
    }

    tag->wind_speed = (uint16_t)
       ( strtod( (const char*) tokens->entry[3]->data, NULL ) * 10 );

    tag->at_time = millis();

    bstrListDestroy( tokens );
}

anmea_poll_status_t
anmea_poll_char( bstring buffer, Stream* serial )
{
    if( buffer == NULL || serial == NULL ) {
        return ANMEA_POLL_ERROR;
    }

    if( serial->available() <= 0 ) {
        return ANMEA_POLL_NOCHANGE;
    }

    char nchar = serial->read();

    // This is the start of the string, only valid if a sentence is not being
    // built already
    if( buffer->slen == 0 && nchar != '$' ) {
        return ANMEA_POLL_NOCHANGE;
    } else if( nchar == '$' && buffer->slen != 0 ) {
        //bassigncstr( buffer, "" );
        return ANMEA_POLL_STRING_FAIL;
    }

    // The star character plus two hex digits means end of sentence
    uint8_t len = buffer->slen;
    if( len > 3 && buffer->data[len - 3] == '*' ) {
        return ANMEA_POLL_STRING_READY;
    }

    // The string is too long to be valid
    if( len > ANMEA_POLL_MAX_STRING_LEN ) {
        //bassigncstr( buffer, "" );
        return ANMEA_POLL_STRING_FAIL;
    }

    // Ignore newline characters
    if( nchar == '\r' || nchar == '\n' ) {
        return ANMEA_POLL_NOCHANGE;
    }

    bconchar( buffer, nchar );
    return ANMEA_POLL_NEWCHAR;
}

uint8_t
anmea_is_string_valid( bstring buffer )
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
