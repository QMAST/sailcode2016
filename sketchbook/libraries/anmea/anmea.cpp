#include "anmea.h"

void
anmea_poll_string(
        Stream* port,
        anmea_buffer_t* buf,
        const char* target )
{
    char nchar;
    if( buf->state == ANMEA_BUF_COMPLETE ) {
        Serial.println(F("SENTENCE COMPLETE (WAT)"));

        //strncasecmp( (char*) airmar_nmea_buffer->data,
                //"$WIMWV",
                //min( airmar_nmea_buffer->slen, 6 )
            //);
        return;
    }

    if( port->available() <= 0 ) {
        //Serial.println(F("NOCHAR"));
        return;
    }

    nchar = port->read();

    if( buf->state == ANMEA_BUF_SEARCHING ) {
        if( nchar == '$' ) {
            buf->state = ANMEA_BUF_BUFFERING;
            bconchar( buf->data, nchar );
        }
        return;
    }

    if( buf->state == ANMEA_BUF_BUFFERING ) {
        if(     buf->data->slen > 3
            &&  buf->data->data[buf->data->slen - 3] == '*' ) {
            buf->state = ANMEA_BUF_COMPLETE;
            Serial.println(F("SENTENCE COMPLETE"));
            return;
        }

        // Check for a character in an invalid location
        if( nchar == '$' || buf->data->slen >= buf->data->mlen ) {
            Serial.println(F("ERASE STRING, BAD"));
            anmea_poll_erase( buf );
        }

        // Ignore line ending characters
        if( nchar == '\r' || nchar == '\n' ) {
            return;
        }

        bconchar( buf->data, nchar );
        return;
    }
}

void
anmea_poll_erase( anmea_buffer_t* buf )
{
    buf->state = ANMEA_BUF_SEARCHING;
    bassigncstr( buf->data, "" );
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
