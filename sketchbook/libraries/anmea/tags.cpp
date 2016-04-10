#include "anmea.h"
#include "TinyGPS++.h"

void anmea_update_wiwmv( anmea_tag_wiwmv_t* tag, bstring rawtag )
{
    //if( tag == NULL || data == NULL ) {
        //return;
    //}

    blist tokens = bsplit( rawtag, ',' );

    //Serial.println( (char*) tokens->entry[0]->data );

    // Store the first token to corresponding tag
    tag->wind_angle = (uint16_t)
       ( strtod( (const char*) tokens->entry[1]->data, NULL ) * 10);

    // Check the Relative/Theoretical flag
    if(     tokens->entry[2]->slen > 0
        &&  strncmp( (char*) tokens->entry[2]->data, "T", 1 ) == 0
            ) {
        tag->flags &= ~ANMEA_TAG_WIMV_WIND_RELATIVE;
    } else if(
            tokens->entry[2]->slen > 0
        &&  strncmp( (char*) tokens->entry[2]->data, "R", 1 ) == 0
            ) {
        tag->flags |= ANMEA_TAG_WIMV_WIND_RELATIVE;
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
       ( strtod( (const char*) tokens->entry[3]->data, NULL ) * 10);

    tag->at_time = millis();

    bstrListDestroy( tokens );
}

void
anmea_update_hchdg( anmea_tag_hchdg_t* tag, bstring rawtag )
{
    blist tokens = bsplit( rawtag, ',' );

    // Store the first token to corresponding tag
    tag->mag_angle_deg = (uint16_t)
       ( strtod( (const char*) tokens->entry[1]->data, NULL ) * 10 );
    bstrListDestroy( tokens );
}

void
anmea_update_gpgll( anmea_tag_gpgll_t* tag, bstring rawtag )
{
	uint32_t gps_time = millis();
	TinyGPSPlus gps;
	while(gps.location.isUpdated() == 0 && ( millis() - gps_time < 3000)){
		if (Serial3.available()){
			gps.encode(Serial3.read());
		}
	}

	tag->latitude =  gps.location.lat() * 1000000;
	tag->longitude = abs(gps.location.lng()) * 1000000;
}

void
anmea_print_wiwmv( anmea_tag_wiwmv_t* tag, Stream* port )
{
    char buf[80];

    snprintf_P( buf, sizeof(buf),
            PSTR("WIND->SPD:%u ANG:%u REL:%c"),
            tag->wind_speed,
            tag->wind_angle,
            (tag->flags & ANMEA_TAG_WIMV_WIND_RELATIVE ) != 0 ? 'Y' : 'N'
        );
    port->print( buf );
}

void
anmea_print_hchdg( anmea_tag_hchdg_t* tag, Stream* port )
{
    char buf[80];

    snprintf_P( buf, sizeof(buf),
            PSTR("HEAD->ANG:%u "),
            tag->mag_angle_deg
        );
    port->print( buf );
}

void
anmea_print_gpgll( anmea_tag_gpgll_t* tag, Stream* port )
{
    char buf[80];
    snprintf_P( buf, sizeof(buf),
            PSTR("GPS->LAT:%u LONG:%u"),
            tag->latitude,
            tag->longitude
        );
    port->print( buf );
}

