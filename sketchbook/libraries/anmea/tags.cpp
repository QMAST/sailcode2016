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

void
anmea_print_wiwmv( anmea_tag_wiwmv_t* tag, Stream* port )
{
    char buf[80];

    snprintf_P( buf, sizeof(buf),
            PSTR("WIND->SPD:%u ANG:%u REL:%c\n"),
            tag->wind_speed,
            tag->wind_angle,
            (tag->flags & ANEAM_TAG_WIMV_WIND_RELATIVE ) != 0 ? 'Y' : 'N'
        );
    port->print( buf );
}

